# 恢复日志：brighton-chi the-lab 引入（待决策）

日期：2026-08-01
分支：`PlayerBotsPlus2026Edition`
工作区状态：**clean**（无未提交改动）

## 已完成的阶段一：上游合并（已完成并提交）

- 合并上游 mod-playerbots `355a3abf..ba46fcde`（6 个提交，ICC 优化/GPL 头/boss helpers 等）
- 11 个冲突文件全部解决：保留我方 4 参 `IsMechanicTrackerBot(PlayerbotAI*, Player*, uint32, Player* = nullptr)`（DPS 限定语义），采纳上游 ICC 重构、TKMultipliers 武器阶段仇恨抑制、TKTriggers Alar 逻辑
- 27 处 2 参 `IsMechanicTrackerBot(bot, MAP)` 调用点已迁移为 4 参（BT/Kara/Mag/SSC/ZA）
- 提交记录：
  - `98fdb307` Merge upstream/master (355a3abf..ba46fcde) into PlayerBotsPlus2026Edition
  - `d66a3d19` Fix TK compile errors: extra brace in TKMultipliers, duplicate IsActive definitions in TKTriggers
- 备份分支：`backup-before-merge-20260731`

## 阶段二：brighton-chi the-lab 引入 —— ⚠️ 用户尚未决策

### 关键事实

- 本地 checkout：`D:\1.PlusPB\OtherGuysPBMod\brighton-chi\mod-playerbots`（分支 `the-lab`）
- 已 fetch 为本地 ref：`brighton/lab`（= the-lab @ `7cae49e4`）
- **共同祖先**：`a8f8780a`（正是我们刚合并的上游点）→ 干净三方合并基础
- the-lab = a8f8780a + **21 个提交**（142 文件变化，78 新增文件）
- 我们 = a8f8780a + **80 个提交**（373 文件变化）
- **122 个重叠文件**（双方都改过 = 冲突面），集中在：
  - `src/Ai/Raid`：54 文件（Kara/TK/SWP/Hyjal 双方独立重写）
  - `src/Ai/Dungeon`：50 文件
  - 其余分散在 Base/Engine/Factory/PlayerbotAI 等

### 风险点（全量 merge 会破坏的内容）

1. **中文名字 SQL**：我们 `data/sql/characters/base/playerbots_names.sql`（100,010 行中文名）vs 他们（100,011 行英文名）—— 必须保留我们的
2. **VanillaNaxx**：the-lab 完全没有（0 文件），我们独有 26 文件 —— 必须保留
3. **HFR/Mech/Seth/UB 副本策略**：我们其实**已引入过**（leewheel 2026-07-27 注释）—— 与 the-lab 差异只剩注释删除等小改动
4. **BWL/MC**：双方都有，但 the-lab 版本缺文件（BWLHelpers.cpp 等差异）
5. **Kara/TK/SWP/Hyjal**：双方各自独立重写，直接 merge 冲突巨大（KaraActions 1758 行、TKMultipliers 374 行、HyjalHelpers 392 行、SWP 795/982 行）
6. **the-lab 独有提交多为实验性**：felmyst 系列 9 个（"untested"/"try to get this thing to work"）、Muru tweaks、TK/Kara static role members、SWP spell hook 修改

### 用户此前模式（重要线索）

- 选择性引入：`//By leewheel 2026-07-27 引入brighton-chi的UB策略` 等注释
- 已有提交 `118ff09b` "Raid AI: static role checks & SWP/Kalec updates"、`1b3f0f87` "Sync SWP/TK raid AI & LFG fixes" —— 说明此前已同步过 brighton-chi 的部分改动

### 已向用户提供的选项（未选择，关机前）

- **A. 选择性引入（推荐）**：只引入稳定有价值的改动（Felmyst/Muru 完整 encounter、TK/Kara static role members、SWP spell hook 等），跳过实验性提交，保护中文名 SQL 和 VanillaNaxx
- **B. 全量合并 the-lab**：`git merge brighton/lab` 后解决 122 个重叠文件冲突（工作量巨大），需手动保护中文名/VanillaNaxx
- **C. 先出详细差异报告**：逐提交分析 21 个提交后再决定

### 下次继续的步骤

1. 恢复 ref：`git fetch "D:\1.PlusPB\OtherGuysPBMod\brighton-chi\mod-playerbots" the-lab:refs/remotes/brighton/lab`（若已被清理）
2. 向用户确认 A/B/C 策略
3. 按策略执行（推荐 A：逐提交 cherry-pick/手动移植，排除实验性提交）
4. 合并后验证：编译检查、`git diff --check`、中文名 SQL 与 VanillaNaxx 完整性

### 临时文件

- `$env:TEMP\opencode\bc_files.txt`、`bc_their.txt`、`bc_ours.txt`、`tk_actions_upstream.diff`（分析产物，可重建）
