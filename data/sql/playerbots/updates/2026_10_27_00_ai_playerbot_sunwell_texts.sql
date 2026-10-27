DELETE FROM ai_playerbot_texts WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_below_twenty_percent_health',
    'sathrovarr_health_when_kalecgos_below_twenty_percent_health',
    'felmyst_flight_leader',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'eredar_twins_alythess_tank_main_tank_paladin',
    'eredar_twins_alythess_tank_paladin_tank',
    'eredar_twins_alythess_tank_no_paladin'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_below_twenty_percent_health',
    'sathrovarr_health_when_kalecgos_below_twenty_percent_health',
    'felmyst_flight_leader',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'eredar_twins_alythess_tank_main_tank_paladin',
    'eredar_twins_alythess_tank_paladin_tank',
    'eredar_twins_alythess_tank_no_paladin'
);

-- By leewheel 2026-10-06 合并 brighton the-lab 处置（本文件 2026_09_26_00 → 2026_10_06_00 重命名）：
--   上游把本批太阳井台词 id 从 1914+ 重编为 1928+（同时新增英文原文行），本 fork 保留中文文本，
--   id 采纳上游重编号（避免与上游其它新增行主键冲突）；文本键一律是 name，代码按 name 取用不受影响。
-- By leewheel 2026-10-13 合并 brighton the-lab(b0e59395)：上游把本批 id 再整体顺移 +1（1928+ → 1929+，
--   为 1928 腾位），文本零改动 ⇒ 沿用 2026-10-06 既定处置：id 跟随上游、文本保留中文。
-- By leewheel 2026-10-24 合并 brighton the-lab(85a7ca1e "update swp text ids")：上游把本文件
--   2026_10_06_00 → 2026_10_23_00 重命名，并把本批 id 再顺移（1929+ → 1930+，且 1933 被腾空：
--   kalecgos 三条 1930/1931/1932，sathrovarr 起跳到 1934，末条 1940）。文本改回英文原文。
--   ⇒ 沿用既定处置：文件名跟随上游重命名，id 跟随上游新号段，文本**保留本核中文**。
--   本文件是 DELETE + INSERT 定值幂等写入，重放安全。
-- By leewheel 2026-10-28 合并 brighton the-lab(9 提交批次，1c217494)：上游把本文件
--   2026_10_23_00 → 2026_10_27_00 重命名，id 再顺移（1930+ → 1945+，十条 1945-1954），
--   文本改回英文原文。⇒ 沿用既定处置：文件名跟随上游重命名，id 跟随上游新号段，
--   文本**保留本核中文**。本文件是 DELETE + INSERT 定值幂等写入，重放安全。
INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1945, 'kalecgos_tank_sent_to_spectral_realm', '坦克 %tank 已被送入灵魂世界。当前卡雷苟斯的坦克是 %current。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1946, 'kalecgos_tank_should_enter_spectral_realm', '坦克 %tank 应进入灵魂世界。当前卡雷苟斯的坦克是 %current。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1947, 'kalecgos_below_twenty_percent_health', '卡雷苟斯的生命值已低于20%！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1948, 'sathrovarr_health_when_kalecgos_below_twenty_percent_health', '腐蚀者萨索瓦尔的生命值是 %sathrovarrHealth%！别忘了我们需要差不多同时击败他们！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1949, 'felmyst_flight_leader', '[NAME] 现在是飞行阶段领队。飞行阶段所有人都需要集合到 [NAME] 身边。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1950, 'kiljaeden_designated_dragon_orb_user', '%bot 是第一助理，也是指定的龙珠使用者！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1951, 'kiljaeden_no_designated_dragon_orb_user', '没有机器人被指定为龙珠使用者，因此需要玩家来控制巨龙。如果您希望机器人使用龙珠，请为机器人设置助理标记。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1952, 'eredar_twins_alythess_tank_main_tank_paladin', '艾利瑟丝需要圣骑士坦克。%bot 是主坦克且是一名圣骑士，被指派来坦克艾利瑟丝。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1953, 'eredar_twins_alythess_tank_paladin_tank', '艾利瑟丝需要圣骑士坦克。主坦克不是圣骑士。%bot 是装备最好的圣骑士坦克，被指派来坦克艾利瑟丝。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1954, 'eredar_twins_alythess_tank_no_paladin', '艾利瑟丝需要圣骑士坦克。但当前没有圣骑士坦克。因此，主坦克 %bot 被指派来坦克艾利瑟丝。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('kalecgos_tank_sent_to_spectral_realm', 100),
    ('kalecgos_tank_should_enter_spectral_realm', 100),
    ('kalecgos_below_twenty_percent_health', 100),
    ('sathrovarr_health_when_kalecgos_below_twenty_percent_health', 100),
    ('felmyst_flight_leader', 100),
    ('kiljaeden_designated_dragon_orb_user', 100),
    ('kiljaeden_no_designated_dragon_orb_user', 100),
    ('eredar_twins_alythess_tank_main_tank_paladin', 100),
    ('eredar_twins_alythess_tank_paladin_tank', 100),
    ('eredar_twins_alythess_tank_no_paladin', 100);
