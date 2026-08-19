DELETE FROM ai_playerbot_texts WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_below_twenty_percent_health',
    'sathrovarr_health_when_kalecgos_below_twenty_percent_health',
    'felmyst_flight_leader',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kalecgos_below_twenty_percent_health',
    'sathrovarr_health_when_kalecgos_below_twenty_percent_health',
    'felmyst_flight_leader',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user'
);

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1913, 'kalecgos_tank_sent_to_spectral_realm', '坦克 %tank 已被送入灵魂世界。当前卡雷苟斯的坦克是 %current。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1914, 'kalecgos_tank_should_enter_spectral_realm', '坦克 %tank 应进入灵魂世界。当前卡雷苟斯的坦克是 %current。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1915, 'kalecgos_below_twenty_percent_health', '卡雷苟斯的生命值已低于20%！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1916, 'sathrovarr_health_when_kalecgos_below_twenty_percent_health', '腐蚀者萨索瓦尔的生命值是 %sathrovarrHealth%！别忘了我们需要差不多同时击败他们！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1917, 'felmyst_flight_leader', '[NAME] 现在是飞行阶段领队。飞行阶段所有人都需要集合到 [NAME] 身边。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1918, 'kiljaeden_designated_dragon_orb_user', '%bot 是第一助理，也是指定的龙珠使用者！', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1919, 'kiljaeden_no_designated_dragon_orb_user', '没有机器人被指定为龙珠使用者，因此需要玩家来控制巨龙。如果您希望机器人使用龙珠，请为机器人设置助理标记。', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('kalecgos_tank_sent_to_spectral_realm', 100),
    ('kalecgos_tank_should_enter_spectral_realm', 100),
    ('kalecgos_below_twenty_percent_health', 100),
    ('sathrovarr_health_when_kalecgos_below_twenty_percent_health', 100),
    ('felmyst_flight_leader', 100),
    ('kiljaeden_designated_dragon_orb_user', 100),
    ('kiljaeden_no_designated_dragon_orb_user', 100);