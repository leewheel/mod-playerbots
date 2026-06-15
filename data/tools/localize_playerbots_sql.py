#!/usr/bin/env python3
"""Localize playerbots SQL: Chinese bot texts + Chinese unique names."""

import re
import random
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TEXTS_BASE = ROOT / "sql/playerbots/base/ai_playerbot_texts.sql"
NAMES_BASE = ROOT / "sql/characters/base/playerbots_names.sql"
GUILD_NAMES_BASE = ROOT / "sql/characters/base/playerbots_guild_names.sql"
ARENA_NAMES_BASE = ROOT / "sql/characters/base/playerbots_arena_team_names.sql"
UPDATES_DIR = ROOT / "sql/playerbots/updates"
CHAR_UPDATES_DIR = ROOT / "sql/characters/updates"

# WoW name max 12 bytes UTF-8 (character names)
MAX_NAME_BYTES = 12
# Guild / arena team names (varchar 24)
MAX_GUILD_BYTES = 24

MALE_SURNAMES = list(
    "赵钱孙李周吴郑王冯陈褚卫蒋沈韩杨朱秦尤许何吕施张孔曹严华金魏陶姜戚谢邹喻柏水窦章云苏潘葛奚范彭郎鲁韦昌马苗凤花方俞任袁柳酆鲍史唐费廉岑薛雷贺倪汤滕殷罗毕郝邬安常乐于时傅皮卞齐康伍余元卜顾孟平黄和穆萧尹姚邵湛汪祁毛禹狄米贝明臧计伏成戴谈宋茅庞熊纪舒屈项祝董梁杜阮蓝闵席季麻强贾路娄危江童颜郭梅盛林刁钟徐邱骆高夏蔡田樊胡凌霍虞万支柯昝管卢莫经房裘缪干解应宗丁宣贲邓郁单杭洪包诸左石崔吉钮龚程嵇邢滑裴陆荣翁荀羊於惠甄曲家封芮羿储靳汲邴糜松井段富巫乌焦巴弓牧隗山谷车侯宓蓬全郗班仰秋仲伊宫宁仇栾暴甘钭厉戎祖武符刘景詹束龙叶幸司韶郜黎蓟薄印宿白怀蒲邰从鄂索咸籍赖卓蔺屠蒙池乔阴郁胥能苍双闻莘党翟谭贡劳逄姬申扶堵冉宰郦雍却璩桑桂濮牛寿通边扈燕冀郏浦尚农温别庄晏柴瞿阎充慕连茹习宦艾鱼容向古易慎戈廖庾终暨居衡步都耿满弘匡国文寇广禄阙东欧殳沃利蔚越夔隆师巩厍聂晁勾敖融冷訾辛阚那简饶空曾毋沙乜养鞠须丰巢关蒯相查后荆红游竺权逯盖益桓公万俟司马上官欧阳夏侯诸葛闻人东方赫连皇甫尉迟公羊澹台公冶宗政濮阳淳于单于太叔申屠公孙仲孙轩辕令狐钟离宇文长孙慕容司徒司空丌司寇仉督子车颛孙端木巫马公西漆雕乐正壤驷公良拓跋夹谷宰父谷梁晋楚闫法汝鄢涂钦段干百里东郭南门呼延归海岳帅缑亢况后有琴梁丘左丘东门西门商牟佘佴伯赏南宫墨哈谯笪年爱阳佟"
)
FEMALE_SURNAMES = MALE_SURNAMES  # shared pool

