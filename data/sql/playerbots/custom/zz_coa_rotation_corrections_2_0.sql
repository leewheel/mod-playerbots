-- CoA Bots 2.0: default rotations (owner 0) of three CoA damage specs that dealt the least on the level 60
-- boss bench (09-10/10), on top of zz_coa_rotation_corrections_1_9.sql. Applied once by Installer-Bots
-- (coa_bots_installed). Re-runnable: the same result every time. Rows with owner <> 0 and the 'say'
-- strategy are never touched. A wrong result (count, checksum, apostrophes) aborts the transaction.
-- Rows: 115 in 3 specs. Checksum SUM(CRC32(CONCAT_WS('|',name,idx,owner,action_line))) = 246736494758.
-- Rows per spec: primalist-grovekeeper 39, stormbringer-wind 37, tinker-mechanics 39
-- Why: stormbringer-wind never summoned its Air Elemental (Flurry commands it) and put Cloudburst, a
-- knockback, first; primalist-grovekeeper fights in melee from 2.0 (Wildclaw on the melee action, Primal
-- Rush no longer leaps it out); tinker-mechanics kept Makeshift Dynamite, its Scrap for the Mechsuit, and
-- Mechsuit: Combustion under fillers that are always castable.

DROP PROCEDURE IF EXISTS coa20_check;
DELIMITER $$
CREATE PROCEDURE coa20_check()
BEGIN
  DECLARE n BIGINT DEFAULT 0;
  DECLARE h BIGINT DEFAULT 0;
  DECLARE a BIGINT DEFAULT 0;
  SELECT COUNT(*), COALESCE(SUM(CRC32(CONCAT_WS('|', name, idx, owner, action_line))), 0)
    INTO n, h FROM playerbots_custom_strategy
   WHERE owner = 0 AND name IN ('primalist-grovekeeper', 'stormbringer-wind', 'tinker-mechanics');
  SELECT COUNT(*) INTO a FROM playerbots_custom_strategy WHERE owner = 0 AND name IN ('primalist-grovekeeper', 'stormbringer-wind', 'tinker-mechanics')
     AND action_line LIKE '%''%';
  IF n <> 115 THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'CoA Bots 2.0 rotations: wrong row count, nothing was changed';
  END IF;
  IF a <> 6 THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'CoA Bots 2.0 rotations: apostrophes lost (character set), nothing was changed';
  END IF;
  IF h <> 246736494758 THEN
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'CoA Bots 2.0 rotations: wrong checksum, nothing was changed';
  END IF;
END$$
DELIMITER ;

-- A copy of the default rotations as they were before the first run of this file.
CREATE TABLE IF NOT EXISTS playerbots_custom_strategy_bak_v20 AS
SELECT * FROM playerbots_custom_strategy WHERE owner = 0;

START TRANSACTION;

DELETE FROM playerbots_custom_strategy WHERE owner = 0 AND name IN ('primalist-grovekeeper', 'stormbringer-wind', 'tinker-mechanics');

