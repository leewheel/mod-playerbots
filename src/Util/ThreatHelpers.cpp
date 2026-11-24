/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ThreatHelpers.h"
#include "Creature.h"
#include "Player.h"
#include "ThreatManager.h"
// By leewheel 2026-11-04 entry 匹配需要 std::all_of / std::string / strtoul
#include <algorithm>
#include <cstdlib>
#include <string>
// End By leewheel
// By leewheel 2026-11-08 (bug_106) 登记表漏项时要打一次 WARN，避免再次静默失效
#include "Log.h"
#include <mutex>
#include <unordered_set>
// End By leewheel

namespace
{

char LowerAscii(char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c; }

bool EqualsIgnoringAsciiCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (LowerAscii(a[i]) != LowerAscii(b[i]))
            return false;
    }

    return true;
}

// By leewheel 2026-11-04 entry 铁律兜底：上游 the-lab 改用英文名 qualifier 调 FindThreatTarget
// （BT/SSC/Kara 共 165 个调用点），而本服实库生物名是中文 ⇒ unit->GetName() 与英文名恒不相等，
// 整套 raid 机制会静默失效。处置：不逐个改调用点（改了每轮合并都要重新解 190+ 冲突），
// 改在这里做一次英文名 → NPC entry 的登记表，命中即按 entry 精确匹配。
// 证据（仓内既定）：SSC 见核心 serpent_shrine.h；Kara 见核心 karazhan.h 与模块既有核定注释；
// BT 见核心 black_temple.h。上游将来新增英文名会打 WARN（见下方），按此表补一行即可。
struct ThreatNameEntry
{
    std::string_view name;
    uint32 entry;
};