MALE_GIVEN = [
    "伟", "强", "磊", "军", "勇", "毅", "俊", "峰", "超", "杰", "涛", "明", "辉", "鹏", "宇", "浩", "博", "文", "武", "龙",
    "飞", "翔", "晨", "阳", "轩", "泽", "睿", "哲", "诚", "信", "义", "礼", "智", "仁", "安", "平", "康", "泰", "顺", "昌",
    "盛", "华", "荣", "富", "贵", "福", "禄", "寿", "喜", "乐", "天", "地", "山", "川", "江", "河", "海", "云", "风", "雷",
    "霆", "霖", "霜", "雪", "冰", "炎", "焱", "煜", "晖", "曜", "朗", "清", "澈", "润", "泽", "洋", "瀚", "渊", "深", "远",
    "志", "行", "远", "达", "通", "畅", "扬", "帆", "航", "途", "程", "进", "升", "腾", "跃", "迈", "步", "征", "战", "胜",
    "凯", "旋", "归", "来", "临", "至", "逢", "遇", "见", "闻", "知", "识", "学", "思", "想", "念", "忆", "怀", "慕", "恋",
    "子", "之", "亦", "若", "如", "似", "犹", "尚", "犹", "更", "复", "再", "又", "还", "仍", "依", "凭", "仗", "恃", "靠",
    "承", "继", "传", "绍", "启", "开", "创", "立", "建", "筑", "构", "造", "制", "作", "成", "就", "功", "业", "绩", "效",
    "德", "才", "能", "力", "技", "艺", "术", "法", "道", "理", "论", "辩", "议", "评", "判", "断", "决", "定", "断", "裁",
    "正", "直", "方", "圆", "规", "矩", "律", "度", "衡", "均", "齐", "整", "齐", "序", "列", "排", "布", "置", "设", "配",
    "浩然", "子轩", "宇航", "博文", "天佑", "俊杰", "嘉懿", "鸿煊", "烨磊", "晟睿", "修杰", "黎昕", "远航", "旭尧", "鸿涛", "伟祺",
    "荣轩", "越泽", "浩宇", "瑾瑜", "皓轩", "擎苍", "擎宇", "志泽", "子轩", "睿渊", "弘文", "哲瀚", "雨泽", "楷瑞", "建辉", "晋鹏",
    "天磊", "绍辉", "泽洋", "鑫磊", "鹏煊", "昊强", "伟宸", "博超", "君浩", "子骞", "鹏涛", "炎彬", "鹤轩", "越彬", "风华", "靖琪",
    "明辉", "伟诚", "明轩", "健柏", "修洁", "志泽", "弘文", "峻熙", "嘉懿", "煜城", "懿轩", "烨伟", "苑博", "伟泽", "熠彤", "鸿煊",
    "博涛", "烨霖", "烨华", "煜祺", "智宸", "正豪", "昊然", "明杰", "立诚", "立轩", "立辉", "峻熙", "弘文", "熠彤", "鸿煊", "烨霖",
]

FEMALE_GIVEN = [
    "芳", "娜", "敏", "静", "丽", "艳", "娟", "秀", "英", "华", "慧", "巧", "美", "娜", "娇", "娥", "玲", "琳", "晶", "瑶",
    "莹", "洁", "雪", "梅", "兰", "竹", "菊", "荷", "莲", "蓉", "莉", "薇", "芷", "萱", "蕊", "蕾", "朵", "菲", "芳", "芬",
    "香", "馨", "馥", "薰", "涵", "淑", "娴", "婉", "婷", "婧", "媛", "嫣", "妍", "姝", "姣", "姿", "韵", "雅", "淑", "惠",
    "慈", "善", "和", "柔", "温", "暖", "晴", "朗", "清", "澈", "灵", "秀", "巧", "慧", "颖", "聪", "敏", "捷", "利", "速",
    "雨", "露", "霜", "霞", "虹", "霓", "彩", "云", "月", "星", "辰", "曦", "昕", "晗", "旭", "阳", "春", "夏", "秋", "冬",
    "诗", "词", "歌", "舞", "琴", "棋", "书", "画", "绣", "织", "缝", "裁", "妆", "饰", "佩", "环", "珠", "玉", "翠", "碧",
    "梦", "思", "念", "忆", "怀", "慕", "恋", "爱", "情", "意", "心", "怡", "悦", "欣", "喜", "乐", "笑", "欢", "畅", "舒",
    "若", "如", "似", "犹", "亦", "尚", "犹", "更", "复", "再", "又", "还", "仍", "依", "然", "宛", "宛", "宛如", "宛若", "宛如",
    "雨萱", "诗涵", "梦琪", "欣怡", "可馨", "雨桐", "思妍", "语嫣", "梓涵", "一诺", "依诺", "若曦", "语汐", "梓萱", "雨欣", "思琪",
    "佳怡", "子涵", "欣妍", "诗琪", "雨婷", "思雨", "梦瑶", "佳琪", "雨涵", "思颖", "语彤", "梓晴", "雨晴", "思涵", "梦洁", "佳慧",
    "雨菲", "思佳", "语涵", "梓琳", "雨萌", "思彤", "梦婷", "佳欣", "雨瑶", "思瑶", "语萱", "梓瑶", "雨嘉", "思嘉", "梦嘉", "佳嘉",
    "雨晨", "思晨", "语晨", "梓晨", "雨曦", "思曦", "梦曦", "佳曦", "雨昕", "思昕", "语昕", "梓昕", "雨晗", "思晗", "梦晗", "佳晗",
    "雨菲", "思菲", "语菲", "梓菲", "雨薇", "思薇", "梦薇", "佳薇", "雨蕊", "思蕊", "语蕊", "梓蕊", "雨蕾", "思蕾", "梦蕾", "佳蕾",
]

