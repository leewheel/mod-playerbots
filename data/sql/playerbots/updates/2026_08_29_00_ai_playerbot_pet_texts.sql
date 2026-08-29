-- By leewheel 2026-08-29
-- 补齐宠物/驯服(PetsAction/TameAction)全部机器人文本。
-- 背景：猎人机器人无宠物时反复触发 pet_no_pet_error 等文本缺失报错
--      （GetBotText 查不到该名称的文本即刷"没有此名称的机器人文本"错误日志）。
-- 数据来源：代码内 GetBotTextOrDefault 的中文默认值（游戏文本必须中文），幂等：先 DELETE 后 INSERT。
-- End By leewheel

DELETE FROM `ai_playerbot_texts` WHERE `name` IN (
    'pet_attack_failed', 'pet_attack_success', 'pet_follow_success', 'pet_invalid_target_error',
    'pet_no_pet_error', 'pet_no_target_error', 'pet_pvp_prohibited_error', 'pet_stance_aggressive',
    'pet_stance_defensive', 'pet_stance_passive', 'pet_stance_report', 'pet_stance_set_success',
    'pet_stance_unknown', 'pet_stay_success', 'pet_target_dead_error', 'pet_type_guardian',
    'pet_type_pet', 'pet_unknown_command_error', 'pet_usage_error',
    'tame_create_pet_failed', 'tame_creature_template_not_found', 'tame_exotic_requires_beast_mastery',
    'tame_invalid_id_error', 'tame_no_hunter_pet_to_abandon', 'tame_no_pet_by_family', 'tame_no_pet_by_id',
    'tame_no_pet_by_name', 'tame_no_pet_to_rename', 'tame_pet_abandoned', 'tame_pet_changed',
    'tame_pet_changed_initialized', 'tame_pet_name_alpha_error', 'tame_pet_name_forbidden_error',
    'tame_pet_name_length_error', 'tame_pet_rename_refresh_hint', 'tame_pet_renamed', 'tame_usage_error'
);

INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES
    (2000, 'pet_attack_failed', '宠物未攻击。（已在攻击或无法攻击目标）', 0, 0, '', '', '', '', '', '', '', ''),
    (2001, 'pet_attack_success', '已命令宠物攻击你的目标。', 0, 0, '', '', '', '', '', '', '', ''),
    (2002, 'pet_follow_success', '已命令宠物跟随。', 0, 0, '', '', '', '', '', '', '', ''),
    (2003, 'pet_invalid_target_error', '目标不是机器人的有效攻击目标。', 0, 0, '', '', '', '', '', '', '', ''),
    (2004, 'pet_no_pet_error', '你没有宠物或守卫宠物。', 0, 0, '', '', '', '', '', '', '', ''),
    (2005, 'pet_no_target_error', '主控未选择有效目标。', 0, 0, '', '', '', '', '', '', '', ''),
    (2006, 'pet_pvp_prohibited_error', '在禁止 PvP 的区域无法命令宠物攻击玩家。', 0, 0, '', '', '', '', '', '', '', ''),
    (2007, 'pet_stance_aggressive', '主动', 0, 0, '', '', '', '', '', '', '', ''),
    (2008, 'pet_stance_defensive', '防御', 0, 0, '', '', '', '', '', '', '', ''),
    (2009, 'pet_stance_passive', '被动', 0, 0, '', '', '', '', '', '', '', ''),
    (2010, 'pet_stance_report', '当前 %type "%name" 的姿态: %stance。', 0, 0, '', '', '', '', '', '', '', ''),
    (2011, 'pet_stance_set_success', '宠物姿态已设为 %stance。', 0, 0, '', '', '', '', '', '', '', ''),
    (2012, 'pet_stance_unknown', '未知', 0, 0, '', '', '', '', '', '', '', ''),
    (2013, 'pet_stay_success', '已命令宠物停留。', 0, 0, '', '', '', '', '', '', '', ''),
    (2014, 'pet_target_dead_error', '目标未存活。', 0, 0, '', '', '', '', '', '', '', ''),
    (2015, 'pet_type_guardian', '守卫', 0, 0, '', '', '', '', '', '', '', ''),
    (2016, 'pet_type_pet', '宠物', 0, 0, '', '', '', '', '', '', '', ''),
    (2017, 'pet_unknown_command_error', '未知宠物命令: %param。用法: 宠物 <主动|防御|被动|stance|attack|follow|stay>', 0, 0, '', '', '', '', '', '', '', ''),
    (2018, 'pet_usage_error', '用法: 宠物 <主动|防御|被动|stance|attack|follow|stay>', 0, 0, '', '', '', '', '', '', '', ''),
    (2019, 'tame_create_pet_failed', '创建宠物失败。', 0, 0, '', '', '', '', '', '', '', ''),
    (2020, 'tame_creature_template_not_found', '未找到生物模板。', 0, 0, '', '', '', '', '', '', '', ''),
    (2021, 'tame_exotic_requires_beast_mastery', '除非拥有野兽掌控天赋，否则我无法使用异种宠物。', 0, 0, '', '', '', '', '', '', '', ''),
    (2022, 'tame_invalid_id_error', '无效的驯服 ID。', 0, 0, '', '', '', '', '', '', '', ''),
    (2023, 'tame_no_hunter_pet_to_abandon', '你没有可放弃的猎人宠物。', 0, 0, '', '', '', '', '', '', '', ''),
    (2024, 'tame_no_pet_by_family', '未找到可驯服的宠物，家族: %family', 0, 0, '', '', '', '', '', '', '', ''),
    (2025, 'tame_no_pet_by_id', '未找到可驯服的宠物，ID: %id', 0, 0, '', '', '', '', '', '', '', ''),
    (2026, 'tame_no_pet_by_name', '未找到可驯服的宠物，名称: %name', 0, 0, '', '', '', '', '', '', '', ''),
    (2027, 'tame_no_pet_to_rename', '你没有可重命名的宠物。', 0, 0, '', '', '', '', '', '', '', ''),
    (2028, 'tame_pet_abandoned', '你的宠物已被放弃。', 0, 0, '', '', '', '', '', '', '', ''),
    (2029, 'tame_pet_changed', '宠物已更换为 %name，ID: %id。', 0, 0, '', '', '', '', '', '', '', ''),
    (2030, 'tame_pet_changed_initialized', '宠物已更换并初始化！', 0, 0, '', '', '', '', '', '', '', ''),
    (2031, 'tame_pet_name_alpha_error', '宠物名称只能包含字母（A-Z, a-z）。', 0, 0, '', '', '', '', '', '', '', ''),
    (2032, 'tame_pet_name_forbidden_error', '该宠物名称被禁止，请选择其他名称。', 0, 0, '', '', '', '', '', '', '', ''),
    (2033, 'tame_pet_name_length_error', '宠物名称必须为 1 到 12 个字母字符。', 0, 0, '', '', '', '', '', '', '', ''),
    (2034, 'tame_pet_rename_refresh_hint', '如果未看到新名称，请解散并重新召唤宠物。', 0, 0, '', '', '', '', '', '', '', ''),
    (2035, 'tame_pet_renamed', '你的宠物已重命名为 %name！', 0, 0, '', '', '', '', '', '', '', ''),
    (2036, 'tame_usage_error', '用法: tame name <名称> | tame id <id> | tame family <家族> | tame rename <新名称> | tame abandon', 0, 0, '', '', '', '', '', '', '', '');