constexpr ThreatNameEntry kThreatNameEntries[] = {
    // --- Serpent Shrine Cavern（核心 serpent_shrine.h） ---
    {"hydross the unstable", 21216},      // NPC_HYDROSS_THE_UNSTABLE
    {"the lurker below", 21217},          // NPC_THE_LURKER_BELOW
    {"leotheras the blind", 21215},       // NPC_LEOTHERAS_THE_BLIND
    {"fathom-lord karathress", 21214},    // NPC_FATHOM_LORD_KARATHRESS
    {"lady vashj", 21212},                // NPC_LADY_VASHJ
    {"fathom-guard sharkkis", 21966},     // NPC_FATHOM_GUARD_SHARKKIS
    {"fathom-guard tidalvess", 21965},    // NPC_FATHOM_GUARD_TIDALVESS
    {"fathom-guard caribdis", 21964},     // NPC_FATHOM_GUARD_CARIBDIS
    {"coilfang strider", 22056},          // NPC_COILFANG_STRIDER
    {"tainted elemental", 22009},         // NPC_TAINTED_ELEMENTAL
    {"greyheart tidecaller", 21229},      // 本核 2026-10-24 合并时既定（SSC 水元素图腾）
    {"morogrim tidewalker", 21213},       // 本核既有核定（SSC 莫洛格里·踏潮者）
    // --- Karazhan（核心 karazhan.h / 本核既有核定） ---
    {"midnight", 16151},                  // NPC_MIDNIGHT（阿图门坐骑）
    {"spectral charger", 15547},          // 本核既有核定（Kara 幽灵战马）
    {"astral flare", 17283},              // 本核既有核定（虚空幽龙）
    {"conjured elemental", 17167},        // NPC_CONJURED_ELEMENTAL（馆长）
    // 绿野仙踪（OZ_TARGETS 经 GetOperaActor 间接传入，核心 karazhan.h）
    {"dorothee", 17535},                  // NPC_DOROTHEE
    {"tito", 17548},                      // NPC_TITO
    {"roar", 17546},                      // NPC_ROAR
    {"strawman", 17543},                  // NPC_STRAWMAN
    {"tinhead", 17547},                   // NPC_TINHEAD
    {"the big bad wolf", 17521},          // 本核既有核定（大灰狼）
    // 罗密欧与朱丽叶（核心 karazhan.h NPC_ROMULO / NPC_JULIANNE）
    {"romulo", 17533},                    // NPC_ROMULO
    {"julianne", 17534},                  // NPC_JULIANNE
    // By leewheel 2026-11-08 (bug_106)：Kara 首领名漏登记 ⇒ GetKaraBoss() 恒 nullptr
    {"attumen the huntsman", 16152},      // NPC_ATTUMEN_THE_HUNTSMAN
    {"moroes", 15687},                    // NPC_MOROES
    {"maiden of virtue", 16457},          // NPC_MAIDEN_OF_VIRTUE
    {"the curator", 15691},               // NPC_THE_CURATOR
    {"terestian illhoof", 15688},         // NPC_TERESTIAN_ILLHOOF
    {"shade of aran", 16524},             // NPC_SHADE_OF_ARAN
    {"netherspite", 15689},               // NPC_NETHERSPITE
    {"prince malchezaar", 15690},         // NPC_PRINCE_MALCHEZAAR
    {"nightbane", 17225},                 // NPC_NIGHTBANE
    // 莫罗斯六名宾客（核心 boss_moroes.cpp GuestEntries[]）
    {"lady keira berrybuck", 17007},      // GuestEntries[0]
    {"lady catriona von'indi", 19872},    // GuestEntries[1]
    {"lord crispin ference", 19873},      // GuestEntries[2]
    {"baron rafe dreuger", 19874},        // GuestEntries[3]
    {"baroness dorothea millstipe", 19875}, // GuestEntries[4]
    {"lord robin daris", 19876},          // GuestEntries[5]
    // --- Black Temple（核心 black_temple.h） ---
    {"gathios the shatterer", 22949},     // NPC_GATHIOS_THE_SHATTERER
    // By leewheel 2026-11-08 (bug_106)：BT 首领名全部漏登记 ⇒ GetBtBoss() 恒 nullptr，
    //   老一拔刺/投刺、塔隆血魔杀构造体、伊利丹火人坦克等整套机制静默失效。
    //   名字来源：模块 BTHelpers.cpp BossName() 的字面量，entry 来源核心 black_temple.h。
    {"high warlord naj'entus", 22887},    // NPC_HIGH_WARLORD_NAJENTUS
    {"supremus", 22898},                  // NPC_SUPREMUS
    {"shade of akama", 22841},            // NPC_SHADE_OF_AKAMA
    {"teron gorefiend", 22871},           // NPC_TERON_GOREFIEND
    {"gurtogg bloodboil", 22948},         // NPC_GURTOGG_BLOODBOIL
    {"reliquary of the lost", 22856},     // NPC_RELIQUARY_OF_THE_LOST（BossName 用这个名字）
    {"reliquary of souls", 22856},        // 同上的别名（DATA_RELIQUARY_OF_SOULS）
    {"mother shahraz", 22947},            // NPC_MOTHER_SHAHRAZ
    {"illidan stormrage", 22917},         // NPC_ILLIDAN_STORMRAGE
    {"flame of azzinoth", 22997},         // NPC_FLAME_OF_AZZINOTH
    {"shadowy construct", 23111},         // NPC_SHADOWY_CONSTRUCT（塔隆血魔点名）
    // 灵魂圣骨匣三张脸（核心 black_temple.h）
    {"essence of suffering", 23418},      // NPC_ESSENCE_OF_SUFFERING
    {"essence of desire", 23419},         // NPC_ESSENCE_OF_DESIRE
    {"essence of anger", 23420},          // NPC_ESSENCE_OF_ANGER
    // 伊利达雷议会其余三名（gathios 见上）
    {"high nethermancer zerevor", 22950}, // NPC_HIGH_NETHERMANCER_ZEREVOR
    {"lady malande", 22951},              // NPC_LADY_MALANDE
    {"veras darkshadow", 22952},          // NPC_VERAS_DARKSHADOW
    // By leewheel 2026-11-08 (bug_106 续)：Hyjal 五首领名同罪漏登记。
    //   来源：HyjalTriggerContext.h 以英文名传进 HyjalEncounterTrigger::_bossName，
    //   HyjalActions/HyjalMultipliers 里数字实参走 entry 分支本来可用，
    //   但 Trigger 侧这 5 个走名字 ⇒ IsActiveInEncounter() 恒 false，整套触发器静默失效。
    //   entry 来源核心 hyjal.h:107-111。
    {"rage winterchill", 17767},          // NPC_WINTERCHILL
    {"anetheron", 17808},                 // NPC_ANETHERON
    {"kaz'rogal", 17888},                 // NPC_KAZROGAL
    {"azgalor", 17842},                   // NPC_AZGALOR
    {"archimonde", 17968},                // NPC_ARCHIMONDE
    // By leewheel 2026-11-08 (bug_106 续)：太阳井四首领名同罪漏登记。
    //   来源：SWPActionContext.h / SWPTriggerContext.h 的英文名实参，
    //   entry 来源核心 sunwell_plateau.h:63/67/69/74。
    {"kalecgos", 24850},                  // NPC_KALECGOS
    {"brutallus", 24882},                 // NPC_BRUTALLUS
    {"felmyst", 25038},                   // NPC_FELMYST
    {"grand warlock alythess", 25166},    // NPC_GRAND_WARLOCK_ALYTHESS
    // By leewheel 2026-11-10（the-lab 17 提交合并）：上游把 5 人本的取目标方式也全面
    //   改成了 `FindThreatTarget(bot, "<英文名>")`（原来本核是 `AI_VALUE2(Unit*,"find target","<entry>")`）。
    //   本服库内生物名是中文，`unit->GetName()` 与英文名恒不相等 ⇒ 不登记就是静默 nullptr，
    //   整套 5 人本机制直接失效（与 bug_106 的 raid 侧同一个病）。
    //   处置照旧：上游调用点一个字不改，只在本表按 entry 折算；entry 逐条取自合并前
    //   本核原数字 qualifier，不是猜的。以下按 entry 升序，分组注释标明所属副本。
    // ---- 地狱火城墙 / 鲜血熔炉 ----
    {"fiendish hound", 17280},
    {"omor the unscarred", 17308},
    {"hellfire watcher", 17309},
    {"nazan", 17536},                     // 地狱火城墙坐骑阶段
    {"vazruden", 17537},                  // 地狱火城墙尾王
    // ---- 幽暗沼泽 / 旧希尔斯布莱德 / 塞泰克大厅 ----
    {"hungarfen", 17770},
    {"time-lost controller", 18327},      // 旧希尔斯布莱德：敦霍尔德
    {"sethekk prophet", 18328},
    {"shirrak the dead watcher", 18371},
    {"darkweaver syth", 18472},
    {"talon king ikiss", 18473},
    {"syth arcane elemental", 20702},     // 塞斯召唤的四元素：库里名字带 " (1)" 后缀
    {"syth fire elemental", 20703},       //   ⇒ 即便按英文名也比对不上，只能按 entry
    {"syth frost elemental", 20704},
    {"syth shadow elemental", 20705},
    {"anzu", 23035},                      // 塞泰克大厅英雄难度召唤
    // ---- 能源舰 / 乌特加德城堡 ----
    {"nethermancer sepethrea", 19221},
    {"prince keleseth", 23953},
    {"ingvar the plunderer", 23954},
    // ---- 逃离敦霍尔德（时光之穴） ----
    {"salramm the fleshcrafter", 26530},
    {"chrono-lord epoch", 26532},
    // ---- 达克萨隆要塞 / 魔枢 / 魔环 / 乌特加德之巅 ----
    {"trollgore", 26630},
    {"novos the summoner", 26631},
    {"skadi the ruthless", 26693},
    {"keristrasza", 26723},
    {"grand magus telestra", 26731},
    {"anomalus", 26763},
    {"ormorok the tree-shaper", 26794},
    {"commander stoutbeard", 26796},      // 乌特加德之巅（联盟线）
    {"commander kolurg", 26798},          // 乌特加德之巅（部落线）
    {"king ymiron", 26861},
    {"grauf", 26893},                     // 乌特加德之巅坐骑阶段
    // ---- 紫罗兰监狱 / 岩石大厅 / 闪电大厅 ----
    {"dalronn the controller", 27389},
    {"varos cloudstrider", 27447},
    {"drakos the interrogator", 27654},
    {"mage-lord urom", 27655},
    {"ley-guardian eregos", 27656},
    {"horde commander", 27947},
    {"alliance commander", 27949},
    {"krystallus", 27977},
    {"sjonnir the ironshaper", 27978},
    {"ionar", 28546},
    {"general bjarngrim", 28586},
    {"volkhan", 28587},
    {"loken", 28923},
    // ---- 艾卓-尼鲁布 / 安卡赫特 / 古达克 ----
    {"anub'arak", 29120},
    {"slad'ran", 29304},
    {"gal'darah", 29306},
    {"elder nadox", 29309},
    {"jedoga shadowseeker", 29310},
    {"ichoron", 29313},
    {"zuramat the obliterator", 29314},
    {"erekem", 29315},
    {"ahn'kahar guardian", 30176},
    {"forgotten one", 30414},
    {"cyanigosa", 31134},                 // 魔环尾王
    // ---- 冠军的试炼 / 灵魂洪炉 / 萨隆矿坑 / 映像大厅 ----
    {"eadric the pure", 35119},
    {"ick", 36476},
    {"bronjahm", 36497},
    {"devourer of souls", 36502},
    {"scourgelord tyrannus", 36658},
    // By leewheel 2026-11-18 (bug_106 第四次发作)：上游把太阳井整套改成 boss state 后，
    //   GetSwpBoss() = FindThreatTarget(bot, BossName(boss))，而 BossName(SwpBosses) 返回英文名
    //   （SWPShared.cpp:30-50）⇒ 中文库下 FindThreatTarget 恒 nullptr ⇒ 整个太阳井
    //   （卡雷苟斯 / 布鲁塔卢斯 / 菲米丝 / 双子 / 穆鲁 / 基尔加丹）机制全部静默失效。
    //   卡雷苟斯等四条已在上方 2026-11-08 段登记，这里补漏掉的两条。
    //   entry 来源核心 sunwell_plateau.h:78 / :85。
    {"m'uru", 25741},                     // NPC_MURU
    {"kil'jaeden", 25315},                // NPC_KILJAEDEN
    //End By leewheel
    // By leewheel 2026-11-24（bug_106 第五次发作）：本轮合并 brighton-chi 70 提交，
    //   上游把 TBC 团本整体重构成「按 boss id 取目标」（GetTkBoss / GetZaBoss / …），
    //   底层仍是 FindThreatTarget(bot, BossName(boss))，BossName 返回英文名。
    //   本服库内生物名是中文 ⇒ 不登记就是静默 nullptr。按老规矩：上游调用点一个字不改，
    //   只在本表折算 entry。下面 25 条 entry 逐条取自核心头文件并与 acore_world 实测名核对：
    //     the_eye.h:46-51 / zulaman.h:51-61 / sunwell_plateau.h:65-88 / creature 库实测。
    //   ⛔ 其中 SWP 那 7 条（熵魔 / 熵魔前的虚空戒卫 / 基尔加丹之手 / 萨洛拉丝女王 /
    //     腐蚀者萨索瓦尔 / 影刃狂暴者 / 影刃怒火法师）在本次合并之前就已漏登记，
    //     即穆鲁二阶段、双子、卡雷苟斯、基尔加丹之手、太阳井小怪清单这几套机制
    //     长期静默失效，借这次一并补齐（不是本轮新引入的）。
    // ---- 风暴要塞（The Eye，核心 the_eye.h + 库实测）----
    {"al'ar", 19514},                     // NPC_ALAR
    {"void reaver", 19516},               // 空灵机甲（the_eye.h 未列，库实测）
    {"high astromancer solarian", 18805}, // 大星术师索兰莉安（同上）
    {"kael'thas sunstrider", 19622},      // NPC_KAELTHAS
    {"thaladred the darkener", 20064},    // NPC_THALADRED
    {"lord sanguinar", 20060},            // NPC_LORD_SANGUINAR
    {"grand astromancer capernian", 20062}, // NPC_CAPERNIAN
    {"master engineer telonicus", 20063}, // NPC_TELONICUS
    // ---- 祖阿曼（核心 zulaman.h）----
    {"akil'zon", 23574},                  // 埃基尔松（zulaman.h 未列常量，库实测）
    {"nalorakk", 23576},                  // NPC_NALORAKK
    {"jan'alai", 23578},                  // NPC_JANALAI
    {"halazzi", 23577},                   // NPC_HALAZZI
    {"hex lord malacrass", 24239},        // NPC_HEXLORD
    {"zul'jin", 23863},                   // NPC_ZULJIN
    {"amani'shi medicine man", 23581},    // NPC_AMANISHI_MEDICINE_MAN（注意 23542 是掷斧者）
    {"spirit of the lynx", 24143},        // NPC_SPIRIT_LYNX
    // ---- 格鲁尔的巢穴 / 玛瑟里顿的巢穴 ----
    {"gruul the dragonkiller", 19044},    // 屠龙者格鲁尔（库实测）
    {"magtheridon", 17257},               // 玛瑟里顿（库实测）
    // ---- 太阳井高地（核心 sunwell_plateau.h + 库实测，见上方说明）----
    {"sathrovarr the corruptor", 24892},  // NPC_SATHROVARR
    {"lady sacrolash", 25165},            // NPC_LADY_SACROLASH
    {"entropius", 25840},                 // NPC_ENTROPIUS（穆鲁二阶段）
    {"void sentinel", 25772},             // NPC_VOID_SENTINEL
    {"hand of the deceiver", 25588},      // NPC_HAND_OF_THE_DECEIVER
    {"shadowsword berserker", 25798},     // 影刃狂暴者（库实测）
    {"shadowsword fury mage", 25799},     // 影刃怒火法师（库实测）
    //End By leewheel
};