GUILD_ADJECTIVES = [
    "暗夜", "黎明", "风暴", "烈焰", "寒冰", "雷霆", "星辰", "银月", "黄金", "翡翠", "赤红", "幽暗", "神圣", "永恒", "无畏", "荣耀",
    "钢铁", "龙牙", "狮心", "狼魂", "鹰翼", "虎爪", "熊胆", "蛇影", "凤凰", "麒麟", "苍狼", "白鹿", "黑鸦", "紫晶", "碧蓝", "赤焰",
    "霜刃", "风语", "雷怒", "光誓", "影刃", "血誓", "铁壁", "金盾", "银枪", "铜锤", "秘银", "精金", "泰坦", "奥术", "自然", "元素",
]

GUILD_NOUNS = [
    "守护者", "兄弟会", "联盟", "军团", "骑士团", "议会", "商会", "行会", "远征军", "义勇军", "守望者", "先锋", "精锐", "禁卫", "战团",
    "氏族", "部族", "公会", "结社", "同盟", "联合体", "突击队", "特遣队", "护卫队", "先锋队",
]

GUILD_THEMES = [
    "艾泽拉斯", "暴风城", "奥格瑞玛", "铁炉堡", "达纳苏斯", "幽暗城", "雷霆崖", "银月城", "沙塔斯", "达拉然", "诺森德", "外域",
    "燃烧军团", "天灾军团", "龙眠联军", "银色北伐", "探险者", "冒险者", "流浪者", "游侠", "法师", "战士", "牧师", "盗贼", "猎人",
    "术士", "萨满", "德鲁伊", "圣骑士", "死亡骑士", "武僧", "恶魔猎手", "虚空", "暗影", "圣光", "自然之力", "元素之力",
]

ARENA_PREFIXES = [
    "一言不合", "没蓝了", "技能加载", "从零到神", "力竭而亡", "输出靠吼", "治疗何在", "坦克倒了", "全员躺平", "团灭先锋",
    "躺尸专家", "地板温暖", "复活快来", "别奶我了", "快驱散啊", "打断失败", "仇恨乱了", "OT了快跑", "开怪开怪", "等等我",
    "蓝不够了", "没怒气啊", "能量枯竭", "符文不足", "圣能告急", "连击断了", "饰品没亮", "爆发空窗", "走位失误", "踩火踩火",
]

ARENA_SUFFIXES = [
    "战队", "小队", "组合", "搭档", "双子", "铁三角", "五人组", "竞技场", "角斗士", "挑战者", "征服者", "统治者", "霸主", "之王",
    "传说", "神话", "史诗", "传奇", "精英", "王牌", "尖兵", "先锋", "突击队", "特战队",
]

ARENA_SOLO = [
    "暴毙", "躺赢", "混分", "上分", "掉分", "连跪", "连胜", "一血", "绝杀", "翻盘", "碾压", "吊打", "被虐", "真香", "下饭",
    "刮痧", "爆发", "收割", "开团", "收尾", "补刀", "抢人头", "卖队友", "背锅侠", "锅王", "混子", "大腿", "挂件", "工具人",
]


def utf8_len(s: str) -> int:
    return len(s.encode("utf-8"))


def parse_text_row(line: str) -> dict | None:
    """Parse one ai_playerbot_texts VALUES tuple."""
    m = re.match(
        r"\s*\((\d+),\s*'((?:[^'\\]|\\.)*)',\s*'((?:[^'\\]|\\.)*)',\s*(\d+),\s*(\d+),"
        r"\s*'((?:[^'\\]|\\.)*)',\s*'((?:[^'\\]|\\.)*)',\s*'((?:[^'\\]|\\.)*)',\s*'((?:[^'\\]|\\.)*)',"
        r"\s*'((?:[^'\\]|\\.)*)',\s*'((?:[^'\\]|\\.)*)',\s*'((?:[^'\\]|\\.)*)',\s*'((?:[^'\\]|\\.)*)'\)",
        line.strip().rstrip(","),
    )
    if not m:
        return None
    g = m.groups()
    return {
        "id": int(g[0]),
        "name": g[1],
        "text": g[2],
        "say_type": g[3],
        "reply_type": g[4],
        "loc1": g[5],
        "loc2": g[6],
        "loc3": g[7],
        "loc4": g[8],
        "loc5": g[9],
        "loc6": g[10],
        "loc7": g[11],
        "loc8": g[12],
        "raw": line,
    }


