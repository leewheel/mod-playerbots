-- Mystic Enchants of the Warcraft Reborn bots (CoA.ClassModel = "wcr"), one list per talent tree.
--
-- class, spec:  as in playerbots_coa_wcr_builds (spec 0 to 2 in talent tab order).
-- ord:          the order the core places them in (AscensionMysticEnchant::ApplyGuide); the bot is taught each one.
-- spell:        MysticEnchant.dbc spell; stacks: how many slots it takes; level: from this character level.
-- The core keeps its own limits (17 slots, Reborn: 1 legendary, 1 artifact, 5 epic) and skips an enchant the bot's
-- class, level, realm or Character Advancement investment does not allow.

CREATE TABLE IF NOT EXISTS `playerbots_coa_wcr_enchants` (
  `class` TINYINT UNSIGNED NOT NULL,
  `spec` TINYINT UNSIGNED NOT NULL,
  `ord` SMALLINT UNSIGNED NOT NULL,
  `spell` INT UNSIGNED NOT NULL,
  `stacks` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `level` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`class`, `spec`, `ord`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- First list, picked from the client catalog by tree (MysticEnchant.dbc tab), rotation keywords and talents of the
-- tree's build; to be checked against the Reborn bench.
DELETE FROM `playerbots_coa_wcr_enchants`;

INSERT INTO `playerbots_coa_wcr_enchants` (`class`,`spec`,`ord`,`spell`,`stacks`,`level`) VALUES
(1, 0, 1, 1134995, 1, 30), -- LEG Arms Mastery
(1, 0, 2, 1181080, 1, 15), -- EPI Heavy Thunder
(1, 0, 3, 1181050, 1, 20), -- EPI Cleaving Slam
(1, 0, 4, 1585059, 1, 24), -- EPI A Quick Death
(1, 0, 5, 1586195, 1, 40), -- EPI Master of Arms
(1, 0, 6, 1180530, 1, 46), -- EPI Throwing Mastery
(1, 0, 7, 58357, 1, 10), -- RAR Glyph of Heroic Strike
(1, 0, 8, 58356, 1, 10), -- RAR Glyph of Resonating Power
(1, 0, 9, 58386, 1, 12), -- RAR Glyph of Overpower
(1, 0, 10, 58385, 1, 15), -- RAR Glyph of Rending
(1, 0, 11, 58355, 1, 25), -- RAR Glyph of Rapid Charge
(1, 0, 12, 58384, 1, 30), -- RAR Glyph of Sweeping Strikes
(1, 0, 13, 58368, 1, 40), -- RAR Glyph of Mortal Strike
(1, 0, 14, 1312879, 1, 45), -- RAR Improved Demoralizing Shout
(1, 0, 15, 1320496, 1, 45), -- RAR Improved Cleave
(1, 0, 16, 63324, 1, 60), -- RAR Glyph of Bladestorm
(1, 1, 1, 1134994, 1, 30), -- LEG Fury Mastery
(1, 1, 2, 1180620, 1, 15), -- EPI Relentless Fury
(1, 1, 3, 1584484, 1, 29), -- EPI Outrage
(1, 1, 4, 1585249, 1, 40), -- EPI Bloodthirsty
(1, 1, 5, 1587198, 1, 40), -- EPI Here Comes The Big One
(1, 1, 6, 1180250, 1, 52), -- EPI Power Slam
(1, 1, 7, 1100160, 1, 0), -- RAR Glyph of Slam
(1, 1, 8, 58367, 1, 25), -- RAR Glyph of Execution
(1, 1, 9, 58366, 1, 25), -- RAR Glyph of Cleaving
(1, 1, 10, 58370, 1, 30), -- RAR Glyph of Whirlwind
(1, 1, 11, 58369, 1, 40), -- RAR Glyph of Bloodthirst
(1, 2, 1, 1134996, 1, 30), -- LEG Protection Mastery
(1, 2, 2, 1181340, 1, 15), -- EPI Iron Guard
(1, 2, 3, 1556000, 1, 15), -- EPI Roar of Dread
(1, 2, 4, 1571793, 1, 50), -- EPI Devastating Blademaster
(1, 2, 5, 1590179, 1, 28), -- EPI Shield Finesse
(1, 2, 6, 1567776, 1, 16), -- EPI Spell Block
(1, 2, 7, 58387, 1, 10), -- RAR Glyph of Sunder Armor
(1, 2, 8, 58375, 1, 25), -- RAR Glyph of Blocking
(1, 2, 9, 63326, 1, 40), -- RAR Glyph of Vigilance
(1, 2, 10, 58388, 1, 50), -- RAR Glyph of Devastate
(1, 2, 11, 58376, 1, 20), -- RAR Glyph of Last Stand
(1, 2, 12, 63328, 1, 50), -- RAR Glyph of Spell Reflection
(2, 0, 1, 1134990, 1, 30), -- LEG Holy Mastery
(2, 0, 2, 1586786, 1, 15), -- EPI Spark of Hope
(2, 0, 3, 1180910, 1, 16), -- EPI Divine Reprieve
(2, 0, 4, 1590048, 1, 30), -- EPI Swift Favor
(2, 0, 5, 1180430, 1, 35), -- EPI A New Dawn
(2, 0, 6, 1567749, 1, 50), -- EPI Divine Order
(2, 0, 7, 54940, 1, 38), -- RAR Glyph of Seal of Wisdom
(2, 0, 8, 63224, 1, 40), -- RAR Glyph of Holy Shock
(2, 0, 9, 1325988, 1, 45), -- RAR Eye for an Eye
(2, 0, 10, 56420, 1, 50), -- RAR Glyph of Holy Wrath
(2, 0, 11, 63218, 1, 60), -- RAR Glyph of Beacon of Light
(2, 1, 1, 1572131, 1, 40), -- LEG Crimson Defense
(2, 1, 2, 1134989, 1, 30), -- LEG Protection Mastery
(2, 1, 3, 1581647, 1, 15), -- EPI Pious Strikes
(2, 1, 4, 1581319, 1, 50), -- EPI Shield of Wrath
(2, 1, 5, 1585146, 1, 40), -- EPI Baleful Hammer
(2, 1, 6, 1180960, 1, 40), -- EPI Righteous Light
(2, 1, 7, 1585123, 1, 60), -- EPI Blessed Hammer
(2, 1, 8, 63222, 1, 40), -- RAR Glyph of Shield of Righteousness
(2, 1, 9, 63219, 1, 60), -- RAR Glyph of Hammer of the Righteous
(2, 2, 1, 1134991, 1, 30), -- LEG Retribution Mastery
(2, 2, 2, 1180700, 1, 15), -- EPI Crusader's Judgement
(2, 2, 3, 1570582, 1, 15), -- EPI Consecrated Weapon
(2, 2, 4, 1590668, 1, 35), -- EPI Fierce Vengeance
(2, 2, 5, 1590816, 1, 40), -- EPI Sword of Vengeance
(2, 2, 6, 1554923, 1, 60), -- EPI Divine Resurgence
(2, 2, 7, 54927, 1, 10), -- RAR Glyph of Crusader Strike
(2, 2, 8, 56414, 1, 10), -- RAR Glyph of Seal of Righteousness
(2, 2, 9, 54925, 1, 20), -- RAR Glyph of Seal of Command
(2, 2, 10, 56416, 1, 25), -- RAR Glyph of Vengeance
(2, 2, 11, 54922, 1, 40), -- RAR Glyph of Judgement
(2, 2, 12, 54938, 1, 48), -- RAR Glyph of Avenging Wrath
(2, 2, 13, 63220, 1, 60), -- RAR Glyph of Divine Storm
(2, 2, 14, 54926, 1, 60), -- RAR Glyph of Hammer of Wrath
(3, 0, 1, 1180820, 1, 15), -- EPI Hunting Frenzy
(3, 0, 2, 1587364, 1, 34), -- EPI Fetch Me Their Bones!
(3, 0, 3, 1180510, 1, 35), -- EPI Beastmaster's Gambit
(3, 0, 4, 56856, 1, 40), -- RAR Glyph of the Hawk
(3, 0, 5, 56830, 1, 40), -- RAR Glyph of Bestial Wrath
(3, 0, 6, 56857, 1, 40), -- RAR Glyph of the Beast
(3, 0, 7, 1363458, 1, 45), -- RAR Trap Mastery
(3, 0, 8, 1319423, 1, 45), -- RAR Improved Hunter's Mark
(3, 0, 9, 1319412, 1, 45), -- RAR Improved Concussive Shot
(3, 0, 10, 1353622, 1, 45), -- RAR Focused Aim
(3, 1, 1, 1134985, 1, 30), -- LEG Marksmanship Mastery
(3, 1, 2, 1590490, 1, 15), -- EPI Sting Specialist
(3, 1, 3, 1590555, 1, 15), -- EPI Hydra's Sting
(3, 1, 4, 1584972, 1, 20), -- EPI Deadly Aim
(3, 1, 5, 1180230, 1, 30), -- EPI Power Through
(3, 1, 6, 1569166, 1, 60), -- EPI Steady Pace
(3, 1, 7, 56829, 1, 10), -- RAR Glyph of Hunter's Mark
(3, 1, 8, 56832, 1, 10), -- RAR Glyph of Serpent Sting
(3, 1, 9, 56841, 1, 10), -- RAR Glyph of Arcane Shot
(3, 1, 10, 63073, 1, 10), -- RAR Glyph of Quick Shot
(3, 1, 11, 56836, 1, 18), -- RAR Glyph of Multi-Shot
(3, 1, 12, 56824, 1, 20), -- RAR Glyph of Aimed Shot
(3, 1, 13, 56828, 1, 26), -- RAR Glyph of Rapid Fire
(3, 1, 14, 56826, 1, 30), -- RAR Glyph of Steady Shot
(3, 1, 15, 56842, 1, 40), -- RAR Glyph of Trueshot Aura
(3, 1, 16, 63065, 1, 60), -- RAR Glyph of Chimera Shot
(3, 2, 1, 1134986, 1, 34), -- LEG Survival Mastery
(3, 2, 2, 1573558, 1, 15), -- EPI Fire Watch
(3, 2, 3, 1590263, 1, 15), -- EPI Combat Rhythm
(3, 2, 4, 1556006, 1, 25), -- EPI Quickfire Tactics
(3, 2, 5, 1180340, 1, 60), -- EPI Molten Ammo
(3, 2, 6, 1163072, 1, 60), -- EPI Apex Predator
(3, 2, 7, 63086, 1, 10), -- RAR Glyph of Raptor Strike
(3, 2, 8, 56846, 1, 14), -- RAR Glyph of Immolation Trap
(3, 2, 9, 63068, 1, 34), -- RAR Glyph of Fire Traps
(3, 2, 10, 56848, 1, 40), -- RAR Glyph of Wyvern Sting
(3, 2, 11, 63066, 1, 60), -- RAR Glyph of Explosive Shot
(3, 2, 12, 56850, 1, 60), -- RAR Glyph of Deterrence
(3, 2, 13, 1163066, 1, 60), -- RAR Glyph of Carve
(4, 0, 1, 1134972, 1, 30), -- LEG Assassination Mastery
(4, 0, 2, 1180360, 1, 15), -- EPI Bleeding Edge
(4, 0, 3, 1590974, 1, 30), -- EPI Blood-Soaked Mutilation
(4, 0, 4, 1573425, 1, 40), -- EPI Venomous Edge
(4, 0, 5, 1570716, 1, 40), -- EPI Natural Energy
(4, 0, 6, 1180400, 1, 60), -- EPI Bloodshed
(4, 0, 7, 56812, 1, 14), -- RAR Glyph of Garrote
(4, 0, 8, 56813, 1, 18), -- RAR Glyph of Ambush
(4, 0, 9, 56805, 1, 20), -- RAR Glyph of Vigor
(4, 0, 10, 56801, 1, 20), -- RAR Glyph of Rupture
(4, 0, 11, 56820, 1, 20), -- RAR Glyph of Crippling Poison
(4, 0, 12, 63268, 1, 30), -- RAR Glyph of Mutilate
(4, 0, 13, 63254, 1, 36), -- RAR Glyph of Fan of Knives
(4, 0, 14, 1330893, 1, 45), -- RAR Sleight of Hand
(4, 0, 15, 63249, 1, 60), -- RAR Glyph of Hunger For Blood
(4, 0, 16, 63256, 1, 60), -- RAR Glyph of Tricks of the Trade
(4, 1, 1, 1134973, 1, 30), -- LEG Combat Mastery
(4, 1, 2, 1589904, 1, 30), -- EPI Blade Vortex
(4, 1, 3, 1589466, 1, 40), -- EPI Adrenaline Junkie
(4, 1, 4, 1180380, 1, 45), -- EPI Combat Proficiency
(4, 1, 5, 1581852, 1, 50), -- EPI Prison Tactics
(4, 1, 6, 1583657, 1, 60), -- EPI Massacre
(4, 1, 7, 56802, 1, 10), -- RAR Glyph of Eviscerate
(4, 1, 8, 56821, 1, 10), -- RAR Glyph of Sinister Strike
(4, 1, 9, 56810, 1, 10), -- RAR Glyph of Slice and Dice
(4, 1, 10, 56803, 1, 14), -- RAR Glyph of Expose Armor
(4, 1, 11, 56804, 1, 16), -- RAR Glyph of Feint
(4, 1, 12, 56818, 1, 30), -- RAR Glyph of Blade Flurry
(4, 1, 13, 56808, 1, 40), -- RAR Glyph of Adrenaline Rush
(4, 1, 14, 1582447, 1, 40), -- RAR Hastened Feint
(4, 1, 15, 1351633, 1, 45), -- RAR Blood Spatter
(4, 1, 16, 63252, 1, 60), -- RAR Glyph of Killing Spree
(4, 2, 1, 1134974, 1, 30), -- LEG Subtlety Mastery
(4, 2, 2, 1589946, 1, 18), -- EPI Methodical Approach
(4, 2, 3, 1181170, 1, 20), -- EPI Phantom Stab
(4, 2, 4, 1571172, 1, 30), -- EPI Shadow of Death
(4, 2, 5, 1251714, 1, 60), -- EPI From the Shadows
(4, 2, 6, 1580266, 1, 60), -- EPI Veiled Wire
(4, 2, 7, 56814, 1, 20), -- RAR Glyph of Ghostly Strike
(4, 2, 8, 56807, 1, 30), -- RAR Glyph of Hemorrhage
(5, 0, 1, 1134969, 1, 50), -- LEG Discipline Mastery
(5, 0, 2, 1180720, 1, 20), -- EPI Intense Focus
(5, 0, 3, 1567626, 1, 50), -- EPI Words of Healing
(5, 0, 4, 1567650, 1, 50), -- EPI Words of Time
(5, 0, 5, 1571260, 1, 60), -- EPI Divine Symmetry
(5, 0, 6, 1571422, 1, 60), -- EPI Renewed Through Absolution
(5, 0, 7, 1327816, 1, 45), -- RAR Blessed Recovery
(5, 0, 8, 1315017, 1, 45), -- RAR Searing Light
(5, 0, 9, 1315448, 1, 45), -- RAR Improved Psychic Scream
(5, 0, 10, 1571655, 1, 45), -- RAR Blessed Vengeance
(5, 0, 11, 1582800, 1, 45), -- RAR Holy Power Well
(5, 0, 12, 63248, 1, 50), -- RAR Glyph of Pain Suppression
(5, 0, 13, 63235, 1, 60), -- RAR Glyph of Penance
(5, 1, 1, 1134968, 1, 30), -- LEG Holy Mastery
(5, 1, 2, 1180610, 1, 15), -- EPI Overflowing Light
(5, 1, 3, 1181090, 1, 15), -- EPI Promise of Renewal
(5, 1, 4, 1568799, 1, 35), -- EPI Circle of Life
(5, 1, 5, 1581027, 1, 35), -- EPI Purity of Light
(5, 1, 6, 1581566, 1, 40), -- EPI Higher Power
(5, 1, 7, 1100161, 1, 0), -- RAR Glyph of Long Renewal
(5, 1, 8, 55679, 1, 20), -- RAR Glyph of Flash Heal
(5, 1, 9, 55683, 1, 20), -- RAR Glyph of Holy Nova
(5, 1, 10, 55685, 1, 30), -- RAR Glyph of Spirit of Redemption
(5, 1, 11, 55680, 1, 30), -- RAR Glyph of Prayer of Healing
(5, 1, 12, 1571663, 1, 35), -- RAR Glyph of Surge of Light
(5, 1, 13, 1314772, 1, 45), -- RAR Improved Mana Burn
(5, 1, 14, 1571640, 1, 45), -- RAR Afterkindling
(5, 1, 15, 55675, 1, 50), -- RAR Glyph of Circle of Healing
(5, 1, 16, 63231, 1, 60), -- RAR Glyph of Guardian Spirit
(5, 2, 1, 1134970, 1, 30), -- LEG Shadow Mastery
(5, 2, 2, 1180280, 1, 15), -- EPI Dark Focus
(5, 2, 3, 1180670, 1, 15), -- EPI Shadow Reserves
(5, 2, 4, 1580989, 1, 15), -- EPI Pendulum
(5, 2, 5, 1300006, 1, 20), -- EPI Psychic Influence
(5, 2, 6, 1180080, 1, 50), -- EPI Vampiric Death
(5, 2, 7, 55681, 1, 10), -- RAR Glyph of Shadow Word: Pain
(5, 2, 8, 55684, 1, 10), -- RAR Glyph of Fade
(5, 2, 9, 55676, 1, 14), -- RAR Glyph of Psychic Scream
(5, 2, 10, 55687, 1, 20), -- RAR Glyph of Mind Flay
(5, 2, 11, 55689, 1, 40), -- RAR Glyph of Shadow
(5, 2, 12, 1571655, 1, 45), -- RAR Blessed Vengeance
(5, 2, 13, 1582800, 1, 45), -- RAR Holy Power Well
(5, 2, 14, 55682, 1, 46), -- RAR Glyph of Shadow Word: Death
(5, 2, 15, 63229, 1, 60), -- RAR Glyph of Dispersion
(7, 0, 1, 1570759, 1, 30), -- LEG Booming Thunder
(7, 0, 2, 1590346, 1, 15), -- EPI Ungrounded
(7, 0, 3, 1180600, 1, 15), -- EPI Shock Tactics
(7, 0, 4, 1181200, 1, 20), -- EPI Elemental Momentum
(7, 0, 5, 1572975, 1, 40), -- EPI Mastery of Lightning
(7, 0, 6, 1584711, 1, 46), -- EPI Molten Outburst
(7, 0, 7, 55450, 1, 0), -- RAR Glyph of Fire Nova
(7, 0, 8, 55447, 1, 10), -- RAR Glyph of Flame Shock
(7, 0, 9, 55453, 1, 10), -- RAR Glyph of Lightning Bolt
(7, 0, 10, 55451, 1, 10), -- RAR Glyph of Flametongue Weapon
(7, 0, 11, 55449, 1, 24), -- RAR Glyph of Chain Lightning
(7, 0, 12, 55452, 1, 40), -- RAR Glyph of Elemental Mastery
(7, 0, 13, 63270, 1, 40), -- RAR Glyph of Thunder
(7, 0, 14, 55454, 1, 40), -- RAR Glyph of Lava
(7, 0, 15, 1316221, 1, 45), -- RAR Tidal Mastery
(7, 0, 16, 63280, 1, 50), -- RAR Glyph of Totem of Wrath
(7, 1, 1, 1134983, 1, 30), -- LEG Enhancement Mastery
(7, 1, 2, 1181220, 1, 15), -- EPI Converging Elements
(7, 1, 3, 1572909, 1, 30), -- EPI Stormborn
(7, 1, 4, 1587220, 1, 30), -- EPI Zephyr
(7, 1, 5, 1180170, 1, 45), -- EPI Dynamic Charge
(7, 1, 6, 1579878, 1, 60), -- EPI Pack Alpha
(7, 1, 7, 1182060, 1, 10), -- RAR Glyph of Shamanistic Strikes
(7, 1, 8, 55448, 1, 10), -- RAR Glyph of Lightning Shield
(7, 1, 9, 1182061, 1, 10), -- RAR Glyph of Earthen Guardian
(7, 1, 10, 63298, 1, 10), -- RAR Glyph of Stoneclaw Totem
(7, 1, 11, 55444, 1, 20), -- RAR Glyph of Lava Lash
(7, 1, 12, 55443, 1, 20), -- RAR Glyph of Frost Shock
(7, 1, 13, 55445, 1, 30), -- RAR Glyph of Windfury Weapon
(7, 1, 14, 55446, 1, 40), -- RAR Glyph of Stormstrike
(7, 1, 15, 1316544, 1, 45), -- RAR Improved Fire Nova
(7, 1, 16, 63271, 1, 60), -- RAR Glyph of Feral Spirit
(7, 2, 1, 1134982, 1, 30), -- LEG Restoration Mastery
(7, 2, 2, 1180560, 1, 15), -- EPI Resurgent Waves
(7, 2, 3, 1589255, 1, 20), -- EPI Nature's Flow
(7, 2, 4, 1180570, 1, 20), -- EPI Healing Tide
(7, 2, 5, 1180920, 1, 20), -- EPI Tides of Restoration
(7, 2, 6, 1567426, 1, 60), -- EPI Low Tide
(7, 2, 7, 55440, 1, 10), -- RAR Glyph of Healing Wave
(7, 2, 8, 55438, 1, 20), -- RAR Glyph of Lesser Healing Wave
(7, 2, 9, 1182062, 1, 20), -- RAR Glyph of Water Shield
(7, 2, 10, 55439, 1, 30), -- RAR Glyph of Earthliving Weapon
(7, 2, 11, 55441, 1, 40), -- RAR Glyph of Mana Tide
(7, 2, 12, 55437, 1, 40), -- RAR Glyph of Chain Heal
(7, 2, 13, 63279, 1, 50), -- RAR Glyph of Earth Shield
(7, 2, 14, 63273, 1, 60), -- RAR Glyph of Riptide
(8, 0, 1, 1134962, 1, 30), -- LEG Arcane Mastery
(8, 0, 2, 1585312, 1, 15), -- EPI Infused Gems
(8, 0, 3, 1572875, 1, 24), -- EPI Resonant Barrage
(8, 0, 4, 1585102, 1, 30), -- EPI Power Overwhelming
(8, 0, 5, 1181430, 1, 30), -- EPI Arcane Conduit
(8, 0, 6, 1584079, 1, 50), -- EPI Impeccable Timing
(8, 0, 7, 56363, 1, 10), -- RAR Glyph of Arcane Missiles
(8, 0, 8, 56383, 1, 10), -- RAR Glyph of Mage Armor
(8, 0, 9, 56360, 1, 14), -- RAR Glyph of Arcane Explosion
(8, 0, 10, 62210, 1, 24), -- RAR Glyph of Arcane Blast
(8, 0, 11, 1587962, 1, 35), -- RAR Abjurer's Ward: Arcane
(8, 0, 12, 56381, 1, 40), -- RAR Glyph of Arcane Power
(8, 0, 13, 56367, 1, 40), -- RAR Glyph of Mana Gem
(8, 0, 14, 1328332, 1, 45), -- RAR Frost Warding
(8, 0, 15, 1312873, 1, 45), -- RAR Improved Scorch
(8, 0, 16, 63092, 1, 60), -- RAR Glyph of Arcane Barrage
(8, 1, 1, 1134960, 1, 30), -- LEG Fire Mastery
(8, 1, 2, 1579811, 1, 10), -- EPI Trisolaris
(8, 1, 3, 1585270, 1, 15), -- EPI Fired Up
(8, 1, 4, 1180200, 1, 30), -- EPI Precision Impact
(8, 1, 5, 1590832, 1, 40), -- EPI Brand of the Pyromaniac
(8, 1, 6, 1583854, 1, 60), -- EPI Scorched Ground
(8, 1, 7, 56369, 1, 10), -- RAR Glyph of Fire Blast
(8, 1, 8, 56368, 1, 10), -- RAR Glyph of Fireball
(8, 1, 9, 1590891, 1, 20), -- RAR Glyph of Impact
(8, 1, 10, 56371, 1, 22), -- RAR Glyph of Scorch
(8, 1, 11, 1587982, 1, 35), -- RAR Abjurer's Ward: Fire
(8, 1, 12, 1312605, 1, 45), -- RAR Arcane Shielding
(8, 1, 13, 1329444, 1, 45), -- RAR Magic Absorption
(8, 1, 14, 1582821, 1, 45), -- RAR Fiery Power Well
(8, 1, 15, 56382, 1, 50), -- RAR Glyph of Molten Armor
(8, 1, 16, 63091, 1, 60), -- RAR Glyph of Living Bomb
(8, 2, 1, 1134961, 1, 30), -- LEG Frost Mastery
(8, 2, 2, 1181020, 1, 15), -- EPI Biting Frost
(8, 2, 3, 1180760, 1, 20), -- EPI Icy Slaughter
(8, 2, 4, 1583513, 1, 45), -- EPI Stay Frosty
(8, 2, 5, 1582381, 1, 60), -- EPI Frozen Flesh
(8, 2, 6, 1591163, 1, 60), -- EPI Winter's Blaze
(8, 2, 7, 56370, 1, 10), -- RAR Glyph of Frostbolt
(8, 2, 8, 56374, 1, 20), -- RAR Glyph of Icy Veins
(8, 2, 9, 56377, 1, 30), -- RAR Glyph of Ice Lance
(8, 2, 10, 56384, 1, 30), -- RAR Glyph of Ice Armor
(8, 2, 11, 1588050, 1, 35), -- RAR Abjurer's Ward: Frost
(8, 2, 12, 56373, 1, 40), -- RAR Glyph of Water Elemental
(8, 2, 13, 70937, 1, 40), -- RAR Glyph of Eternal Water
(8, 2, 14, 63095, 1, 40), -- RAR Glyph of Ice Barrier
(8, 2, 15, 1582821, 1, 45), -- RAR Fiery Power Well
(8, 2, 16, 61205, 1, 60), -- RAR Glyph of Frostfire
(9, 0, 1, 1134967, 1, 30), -- LEG Affliction Mastery
(9, 0, 2, 1180950, 1, 15), -- EPI Dark Harvest
(9, 0, 3, 1577360, 1, 15), -- EPI Cursed Shadows
(9, 0, 4, 1580989, 1, 15), -- EPI Pendulum
(9, 0, 5, 1180490, 1, 42), -- EPI Doomcaller's Wrath
(9, 0, 6, 1180450, 1, 60), -- EPI Eternal Torment
(9, 0, 7, 63320, 1, 10), -- RAR Glyph of Life Tap
(9, 0, 8, 56241, 1, 10), -- RAR Glyph of Curse of Agony
(9, 0, 9, 56216, 1, 30), -- RAR Glyph of Siphon Life
(9, 0, 10, 70947, 1, 40), -- RAR Glyph of Quick Decay
(9, 0, 11, 56232, 1, 42), -- RAR Glyph of Death Coil
(9, 0, 12, 56233, 1, 50), -- RAR Glyph of Unstable Affliction
(9, 0, 13, 63302, 1, 60), -- RAR Glyph of Haunt
(9, 1, 1, 1134965, 1, 30), -- LEG Demonology Mastery
(9, 1, 2, 1576077, 1, 15), -- EPI Demonic Siphon
(9, 1, 3, 1580989, 1, 15), -- EPI Pendulum
(9, 1, 4, 1583447, 1, 35), -- EPI Fire Attunement
(9, 1, 5, 1180473, 1, 40), -- EPI Dread Communion
(9, 1, 6, 1570974, 1, 60), -- EPI Demonic Influence
(9, 1, 7, 63303, 1, 10), -- RAR Glyph of Metamorphosis
(9, 1, 8, 56238, 1, 12), -- RAR Glyph of Health Funnel
(9, 1, 9, 56248, 1, 15), -- RAR Glyph of Imp
(9, 1, 10, 63312, 1, 20), -- RAR Glyph of Soul Link
(9, 1, 11, 56246, 1, 40), -- RAR Glyph of Felguard
(9, 1, 12, 56250, 1, 40), -- RAR Glyph of Succubus
(9, 1, 13, 56217, 1, 40), -- RAR Glyph of Howl of Terror
(9, 1, 14, 63309, 1, 40), -- RAR Glyph of Demonic Circle
(9, 2, 1, 1134964, 1, 30), -- LEG Destruction Mastery
(9, 2, 2, 1567541, 1, 40), -- EPI Heating Up!
(9, 2, 3, 1585375, 1, 50), -- EPI Unstable Void
(9, 2, 4, 1554922, 1, 60), -- EPI Agent of Chaos
(9, 2, 5, 1571016, 1, 60), -- EPI Wild Felfire
(9, 2, 6, 1578703, 1, 60), -- EPI Gul'dan's Chosen
(9, 2, 7, 56228, 1, 10), -- RAR Glyph of Immolate
(9, 2, 8, 56240, 1, 10), -- RAR Glyph of Shadow Bolt
(9, 2, 9, 56226, 1, 14), -- RAR Glyph of Searing Pain
(9, 2, 10, 56229, 1, 20), -- RAR Glyph of Shadowburn
(9, 2, 11, 56242, 1, 30), -- RAR Glyph of Incinerate
(9, 2, 12, 56235, 1, 40), -- RAR Glyph of Conflagrate
(9, 2, 13, 63304, 1, 60), -- RAR Glyph of Chaos Bolt
(11, 0, 1, 1134979, 1, 30), -- LEG Balance Mastery
(11, 0, 2, 1583363, 1, 15), -- EPI Elune's Fury
(11, 0, 3, 1567772, 1, 15), -- EPI Languish
(11, 0, 4, 1580989, 1, 15), -- EPI Pendulum
(11, 0, 5, 1567756, 1, 32), -- EPI Surging Eclipse
(11, 0, 6, 1580025, 1, 50), -- EPI TIMBERRR
(11, 0, 7, 54756, 1, 10), -- RAR Glyph of Wrath
(11, 0, 8, 54876, 1, 10), -- RAR Glyph of Moonfire
(11, 0, 9, 54845, 1, 20), -- RAR Glyph of Starfire
(11, 0, 10, 54830, 1, 30), -- RAR Glyph of Insect Swarm
(11, 0, 11, 63056, 1, 40), -- RAR Glyph of Monsoon
(11, 0, 12, 62080, 1, 40), -- RAR Glyph of Focus
(11, 0, 13, 1316862, 1, 45), -- RAR Feral Aggression
(11, 0, 14, 1348412, 1, 45), -- RAR Master Shapeshifter
(11, 0, 15, 1316835, 1, 45), -- RAR Natural Shapeshifter
(11, 0, 16, 1582842, 1, 45), -- RAR Earthen Power Well
(11, 1, 1, 1134977, 1, 30), -- LEG Guardian Mastery
(11, 1, 2, 1589853, 1, 15), -- EPI Ursoc's Rage
(11, 1, 3, 1581667, 1, 30), -- EPI Relentless Laceration
(11, 1, 4, 1122816, 1, 44), -- EPI Shifting Bark
(11, 1, 5, 1589878, 1, 50), -- EPI Bash and Thrash
(11, 1, 6, 1100800, 1, 50), -- EPI Viscera
(11, 1, 7, 54811, 1, 10), -- RAR Glyph of Maul
(11, 1, 8, 65243, 1, 15), -- RAR Glyph of Survival Instincts
(11, 1, 9, 54813, 1, 40), -- RAR Glyph of Mangle
(11, 1, 10, 62969, 1, 60), -- RAR Glyph of Berserk
(11, 2, 1, 1134978, 1, 30), -- LEG Restoration Mastery
(11, 2, 2, 1180800, 1, 15), -- EPI Nature's Abundance
(11, 2, 3, 1418500, 1, 15), -- EPI Nature's Synthesis
(11, 2, 4, 1583698, 1, 40), -- EPI Everliving
(11, 2, 5, 1586130, 1, 50), -- EPI Verdant Insight
(11, 2, 6, 1590237, 1, 60), -- EPI Frenzied Growth
(11, 2, 7, 54754, 1, 10), -- RAR Glyph of Rejuvenation
(11, 2, 8, 54825, 1, 10), -- RAR Glyph of Healing Touch
(11, 2, 9, 54743, 1, 12), -- RAR Glyph of Regrowth
(11, 2, 10, 54824, 1, 40), -- RAR Glyph of Swiftmend
(11, 2, 11, 71013, 1, 40), -- RAR Glyph of Rapid Rejuvenation
(11, 2, 12, 1582842, 1, 45), -- RAR Earthen Power Well
(11, 2, 13, 54826, 1, 50), -- RAR Glyph of Lifebloom
(11, 2, 14, 1162971, 1, 58), -- RAR Glyph of Nourish
(11, 2, 15, 62970, 1, 60); -- RAR Glyph of Wild Growth
