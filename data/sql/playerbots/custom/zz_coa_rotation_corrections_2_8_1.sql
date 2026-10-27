-- CoA Bots 2.8.1: player reports on the Bloodmage Eternal tank (#38, #51), on top of zz_coa_rotation_corrections_2_8.sql.
-- Default rotations only (owner 0). Rows are chosen by their CONTENT (name + owner 0 + action_line), never by id or idx.
-- Re-runnable: once applied it changes nothing. Rows with owner <> 0 are never touched.
-- Rotations are cached per process (CustomStrategy::actionLinesCache): the change shows after a restart.

DROP PROCEDURE IF EXISTS coa281_add;
DELIMITER $$
CREATE PROCEDURE coa281_add(IN p_name VARCHAR(255), IN p_line TEXT)
BEGIN
  IF EXISTS (SELECT 1 FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name)
     AND NOT EXISTS (SELECT 1 FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_line) THEN
    INSERT INTO playerbots_custom_strategy (name, idx, owner, action_line)
    SELECT p_name, MAX(idx) + 1, 0, p_line FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name;
  END IF;
END$$
DELIMITER ;

DROP PROCEDURE IF EXISTS coa281_set;
DELIMITER $$
CREATE PROCEDURE coa281_set(IN p_name VARCHAR(255), IN p_old TEXT, IN p_new TEXT)
BEGIN
  IF NOT EXISTS (SELECT 1 FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_new) THEN
    UPDATE playerbots_custom_strategy SET action_line = p_new WHERE owner = 0 AND name = p_name AND action_line = p_old;
  ELSE
    DELETE FROM playerbots_custom_strategy WHERE owner = 0 AND name = p_name AND action_line = p_old;
  END IF;
END$$
DELIMITER ;

-- 1. Bloodmage Eternal: no line took the cursed form, Eternal Curse (800157, shapeshift 57). A bot only fought in it when
--    it was already in it; "coa buff" leaves a tank without a threat stance to the stances its rotation asks for (#38).
--    The specialization aura is a passive namesake (92114): "buff missing" sets it aside and looks at the form itself.
CALL coa281_add('bloodmage-eternal', 'buff missing::Eternal Curse>cast buff::Eternal Curse!99');

-- 2. Bloodmage Eternal: Hypovolemic Shock is a debuff lowering the target's melee damage for a while. As "can cast" it went
--    out every time it was ready, again and again on the same target (#51). It is kept up on the target instead.
CALL coa281_set('bloodmage-eternal', 'can cast::Hypovolemic Shock>cast melee::Hypovolemic Shock!13',
                'debuff missing::Hypovolemic Shock>cast debuff::Hypovolemic Shock!13');

-- 3. Bloodmage Eternal: Sanguine Rupture is the mortal form's area spell. Out of form it took the place of Rotclaw and Claw
--    Sweep on packs (#51); in form the tank has those two. The tank no longer casts it.
DELETE FROM `playerbots_custom_strategy`
 WHERE `owner` = 0 AND `name` = 'bloodmage-eternal'
   AND `action_line` = 'can cast::Sanguine Rupture>cast melee::Sanguine Rupture!8';

DROP PROCEDURE IF EXISTS coa281_add;
DROP PROCEDURE IF EXISTS coa281_set;
