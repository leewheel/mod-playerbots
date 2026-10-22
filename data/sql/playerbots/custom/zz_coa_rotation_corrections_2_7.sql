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

-- 4. Rows that fire but never land (ladiabale, raid logs of 21/10: Black Temple and two Gruul runs, 41 bots, 32 specs).
--    Safe to delete (51): rival buff families (only one of a family holds on a target: Marks of Zeliek, Greater Rites,
--    Devotions, Greater Call of the Storm, Edicts, Adaptations, Wisdoms, Temporal Swiftness, Aegis), utility spells in
--    combat (Detect Life, Melt Lock, Pick Lock, Conjure Thunder Ale, Brew Stew, Control Mechanical, Charming Conversation,
--    extra pet builds), rows duplicated by run-time swaps (Unmake, Murder, Flames of Xoroth), Dominion on damage specs,
--    melee spells on ranged specs.
DELETE FROM `playerbots_custom_strategy` WHERE `owner` = 0 AND (`name`, `action_line`) IN (
  ('bloodmage-accursed', 'buff missing::Detect Life>cast buff::Detect Life!13'),
  ('bloodmage-sanguine', 'can cast::Scarlet Delirium>cast::Scarlet Delirium!22'),
  ('chronomancer-artificer', 'can cast::Desynchronization>cast::Desynchronization!29'),
  ('chronomancer-artificer', 'buff missing::Temporal Swiftness>cast buff::Temporal Swiftness!28'),
  ('chronomancer-artificer', 'debuff missing::Unmake>cast debuff::Unmake!4'),
  ('chronomancer-time', 'buff missing::Temporal Swiftness>cast buff::Temporal Swiftness!27'),
  ('chronomancer-time', 'buff missing::Greater Chromie''s Wisdom>cast buff party::Greater Chromie''s Wisdom!19'),
  ('knight-of-xoroth-hellfire', 'buff missing::Greater Mark of Zeliek>cast buff party::Greater Mark of Zeliek!17'),
  ('knight-of-xoroth-hellfire', 'buff missing::Mark of Zeliek>cast buff::Mark of Zeliek!16'),
  ('knight-of-xoroth-war', 'buff missing::Greater Mark of Zeliek>cast buff party::Greater Mark of Zeliek!20'),
  ('knight-of-xoroth-war', 'buff missing::Mark of Zeliek>cast buff::Mark of Zeliek!19'),
  ('knight-of-xoroth-war', 'can cast::Flames of Xoroth>cast melee::Flames of Xoroth!3'),
  ('pyromancer-draconic', 'can cast::Melt Lock>cast::Melt Lock!11'),
  ('ranger-archery', 'buff missing::Greater Woodsman''s Adaptation>cast buff party::Greater Woodsman''s Adaptation!25'),
  ('ranger-archery', 'can cast::Wild Strike>cast::Wild Strike!3'),
  ('reaper-domination', 'buff missing::Greater Rite of Perseverance>cast buff party::Greater Rite of Perseverance!26'),
  ('reaper-domination', 'buff missing::Greater Rite of Resolve>cast buff party::Greater Rite of Resolve!24'),
  ('reaper-harvest', 'can cast::Murder>cast melee::Murder!82'),
  ('reaper-harvest', 'buff missing::Greater Rite of Resolve>cast buff party::Greater Rite of Resolve!25'),
  ('reaper-harvest', 'buff missing::Dominion>cast buff::Dominion!10'),
  ('reaper-soul', 'buff missing::Greater Rite of Power>cast buff party::Greater Rite of Power!26'),
  ('reaper-soul', 'buff missing::Dominion>cast buff::Dominion!14'),
  ('stormbringer-lightning', 'buff missing::Greater Call of the Storm>cast buff party::Greater Call of the Storm!22'),
  ('stormbringer-lightning', 'buff missing::Shocking Aegis>cast buff::Shocking Aegis!12'),
  ('stormbringer-lightning', 'can cast::Conjure Thunder Ale>cast::Conjure Thunder Ale!5'),
  ('stormbringer-lightning', 'buff missing::Whirlwind Aegis>cast buff::Whirlwind Aegis!3'),
  ('stormbringer-maelstrom', 'buff missing::Greater Call of the Storm>cast buff party::Greater Call of the Storm!18'),
  ('stormbringer-wind', 'buff missing::Greater Call of the Storm>cast buff party::Greater Call of the Storm!15'),
  ('sun-cleric-blessings', 'buff missing::Devotion of Grace>cast buff::Devotion of Grace!2'),
  ('sun-cleric-piety', 'buff missing::Greater Devotion of Grace>cast buff party::Greater Devotion of Grace!26'),
  ('sun-cleric-piety', 'can cast::Glare>cast::Glare!22'),
  ('sun-cleric-piety', 'buff missing::Greater Devotion of Radiance>cast buff party::Greater Devotion of Radiance!21'),
  ('sun-cleric-piety', 'can cast::Gavel of Grace>cast melee::Gavel of Grace!9'),
  ('sun-cleric-piety', 'can cast::Gavel of Light>cast::Gavel of Light!5'),
  ('sun-cleric-seraphim', 'buff missing::Greater Devotion of Grace>cast buff party::Greater Devotion of Grace!25'),
  ('sun-cleric-seraphim', 'buff missing::Greater Devotion of Radiance>cast buff party::Greater Devotion of Radiance!19'),
  ('sun-cleric-seraphim', 'can cast::Gavel of Grace>cast melee::Gavel of Grace!7'),
  ('sun-cleric-seraphim', 'can cast::Gavel of Light>cast melee::Gavel of Light!3'),
  ('tinker-demolition', 'debuff missing::Control Mechanical>cast debuff::Control Mechanical!12'),
  ('tinker-demolition', 'can cast::Build: Clockwork Assistant>cast::Build: Clockwork Assistant!1'),
  ('tinker-invention', 'can cast::Build: Clockwork Assistant>cast::Build: Clockwork Assistant!5'),
  ('tinker-mechanics', 'debuff missing::Control Mechanical>cast debuff::Control Mechanical!12'),
  ('tinker-mechanics', 'can cast::Build: Mechano-Bear>cast::Build: Mechano-Bear!5'),
  ('tinker-mechanics', 'can cast::''Pick Lock''>cast::''Pick Lock''!4'),
  ('tinker-mechanics', 'can cast::Build: Clockwork Assistant>cast::Build: Clockwork Assistant!1'),
  ('venomancer-vizier', 'buff missing::Skulk>cast buff::Skulk!5'),
  ('witch-doctor-brewing', 'can cast::Concoct Rejuvenating Mojo>cast::Concoct Rejuvenating Mojo!4'),
  ('witch-doctor-brewing', 'can cast::Brew Stew>cast::Brew Stew!1'),
  ('witch-hunter-black-knight', 'buff missing::Greater Witching Edict>cast buff party::Greater Witching Edict!18'),
  ('witch-hunter-black-knight', 'can cast::Charming Conversation>cast melee::Charming Conversation!5'),
  ('witch-hunter-boltslinger', 'can cast::Pommel Smash>cast::Pommel Smash!8')
);