def esc_sql(s: str) -> str:
    return s.replace("\\", "\\\\").replace("'", "\\'")


def format_text_row(row: dict) -> str:
    cn = row["loc4"].strip()
    text = cn if cn else row["text"]
    return (
        f"\t({row['id']}, '{esc_sql(row['name'])}', '{esc_sql(text)}', {row['say_type']}, {row['reply_type']}, "
        f"'{esc_sql(row['loc1'])}', '{esc_sql(row['loc2'])}', '{esc_sql(row['loc3'])}', '{esc_sql(row['loc4'])}', "
        f"'{esc_sql(row['loc5'])}', '{esc_sql(row['loc6'])}', '{esc_sql(row['loc7'])}', '{esc_sql(row['loc8'])}')"
    )


def localize_texts_file(path: Path) -> tuple[int, int]:
    content = path.read_text(encoding="utf-8")
    lines = content.splitlines()
    out_lines = []
    changed = 0
    kept_english = 0
    for line in lines:
        if line.strip().startswith("(") and ", '" not in line[:3]:
            row = parse_text_row(line)
            if row:
                cn = row["loc4"].strip()
                if cn:
                    if row["text"] != cn:
                        changed += 1
                    row["text"] = cn
                    out_lines.append(format_text_row(row) + ("," if line.rstrip().endswith(",") else ""))
                else:
                    kept_english += 1
                    out_lines.append(line)
            else:
                out_lines.append(line)
        else:
            out_lines.append(line)
    path.write_text("\n".join(out_lines) + "\n", encoding="utf-8")
    return changed, kept_english


def localize_texts_insert_block(content: str) -> str:
    """Localize INSERT value lines inside update SQL files."""
    lines = content.splitlines()
    out = []
    for line in lines:
        if line.strip().startswith("("):
            row = parse_text_row(line)
            if row and row["loc4"].strip():
                row["text"] = row["loc4"].strip()
                suffix = "," if line.rstrip().endswith(",") else ""
                out.append(format_text_row(row) + suffix)
                continue
        out.append(line)
    return "\n".join(out)


def generate_name_pool(count: int, is_female: bool) -> list[str]:
    surnames = FEMALE_SURNAMES if is_female else MALE_SURNAMES
    givens = FEMALE_GIVEN if is_female else MALE_GIVEN
    names: set[str] = set()
    attempts = 0
    while len(names) < count and attempts < count * 50:
        attempts += 1
        surname = random.choice(surnames)
        given = random.choice(givens)
        # try 2-char and 3-char forms
        if random.random() < 0.35 and len(given) >= 2:
            name = surname + given[:2]
        else:
            name = surname + given[:1]
        if utf8_len(name) > MAX_NAME_BYTES:
            continue
        if len(name) < 2:
            continue
        names.add(name)
    if len(names) < count:
        # numeric suffix fallback using rare chars
        i = 0
        while len(names) < count:
            base = random.choice(surnames) + random.choice(givens)[:1]
            suffix = chr(0x4e00 + (i % 200))  # CJK fallback
            name = (base + suffix)[:4]
            if utf8_len(name) <= MAX_NAME_BYTES:
                names.add(name)
            i += 1
    return list(names)


