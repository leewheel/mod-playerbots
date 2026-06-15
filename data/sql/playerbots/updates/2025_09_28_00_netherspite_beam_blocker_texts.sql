DELETE FROM ai_playerbot_texts WHERE name IN (
  'netherspite_beam_blocking_red',
  'netherspite_beam_blocking_blue',
  'netherspite_beam_blocking_green',
  'netherspite_beam_leaving_blue',
  'netherspite_beam_leaving_green'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
  'netherspite_beam_blocking_red',
  'netherspite_beam_blocking_blue',
  'netherspite_beam_blocking_green',
  'netherspite_beam_leaving_blue',
  'netherspite_beam_leaving_green'
);

INSERT INTO ai_playerbot_texts (name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8) VALUES
  ('netherspite_beam_blocking_red',   '%player 正在移动去挡红线！',   0, 0, '', '', '', '%player 正在移动去挡红线！', '', '', '', ''),
  ('netherspite_beam_blocking_blue',  '%player 正在移动去挡蓝线！',  0, 0, '', '', '', '%player 正在移动去挡蓝线！', '', '', '', ''),
  ('netherspite_beam_blocking_green', '%player 正在移动去挡绿线！', 0, 0, '', '', '', '%player 正在移动去挡绿线！', '', '', '', ''),
  ('netherspite_beam_leaving_blue',   '%player 离开蓝线——下一个来挡！', 0, 0, '', '', '', '%player 离开蓝线——下一个来挡！', '', '', '', ''),
  ('netherspite_beam_leaving_green',  '%player 离开绿线——下一个来挡！', 0, 0, '', '', '', '%player 离开绿线——下一个来挡！', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
  ('netherspite_beam_blocking_red',   100),
  ('netherspite_beam_blocking_blue',  100),
  ('netherspite_beam_blocking_green', 100),
  ('netherspite_beam_leaving_blue',   100),
  ('netherspite_beam_leaving_green',  100);