-- 5. Rows whose spell is useful but whose row could never cast it: 20-35 yd or minimum-range spells on `cast melee::`
--    (Flesh Hook 5-30 yd, Scythe Rush 8-25 yd), Banes as debuffs, enemy spells on buff or heal actions.
CALL coa27_set('bloodmage-accursed', 'can cast::Hypovolemic Shock>cast melee::Hypovolemic Shock!7', 'can cast::Hypovolemic Shock>cast::Hypovolemic Shock!7');
CALL coa27_set('felsworn-tyrant', 'can cast::Fel Fireball>cast melee::Fel Fireball!2', 'can cast::Fel Fireball>cast::Fel Fireball!2');
CALL coa27_set('runemaster-riftblade', 'can cast::Elemental Burst>cast melee::Elemental Burst!1', 'can cast::Elemental Burst>cast::Elemental Burst!1');
CALL coa27_set('starcaller-moon-guard', 'can cast::Moon Arrow>cast melee::Moon Arrow!7', 'can cast::Moon Arrow>cast::Moon Arrow!7');
CALL coa27_set('sun-cleric-seraphim', 'can cast::Chains of Light>cast melee::Chains of Light!9', 'can cast::Chains of Light>cast::Chains of Light!9');
CALL coa27_set('witch-hunter-black-knight', 'can cast::Burrow Bolt>cast melee::Burrow Bolt!20', 'can cast::Burrow Bolt>cast::Burrow Bolt!20');
CALL coa27_set('witch-hunter-black-knight', 'can cast::Brand of the Condemned>cast melee::Brand of the Condemned!13', 'can cast::Brand of the Condemned>cast::Brand of the Condemned!13');
CALL coa27_set('witch-hunter-black-knight', 'can cast::Brand of the Damned>cast melee::Brand of the Damned!7', 'can cast::Brand of the Damned>cast::Brand of the Damned!7');
CALL coa27_set('witch-hunter-black-knight', 'can cast::Bola Throw>cast melee::Bola Throw!2', 'can cast::Bola Throw>cast::Bola Throw!2');
CALL coa27_set('knight-of-xoroth-hellfire', 'can cast::Flesh Hook>cast melee::Flesh Hook!1', 'can cast::Flesh Hook>cast::Flesh Hook!1');
CALL coa27_set('knight-of-xoroth-war', 'medium aoe>cast melee::Flesh Hook!88', 'medium aoe>cast::Flesh Hook!88');
CALL coa27_set('knight-of-xoroth-war', 'can cast::Flesh Hook>cast melee::Flesh Hook!78', 'can cast::Flesh Hook>cast::Flesh Hook!78');
CALL coa27_set('reaper-domination', 'can cast::Scythe Rush>cast melee::Scythe Rush!30', 'can cast::Scythe Rush>cast::Scythe Rush!30');
CALL coa27_set('reaper-harvest', 'can cast::Scythe Rush>cast melee::Scythe Rush!30', 'can cast::Scythe Rush>cast::Scythe Rush!30');
CALL coa27_set('reaper-soul', 'can cast::Scythe Rush>cast melee::Scythe Rush!30', 'can cast::Scythe Rush>cast::Scythe Rush!30');
CALL coa27_set('felsworn-tyrant', 'can cast::Bane of Power>cast melee::Bane of Power!16', 'debuff missing::Bane of Power>cast debuff::Bane of Power!16');
CALL coa27_set('felsworn-tyrant', 'can cast::Bane of Fire>cast melee::Bane of Fire!4', 'debuff missing::Bane of Fire>cast debuff::Bane of Fire!4');
CALL coa27_set('felsworn-tyrant', 'buff missing::Manafeed>cast buff::Manafeed!19', 'can cast::Manafeed>cast::Manafeed!19');
CALL coa27_set('sun-cleric-piety', 'buff missing::Radiant Flame>cast buff::Radiant Flame!80', 'debuff missing::Radiant Flame>cast debuff::Radiant Flame!80');
CALL coa27_set('reaper-domination', 'medium health>cast heal::Deathwind!84', 'medium health>cast melee::Deathwind!84');
CALL coa27_set('sun-cleric-piety', 'medium aoe heal>cast heal tank::Radiance!25', 'medium aoe heal>cast buff::Radiance!25');