def generate_names_sql() -> str:
    # gender distribution from original file
    gender_counts = {0: 10000, 1: 10000}
    for g in range(2, 18):
        gender_counts[g] = 5000
    gender_counts[17] = 4999

    lines = [
        "DROP TABLE IF EXISTS `playerbots_names`;",
        "CREATE TABLE `playerbots_names` (",
        "  `name_id` INT(11) NOT NULL UNIQUE,",
        "  `name` varchar(255) NOT NULL,",
        "  `gender` tinyint(3) unsigned NOT NULL,",
        "PRIMARY KEY (`name_id`)",
        ") ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Playerbot RandomBot names';",
        "",
        "INSERT INTO `playerbots_names` VALUES",
    ]

    name_id = 0
    tuple_lines = []
    used_pairs: set[tuple[str, int]] = set()

    for gender, count in sorted(gender_counts.items()):
        is_female = gender % 2 == 1
        pool = generate_name_pool(count * 3, is_female)  # extra for collisions
        pi = 0
        assigned = 0
        while assigned < count:
            if pi >= len(pool):
                pool.extend(generate_name_pool(count, is_female))
            name = pool[pi]
            pi += 1
            pair = (name, gender)
            if pair in used_pairs:
                continue
            used_pairs.add(pair)
            tuple_lines.append(f"({name_id},'{esc_sql(name)}',{gender})")
            name_id += 1
            assigned += 1

    # join tuples with commas
    for i, t in enumerate(tuple_lines):
        sep = "," if i < len(tuple_lines) - 1 else ";"
        lines.append(t + sep)

    lines.extend(
        [
            "",
            "DELETE FROM `playerbots_names` WHERE LENGTH(`name`) > 12;",
            "ALTER TABLE `playerbots_names` MODIFY `name` varchar(12);",
            "ALTER TABLE `playerbots_names` ADD UNIQUE INDEX name(name, gender);",
            "",
        ]
    )
    return "\n".join(lines)


def fits_max_bytes(s: str, max_bytes: int) -> bool:
    return utf8_len(s) <= max_bytes


def generate_unique_labels(
    count: int,
    max_bytes: int,
    builders: list,
    seed_offset: int = 0,
) -> list[str]:
    """Generate unique strings using callable builders that return candidate names."""
    random.seed(20260615 + seed_offset)
    names: set[str] = set()
    attempts = 0
    while len(names) < count and attempts < count * 200:
        attempts += 1
        builder = random.choice(builders)
        name = builder()
        if not name or not fits_max_bytes(name, max_bytes):
            continue
        names.add(name)
    if len(names) < count:
        i = 0
        while len(names) < count:
            base = f"战队{i}"
            if fits_max_bytes(base, max_bytes):
                names.add(base)
            i += 1
    return list(names)


def generate_guild_names_sql(count: int = 400) -> str:
    def b_adj_noun():
        return random.choice(GUILD_ADJECTIVES) + random.choice(GUILD_NOUNS)

    def b_theme_noun():
        return random.choice(GUILD_THEMES) + random.choice(GUILD_NOUNS)

    def b_adj_theme():
        adj = random.choice(GUILD_ADJECTIVES)
        theme = random.choice(GUILD_THEMES)
        name = adj + theme
        return name if fits_max_bytes(name, MAX_GUILD_BYTES) else adj + theme[:2]

    builders = [b_adj_noun, b_theme_noun, b_adj_theme]
    names = generate_unique_labels(count, MAX_GUILD_BYTES, builders, seed_offset=1)

    lines = [
        "DROP TABLE IF EXISTS `playerbots_guild_names`;",
        "CREATE TABLE `playerbots_guild_names` (",
        "                                          `name_id` INT(11) NOT NULL AUTO_INCREMENT UNIQUE,",
        "                                          `name` varchar(24) NOT NULL UNIQUE,",
        "                                          PRIMARY KEY (`name_id`)",
        ") ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Playerbot guild names';",
        "",
        "INSERT INTO `playerbots_guild_names` VALUES",
    ]
    for i, name in enumerate(names):
        sep = "," if i < len(names) - 1 else ";"
        lines.append(f"(NULL, '{esc_sql(name)}')" + sep)
    lines.append("")
    return "\n".join(lines)


