-- CoA Bots 2.7: a player's raid reports (Karazhan, Magtheridon, Gruul's Lair, 18-21/10), on top of
-- zz_coa_rotation_corrections_2_5.sql. Default rotations only (owner 0). Rows are chosen by their CONTENT (name + owner 0 +
-- action_line), never by id or idx. Re-runnable: once applied it changes nothing. Rows with owner <> 0 are never touched.
-- Rotations are cached per process (CustomStrategy::actionLinesCache): the change shows after a restart.

DROP PROCEDURE IF EXISTS coa27_set;
DELIMITER $$
CREATE PROCEDURE coa27_set(IN p_name VARCHAR(255), IN p_old TEXT, IN p_new TEXT)
BEGIN
  IF NOT EXISTS (SELECT 1 FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_new) THEN
    UPDATE playerbots_custom_strategy SET action_line = p_new WHERE owner = 0 AND name = p_name AND action_line = p_old;
  ELSE
    DELETE FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_old;
  END IF;
END$$
DELIMITER ;

-- 1. Witch Hunter: Vault is a scripted leap of about 16 yd forward. As a "buff missing" line it came back every 12 s: a
--    Black Knight main tank leapt through Maulgar, pulled two Gronn-Priests on trash and once left four of five healers
--    41 yd behind. A bot has no use for it.
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` IN ('witch-hunter-black-knight', 'witch-hunter-boltslinger')
   AND `action_line` IN ('buff missing::Vault>cast buff::Vault!30', 'buff missing::Vault>cast buff::Vault!85');

-- 2. Witch Doctor: Spiritual Recall destroys every totem and ward of the caster. The Brewing healer put its Healing Ward
--    down, recalled it right away and put it down again, 387 times at Gruul: the ward almost never healed.
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` IN ('witch-doctor-brewing', 'witch-doctor-shadowhunting', 'witch-doctor-voodoo')
   AND `action_line` IN ('can cast::Spiritual Recall>cast::Spiritual Recall!11',
                         'can cast::Spiritual Recall>cast::Spiritual Recall!14',
                         'can cast::Spiritual Recall>cast::Spiritual Recall!13');

-- 3. Pyromancer Flameweaving (healer). Supernova is a 5 s channel that only hurts at its end: 27 casts for 1 hit, the
--    healer's mana and time gone. Its spells for itself now fire (they were tested on the enemy, CoA Bots 2.7): Essence
--    of Malygos when mana runs low, as for Incineration, not on cooldown; Cauterize goes.
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'pyromancer-flameweaving'
   AND `action_line` IN ('can cast::Supernova>cast::Supernova!25', 'can cast::Cauterize>cast::Cauterize!12');
CALL coa27_set('pyromancer-flameweaving', 'can cast::Essence of Malygos>cast::Essence of Malygos!26',
               'low mana>cast::Essence of Malygos!89');

DROP PROCEDURE IF EXISTS coa27_set;
