DELETE FROM ai_playerbot_texts WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_health_call_first_under',
    'kalecgos_health_call_hold',
    'kalecgos_health_call_together',
    'kalecgos_health_call_release',
    'kalecgos_end_hold',
    'kalecgos_sathrovarr_banished',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'class_role_alythess_paladin_tank',
    'class_role_class_protection_paladin'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_health_call_first_under',
    'kalecgos_health_call_hold',
    'kalecgos_health_call_together',
    'kalecgos_health_call_release',
    'kalecgos_end_hold',
    'kalecgos_sathrovarr_banished',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user',
    'class_role_alythess_paladin_tank',
    'class_role_class_protection_paladin'
);

-- By leewheel 2026-11-18 合并 brighton 8ec3850d3 处置（本文件为冲突文件）：
--   上游本批把 1948~1959 共 12 条的 name 重新排定（新增 kalecgos health_call 六条、
--   sathrovarr_banished、class_role_* 两条），本核旧的中文台词键
--   kalecgos_below_twenty_percent_health / sathrovarr_health_when_* / felmyst_flight_leader /
--   eredar_twins_alythess_tank_* 全部废弃（代码已改用 class_role_* 新键，旧键在 src/ 已无引用）。
--   沿用 2026-10-06 / 10-13 / 10-24 / 11-04 既定处置：**id 与 name 跟随上游，文本保留中文**。
--   12 条译文为本轮按上游英文原文重写；模板占位符（%boss / %health% / %low / %tank / %current /
--   %bot）与原文一一对应，不得改写。本文件是 DELETE + INSERT 定值幂等写入，重放安全。

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1948, 'kalecgos_tank_sent_to_spectral_realm', '坦克 %tank 已被送入灵魂领域。当前负责卡雷苟斯的坦克是 %current。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1949, 'kalecgos_tank_should_enter_spectral_realm', '坦克 %tank 应该进入灵魂领域。当前负责卡雷苟斯的坦克是 %current。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1950, 'kalecgos_health_call_first_under', '%boss 的生命值已到 15%！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1951, 'kalecgos_health_call_hold', '%boss 的生命值是 %health%。在 %boss 降到 15% 之前停止攻击 %low！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1952, 'kalecgos_health_call_together', '%boss 的生命值是 %health%。继续同时攻击他们两个！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1953, 'kalecgos_health_call_release', '%boss 已降到 15%。重新攻击 %low！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1954, 'kalecgos_end_hold', '卡雷苟斯生命值只剩 5%。在萨索瓦尔被放逐之前停止攻击他！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1955, 'kalecgos_sathrovarr_banished', '萨索瓦尔已被放逐。全力击杀卡雷苟斯！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1956, 'kiljaeden_designated_dragon_orb_user', '机器人 %bot 是第一助理，也是指定的龙珠使用者。如果您希望只由玩家控制巨龙，请移除所有机器人的助理标记。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1957, 'kiljaeden_no_designated_dragon_orb_user', '没有机器人带有助理标记，因此必须由玩家来控制巨龙。如果您希望由机器人控制巨龙，请为某个机器人设置助理标记。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1958, 'class_role_alythess_paladin_tank', '艾利瑟丝圣骑士坦克', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1959, 'class_role_class_protection_paladin', '防护圣骑士', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('kalecgos_tank_sent_to_spectral_realm', 100),
    ('kalecgos_tank_should_enter_spectral_realm', 100),
    ('kalecgos_health_call_first_under', 100),
    ('kalecgos_health_call_hold', 100),
    ('kalecgos_health_call_together', 100),
    ('kalecgos_health_call_release', 100),
    ('kalecgos_end_hold', 100),
    ('kalecgos_sathrovarr_banished', 100),
    ('kiljaeden_designated_dragon_orb_user', 100),
    ('kiljaeden_no_designated_dragon_orb_user', 100),
    ('class_role_alythess_paladin_tank', 100),
    ('class_role_class_protection_paladin', 100);
