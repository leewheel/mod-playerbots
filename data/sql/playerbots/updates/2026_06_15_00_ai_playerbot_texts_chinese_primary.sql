-- Promote Chinese (text_loc4) to primary text column for all bot texts
UPDATE `ai_playerbot_texts` SET `text` = `text_loc4` WHERE `text_loc4` IS NOT NULL AND `text_loc4` <> '';
