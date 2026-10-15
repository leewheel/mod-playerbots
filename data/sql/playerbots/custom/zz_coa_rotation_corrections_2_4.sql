-- CoA Bots 2.4: dungeon runs without a player (Maraudon, Scholomance, Lower Blackrock Spire, 14-15/10), on top of
-- zz_coa_rotation_corrections_2_3.sql. Default rotations only (owner 0). Rows are chosen by their CONTENT (name + owner 0 +
-- action_line), never by id or idx. Re-runnable: once applied it changes nothing. Rows with owner <> 0 are never
-- touched. Rotations are cached per process (CustomStrategy::actionLinesCache): the change shows after a restart.

DROP PROCEDURE IF EXISTS coa24_add;
DELIMITER $$
CREATE PROCEDURE coa24_add(IN p_name VARCHAR(255), IN p_line TEXT)
BEGIN
  IF EXISTS (SELECT 1 FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name)
     AND NOT EXISTS (SELECT 1 FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_line) THEN
    INSERT INTO playerbots_custom_strategy (name, idx, owner, action_line)
    SELECT p_name, MAX(idx) + 1, 0, p_line FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name;
  END IF;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS coa24_set;
DELIMITER $$
CREATE PROCEDURE coa24_set(IN p_name VARCHAR(255), IN p_old TEXT, IN p_new TEXT)
BEGIN
  IF NOT EXISTS (SELECT 1 FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_new) THEN
    UPDATE playerbots_custom_strategy SET action_line = p_new WHERE owner = 0 AND name = p_name AND action_line = p_old;
  ELSE
    DELETE FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_old;
  END IF;
END$$
DELIMITER ;

-- 1. Tanks. The 30 yd taunts were on melee rows, so a caster hitting the healer from range was never taunted
--    (Shield Challenge and Writhe: over 1,000 refusals each, "enemy out of melee range"). Unyielding Stand, a 10 yd
--    area taunt, takes the pack instead of waiting for the current target to turn away.
CALL coa24_set('guardian-vanguard', 'lose aggro>cast melee::Shield Challenge!94', 'lose aggro>cast::Shield Challenge!94');
CALL coa24_set('guardian-vanguard', 'lose aggro>cast melee::Unyielding Stand!93', 'medium aoe>cast::Unyielding Stand!93');
CALL coa24_set('reaper-domination', 'lose aggro>cast melee::Writhe!94', 'lose aggro>cast::Writhe!94');

-- 2. Guardian: Reclaim Standards took the standard back one tick after it was placed (about 270 times a run), so
--    Spellwarding never stayed and a fifth of the energy went into standards.
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` IN ('guardian-vanguard', 'guardian-gladiator')
   AND `action_line` = 'can cast::Reclaim Standards>cast melee::Reclaim Standards!1';

-- 3. Reaper: Reap, free and without cooldown, sat above every spender, so Soul Strike (the self-heal) and Requiem
--    were never cast. Spenders first; without the power they fall through to Reap in the same tick.
CALL coa24_set('reaper-domination', 'medium aoe>cast melee::Reap!87', 'medium aoe>cast melee::Reap!83');
CALL coa24_set('reaper-domination', 'can cast::Reap>cast melee::Reap!82', 'can cast::Reap>cast melee::Reap!78');
CALL coa24_set('reaper-harvest', 'can cast::Soul Strike>cast melee::Soul Strike!8', 'can cast::Soul Strike>cast melee::Soul Strike!78');
CALL coa24_set('reaper-soul', 'can cast::Soul Strike>cast melee::Soul Strike!12', 'can cast::Soul Strike>cast melee::Soul Strike!76');
-- the single-target Rites aim at the enemy; the party Rites stay
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` IN ('reaper-domination', 'reaper-harvest', 'reaper-soul')
   AND `action_line` LIKE 'can cast::Rite of %>cast melee::Rite of %';

