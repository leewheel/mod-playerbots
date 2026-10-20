-- Character Advancement builds of the Warcraft Reborn bots (CoA.ClassModel = "wcr"), one per talent tree.
--
-- class:        1 to 11 (not 10), spec: the talent tree, 0 to 2 in talent tab order (Druid 1 = Feral, DK 0 = Blood).
-- role:         tank, heal, mdps or rdps; spec_name: shown to players ("Protection"); both read on the first row of a
--               tree. primary_stat: passed to AscensionFreepick::ApplyGuide.
-- ord, spell_id, level: the guide, in the order its spells are taken, each from that level.
--
-- Only the table: its rows are in playerbots_coa_wcr_builds_data.sql (talents only; class spells come from the
-- Reborn trainers).

CREATE TABLE IF NOT EXISTS `playerbots_coa_wcr_builds` (
  `class` TINYINT UNSIGNED NOT NULL,
  `spec` TINYINT UNSIGNED NOT NULL,
  `role` VARCHAR(8) NOT NULL,
  `spec_name` VARCHAR(32) NOT NULL DEFAULT '',
  `primary_stat` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `ord` SMALLINT UNSIGNED NOT NULL,
  `spell_id` INT UNSIGNED NOT NULL,
  `level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  PRIMARY KEY (`class`, `spec`, `ord`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
