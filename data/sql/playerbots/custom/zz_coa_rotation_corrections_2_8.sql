-- CoA Bots 2.8: a player's report from Road to De Other Side (24/10), on top of zz_coa_rotation_corrections_2_7.sql.
-- Default rotations only (owner 0). Rows are chosen by their CONTENT (name + owner 0 + action_line), never by id or idx.
-- Re-runnable: once applied it changes nothing. Rows with owner <> 0 are never touched.
-- Rotations are cached per process (CustomStrategy::actionLinesCache): the change shows after a restart.

-- 1. Tinker Invention: Mana Module is a 30-min self buff. On "low mana" it was cast again while up, as soon as its 38 mana
--    came back, and outranked Repair Shot (119 mana): at Muzah 10 casts in 39 s and no heal, 119 Repair Shot FAILED.
--    'can cast::Mana Module' keeps the buff up and skips it while it is on the bot.
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'tinker-invention'
   AND `action_line` IN ('low mana>cast::Mana Module!86', 'low mana>cast::Mana Module!87');