-- 6. Rows that need a caster aura the line never checks (Hellfire Stirrups: Xorothian Warsteed; Turn The Blade: Tower
--    Formation; Deathchaser: Soul Infusion; Desecrate: Recently Avoided; Melt: Demonfire, which "coa attack" already
--    handles), and Blessing of Light (2.5), a spell no level 70 Blessings bot knows.
DELETE FROM `playerbots_custom_strategy` WHERE `owner` = 0 AND (`name`, `action_line`) IN (
  ('guardian-inspiration', 'buff missing::Turn The Blade>cast buff::Turn The Blade!8'),
  ('knight-of-xoroth-hellfire', 'buff missing::Hellfire Stirrups>cast buff::Hellfire Stirrups!23'),
  ('knight-of-xoroth-war', 'buff missing::Hellfire Stirrups>cast buff::Hellfire Stirrups!23'),
  ('reaper-soul', 'debuff missing::Deathchaser>cast debuff::Deathchaser!81'),
  ('witch-hunter-black-knight', 'can cast::Desecrate>cast melee::Desecrate!24'),
  ('knight-of-xoroth-hellfire', 'can cast::Melt>cast melee::Melt!9'),
  ('knight-of-xoroth-war', 'can cast::Melt>cast melee::Melt!11'),
  ('sun-cleric-blessings', 'party member critical health>cast heal party::Blessing of Light!94')
);

-- 7. Rows that fire but never win the turn (same logs). Ranged spells on `cast melee::` (Tyrant's Gaze 10 yd, Cripple 30 yd,
--    Murder 15 yd, Knight's Seal) go to `cast::`. Dropped: Broad Sweep (swapped at run time for Ballad of the Dragonslayer,
--    which has its own row), Warbringer (needs Demonfire, "coa attack" casts it), the Soul Infusion spells (Soulslam,
--    Soulrend, Withering Touch), Stormbreaker (a purge, "coa purge" handles them). Priority changes wait for a bench run
--    after the Auto Attack fix, which already moved these specs (Reaper Harvest +30 %).
CALL coa27_set('felsworn-tyrant', 'can cast::Tyrant''s Gaze>cast melee::Tyrant''s Gaze!23', 'can cast::Tyrant''s Gaze>cast::Tyrant''s Gaze!23');
CALL coa27_set('felsworn-tyrant', 'can cast::Cripple>cast melee::Cripple!6', 'can cast::Cripple>cast::Cripple!6');
CALL coa27_set('reaper-domination', 'can cast::Murder>cast melee::Murder!5', 'can cast::Murder>cast::Murder!5');
CALL coa27_set('reaper-soul', 'can cast::Murder>cast melee::Murder!77', 'can cast::Murder>cast::Murder!77');
CALL coa27_set('witch-hunter-black-knight', 'can cast::Knight''s Seal>cast melee::Knight''s Seal!25', 'can cast::Knight''s Seal>cast::Knight''s Seal!25');
DELETE FROM `playerbots_custom_strategy` WHERE `owner` = 0 AND (`name`, `action_line`) IN (
  ('guardian-inspiration', 'medium aoe>cast melee::Broad Sweep!85'),
  ('guardian-inspiration', 'can cast::Broad Sweep>cast melee::Broad Sweep!76'),
  ('knight-of-xoroth-war', 'can cast::Warbringer>cast melee::Warbringer!79'),
  ('reaper-domination', 'can cast::Soulslam>cast melee::Soulslam!14'),
  ('reaper-domination', 'can cast::Soulrend>cast melee::Soulrend!4'),
  ('reaper-soul', 'can cast::Soulrend>cast melee::Soulrend!74'),
  ('reaper-soul', 'can cast::Withering Touch>cast melee::Withering Touch!28'),
  ('stormbringer-wind', 'can cast::Stormbreaker>cast::Stormbreaker!3')
);

DROP PROCEDURE IF EXISTS coa27_set;
