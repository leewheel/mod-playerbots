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
