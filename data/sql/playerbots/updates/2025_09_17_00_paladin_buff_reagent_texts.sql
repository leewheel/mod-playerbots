DELETE FROM ai_playerbot_texts
WHERE name IN (
  'rp_missing_reagent_greater_blessing',
  'rp_missing_reagent_gift_of_the_wild',
  'rp_missing_reagent_arcane_brilliance',
  'rp_missing_reagent_generic'
);

DELETE FROM ai_playerbot_texts_chance
WHERE name IN (
  'rp_missing_reagent_greater_blessing',
  'rp_missing_reagent_gift_of_the_wild',
  'rp_missing_reagent_arcane_brilliance',
  'rp_missing_reagent_generic'
);

INSERT INTO ai_playerbot_texts (name, text, say_type, reply_type, text_loc1, text_loc2, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`) VALUES
  ('rp_missing_reagent_greater_blessing',
    '圣光啊……忘了带王者印记，先用 %base_spell 凑合吧！', 0, 0,
    '', 'Par la Lumière... J''ai oublié mes Symboles du roi. On se contentera de %base_spell !', '', '圣光啊……忘了带王者印记，先用 %base_spell 凑合吧！', '', '', '', ''),
  ('rp_missing_reagent_gift_of_the_wild',
    '大自然很慷慨，我的包可不行……%group_spell 的草药没了，先给你们 %base_spell！', 0, 0,
    '', 'La nature est généreuse, pas mes sacs... plus d''herbes pour %group_spell. Prenez %base_spell pour l''instant !', '', '大自然很慷慨，我的包可不行……%group_spell 的草药没了，先给你们 %base_spell！', '', '', '', ''),
  ('rp_missing_reagent_arcane_brilliance',
    '奥术粉尘用完了……%group_spell 得等等，先施放 %base_spell！', 0, 0,
    '', 'Plus de poudre des arcanes... %group_spell attendra. Je lance %base_spell !', '', '奥术粉尘用完了……%group_spell 得等等，先施放 %base_spell！', '', '', '', ''),
  ('rp_missing_reagent_generic',
    '哎呀，%group_spell 的材料用完了，改用 %base_spell 吧！', 0, 0,
    '', 'Oups, je n''ai plus de composants pour %group_spell. On fera avec %base_spell !', '', '哎呀，%group_spell 的材料用完了，改用 %base_spell 吧！', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
  ('rp_missing_reagent_greater_blessing', 100),
  ('rp_missing_reagent_gift_of_the_wild', 100),
  ('rp_missing_reagent_arcane_brilliance', 100),
  ('rp_missing_reagent_generic', 100);