-- 4. Ranger: Survival Potion went at the pull at full health and was on cooldown at 5 of 6 deaths.
CALL coa24_set('ranger-archery', 'can cast::Survival Potion>cast::Survival Potion!21', 'low health>cast::Survival Potion!29');
CALL coa24_set('ranger-brigand', 'can cast::Survival Potion>cast melee::Survival Potion!23', 'low health>cast::Survival Potion!29');
CALL coa24_set('ranger-farstrider', 'can cast::Survival Potion>cast::Survival Potion!83', 'low health>cast::Survival Potion!29');
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'ranger-farstrider' AND `action_line` = 'medium aoe>cast::Survival Potion!87';
-- stealth-only spells: the Ranger has no stealth in a fight
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'ranger-archery'
   AND `action_line` IN ('debuff missing::Toxic Dart>cast debuff::Toxic Dart!7', 'can cast::Rusty Shiv>cast::Rusty Shiv!16',
                         'can cast::Sticky Fingers>cast::Sticky Fingers!10', 'can cast::Flank>cast::Flank!8');

-- 5. Sun Cleric: Bless and Daybreak aimed at the enemy (0 casts); Vow of Grace too (Vow of the Eclipse stays);
--    Radiance pulses around the caster, not the tank. Daybreak's instant charges come before a 3 s Illumination.
CALL coa24_set('sun-cleric-blessings', 'party member low health>cast::Bless!85', 'party member low health>cast heal party::Bless!85');
CALL coa24_set('sun-cleric-blessings', 'can cast::Daybreak>cast::Daybreak!29', 'party member critical health>cast heal party::Daybreak!93');
CALL coa24_add('sun-cleric-blessings', 'party member low health>cast heal party::Daybreak!86');
CALL coa24_add('sun-cleric-blessings', 'medium aoe heal>cast::Solar Invocation: Ascension!86');
CALL coa24_set('sun-cleric-blessings', 'can cast::Gavel of Light>cast::Gavel of Light!23', 'can cast::Gavel of Light>cast melee::Gavel of Light!23');
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'sun-cleric-blessings'
   AND `action_line` IN ('party member low health>cast::Vow of Grace!82', 'medium aoe heal>cast heal tank::Radiance!24',
                         'buff missing::Blazing Chariot>cast buff::Blazing Chariot!30');

-- 6. Starcaller Sentinel: the mana-return shot comes before the bot runs dry, not after.
CALL coa24_set('starcaller-sentinel', 'low mana>cast::Moon Arrow!60', 'medium mana>cast::Moon Arrow!73');

-- 7. Witch Hunter: Infiltrate is refused in any instance and was retried every tick (2,331 times); Stoicism, a
--    3 min defensive, went off with nothing on the bot (coa defensive already uses it).
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` LIKE 'witch-hunter-%' AND `action_line` LIKE 'can cast::Infiltrate>cast::Infiltrate!%';
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'witch-hunter-houndmaster' AND `action_line` = 'buff missing::Stoicism>cast buff::Stoicism!11';

-- 8. Lines that could never win.
CALL coa24_set('chronomancer-infinite', 'medium aoe>cast::Discordance!60', 'medium aoe>cast::Discordance!85');
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'chronomancer-infinite'
   AND `action_line` IN ('buff missing::Incarnation of Chaos>cast buff::Incarnation of Chaos!87',
                         'can cast::Wand of Time>cast::Wand of Time!80');
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'runemaster-riftblade'
   AND `action_line` = 'buff missing::Palm Sigil: Fire>cast buff::Palm Sigil: Fire!77';
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'templar-crusader' AND `action_line` = 'can cast::Chastise>cast melee::Chastise!9';
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'barbarian-ancestry'
   AND `action_line` = 'can cast::Whirling Advance>cast melee::Whirling Advance!5';

DROP PROCEDURE IF EXISTS coa24_add;
DROP PROCEDURE IF EXISTS coa24_set;