def generate_arena_names_sql(per_type: int = 100) -> str:
    type_counts = {2: per_type, 3: per_type, 5: per_type}

    def b_prefix_suffix():
        return random.choice(ARENA_PREFIXES) + random.choice(ARENA_SUFFIXES)

    def b_solo():
        return random.choice(ARENA_SOLO) + random.choice(ARENA_SUFFIXES)

    def b_adj_solo():
        return random.choice(GUILD_ADJECTIVES) + random.choice(ARENA_SOLO)

    builders = [b_prefix_suffix, b_solo, b_adj_solo]
    used: set[str] = set()
    all_rows: list[tuple[str, int]] = []

    for arena_type, count in sorted(type_counts.items()):
        names: list[str] = []
        attempts = 0
        random.seed(20260615 + arena_type * 10)
        while len(names) < count and attempts < count * 300:
            attempts += 1
            name = random.choice(builders)()
            if not name or not fits_max_bytes(name, MAX_GUILD_BYTES) or name in used:
                continue
            used.add(name)
            names.append(name)
        if len(names) < count:
            i = 0
            while len(names) < count:
                fallback = f"竞技{i}"
                if fits_max_bytes(fallback, MAX_GUILD_BYTES) and fallback not in used:
                    used.add(fallback)
                    names.append(fallback)
                i += 1
        for name in names:
            all_rows.append((name, arena_type))

    lines = [
        "DROP TABLE IF EXISTS `playerbots_arena_team_names`;",
        "CREATE TABLE `playerbots_arena_team_names` (",
        "                                               `name_id` mediumint(8) NOT NULL AUTO_INCREMENT UNIQUE,",
        "                                               `name` varchar(24) NOT NULL UNIQUE,",
        "                                               `type` TINYINT(3) NOT NULL,",
        "                                               PRIMARY KEY (`name_id`)",
        ") ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Playerbot arena team names';",
        "",
        "",
        "DELETE FROM `playerbots_arena_team_names`;",
        "INSERT INTO `playerbots_arena_team_names` (`name_id`,`name`, `type`) VALUES",
    ]
    for i, (name, arena_type) in enumerate(all_rows):
        sep = "," if i < len(all_rows) - 1 else ";"
        lines.append(f"(NULL, '{esc_sql(name)}',{arena_type})" + sep)
    lines.append("")
    return "\n".join(lines)


def main():
    random.seed(20260615)
    print("Localizing ai_playerbot_texts base...")
    ch, en = localize_texts_file(TEXTS_BASE)
    print(f"  Updated {ch} rows from text_loc4, {en} rows still English (no loc4)")

    update_files = sorted(UPDATES_DIR.glob("*.sql"))
    for uf in update_files:
        text = uf.read_text(encoding="utf-8")
        if "INSERT INTO ai_playerbot_texts" in text or "INSERT INTO `ai_playerbot_texts`" in text:
            new_text = localize_texts_insert_block(text)
            if new_text != text:
                uf.write_text(new_text, encoding="utf-8")
                print(f"  Localized inserts in {uf.name}")

    print("Generating Chinese playerbots_names.sql ...")
    NAMES_BASE.write_text(generate_names_sql(), encoding="utf-8")
    print(f"  Written {NAMES_BASE} ({NAMES_BASE.stat().st_size // 1024} KB)")

    print("Generating Chinese playerbots_guild_names.sql ...")
    GUILD_NAMES_BASE.write_text(generate_guild_names_sql(400), encoding="utf-8")
    print(f"  Written {GUILD_NAMES_BASE}")

    print("Generating Chinese playerbots_arena_team_names.sql ...")
    ARENA_NAMES_BASE.write_text(generate_arena_names_sql(100), encoding="utf-8")
    print(f"  Written {ARENA_NAMES_BASE}")

    # migration update for existing DBs
    mig = ROOT / "sql/playerbots/updates/2026_06_15_00_ai_playerbot_texts_chinese_primary.sql"
    mig.write_text(
        "-- Promote Chinese (text_loc4) to primary text column for all bot texts\n"
        "UPDATE `ai_playerbot_texts` SET `text` = `text_loc4` WHERE `text_loc4` IS NOT NULL AND `text_loc4` <> '';\n",
        encoding="utf-8",
    )
    print(f"  Created migration {mig.name}")

    names_mig = CHAR_UPDATES_DIR / "2026_06_15_00_playerbots_names_chinese.sql"
    names_mig.parent.mkdir(parents=True, exist_ok=True)
    names_mig.write_text(
        "-- Replace random bot names with Chinese names (re-import from base playerbots_names.sql)\n"
        "TRUNCATE TABLE `playerbots_names`;\n",
        encoding="utf-8",
    )

    guild_mig = CHAR_UPDATES_DIR / "2026_06_15_01_playerbots_guild_names_chinese.sql"
    guild_mig.write_text(
        "-- Replace bot guild names with Chinese names (re-import from base playerbots_guild_names.sql)\n"
        "TRUNCATE TABLE `playerbots_guild_names`;\n",
        encoding="utf-8",
    )

    arena_mig = CHAR_UPDATES_DIR / "2026_06_15_02_playerbots_arena_names_chinese.sql"
    arena_mig.write_text(
        "-- Replace arena team names with Chinese names (re-import from base playerbots_arena_team_names.sql)\n"
        "TRUNCATE TABLE `playerbots_arena_team_names`;\n",
        encoding="utf-8",
    )
    print("Done.")


if __name__ == "__main__":
    main()