uint32 EntryForThreatName(std::string_view name)
{
    for (auto const& item : kThreatNameEntries)
    {
        if (EqualsIgnoringAsciiCase(item.name, name))
            return item.entry;
    }

    return 0;
}

//By leewheel 2026-11-08 (bug_106): 登记表漏项告警（每个名字只报一次）
//  上游新增英文名 qualifier 而没补表时，中文库下 unit->GetName() 与英文名恒不相等，
//  机制会静默失效（本次 BT/Kara 全灭就是这么来的）。这里打一条 WARN 让问题立刻可见，
//  按名字去重 + 加锁，避免 165 个调用点每 tick 刷屏。
void WarnMissingThreatName(std::string_view name)
{
    static std::mutex warnMutex;
    static std::unordered_set<std::string> warnedNames;

    std::lock_guard<std::mutex> lock(warnMutex);
    if (!warnedNames.emplace(std::string(name)).second)
        return;

    LOG_WARN("playerbots",
        "FindThreatTarget: 英文名 \"{}\" 不在 ThreatHelpers 登记表内，且库内生物名不匹配（中文库必然失配）"
        " ⇒ 该 raid 机制会静默失效。请在 src/Util/ThreatHelpers.cpp 的 kThreatNameEntries 补一行 entry。",
        std::string(name));
}
//End By leewheel

} // end anonymous namespace