INSERT INTO playerbots_custom_strategy (name, idx, owner, action_line) VALUES
 ('primalist-grovekeeper', 1, 0, 'medium aoe>cast::Spirit Charge!88'),
 ('primalist-grovekeeper', 2, 0, 'medium aoe>cast::Seismic Wave!87'),
 ('primalist-grovekeeper', 3, 0, 'medium aoe>cast melee::Wildclaw!86'),
 ('primalist-grovekeeper', 4, 0, 'debuff missing::Seismic Crash>cast debuff::Seismic Crash!85'),
 ('primalist-grovekeeper', 5, 0, 'buff missing::Boon of the Bear>cast buff::Boon of the Bear!84'),
 ('primalist-grovekeeper', 6, 0, 'debuff missing::Earthquake>cast debuff::Earthquake!83'),
 ('primalist-grovekeeper', 7, 0, 'medium health>cast heal::Hand of the Earthmother!82'),
 ('primalist-grovekeeper', 8, 0, 'can cast::Therazane''s Rage>cast::Therazane''s Rage!79'),
 ('primalist-grovekeeper', 9, 0, 'can cast::Spirit Charge>cast::Spirit Charge!78'),
 ('primalist-grovekeeper', 10, 0, 'can cast::Primal Rush>cast::Primal Rush!30'),
 ('primalist-grovekeeper', 11, 0, 'can cast::Seismic Spike>cast::Seismic Spike!76'),
 ('primalist-grovekeeper', 12, 0, 'can cast::Seismic Wave>cast::Seismic Wave!80'),
 ('primalist-grovekeeper', 13, 0, 'can cast::Primal Instinct>cast::Primal Instinct!75'),
 ('primalist-grovekeeper', 14, 0, 'can cast::Geode Barrage>cast::Geode Barrage!74'),
 ('primalist-grovekeeper', 15, 0, 'can cast::Wildclaw>cast melee::Wildclaw!81'),
 ('primalist-grovekeeper', 16, 0, 'buff missing::Boulder Dash>cast buff::Boulder Dash!30'),
 ('primalist-grovekeeper', 17, 0, 'can cast::Grove Guardian>cast::Grove Guardian!77'),
 ('primalist-grovekeeper', 18, 0, 'buff missing::Greater Earthen Endurance>cast buff party::Greater Earthen Endurance!28'),
 ('primalist-grovekeeper', 19, 0, 'buff missing::Greater Grove Instinct>cast buff party::Greater Grove Instinct!27'),
 ('primalist-grovekeeper', 20, 0, 'buff missing::Greater Essence of Nature>cast buff party::Greater Essence of Nature!26'),
 ('primalist-grovekeeper', 21, 0, 'buff missing::Greater Primal Instinct>cast buff party::Greater Primal Instinct!25'),
 ('primalist-grovekeeper', 22, 0, 'can cast::Sacred Grove>cast::Sacred Grove!24'),
 ('primalist-grovekeeper', 23, 0, 'buff missing::Primal Convergence>cast buff::Primal Convergence!23'),
 ('primalist-grovekeeper', 24, 0, 'can cast::Protective Roar>cast::Protective Roar!22'),
 ('primalist-grovekeeper', 25, 0, 'can cast::Ancient of Lore>cast::Ancient of Lore!21'),
 ('primalist-grovekeeper', 26, 0, 'buff missing::Flourishing Growth>cast buff party::Flourishing Growth!20'),
 ('primalist-grovekeeper', 27, 0, 'can cast::Neptulon''s Wrath>cast::Neptulon''s Wrath!19'),
 ('primalist-grovekeeper', 28, 0, 'can cast::Primal Awakening>cast::Primal Awakening!18'),
 ('primalist-grovekeeper', 29, 0, 'can cast::Essence of Nature>cast::Essence of Nature!17'),
 ('primalist-grovekeeper', 30, 0, 'can cast::Earthen Endurance>cast::Earthen Endurance!16'),
 ('primalist-grovekeeper', 31, 0, 'can cast::Soothing Touch>cast::Soothing Touch!15'),
 ('primalist-grovekeeper', 32, 0, 'can cast::Ursoc''s Bellow>cast::Ursoc''s Bellow!14'),
 ('primalist-grovekeeper', 33, 0, 'can cast::Nature''s Call>cast::Nature''s Call!13'),
 ('primalist-grovekeeper', 35, 0, 'can cast::Return to Life>cast::Return to Life!11'),
 ('primalist-grovekeeper', 36, 0, 'can cast::Stoneshard>cast::Stoneshard!8'),
 ('primalist-grovekeeper', 37, 0, 'can cast::Grove Instinct>cast::Grove Instinct!7'),
 ('primalist-grovekeeper', 38, 0, 'can cast::Throw>cast::Throw!6'),
 ('primalist-grovekeeper', 39, 0, 'can cast::Shoot>cast::Shoot!2'),
 ('primalist-grovekeeper', 40, 0, 'can cast::Wand>cast::Wand!1'),
 ('stormbringer-wind', 1, 0, 'coa summon missing::500941>cast::Summon: Air Elemental!88'),
 ('stormbringer-wind', 2, 0, 'can cast::Charge>cast::Charge!87'),
 ('stormbringer-wind', 3, 0, 'can cast::Flurry>cast::Flurry!83'),
 ('stormbringer-wind', 4, 0, 'can cast::Body of Lightning>cast::Body of Lightning!30'),
 ('stormbringer-wind', 5, 0, 'can cast::Windsurf>cast::Windsurf!29'),
 ('stormbringer-wind', 6, 0, 'can cast::Surge of Might>cast::Surge of Might!28'),
 ('stormbringer-wind', 7, 0, 'buff missing::Lexicon of Servitude>cast buff::Lexicon of Servitude!27'),
 ('stormbringer-wind', 8, 0, 'can cast::Tempest''s Call>cast::Tempest''s Call!26'),
 ('stormbringer-wind', 9, 0, 'buff missing::Greater Call of the Lightning>cast buff party::Greater Call of the Lightning!25'),
 ('stormbringer-wind', 10, 0, 'buff missing::Greater Call of the Wind>cast buff party::Greater Call of the Wind!24'),
 ('stormbringer-wind', 11, 0, 'buff missing::Storm Ascendance>cast buff::Storm Ascendance!23'),
 ('stormbringer-wind', 12, 0, 'can cast::Invigorating Winds>cast::Invigorating Winds!22'),
 ('stormbringer-wind', 13, 0, 'can cast::Shock>cast::Shock!40'),
 ('stormbringer-wind', 14, 0, 'can cast::Tailwind>cast::Tailwind!20'),
 ('stormbringer-wind', 15, 0, 'can cast::Gale>cast::Gale!84'),
 ('stormbringer-wind', 16, 0, 'can cast::Skyfall>cast::Skyfall!18'),
 ('stormbringer-wind', 17, 0, 'buff missing::Stormcloak>cast buff::Stormcloak!17'),
 ('stormbringer-wind', 18, 0, 'buff missing::Aerodynamics>cast buff party::Aerodynamics!16'),
 ('stormbringer-wind', 19, 0, 'buff missing::Greater Call of the Storm>cast buff party::Greater Call of the Storm!15'),
 ('stormbringer-wind', 20, 0, 'can cast::Veering Winds>cast::Veering Winds!14'),
 ('stormbringer-wind', 21, 0, 'buff missing::Tempest Aegis>cast buff::Tempest Aegis!13'),
 ('stormbringer-wind', 22, 0, 'can cast::Conjure Thunderkeg>cast::Conjure Thunderkeg!12'),
 ('stormbringer-wind', 23, 0, 'can cast::Calm the Storm>cast::Calm the Storm!11'),
 ('stormbringer-wind', 24, 0, 'can cast::Call of the Lightning>cast::Call of the Lightning!10'),
 ('stormbringer-wind', 25, 0, 'can cast::Razorwind>cast::Razorwind!9'),
 ('stormbringer-wind', 26, 0, 'can cast::Stabilizing Pressure>cast::Stabilizing Pressure!8'),
 ('stormbringer-wind', 27, 0, 'can cast::Windride>cast::Windride!7'),
 ('stormbringer-wind', 28, 0, 'buff missing::Gale Guard>cast buff::Gale Guard!6'),
 ('stormbringer-wind', 29, 0, 'buff missing::Shocking Aegis>cast buff::Shocking Aegis!5'),
 ('stormbringer-wind', 30, 0, 'can cast::Atmospheric Pressure>cast::Atmospheric Pressure!4'),
 ('stormbringer-wind', 31, 0, 'can cast::Stormbreaker>cast::Stormbreaker!3'),
 ('stormbringer-wind', 32, 0, 'can cast::Aether Current>cast::Aether Current!2'),
 ('stormbringer-wind', 33, 0, 'can cast::Call of the Wind>cast::Call of the Wind!1'),
 ('stormbringer-wind', 34, 0, 'can cast::Wand>cast::Wand!1'),
 ('stormbringer-wind', 101, 0, 'aura stacks::Static,50>cast::Aeroblast!86'),
 ('stormbringer-wind', 102, 0, 'aura stacks::Static,70>cast::Updraft!85'),
 ('stormbringer-wind', 103, 0, 'medium mana>cast::Discharge!89'),
 ('tinker-mechanics', 1, 0, 'medium aoe>cast::Sticky Bomb!88'),
 ('tinker-mechanics', 2, 0, 'medium aoe>cast::Scrap Shot!87'),
 ('tinker-mechanics', 3, 0, 'medium aoe>cast::Makeshift Dynamite!86'),
 ('tinker-mechanics', 4, 0, 'can cast::Sticky Bomb>cast::Sticky Bomb!83'),
 ('tinker-mechanics', 5, 0, 'can cast::Mechsuit: Laser Beam>cast::Mechsuit: Laser Beam!85'),
 ('tinker-mechanics', 6, 0, 'can cast::Build: Sentry Turret>cast::Build: Sentry Turret!82'),
 ('tinker-mechanics', 7, 0, 'can cast::Scrap Shot>cast::Scrap Shot!80'),
 ('tinker-mechanics', 8, 0, 'can cast::Makeshift Dynamite>cast::Makeshift Dynamite!82'),
 ('tinker-mechanics', 9, 0, 'can cast::Build: Scrapmaw>cast::Build: Scrapmaw!27'),
 ('tinker-mechanics', 10, 0, 'buff missing::Greater Power Module>cast buff party::Greater Power Module!29'),
 ('tinker-mechanics', 11, 0, 'can cast::Macro-Gravity Zone>cast::Macro-Gravity Zone!26'),
 ('tinker-mechanics', 12, 0, 'can cast::Upgrade!>cast::Upgrade!!82'),
 ('tinker-mechanics', 13, 0, 'can cast::Mechsuit: Combustion>cast::Mechsuit: Combustion!79'),
 ('tinker-mechanics', 14, 0, 'can cast::Mechsuit: Activate Jets>cast::Mechsuit: Activate Jets!23'),
 ('tinker-mechanics', 15, 0, 'can cast::Air Strike>cast::Air Strike!89'),
 ('tinker-mechanics', 16, 0, 'can cast::Build: Destructo-Bot>cast::Build: Destructo-Bot!22'),
 ('tinker-mechanics', 17, 0, 'can cast::Overclocked Machine>cast::Overclocked Machine!81'),
 ('tinker-mechanics', 18, 0, 'can cast::Greater Mana Module>cast::Greater Mana Module!21'),
 ('tinker-mechanics', 19, 0, 'can cast::Strength Bionics>cast::Strength Bionics!20'),
 ('tinker-mechanics', 20, 0, 'can cast::Warning Shot>cast::Warning Shot!19'),
 ('tinker-mechanics', 21, 0, 'can cast::Arcane Bionics>cast::Arcane Bionics!18'),
 ('tinker-mechanics', 22, 0, 'buff missing::Basic Intuition>cast buff::Basic Intuition!17'),
 ('tinker-mechanics', 23, 0, 'can cast::Build: Battle Turret X-13>cast::Build: Battle Turret X-13!16'),
 ('tinker-mechanics', 24, 0, 'can cast::Build: Rusthound>cast::Build: Rusthound!15'),
 ('tinker-mechanics', 25, 0, 'can cast::Freeze Ray>cast::Freeze Ray!14'),
 ('tinker-mechanics', 26, 0, 'can cast::Build: Replenishment Beacon>cast::Build: Replenishment Beacon!13'),
 ('tinker-mechanics', 27, 0, 'debuff missing::Control Mechanical>cast debuff::Control Mechanical!12'),
 ('tinker-mechanics', 28, 0, 'can cast::Aether Augmentation>cast::Aether Augmentation!11'),
 ('tinker-mechanics', 29, 0, 'can cast::Explosive Augmentation>cast::Explosive Augmentation!31'),
 ('tinker-mechanics', 30, 0, 'can cast::Nanobot Recharger>cast::Nanobot Recharger!9'),
 ('tinker-mechanics', 31, 0, 'can cast::Piercing Augmentation>cast::Piercing Augmentation!9'),
 ('tinker-mechanics', 32, 0, 'can cast::Mana Module>cast::Mana Module!7'),
 ('tinker-mechanics', 34, 0, 'can cast::Build: Mechano-Bear>cast::Build: Mechano-Bear!5'),
 ('tinker-mechanics', 35, 0, 'can cast::''Pick Lock''>cast::''Pick Lock''!4'),
 ('tinker-mechanics', 36, 0, 'can cast::Build: Portable Sawmill>cast::Build: Portable Sawmill!3'),
 ('tinker-mechanics', 37, 0, 'can cast::Defibrillate>cast::Defibrillate!2'),
 ('tinker-mechanics', 38, 0, 'can cast::Build: Clockwork Assistant>cast::Build: Clockwork Assistant!1'),
 ('tinker-mechanics', 39, 0, 'can cast::Auto Shot>cast::Auto Shot!3'),
 ('tinker-mechanics', 40, 0, 'can cast::Build: Mechsuit>cast::Build: Mechsuit!84');

CALL coa20_check();
COMMIT;
DROP PROCEDURE coa20_check;