Unit* FindThreatTarget(Player* bot, std::string_view name)
{
    if (name.empty() || !bot->GetGroup())
        return nullptr;

    // By leewheel 2026-11-04 entry 铁律：纯数字 qualifier 直接按 Entry 精确匹配；
    // 英文名 qualifier 先查登记表折算成 entry。两者都不命中时才退回上游的名字比对。
    uint32 entry = 0;
    if (std::all_of(name.begin(), name.end(), [](char c) { return c >= '0' && c <= '9'; }))
        entry = static_cast<uint32>(strtoul(std::string(name).c_str(), nullptr, 10));
    else
        entry = EntryForThreatName(name);
    // End By leewheel

    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit)
            continue;

        // By leewheel 2026-11-04 优先按 Entry 匹配
        if (entry != 0)
        {
            if (unit->GetEntry() == entry)
                return unit->IsInWorld() ? unit : nullptr;
            continue;
        }
        // End By leewheel

        if (EqualsIgnoringAsciiCase(unit->GetName(), name))
            return unit->IsInWorld() ? unit : nullptr;
    }

    //By leewheel 2026-11-08: 只有「名字没登记 + 墙上也没有」才告警；
    //  登记过但目标不在威胁列表（正常脱战/未进战斗）属于正常情况，不报警。
    if (entry == 0)
        WarnMissingThreatName(name);
    //End By leewheel

    return nullptr;
}
