#include "CoaWildcardPicks.h"

#include <algorithm>
#include <array>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
constexpr uint8 TANK = 1 << uint8(WildcardArchetype::Tank);
constexpr uint8 HEAL = 1 << uint8(WildcardArchetype::Heal);
constexpr uint8 MELEE = 1 << uint8(WildcardArchetype::Melee);
constexpr uint8 RANGED = 1 << uint8(WildcardArchetype::Ranged);
constexpr uint8 CASTER = 1 << uint8(WildcardArchetype::Caster);
constexpr uint8 DAMAGE = MELEE | RANGED | CASTER;
constexpr uint8 ANY = TANK | HEAL | DAMAGE;

// Entries `first` to `last` (one entry when `last` is 0), for the archetypes of `archetypes`.
struct Pick
{
    uint32 first;
    uint32 last;
    uint8 archetypes;
    int8 weight;
};

constexpr Pick PICKS[] = {
    // Tank: a form or stance that holds threat, a taunt, a shield and its strikes, area threat.
    { 42083, 0, TANK, 3 },       // Bear Form
    { 40196, 0, TANK, 2 },       // Maul
    { 40265, 0, TANK, 2 },       // Lacerate
    { 37749, 0, TANK, 2 },       // Thrash (Bear)
    { 40822, 0, TANK, 2 },       // Mangle
    { 41291, 0, TANK, 2 },       // Ironfur
    { 40243, 0, TANK, 1 },       // Frenzied Regeneration
    { 40242, 0, TANK, 1 },       // Barkskin
    { 40583, 0, TANK, 1 },       // Survival Instincts
    { 40007, 0, TANK, 1 },       // Demoralizing Roar
    { 40167, 0, TANK, 1 },       // Challenging Roar
    { 138049, 0, TANK, 1 },      // Fire Swipe (Bear)
    { 138051, 0, TANK, 1 },      // Frost Swipe (Bear)
    { 138053, 0, TANK, 1 },      // Storm Swipe (Bear)
    { 138055, 0, TANK, 1 },      // Shadow Swipe (Bear)
    { 138057, 0, TANK, 1 },      // Light Swipe (Bear)
    { 138059, 0, TANK, 1 },      // Arcane Swipe (Bear)
    { 50018, 50023, TANK, 2 },   // Mystic, Dusk, Light, Flame, Frost and Storm Maul
    { 42069, 0, TANK, 3 },       // Defensive Stance
    { 40137, 0, TANK, 2 },       // Shield Block
    { 40187, 0, TANK, 2 },       // Thunder Clap
    { 50069, 50074, TANK, 2 },   // Aether, Umbral, Dawn, Lava, Glacial and Tempest Smash (Thunder Clap)
    { 40192, 0, TANK, 2 },       // Revenge
    { 138024, 138029, TANK, 2 }, // Flame, Rime, Storm, Dusk, Dawn and Aether Revenge
    { 40245, 0, TANK, 3 },       // Shield Slam
    { 138030, 138033, TANK, 2 }, // Blazing, Glacial, Tempest and Umbral Shield Slam
    { 138035, 0, TANK, 2 },      // Mystic Shield Slam
    { 40715, 0, TANK, 2 },       // Devastate
    { 40054, 0, TANK, 1 },       // Mocking Blow
    { 40090, 0, TANK, 1 },       // Challenging Shout
    { 40070, 0, TANK, 1 },       // Shield Wall
    { 40377, 0, TANK, 1 },       // Last Stand
    { 40881, 0, TANK, 1 },       // Shockwave
    { 42074, 0, TANK, 1 },       // Demoralizing Shout
    { 36692, 0, TANK, 1 },       // Shield Strike
    { 40246, 0, TANK, 3 },       // Righteous Fury
    { 40247, 0, TANK, 2 },       // Consecration
    { 40288, 0, TANK, 2 },       // Shield of Righteousness
    { 40653, 0, TANK, 3 },       // Holy Shield
    { 40785, 0, TANK, 2 },       // Avenger's Shield
    { 40999, 0, TANK, 2 },       // Hammer of the Righteous
    { 40258, 0, TANK, 1 },       // Righteous Defense
    { 42040, 0, TANK, 1 },       // Devotion Aura
    { 41930, 0, TANK, 2 },       // Frost Presence
    { 41934, 0, TANK, 1 },       // Blood Boil
    { 41983, 0, TANK, 1 },       // Bone Shield
    { 41936, 0, TANK, 1 },       // Icebound Fortitude
    { 41939, 0, TANK, 1 },       // Death Grip
    // Tank talents.
    { 40675, 0, TANK, 2 },       // Shield Specialization
    { 40705, 0, TANK, 1 },       // Shield Mastery
    { 40703, 0, TANK, 1 },       // Improved Defensive Stance
    { 40880, 0, TANK, 2 },       // Sword and Board
    { 41019, 0, TANK, 1 },       // Warbringer
    { 40897, 0, TANK, 1 },       // Critical Block
    { 40668, 0, TANK, 2 },       // Improved Righteous Fury
    { 40782, 0, TANK, 1 },       // Sacred Duty
    { 40783, 0, TANK, 2 },       // Ardent Defender
    { 40647, 0, TANK, 1 },       // Redoubt
    { 41004, 0, TANK, 1 },       // Shield of the Templar
    { 40524, 0, TANK, 1 },       // Feral Instinct
    { 40820, 0, TANK, 2 },       // Survival of the Fittest
    { 40819, 0, TANK, 1 },       // Primal Tenacity
    { 41023, 0, TANK, 2 },       // Protector of the Pack
    { 41120, 0, TANK, 2 },       // Savage Defense
    { 41069, 0, TANK, 1 },       // Toughness
    { 41088, 0, TANK, 1 },       // Blade Barrier

    // Healer: an efficient heal, a fast heal, a heal over time or a shield, a group heal.
    { 40047, 0, HEAL, 3 },       // Holy Light
    { 40231, 0, HEAL, 3 },       // Flash of Light
    { 40669, 0, HEAL, 3 },       // Holy Shock
    { 40995, 0, HEAL, 2 },       // Beacon of Light
    { 41293, 0, HEAL, 2 },       // Light of Dawn
    { 40289, 0, HEAL, 1 },       // Sacred Shield
    { 40089, 0, HEAL, 1 },       // Purify
    { 40161, 0, HEAL, 1 },       // Cleanse
    { 40018, 0, HEAL, 3 },       // Renew
    { 40127, 0, HEAL, 2 },       // Greater Heal
    { 40128, 0, HEAL, 3 },       // Flash Heal
    { 40002, 0, HEAL, 3 },       // Power Word: Shield
    { 40043, 0, HEAL, 2 },       // Prayer of Healing
    { 40840, 0, HEAL, 3 },       // Circle of Healing
    { 40264, 0, HEAL, 3 },       // Prayer of Mending
    { 40901, 0, HEAL, 3 },       // Penance
    { 40262, 0, HEAL, 1 },       // Binding Heal
    { 40913, 0, HEAL, 1 },       // Guardian Spirit
    { 40803, 0, HEAL, 1 },       // Pain Suppression
    { 40033, 0, HEAL, 1 },       // Dispel Magic
    { 40301, 0, HEAL, 1 },       // Divine Hymn
    { 41285, 0, HEAL, 2 },       // Divine Star
    { 42019, 0, HEAL, 2 },       // Halo
    { 220, 0, HEAL, 2 },         // Shadow Mend
    { 36992, 0, HEAL, 2 },       // Beacon of Virtue
    { 40021, 0, HEAL, 3 },       // Healing Wave
    { 40200, 0, HEAL, 3 },       // Lesser Healing Wave
    { 40080, 0, HEAL, 3 },       // Chain Heal
    { 40950, 0, HEAL, 3 },       // Riptide
    { 40738, 0, HEAL, 2 },       // Earth Shield
    { 42023, 0, HEAL, 2 },       // Healing Rain
    { 40284, 0, HEAL, 2 },       // Water Shield
    { 42082, 0, HEAL, 1 },       // Healing Stream Totem
    { 40483, 0, HEAL, 1 },       // Nature's Swiftness
    { 40482, 0, HEAL, 1 },       // Mana Tide Totem
    { 40064, 0, HEAL, 3 },       // Rejuvenation
    { 40166, 0, HEAL, 2 },       // Healing Touch
    { 40212, 0, HEAL, 3 },       // Regrowth
    { 40266, 0, HEAL, 3 },       // Lifebloom
    { 40547, 0, HEAL, 2 },       // Swiftmend
    { 40919, 0, HEAL, 3 },       // Wild Growth
    { 40279, 0, HEAL, 2 },       // Nourish
    { 40059, 0, HEAL, 1 },       // Tranquility
    { 41965, 0, HEAL, 1 },       // Efflorescence
    { 40250, 0, HEAL, 1 },       // Innervate
    // Healer talents.
    { 40660, 0, HEAL, 2 },       // Healing Light
    { 40664, 0, HEAL, 2 },       // Illumination
    { 40662, 0, HEAL, 1 },       // Divine Intellect
    { 40996, 0, HEAL, 2 },       // Infusion of Light
    { 40778, 0, HEAL, 1 },       // Light's Grace
    { 40446, 0, HEAL, 2 },       // Improved Renew
    { 40447, 0, HEAL, 1 },       // Improved Healing
    { 40445, 0, HEAL, 2 },       // Spiritual Healing
    { 40443, 0, HEAL, 1 },       // Spiritual Guidance
    { 40797, 0, HEAL, 2 },       // Empowered Healing
    { 40796, 0, HEAL, 1 },       // Surge of Light
    { 40586, 0, HEAL, 2 },       // Borrowed Time
    { 40899, 0, HEAL, 2 },       // Divine Aegis
    { 40903, 0, HEAL, 1 },       // Grace
    { 40904, 0, HEAL, 1 },       // Empowered Renew
    { 40478, 0, HEAL, 2 },       // Improved Healing Wave
    { 40700, 0, HEAL, 2 },       // Healing Way
    { 40736, 0, HEAL, 1 },       // Nature's Blessing
    { 40737, 0, HEAL, 2 },       // Improved Chain Heal
    { 40949, 0, HEAL, 2 },       // Tidal Waves
    { 40948, 0, HEAL, 1 },       // Ancestral Awakening
    { 40486, 0, HEAL, 1 },       // Tidal Mastery
    { 40542, 0, HEAL, 2 },       // Improved Rejuvenation
    { 40540, 0, HEAL, 2 },       // Gift of Nature
    { 40814, 0, HEAL, 1 },       // Empowered Touch
    { 40924, 0, HEAL, 1 },       // Living Seed
    { 40918, 0, HEAL, 1 },       // Gift of the Earthmother
    { 40818, 0, HEAL, 1 },       // Nurturing Instinct

    // Melee damage: a hard-hitting strike, a filler, a finisher or a weapon buff, a cooldown.
    { 40362, 0, MELEE, 4 },      // Mortal Strike
    { 40387, 0, MELEE, 4 },      // Bloodthirst
    { 138000, 138005, MELEE, 3 }, // Flame, Hail, Storm, Dusk, Dawn and Manathirst (Bloodthirst)
    { 40103, 0, MELEE, 3 },      // Whirlwind
    { 50087, 50092, MELEE, 3 },  // Whirling Aether, Shadows, Light, Flames, Ice and Storm (Whirlwind)
    { 40175, 0, MELEE, 2 },      // Execute
    { 138066, 138071, MELEE, 2 }, // Burning, Frozen, Storming, Shadowy, Righteous and Arcane Execution
    { 40199, 0, MELEE, 2 },      // Overpower
    { 40096, 0, MELEE, 2 },      // Slam
    { 43010, 43014, MELEE, 1 },  // Arcane, Shadow, Burning, Frozen and Storm Slam
    { 43033, 0, MELEE, 1 },      // Brilliant Slam
    { 40006, 0, MELEE, 2 },      // Heroic Strike
    { 40068, 0, MELEE, 2 },      // Cleave
    { 50063, 50068, MELEE, 2 },  // Spellbound, Voidbound, Lightbound, Flamebound, Frostbound and Stormbound Cleave
    { 42017, 0, MELEE, 2 },      // Raging Blow
    { 41948, 0, MELEE, 2 },      // Colossus Smash
    { 40872, 0, MELEE, 2 },      // Bladestorm
    { 42077, 0, MELEE, 1 },      // Battle Stance
    { 42078, 0, MELEE, 1 },      // Berserker Stance
    { 41877, 0, MELEE, 1 },      // Death Wish
    { 40107, 0, MELEE, 1 },      // Recklessness
    { 40063, 0, MELEE, 1 },      // Rend
    { 42038, 0, MELEE, 1 },      // Charge
    { 40109, 0, MELEE, 3 },      // Sinister Strike
    { 43000, 43004, MELEE, 2 },  // Aether, Void, Cinder, Rime and Volt Strike (Sinister Strike)
    { 40132, 0, MELEE, 3 },      // Eviscerate
    { 50042, 50047, MELEE, 2 },  // Arcane, Ghostly, Holy, Blazing, Freezing and Shocking Finish (Eviscerate)
    { 42081, 0, MELEE, 3 },      // Slice and Dice
    { 40754, 0, MELEE, 3 },      // Mutilate
    { 40003, 0, MELEE, 2 },      // Backstab
    { 40120, 0, MELEE, 2 },      // Rupture
    { 50048, 50053, MELEE, 1 },  // Aether, Shadow, Dawn, Cinder, Glacial and Lightning Rupture
    { 40263, 0, MELEE, 2 },      // Envenom
    { 40503, 0, MELEE, 2 },      // Hemorrhage
    { 40396, 0, MELEE, 2 },      // Adrenaline Rush
    { 40400, 0, MELEE, 2 },      // Blade Flurry
    { 40961, 0, MELEE, 2 },      // Killing Spree
    { 40283, 0, MELEE, 1 },      // Fan of Knives
    { 138114, 138119, MELEE, 2 }, // Flame, Ice, Storm, Shadow, Dawn and Aetherpierce
    { 36714, 0, MELEE, 2 },      // Eclipse Strike
    { 138108, 138113, MELEE, 2 }, // Flare, Frost, Storm, Void, Dawn and Aether Spree
    { 138060, 138065, MELEE, 2 }, // Flurry of Fire, Ice, Lightning, Shadow, Light and Magic
    { 138090, 138095, MELEE, 2 }, // Blades of Flame, Frost, Lightning, Shadow, Light and Magic
    { 40848, 0, MELEE, 3 },      // Crusader Strike
    { 43032, 0, MELEE, 2 },      // Dawn Strike
    { 40989, 0, MELEE, 3 },      // Divine Storm
    { 42110, 0, MELEE, 3 },      // Seal of Command
    { 42104, 0, MELEE, 2 },      // Seal of Righteousness
    { 42105, 0, MELEE, 2 },      // Seal of Vengeance
    { 42060, 0, MELEE, 2 },      // Judgement of Light
    { 42067, 0, MELEE, 2 },      // Judgement of Wisdom
    { 40259, 0, MELEE, 2 },      // Avenging Wrath
    { 40071, 0, MELEE, 1 },      // Exorcism
    { 41911, 0, MELEE, 1 },      // Hammer of Wrath
    { 40549, 0, MELEE, 4 },      // Stormstrike
    { 41029, 0, MELEE, 3 },      // Lava Lash
    { 43028, 43029, MELEE, 2 },  // Rime Lash, Lightning Lash
    { 50054, 50056, MELEE, 2 },  // Mana, Void and Light Lash
    { 43015, 43018, MELEE, 2 },  // Mana, Dawn, Dusk and Frostreaver
    { 43026, 43027, MELEE, 2 },  // Flame and Stormreaver
    { 42050, 0, MELEE, 3 },      // Windfury Weapon
    { 42048, 0, MELEE, 2 },      // Flametongue Weapon
    { 40945, 0, MELEE, 2 },      // Feral Spirit
    { 40734, 0, MELEE, 1 },      // Shamanistic Rage
    { 41295, 0, MELEE, 1 },      // Sundering
    { 42073, 0, MELEE, 2 },      // Cat Form
    { 40171, 0, MELEE, 3 },      // Shred
    { 50012, 50017, MELEE, 2 },  // Aether, Dusk, Light, Flame, Snow and Stormshred
    { 40114, 0, MELEE, 2 },      // Rake
    { 50006, 50011, MELEE, 1 },  // Aether, Dusk, Dawn, Flame, Frost and Storm Rake
    { 40082, 0, MELEE, 2 },      // Rip
    { 41910, 0, MELEE, 2 },      // Ferocious Bite
    { 40170, 0, MELEE, 2 },      // Tiger's Fury
    { 40929, 0, MELEE, 1 },      // Berserk
    { 40298, 0, MELEE, 1 },      // Swipe (Cat)
    { 41937, 0, MELEE, 2 },      // Obliterate
    { 41987, 0, MELEE, 2 },      // Scourge Strike
    { 41975, 0, MELEE, 2 },      // Frost Strike
    { 41940, 0, MELEE, 1 },      // Death Strike
    { 41924, 0, MELEE, 1 },      // Blood Strike
    { 41920, 0, MELEE, 1 },      // Plague Strike
    // Melee talents.
    { 40363, 0, MELEE, 2 },      // Two-Handed Weapon Specialization (warrior)
    { 40349, 0, MELEE, 2 },      // Deep Wounds
    { 40381, 0, MELEE, 2 },      // Cruelty
    { 40361, 0, MELEE, 1 },      // Weapon Mastery
    { 40869, 0, MELEE, 1 },      // Trauma
    { 40875, 0, MELEE, 2 },      // Bloodsurge
    { 40712, 0, MELEE, 2 },      // Sudden Death
    { 40398, 0, MELEE, 2 },      // Dual Wield Specialization (rogue)
    { 41906, 0, MELEE, 1 },      // Dual Wield Mastery
    { 40850, 0, MELEE, 2 },      // Combat Potency
    { 40415, 0, MELEE, 2 },      // Ruthlessness
    { 41025, 0, MELEE, 2 },      // Relentless Strikes
    { 40412, 0, MELEE, 2 },      // Lethality
    { 40393, 0, MELEE, 1 },      // Improved Sinister Strike
    { 40417, 0, MELEE, 1 },      // Improved Eviscerate
    { 40852, 0, MELEE, 2 },      // Improved Slice and Dice
    { 40423, 0, MELEE, 2 },      // Seal Fate
    { 40413, 0, MELEE, 2 },      // Malice
    { 40645, 0, MELEE, 2 },      // Two-Handed Weapon Specialization (paladin)
    { 40786, 0, MELEE, 2 },      // Crusade
    { 40791, 0, MELEE, 2 },      // Sanctity of Battle
    { 40990, 0, MELEE, 1 },      // The Art of War
    { 40642, 0, MELEE, 2 },      // Vengeance
    { 40790, 0, MELEE, 1 },      // Fanaticism
    { 40944, 0, MELEE, 1 },      // Maelstrom Weapon
    { 40489, 0, MELEE, 2 },      // Flurry
    { 40733, 0, MELEE, 1 },      // Dual Wield Specialization (shaman)
    { 40941, 0, MELEE, 1 },      // Improved Stormstrike
    { 40527, 0, MELEE, 2 },      // Predatory Strikes
    { 40529, 0, MELEE, 1 },      // Savage Fury
    { 40525, 0, MELEE, 1 },      // Primal Fury
    { 40526, 0, MELEE, 1 },      // Shredding Attacks
    { 40923, 0, MELEE, 1 },      // King of the Jungle
    { 41110, 0, MELEE, 1 },      // Two-Handed Weapon Specialization (death knight)
    { 41103, 0, MELEE, 1 },      // Killing Machine
    { 41105, 0, MELEE, 1 },      // Annihilation

    // Ranged weapon damage: Auto Shot first (a Hero shoots only with it), shots, an aspect, a mark.
    { 40309, 0, RANGED, 6 },     // Auto shot
    { 40293, 0, RANGED, 3 },     // Steady Shot
    { 40158, 0, RANGED, 3 },     // Arcane Shot
    { 40619, 0, RANGED, 3 },     // Aimed Shot
    { 40975, 0, RANGED, 3 },     // Chimera Shot
    { 40985, 0, RANGED, 3 },     // Explosive Shot
    { 42054, 0, RANGED, 3 },     // Aspect of the Hawk
    { 40124, 0, RANGED, 2 },     // Serpent Sting
    { 40139, 0, RANGED, 2 },     // Multi-Shot
    { 40287, 0, RANGED, 2 },     // Kill Shot
    { 40613, 0, RANGED, 2 },     // Black Arrow
    { 40159, 0, RANGED, 2 },     // Rapid Fire
    { 40088, 0, RANGED, 2 },     // Hunter's Mark
    { 42068, 0, RANGED, 2 },     // Aspect of the Dragonhawk
    { 42035, 0, RANGED, 2 },     // Glaive Toss
    { 40626, 0, RANGED, 2 },     // Trueshot Aura
    { 41875, 0, RANGED, 1 },     // Barbed Shot
    { 41994, 0, RANGED, 1 },     // Frenzy Shot
    { 40099, 0, RANGED, 1 },     // Volley
    { 40308, 0, RANGED, 1 },     // Tame Beast Abilities
    { 43005, 43007, RANGED, 2 }, // Frost, Holy and Flare Shot
    { 43009, 0, RANGED, 2 },     // Locust Shot
    { 43034, 0, RANGED, 2 },     // Mystic Shot
    { 50036, 50041, RANGED, 2 }, // Arcane Bullet, Voidbound, Lightbound, Flamebound, Frostbound and Stormbound Shot
    { 138012, 138017, RANGED, 2 }, // Blazing, Glacial, Storm, Void, Righteous and Magic Multi-Shot
    { 138018, 138023, RANGED, 3 }, // Flame, Ice, Bolt, Shade, Dawn and Aethersinger Shot
    { 36916, 0, RANGED, 3 },     // Blood Arrow
    { 36917, 0, RANGED, 3 },     // Shadow Shot
    { 36784, 0, RANGED, 2 },     // Compound Shot
    { 37508, 0, RANGED, 2 },     // Big Game Shot
    { 36899, 0, RANGED, 2 },     // Pistol Shot
    { 37164, 37165, RANGED, 2 }, // Arrows of Frost, Arrows of Fire
    { 37642, 0, RANGED, 1 },     // Quick Draw
    { 36901, 0, RANGED, 1 },     // Grape Shot
    { 37187, 0, RANGED, 1 },     // Rain of Arrows
    { 37569, 0, RANGED, 1 },     // Shredding Shot
    { 37094, 0, RANGED, 1 },     // Wailing Arrow
    { 37547, 0, RANGED, 1 },     // Bone Arrow
    { 37550, 0, RANGED, 1 },     // Oil Dipped Arrow
    { 37560, 0, RANGED, 1 },     // Syndicate Sniper Shot
    { 36919, 0, RANGED, 1 },     // Blooming Shot
    { 37063, 0, RANGED, 1 },     // Grove Shot
    { 138216, 0, RANGED, 1 },    // Arrow of the Goddess
    { 138332, 0, RANGED, 1 },    // Crippling Shot
    // Ranged talents.
    { 40618, 0, RANGED, 2 },     // Lethal Shots
    { 40623, 0, RANGED, 2 },     // Mortal Shots
    { 40845, 0, RANGED, 1 },     // Hawk Eye
    { 40629, 0, RANGED, 2 },     // Improved Aspect of the Hawk
    { 40620, 0, RANGED, 1 },     // Improved Arcane Shot
    { 40973, 0, RANGED, 2 },     // Improved Steady Shot
    { 40831, 0, RANGED, 1 },     // Careful Aim
    { 40622, 0, RANGED, 1 },     // Improved Stings
    { 40621, 0, RANGED, 2 },     // Barrage
    { 40627, 0, RANGED, 2 },     // Ranged Weapon Specialization
    { 40983, 0, RANGED, 1 },     // Sniper Training
    { 40836, 0, RANGED, 1 },     // Thrill of the Hunt
    { 40970, 0, RANGED, 2 },     // Piercing Shots
    { 40832, 0, RANGED, 2 },     // Master Marksman
    { 40838, 0, RANGED, 1 },     // Master Tactician
    { 40972, 0, RANGED, 2 },     // Wild Quiver
    { 40607, 0, RANGED, 1 },     // Lock and Load
    { 40844, 0, RANGED, 1 },     // Rapid Killing
    { 40974, 0, RANGED, 2 },     // Marked for Death
    { 40678, 0, RANGED, 2 },     // Lone Wolf
    { 41000, 0, RANGED, 1 },     // Focused Aim

    // Caster damage: a main nuke, a damage over time, an instant, a buff or form for spell damage.
    { 40008, 0, CASTER, 3 },     // Frostbolt
    { 40016, 0, CASTER, 3 },     // Fireball
    { 40255, 0, CASTER, 3 },     // Arcane Blast
    { 40276, 0, CASTER, 3 },     // Frostfire Bolt
    { 40862, 0, CASTER, 3 },     // Living Bomb
    { 40134, 0, CASTER, 2 },     // Fire Blast
    { 40164, 0, CASTER, 2 },     // Arcane Missiles
    { 40316, 0, CASTER, 2 },     // Pyroblast
    { 40857, 0, CASTER, 2 },     // Arcane Barrage
    { 40347, 0, CASTER, 2 },     // Arcane Power
    { 40334, 0, CASTER, 2 },     // Icy Veins
    { 41284, 0, CASTER, 2 },     // Frozen Orb
    { 41286, 0, CASTER, 2 },     // Meteor
    { 42062, 0, CASTER, 2 },     // Molten Armor
    { 40774, 0, CASTER, 2 },     // Summon Water Elemental
    { 40152, 0, CASTER, 1 },     // Scorch
    { 40256, 0, CASTER, 1 },     // Ice Lance
    { 40323, 0, CASTER, 1 },     // Combustion
    { 40346, 0, CASTER, 1 },     // Presence of Mind
    { 40001, 0, CASTER, 1 },     // Blizzard
    { 40133, 0, CASTER, 1 },     // Flamestrike
    { 42044, 0, CASTER, 1 },     // Mage Armor
    { 40291, 0, CASTER, 1 },     // Mirror Image
    { 40094, 0, CASTER, 1 },     // Arcane Intellect
    { 40218, 0, CASTER, 1 },     // Evocation
    { 43025, 0, CASTER, 1 },     // Ray of Frost
    { 40050, 0, CASTER, 3 },     // Shadow Bolt
    { 40019, 0, CASTER, 3 },     // Corruption
    { 40023, 0, CASTER, 3 },     // Immolate
    { 40251, 0, CASTER, 3 },     // Incinerate
    { 40896, 0, CASTER, 3 },     // Chaos Bolt
    { 40719, 0, CASTER, 3 },     // Unstable Affliction
    { 40074, 0, CASTER, 2 },     // Curse of Agony
    { 40559, 0, CASTER, 2 },     // Conflagrate
    { 40934, 0, CASTER, 2 },     // Haunt
    { 40093, 0, CASTER, 2 },     // Life Tap
    { 42061, 0, CASTER, 2 },     // Fel Armor
    { 42111, 0, CASTER, 2 },     // Summon Felguard
    { 40189, 0, CASTER, 1 },     // Soul Fire
    { 40554, 0, CASTER, 1 },     // Shadowburn
    { 40085, 0, CASTER, 1 },     // Drain Soul
    { 40052, 0, CASTER, 1 },     // Drain Life
    { 40097, 0, CASTER, 1 },     // Curse of the Elements
    { 40249, 0, CASTER, 1 },     // Seed of Corruption
    { 40181, 0, CASTER, 1 },     // Rain of Fire
    { 40051, 0, CASTER, 1 },     // Summon Imp
    { 40891, 0, CASTER, 1 },     // Metamorphosis
    { 41964, 0, CASTER, 1 },     // Hand of Gul'dan
    { 40204, 0, CASTER, 3 },     // Mind Blast
    { 40042, 0, CASTER, 3 },     // Shadow Word: Pain
    { 40462, 0, CASTER, 3 },     // Mind Flay
    { 40806, 0, CASTER, 3 },     // Vampiric Touch
    { 40463, 0, CASTER, 3 },     // Shadowform
    { 40151, 0, CASTER, 2 },     // Devouring Plague
    { 41296, 0, CASTER, 2 },     // Void Eruption
    { 40261, 0, CASTER, 1 },     // Shadow Word: Death
    { 40038, 0, CASTER, 1 },     // Smite
    { 40222, 0, CASTER, 1 },     // Holy Fire
    { 41914, 0, CASTER, 1 },     // Mind Sear
    { 40271, 0, CASTER, 1 },     // Shadowfiend
    { 40025, 0, CASTER, 3 },     // Lightning Bolt
    { 41915, 0, CASTER, 3 },     // Lava Burst
    { 40202, 0, CASTER, 3 },     // Flame Shock
    { 40027, 0, CASTER, 2 },     // Chain Lightning
    { 40201, 0, CASTER, 2 },     // Earth Shock
    { 40472, 0, CASTER, 2 },     // Elemental Mastery
    { 42030, 0, CASTER, 2 },     // Elemental Blast
    { 40020, 0, CASTER, 1 },     // Lightning Shield
    { 40940, 0, CASTER, 1 },     // Thunderstorm
    { 42112, 0, CASTER, 1 },     // Totem of Wrath
    { 42079, 0, CASTER, 1 },     // Searing Totem
    { 41282, 0, CASTER, 1 },     // Flame Ascendance
    { 40165, 0, CASTER, 3 },     // Wrath
    { 40150, 0, CASTER, 3 },     // Starfire
    { 40211, 0, CASTER, 3 },     // Moonfire
    { 42031, 0, CASTER, 3 },     // Starsurge
    { 40518, 0, CASTER, 3 },     // Moonkin Form
    { 40514, 0, CASTER, 2 },     // Insect Swarm
    { 40928, 0, CASTER, 2 },     // Starfall
    { 40813, 0, CASTER, 1 },     // Force of Nature
    { 40226, 0, CASTER, 1 },     // Hurricane
    { 40925, 0, CASTER, 1 },     // Typhoon
    { 37416, 0, CASTER, 2 },     // Sunfire
    { 37463, 0, CASTER, 2 },     // Glacial Spike
    { 36516, 0, CASTER, 2 },     // Frost Bomb
    { 37038, 0, CASTER, 2 },     // Cremation
    { 36737, 0, CASTER, 2 },     // Elemental Immolate
    { 36562, 0, CASTER, 2 },     // Frostflame Shock
    { 37506, 0, CASTER, 2 },     // Shadow Waste Bolt
    { 37571, 0, CASTER, 2 },     // Stone Shard
    { 37036, 0, CASTER, 1 },     // Eclipse Blast
    { 36487, 0, CASTER, 1 },     // Astral Storm
    { 36384, 0, CASTER, 1 },     // Eldritch Wrath
    { 37567, 0, CASTER, 1 },     // Shadow Word: Gore
    { 37652, 0, CASTER, 1 },     // Jadefire Corruption
    { 37289, 0, CASTER, 1 },     // Carrion Swarm
    { 37292, 0, CASTER, 1 },     // Flameburst
    { 36455, 0, CASTER, 1 },     // Living Lightning
    { 138320, 0, CASTER, 1 },    // Starcall
    { 138340, 0, CASTER, 1 },    // Ember Touch
    { 138346, 0, CASTER, 1 },    // Corrupt Mind
    // Caster talents.
    { 40313, 0, CASTER, 2 },     // Improved Fireball
    { 40324, 0, CASTER, 2 },     // Improved Frostbolt
    { 40321, 0, CASTER, 2 },     // Ignite
    { 40320, 0, CASTER, 2 },     // Critical Mass
    { 40322, 0, CASTER, 2 },     // Fire Power
    { 40317, 0, CASTER, 1 },     // Impact
    { 40860, 0, CASTER, 2 },     // Hot Streak
    { 40332, 0, CASTER, 1 },     // Shatter
    { 40338, 0, CASTER, 2 },     // Ice Shards
    { 40326, 0, CASTER, 2 },     // Piercing Ice
    { 40863, 0, CASTER, 1 },     // Fingers of Frost
    { 40864, 0, CASTER, 2 },     // Brain Freeze
    { 40340, 0, CASTER, 1 },     // Arcane Concentration
    { 40341, 0, CASTER, 1 },     // Arcane Mind
    { 40759, 0, CASTER, 1 },     // Arcane Potency
    { 41006, 0, CASTER, 2 },     // Missile Barrage
    { 40761, 0, CASTER, 1 },     // Arcane Empowerment
    { 40851, 0, CASTER, 2 },     // Spell Power
    { 40552, 0, CASTER, 2 },     // Improved Shadow Bolt
    { 40560, 0, CASTER, 2 },     // Devastation
    { 40558, 0, CASTER, 2 },     // Ruin
    { 40575, 0, CASTER, 2 },     // Shadow Mastery
    { 40557, 0, CASTER, 2 },     // Emberstorm
    { 40567, 0, CASTER, 2 },     // Improved Corruption
    { 40551, 0, CASTER, 2 },     // Bane
    { 40566, 0, CASTER, 1 },     // Nightfall
    { 40885, 0, CASTER, 1 },     // Eradication
    { 40883, 0, CASTER, 1 },     // Death's Embrace
    { 40893, 0, CASTER, 1 },     // Backdraft
    { 40564, 0, CASTER, 1 },     // Pyroclasm
    { 40602, 0, CASTER, 1 },     // Molten Core
    { 40596, 0, CASTER, 1 },     // Master Demonologist
    { 40723, 0, CASTER, 1 },     // Shadow and Flame
    { 40553, 0, CASTER, 1 },     // Improved Immolate
    { 40453, 0, CASTER, 2 },     // Shadow Weaving
    { 40454, 0, CASTER, 2 },     // Darkness
    { 40805, 0, CASTER, 2 },     // Shadow Power
    { 40909, 0, CASTER, 2 },     // Twisted Faith
    { 40455, 0, CASTER, 1 },     // Shadow Focus
    { 40458, 0, CASTER, 1 },     // Improved Mind Blast
    { 40459, 0, CASTER, 1 },     // Improved Shadow Word: Pain
    { 40841, 0, CASTER, 1 },     // Misery
    { 40908, 0, CASTER, 1 },     // Improved Shadowform
    { 40807, 0, CASTER, 1 },     // Mind Melt
    { 40468, 0, CASTER, 2 },     // Concussion
    { 40467, 0, CASTER, 2 },     // Call of Thunder
    { 40470, 0, CASTER, 2 },     // Elemental Fury
    { 40729, 0, CASTER, 2 },     // Lightning Overload
    { 40506, 0, CASTER, 2 },     // Lightning Mastery
    { 40469, 0, CASTER, 1 },     // Convection
    { 40466, 0, CASTER, 1 },     // Call of Flame
    { 40938, 0, CASTER, 1 },     // Lava Flows
    { 40728, 0, CASTER, 1 },     // Elemental Precision
    { 40936, 0, CASTER, 1 },     // Elemental Oath
    { 41848, 0, CASTER, 1 },     // Improved Lightning Bolt
    { 40508, 0, CASTER, 2 },     // Starlight Wrath
    { 40516, 0, CASTER, 2 },     // Moonfury
    { 40517, 0, CASTER, 2 },     // Vengeance
    { 40926, 0, CASTER, 2 },     // Eclipse
    { 40812, 0, CASTER, 2 },     // Wrath of Cenarius
    { 40512, 0, CASTER, 1 },     // Moonglow
    { 40515, 0, CASTER, 1 },     // Nature's Grace
    { 40809, 0, CASTER, 1 },     // Balance of Power
    { 40914, 0, CASTER, 1 },     // Improved Moonkin Form
    { 40930, 0, CASTER, 1 },     // Earth and Moon

    // Utility no bot casts in a fight, rerolled first by every archetype.
    { 40040, 0, ANY, -3 },       // Conjure Food
    { 40178, 0, ANY, -3 },       // Conjure Water
    { 40061, 0, ANY, -3 },       // Conjure Mana Gem
    { 40274, 0, ANY, -3 },       // Conjure Refreshment
    { 40275, 0, ANY, -3 },       // Ritual of Refreshment
    { 40072, 0, ANY, -3 },       // Pick Pocket
    { 40113, 0, ANY, -3 },       // Pick Lock
    { 40145, 0, ANY, -3 },       // Detect Traps
    { 40116, 0, ANY, -3 },       // Disarm Trap
    { 40108, 0, ANY, -3 },       // Distract
    { 41124, 0, ANY, -3 },       // Track Beasts
    { 40234, 0, ANY, -3 },       // Track Humanoids
    { 40190, 0, ANY, -3 },       // Track Humanoids (Cat Form)
    { 41131, 0, ANY, -3 },       // Track Undead
    { 40235, 0, ANY, -3 },       // Track Hidden
    { 41129, 0, ANY, -3 },       // Track Elementals
    { 41127, 0, ANY, -3 },       // Track Demons
    { 41130, 0, ANY, -3 },       // Track Giants
    { 41128, 0, ANY, -3 },       // Track Dragonkin
    { 40229, 0, ANY, -3 },       // Teleport: Moonglade
    { 40037, 0, ANY, -3 },       // Astral Recall
    { 41909, 0, ANY, -3 },       // Death Gate
    { 40055, 0, ANY, -3 },       // Ritual of Summoning
    { 40253, 0, ANY, -3 },       // Ritual of Souls
    { 40228, 0, ANY, -3 },       // Ritual of Doom
    { 40131, 0, ANY, -3 },       // Mind Vision
    { 40012, 0, ANY, -3 },       // Eye of Kilrogg
    { 40075, 0, ANY, -3 },       // Eyes of the Beast
    { 40104, 0, ANY, -3 },       // Levitate
    { 40013, 0, ANY, -3 },       // Slow Fall
    { 40035, 0, ANY, -3 },       // Water Walking
    { 40014, 0, ANY, -3 },       // Water Breathing
    { 40180, 0, ANY, -3 },       // Unending Breath
    { 40015, 0, ANY, -3 },       // Detect Invisibility
    { 40028, 0, ANY, -3 },       // Mind Soothe
    { 40149, 0, ANY, -3 },       // Soothe Animal
    { 40081, 0, ANY, -3 },       // Aquatic Form
    { 40268, 0, ANY, -3 },       // Flight Form
};

// The builds of each archetype: the Hero Archetypes the Season 10 client ships for its character creation
// (CharacterCreationArchetypes.dbc, the four or five abilities each one shows), and the Wildcard builds players
// published (Plague Caster, Dark Ranger, Arcane Gunslinger, Fel Infused Blade). A build adds its weight to the
// archetype's: 4 for an ability or talent it is made of, 2 or 3 for one that feeds it. Builds that need a pet the bot
// does not tame (Bestial Ranger) or a demon (the Demoniac's Enslave Demon and Soul Link) are left out of them.
struct Build
{
    char const* name;
    WildcardArchetype archetype;
};

struct BuildPick
{
    uint8 build;
    uint32 entry;
    int8 weight;
};

enum BuildId : uint8
{
    BUILD_BLADE_WARDEN,
    BUILD_BULWARK,
    BUILD_SWASHBUCKLER,
    BUILD_RELENTLESS,
    BUILD_VANGUARD_SENTINEL,
    BUILD_CARNAGE_INCARNATE,
    BUILD_TEMPLAR_OF_FLAME,
    BUILD_DEMONIAC,
    BUILD_MYSTIC_KNIGHT,
    BUILD_CRIMSON_CHAMPION,
    BUILD_STORMGUARD,
    BUILD_ICEBOUND,
    BUILD_PLAGUE_BEARER,
    BUILD_BEAST_OF_NIGHTMARE,
    BUILD_THUNDERPAW,
    BUILD_NATURALIST,
    BUILD_DAWNKEEPER,
    BUILD_ETERNAL_CARETAKER,
    BUILD_SACRED_SENTINEL,
    BUILD_TIDE_MENDER,
    BUILD_GROVE_KEEPER,
    BUILD_HIEROPHANT,
    BUILD_STEWARD_OF_ETERNITY,
    BUILD_CRYOTHERAPIST,
    BUILD_TREE_OF_WRATH,
    BUILD_GROVE_RANGER,
    BUILD_FIRE_SAGE,
    BUILD_VAMPIRE_LORD,
    BUILD_HOPLITE,
    BUILD_WARDEN,
    BUILD_HEAVYWEIGHT,
    BUILD_DRAGON_WARRIOR,
    BUILD_SON_OF_GRUUL,
    BUILD_DEFIAS_GUNSLINGER,
    BUILD_GUTRIPPER,
    BUILD_WIND_RAGER,
    BUILD_MASTER_OF_SHADOWS,
    BUILD_DRUID_OF_THE_FANG,
    BUILD_CRUSADER,
    BUILD_DIVINE_SENTINEL,
    BUILD_HELLCAT,
    BUILD_STORM_GIANT,
    BUILD_FIRELORD,
    BUILD_FEL_BLADE,
    BUILD_MARKSMAN,
    BUILD_PARATROOPER,
    BUILD_LOCUST_RANGER,
    BUILD_DARK_RANGER,
    BUILD_ARCANE_GUNSLINGER,
    BUILD_EVOKER,
    BUILD_DRUID_OF_THE_ECLIPSE,
    BUILD_SHADOWSEER,
    BUILD_CRYOMANCER,
    BUILD_ASCENDANT,
    BUILD_STORMCALLER,
    BUILD_FEL_APOSTATE,
    BUILD_WHITE_WALKER,
    BUILD_PLAGUE_CASTER,
    BUILD_COUNT
};

constexpr Build BUILDS[BUILD_COUNT] = {
    { "Blade Warden", WildcardArchetype::Tank },
    { "Bulwark", WildcardArchetype::Tank },
    { "Swashbuckler", WildcardArchetype::Tank },
    { "Relentless", WildcardArchetype::Tank },
    { "Vanguard Sentinel", WildcardArchetype::Tank },
    { "Carnage Incarnate", WildcardArchetype::Tank },
    { "Templar of Flame", WildcardArchetype::Tank },
    { "Demoniac", WildcardArchetype::Tank },
    { "Mystic Knight", WildcardArchetype::Tank },
    { "Crimson Champion", WildcardArchetype::Tank },
    { "Stormguard", WildcardArchetype::Tank },
    { "Icebound", WildcardArchetype::Tank },
    { "Plague Bearer", WildcardArchetype::Tank },
    { "Beast of Nightmare", WildcardArchetype::Tank },
    { "Thunderpaw", WildcardArchetype::Tank },
    { "Naturalist", WildcardArchetype::Heal },
    { "Dawnkeeper", WildcardArchetype::Heal },
    { "Eternal Caretaker", WildcardArchetype::Heal },
    { "Sacred Sentinel", WildcardArchetype::Heal },
    { "Tide Mender", WildcardArchetype::Heal },
    { "Grove Keeper", WildcardArchetype::Heal },
    { "Hierophant", WildcardArchetype::Heal },
    { "Steward of Eternity", WildcardArchetype::Heal },
    { "Cryotherapist", WildcardArchetype::Heal },
    { "Tree of Wrath", WildcardArchetype::Heal },
    { "Grove Ranger", WildcardArchetype::Heal },
    { "Fire Sage", WildcardArchetype::Heal },
    { "Vampire Lord", WildcardArchetype::Heal },
    { "Hoplite", WildcardArchetype::Melee },
    { "Warden", WildcardArchetype::Melee },
    { "Heavyweight", WildcardArchetype::Melee },
    { "Dragon Warrior", WildcardArchetype::Melee },
    { "Son of Gruul", WildcardArchetype::Melee },
    { "Defias Gunslinger", WildcardArchetype::Melee },
    { "Gutripper", WildcardArchetype::Melee },
    { "Wind Rager", WildcardArchetype::Melee },
    { "Master of Shadows", WildcardArchetype::Melee },
    { "Druid of the Fang", WildcardArchetype::Melee },
    { "Crusader", WildcardArchetype::Melee },
    { "Divine Sentinel", WildcardArchetype::Melee },
    { "Hellcat", WildcardArchetype::Melee },
    { "Storm Giant", WildcardArchetype::Melee },
    { "Firelord", WildcardArchetype::Melee },
    { "Fel Infused Blade", WildcardArchetype::Melee },
    { "Marksman", WildcardArchetype::Ranged },
    { "Paratrooper", WildcardArchetype::Ranged },
    { "Locust Ranger", WildcardArchetype::Ranged },
    { "Dark Ranger", WildcardArchetype::Ranged },
    { "Arcane Gunslinger", WildcardArchetype::Ranged },
    { "Evoker", WildcardArchetype::Caster },
    { "Druid of the Eclipse", WildcardArchetype::Caster },
    { "Shadowseer", WildcardArchetype::Caster },
    { "Cryomancer", WildcardArchetype::Caster },
    { "Ascendant", WildcardArchetype::Caster },
    { "Stormcaller", WildcardArchetype::Caster },
    { "Fel Apostate", WildcardArchetype::Caster },
    { "White Walker", WildcardArchetype::Caster },
    { "Plague Caster", WildcardArchetype::Caster },
};

constexpr BuildPick BUILD_PICKS[] = {
    // Blade Warden (S10 Hero Archetype)
    { BUILD_BLADE_WARDEN, 40715, 4 },               // Devastate
    { BUILD_BLADE_WARDEN, 40989, 4 },               // Divine Storm
    { BUILD_BLADE_WARDEN, 40426, 4 },               // Ghostly Strike
    { BUILD_BLADE_WARDEN, 40425, 4 },               // Riposte
    // Bulwark (S10 Hero Archetype)
    { BUILD_BULWARK, 36772, 4 },                    // Bulwark
    { BUILD_BULWARK, 40187, 4 },                    // Thunder Clap
    { BUILD_BULWARK, 40245, 4 },                    // Shield Slam
    { BUILD_BULWARK, 40881, 4 },                    // Shockwave
    // Swashbuckler (S10 Hero Archetype)
    { BUILD_SWASHBUCKLER, 36931, 4 },               // Panache
    { BUILD_SWASHBUCKLER, 40187, 4 },               // Thunder Clap
    { BUILD_SWASHBUCKLER, 36929, 4 },               // Hammer the Gap
    { BUILD_SWASHBUCKLER, 37152, 3 },               // Swashbuckler: Matched Blows
    { BUILD_SWASHBUCKLER, 37153, 3 },               // Swashbuckler: Flurry of Blows
    { BUILD_SWASHBUCKLER, 37200, 3 },               // Swashbuckler: Shocktrooper
    { BUILD_SWASHBUCKLER, 37573, 3 },               // Swashbuckler: Bloodsail
    // Relentless (S10 Hero Archetype)
    { BUILD_RELENTLESS, 40132, 4 },                 // Eviscerate
    { BUILD_RELENTLESS, 42095, 4 },                 // Expose Armor
    { BUILD_RELENTLESS, 40503, 4 },                 // Hemorrhage
    { BUILD_RELENTLESS, 40245, 4 },                 // Shield Slam
    // Vanguard Sentinel (S10 Hero Archetype)
    { BUILD_VANGUARD_SENTINEL, 36429, 4 },          // Battle Charge
    { BUILD_VANGUARD_SENTINEL, 40187, 4 },          // Thunder Clap
    { BUILD_VANGUARD_SENTINEL, 40270, 4 },          // Victory Rush
    { BUILD_VANGUARD_SENTINEL, 40426, 4 },          // Ghostly Strike
    // Carnage Incarnate (S10 Hero Archetype)
    { BUILD_CARNAGE_INCARNATE, 36853, 4 },          // Carnage Incarnate
    { BUILD_CARNAGE_INCARNATE, 40196, 4 },          // Maul
    { BUILD_CARNAGE_INCARNATE, 40265, 4 },          // Lacerate
    { BUILD_CARNAGE_INCARNATE, 42083, 3 },          // Bear Form
    // Templar of Flame (S10 Hero Archetype)
    { BUILD_TEMPLAR_OF_FLAME, 41990, 4 },           // Mana-forged Barrier
    { BUILD_TEMPLAR_OF_FLAME, 36528, 4 },           // Seal of Fervor
    { BUILD_TEMPLAR_OF_FLAME, 41991, 4 },           // Invocation
    { BUILD_TEMPLAR_OF_FLAME, 40133, 4 },           // Flamestrike
    { BUILD_TEMPLAR_OF_FLAME, 36532, 4 },           // Templar of Flame
    { BUILD_TEMPLAR_OF_FLAME, 42067, 3 },           // Judgement of Wisdom
    // Demoniac (S10 Hero Archetype)
    { BUILD_DEMONIAC, 38634, 4 },                   // Dark Apotheosis
    { BUILD_DEMONIAC, 40251, 4 },                   // Incinerate
    // Mystic Knight (S10 Hero Archetype)
    { BUILD_MYSTIC_KNIGHT, 41990, 4 },              // Mana-forged Barrier
    { BUILD_MYSTIC_KNIGHT, 36384, 4 },              // Eldritch Wrath
    { BUILD_MYSTIC_KNIGHT, 37343, 4 },              // Deconstruct
    { BUILD_MYSTIC_KNIGHT, 36383, 2 },              // Eldritch Weapon
    // Crimson Champion (S10 Hero Archetype)
    { BUILD_CRIMSON_CHAMPION, 40288, 4 },           // Shield of Righteousness
    { BUILD_CRIMSON_CHAMPION, 42024, 4 },           // Light's Hammer
    { BUILD_CRIMSON_CHAMPION, 40002, 4 },           // Power Word: Shield
    { BUILD_CRIMSON_CHAMPION, 40999, 4 },           // Hammer of the Righteous
    // Stormguard (S10 Hero Archetype)
    { BUILD_STORMGUARD, 40187, 4 },                 // Thunder Clap
    { BUILD_STORMGUARD, 40025, 4 },                 // Lightning Bolt
    { BUILD_STORMGUARD, 36667, 4 },                 // Stormhammer
    { BUILD_STORMGUARD, 40027, 4 },                 // Chain Lightning
    // Icebound (S10 Hero Archetype)
    { BUILD_ICEBOUND, 41990, 4 },                   // Mana-forged Barrier
    { BUILD_ICEBOUND, 41991, 4 },                   // Invocation
    { BUILD_ICEBOUND, 40336, 4 },                   // Ice Barrier
    // Plague Bearer (S10 Hero Archetype)
    { BUILD_PLAGUE_BEARER, 40822, 4 },              // Mangle
    { BUILD_PLAGUE_BEARER, 40243, 4 },              // Frenzied Regeneration
    { BUILD_PLAGUE_BEARER, 41291, 4 },              // Ironfur
    { BUILD_PLAGUE_BEARER, 36403, 4 },              // Plague Bearer
    { BUILD_PLAGUE_BEARER, 42083, 3 },              // Bear Form
    // Beast of Nightmare (S10 Hero Archetype)
    { BUILD_BEAST_OF_NIGHTMARE, 36472, 4 },         // Corrupted Bear Form
    { BUILD_BEAST_OF_NIGHTMARE, 40019, 4 },         // Corruption
    { BUILD_BEAST_OF_NIGHTMARE, 40050, 4 },         // Shadow Bolt
    { BUILD_BEAST_OF_NIGHTMARE, 40822, 4 },         // Mangle
    { BUILD_BEAST_OF_NIGHTMARE, 42083, 3 },         // Bear Form
    // Thunderpaw (S10 Hero Archetype)
    { BUILD_THUNDERPAW, 36935, 4 },                 // Thunder Hide
    { BUILD_THUNDERPAW, 36934, 4 },                 // Storm Smash
    { BUILD_THUNDERPAW, 36936, 4 },                 // Voltaic Bite
    { BUILD_THUNDERPAW, 40027, 4 },                 // Chain Lightning
    // Naturalist (S10 Hero Archetype)
    { BUILD_NATURALIST, 40064, 4 },                 // Rejuvenation
    { BUILD_NATURALIST, 40212, 4 },                 // Regrowth
    { BUILD_NATURALIST, 40200, 4 },                 // Lesser Healing Wave
    { BUILD_NATURALIST, 40919, 4 },                 // Wild Growth
    // Dawnkeeper (S10 Hero Archetype)
    { BUILD_DAWNKEEPER, 40047, 4 },                 // Holy Light
    { BUILD_DAWNKEEPER, 40995, 4 },                 // Beacon of Light
    { BUILD_DAWNKEEPER, 40043, 4 },                 // Prayer of Healing
    // Eternal Caretaker (S10 Hero Archetype)
    { BUILD_ETERNAL_CARETAKER, 40127, 4 },          // Greater Heal
    { BUILD_ETERNAL_CARETAKER, 40128, 4 },          // Flash Heal
    { BUILD_ETERNAL_CARETAKER, 40840, 4 },          // Circle of Healing
    { BUILD_ETERNAL_CARETAKER, 42019, 3 },          // Halo
    // Sacred Sentinel (S10 Hero Archetype)
    { BUILD_SACRED_SENTINEL, 40047, 4 },            // Holy Light
    { BUILD_SACRED_SENTINEL, 36857, 4 },            // Prayer of Preservation
    { BUILD_SACRED_SENTINEL, 36858, 4 },            // Preservation
    { BUILD_SACRED_SENTINEL, 40669, 4 },            // Holy Shock
    // Tide Mender (S10 Hero Archetype)
    { BUILD_TIDE_MENDER, 40021, 4 },                // Healing Wave
    { BUILD_TIDE_MENDER, 40950, 4 },                // Riptide
    { BUILD_TIDE_MENDER, 40080, 4 },                // Chain Heal
    { BUILD_TIDE_MENDER, 40738, 4 },                // Earth Shield
    // Grove Keeper (S10 Hero Archetype)
    { BUILD_GROVE_KEEPER, 40166, 4 },               // Healing Touch
    { BUILD_GROVE_KEEPER, 40059, 4 },               // Tranquility
    { BUILD_GROVE_KEEPER, 40080, 4 },               // Chain Heal
    { BUILD_GROVE_KEEPER, 40738, 4 },               // Earth Shield
    // Hierophant (S10 Hero Archetype)
    { BUILD_HIEROPHANT, 40127, 4 },                 // Greater Heal
    { BUILD_HIEROPHANT, 40018, 4 },                 // Renew
    { BUILD_HIEROPHANT, 40128, 4 },                 // Flash Heal
    { BUILD_HIEROPHANT, 40901, 4 },                 // Penance
    // Steward of Eternity (S10 Hero Archetype)
    { BUILD_STEWARD_OF_ETERNITY, 36865, 4 },        // Align
    { BUILD_STEWARD_OF_ETERNITY, 36866, 4 },        // Time-bind
    { BUILD_STEWARD_OF_ETERNITY, 36867, 4 },        // Synchronize
    { BUILD_STEWARD_OF_ETERNITY, 36868, 4 },        // Temporal Distortion
    // Cryotherapist (S10 Hero Archetype)
    { BUILD_CRYOTHERAPIST, 40008, 4 },              // Frostbolt
    { BUILD_CRYOTHERAPIST, 40127, 4 },              // Greater Heal
    { BUILD_CRYOTHERAPIST, 36468, 4 },              // Icy Penance
    { BUILD_CRYOTHERAPIST, 40043, 4 },              // Prayer of Healing
    { BUILD_CRYOTHERAPIST, 36463, 4 },              // Cryotherapist
    { BUILD_CRYOTHERAPIST, 40256, 2 },              // Ice Lance
    { BUILD_CRYOTHERAPIST, 40276, 2 },              // Frostfire Bolt
    // Tree of Wrath (S10 Hero Archetype)
    { BUILD_TREE_OF_WRATH, 40025, 4 },              // Lightning Bolt
    { BUILD_TREE_OF_WRATH, 40021, 4 },              // Healing Wave
    { BUILD_TREE_OF_WRATH, 36454, 4 },              // Tree of Wrath
    { BUILD_TREE_OF_WRATH, 36455, 4 },              // Living Lightning
    // Grove Ranger (S10 Hero Archetype)
    { BUILD_GROVE_RANGER, 36919, 4 },               // Blooming Shot
    { BUILD_GROVE_RANGER, 40166, 4 },               // Healing Touch
    { BUILD_GROVE_RANGER, 36920, 4 },               // Medicinal Sting
    { BUILD_GROVE_RANGER, 40919, 4 },               // Wild Growth
    { BUILD_GROVE_RANGER, 36922, 4 },               // Grove Ranger
    { BUILD_GROVE_RANGER, 36923, 3 },               // Grove Ranger's Agility
    // Fire Sage (S10 Hero Archetype)
    { BUILD_FIRE_SAGE, 36459, 4 },                  // Cauterizing Fire
    { BUILD_FIRE_SAGE, 40133, 4 },                  // Flamestrike
    { BUILD_FIRE_SAGE, 40222, 4 },                  // Holy Fire
    { BUILD_FIRE_SAGE, 40840, 4 },                  // Circle of Healing
    // Vampire Lord (S10 Hero Archetype)
    { BUILD_VAMPIRE_LORD, 36926, 4 },               // Beacon of Shadow
    { BUILD_VAMPIRE_LORD, 37506, 4 },               // Shadow Waste Bolt
    { BUILD_VAMPIRE_LORD, 36928, 4 },               // Vampire Lord
    { BUILD_VAMPIRE_LORD, 220, 3 },                 // Shadow Mend
    { BUILD_VAMPIRE_LORD, 40462, 2 },               // Mind Flay
    { BUILD_VAMPIRE_LORD, 40042, 2 },               // Shadow Word: Pain
    // Hoplite (S10 Hero Archetype)
    { BUILD_HOPLITE, 36695, 4 },                    // Thrust
    { BUILD_HOPLITE, 36696, 4 },                    // Javelin
    { BUILD_HOPLITE, 41948, 4 },                    // Colossus Smash
    { BUILD_HOPLITE, 36692, 4 },                    // Shield Strike
    // Warden (S10 Hero Archetype)
    { BUILD_WARDEN, 40098, 4 },                     // Mongoose Bite
    { BUILD_WARDEN, 40153, 4 },                     // Raptor Strike
    { BUILD_WARDEN, 42035, 4 },                     // Glaive Toss
    { BUILD_WARDEN, 36557, 4 },                     // Savage Blade
    // Heavyweight (S10 Hero Archetype)
    { BUILD_HEAVYWEIGHT, 40063, 4 },                // Rend
    { BUILD_HEAVYWEIGHT, 40006, 4 },                // Heroic Strike
    { BUILD_HEAVYWEIGHT, 40199, 4 },                // Overpower
    { BUILD_HEAVYWEIGHT, 40362, 4 },                // Mortal Strike
    { BUILD_HEAVYWEIGHT, 36624, 4 },                // Heavyweight
    { BUILD_HEAVYWEIGHT, 40096, 2 },                // Slam
    { BUILD_HEAVYWEIGHT, 40175, 2 },                // Execute
    // Dragon Warrior (S10 Hero Archetype)
    { BUILD_DRAGON_WARRIOR, 40096, 4 },             // Slam
    { BUILD_DRAGON_WARRIOR, 36511, 4 },             // Dragon Roar
    { BUILD_DRAGON_WARRIOR, 36512, 4 },             // Dragon Charge
    { BUILD_DRAGON_WARRIOR, 40362, 4 },             // Mortal Strike
    { BUILD_DRAGON_WARRIOR, 36513, 4 },             // Dragon Warrior
    // Son of Gruul (S10 Hero Archetype)
    { BUILD_SON_OF_GRUUL, 40096, 4 },               // Slam
    { BUILD_SON_OF_GRUUL, 36850, 4 },               // Upheaval
    { BUILD_SON_OF_GRUUL, 36849, 4 },               // Ground Slam
    // Defias Gunslinger (S10 Hero Archetype)
    { BUILD_DEFIAS_GUNSLINGER, 36899, 4 },          // Pistol Shot
    { BUILD_DEFIAS_GUNSLINGER, 40153, 4 },          // Raptor Strike
    { BUILD_DEFIAS_GUNSLINGER, 40098, 4 },          // Mongoose Bite
    { BUILD_DEFIAS_GUNSLINGER, 36904, 4 },          // Defias Gunslinger
    { BUILD_DEFIAS_GUNSLINGER, 40109, 3 },          // Sinister Strike
    // Gutripper (S10 Hero Archetype)
    { BUILD_GUTRIPPER, 36660, 4 },                  // Disembowel
    { BUILD_GUTRIPPER, 40754, 4 },                  // Mutilate
    { BUILD_GUTRIPPER, 40175, 4 },                  // Execute
    // Wind Rager (S10 Hero Archetype)
    { BUILD_WIND_RAGER, 40103, 4 },                 // Whirlwind
    { BUILD_WIND_RAGER, 42050, 4 },                 // Windfury Weapon
    { BUILD_WIND_RAGER, 36739, 4 },                 // Wind Rager
    { BUILD_WIND_RAGER, 40549, 4 },                 // Stormstrike
    // Master of Shadows (S10 Hero Archetype)
    { BUILD_MASTER_OF_SHADOWS, 36796, 4 },          // Master of Shadows
    { BUILD_MASTER_OF_SHADOWS, 40109, 4 },          // Sinister Strike
    { BUILD_MASTER_OF_SHADOWS, 40132, 4 },          // Eviscerate
    { BUILD_MASTER_OF_SHADOWS, 41292, 4 },          // Gloomblade
    { BUILD_MASTER_OF_SHADOWS, 40210, 2 },          // Ambush
    // Druid of the Fang (S10 Hero Archetype)
    { BUILD_DRUID_OF_THE_FANG, 36805, 4 },          // Serpent Form
    { BUILD_DRUID_OF_THE_FANG, 36804, 4 },          // Serpent Strike
    { BUILD_DRUID_OF_THE_FANG, 42435, 4 },          // Poison Mastery
    { BUILD_DRUID_OF_THE_FANG, 36806, 4 },          // Serpent Venom
    // Crusader (S10 Hero Archetype)
    { BUILD_CRUSADER, 42067, 4 },                   // Judgement of Wisdom
    { BUILD_CRUSADER, 40848, 4 },                   // Crusader Strike
    { BUILD_CRUSADER, 40989, 4 },                   // Divine Storm
    { BUILD_CRUSADER, 42110, 4 },                   // Seal of Command
    { BUILD_CRUSADER, 42025, 4 },                   // Execution Sentence
    // Divine Sentinel (S10 Hero Archetype)
    { BUILD_DIVINE_SENTINEL, 36786, 4 },            // One With The Light
    { BUILD_DIVINE_SENTINEL, 40848, 4 },            // Crusader Strike
    { BUILD_DIVINE_SENTINEL, 40989, 4 },            // Divine Storm
    { BUILD_DIVINE_SENTINEL, 40222, 4 },            // Holy Fire
    // Hellcat (S10 Hero Archetype)
    { BUILD_HELLCAT, 42073, 4 },                    // Cat Form
    { BUILD_HELLCAT, 36702, 4 },                    // Devil's Ignition
    { BUILD_HELLCAT, 40179, 4 },                    // Searing Pain
    // Storm Giant (S10 Hero Archetype)
    { BUILD_STORM_GIANT, 40187, 4 },                // Thunder Clap
    { BUILD_STORM_GIANT, 40025, 4 },                // Lightning Bolt
    { BUILD_STORM_GIANT, 36761, 4 },                // Lightning Blade
    { BUILD_STORM_GIANT, 36762, 4 },                // Thunder Slam
    { BUILD_STORM_GIANT, 40027, 4 },                // Chain Lightning
    // Firelord (S10 Hero Archetype)
    { BUILD_FIRELORD, 36483, 4 },                   // Lava Sweep
    { BUILD_FIRELORD, 40134, 4 },                   // Fire Blast
    { BUILD_FIRELORD, 42048, 4 },                   // Flametongue Weapon
    { BUILD_FIRELORD, 41295, 4 },                   // Sundering
    { BUILD_FIRELORD, 40316, 4 },                   // Pyroblast
    // Fel Infused Blade (community build)
    { BUILD_FEL_BLADE, 36845, 4 },                  // Fel Infused Weapon
    { BUILD_FEL_BLADE, 36798, 4 },                  // Dark Intent
    { BUILD_FEL_BLADE, 40109, 3 },                  // Sinister Strike
    { BUILD_FEL_BLADE, 40132, 3 },                  // Eviscerate
    // Marksman (S10 Hero Archetype)
    { BUILD_MARKSMAN, 36782, 4 },                   // Compound Power
    { BUILD_MARKSMAN, 40293, 4 },                   // Steady Shot
    { BUILD_MARKSMAN, 42081, 4 },                   // Slice and Dice
    { BUILD_MARKSMAN, 40158, 4 },                   // Arcane Shot
    { BUILD_MARKSMAN, 36784, 4 },                   // Compound Shot
    // Paratrooper (S10 Hero Archetype)
    { BUILD_PARATROOPER, 40158, 4 },                // Arcane Shot
    { BUILD_PARATROOPER, 37450, 4 },                // Focused Burst
    { BUILD_PARATROOPER, 40619, 4 },                // Aimed Shot
    // Locust Ranger (S10 Hero Archetype)
    { BUILD_LOCUST_RANGER, 43009, 4 },              // Locust Shot
    { BUILD_LOCUST_RANGER, 40124, 4 },              // Serpent Sting
    { BUILD_LOCUST_RANGER, 40165, 4 },              // Wrath
    { BUILD_LOCUST_RANGER, 40975, 4 },              // Chimera Shot
    { BUILD_LOCUST_RANGER, 36501, 4 },              // Locust Ranger
    { BUILD_LOCUST_RANGER, 40293, 3 },              // Steady Shot
    { BUILD_LOCUST_RANGER, 40972, 3 },              // Wild Quiver
    // Dark Ranger (S10 Hero Archetype and community build)
    { BUILD_DARK_RANGER, 36916, 4 },                // Blood Arrow
    { BUILD_DARK_RANGER, 40050, 4 },                // Shadow Bolt
    { BUILD_DARK_RANGER, 36918, 4 },                // Shadow Artillery
    { BUILD_DARK_RANGER, 36917, 4 },                // Shadow Shot
    { BUILD_DARK_RANGER, 36915, 4 },                // Dark Arrows
    { BUILD_DARK_RANGER, 40613, 3 },                // Black Arrow
    { BUILD_DARK_RANGER, 41932, 3 },                // Blood Presence
    { BUILD_DARK_RANGER, 40019, 3 },                // Corruption
    { BUILD_DARK_RANGER, 40093, 3 },                // Life Tap
    { BUILD_DARK_RANGER, 37317, 3 },                // Wave of Despair
    { BUILD_DARK_RANGER, 37428, 3 },                // Growing Corruption
    { BUILD_DARK_RANGER, 41053, 3 },                // Improved Blood Presence
    { BUILD_DARK_RANGER, 41919, 2 },                // Death and Decay
    { BUILD_DARK_RANGER, 41847, 2 },                // Answered Prayers
    { BUILD_DARK_RANGER, 41017, 2 },                // Unending Fury
    { BUILD_DARK_RANGER, 40696, 2 },                // Enhanced Weapon Mastery
    { BUILD_DARK_RANGER, 40463, 2 },                // Shadowform
    { BUILD_DARK_RANGER, 36834, 2 },                // Biting Drain
    { BUILD_DARK_RANGER, 37684, 2 },                // Shadow Infusion
    { BUILD_DARK_RANGER, 40566, 2 },                // Nightfall
    { BUILD_DARK_RANGER, 36831, 2 },                // Necromancer's Touch
    { BUILD_DARK_RANGER, 40934, 2 },                // Haunt
    // Arcane Gunslinger (community build)
    { BUILD_ARCANE_GUNSLINGER, 36722, 4 },          // Arcane Gunslinger
    { BUILD_ARCANE_GUNSLINGER, 40158, 4 },          // Arcane Shot
    // Evoker (S10 Hero Archetype)
    { BUILD_EVOKER, 40008, 4 },                     // Frostbolt
    { BUILD_EVOKER, 40016, 4 },                     // Fireball
    { BUILD_EVOKER, 40164, 4 },                     // Arcane Missiles
    { BUILD_EVOKER, 40864, 4 },                     // Brain Freeze
    { BUILD_EVOKER, 41006, 4 },                     // Missile Barrage
    { BUILD_EVOKER, 36495, 4 },                     // Evoker
    { BUILD_EVOKER, 36497, 3 },                     // Spellstorm
    { BUILD_EVOKER, 40276, 2 },                     // Frostfire Bolt
    // Druid of the Eclipse (S10 Hero Archetype)
    { BUILD_DRUID_OF_THE_ECLIPSE, 40165, 4 },       // Wrath
    { BUILD_DRUID_OF_THE_ECLIPSE, 37416, 4 },       // Sunfire
    { BUILD_DRUID_OF_THE_ECLIPSE, 40150, 4 },       // Starfire
    { BUILD_DRUID_OF_THE_ECLIPSE, 40926, 4 },       // Eclipse
    { BUILD_DRUID_OF_THE_ECLIPSE, 42031, 4 },       // Starsurge
    // Shadowseer (S10 Hero Archetype)
    { BUILD_SHADOWSEER, 40050, 4 },                 // Shadow Bolt
    { BUILD_SHADOWSEER, 40019, 4 },                 // Corruption
    { BUILD_SHADOWSEER, 40554, 4 },                 // Shadowburn
    { BUILD_SHADOWSEER, 36632, 4 },                 // Shadow Crash
    // Cryomancer (S10 Hero Archetype)
    { BUILD_CRYOMANCER, 40008, 4 },                 // Frostbolt
    { BUILD_CRYOMANCER, 36516, 4 },                 // Frost Bomb
    { BUILD_CRYOMANCER, 40256, 4 },                 // Ice Lance
    { BUILD_CRYOMANCER, 40774, 4 },                 // Summon Water Elemental
    { BUILD_CRYOMANCER, 40867, 4 },                 // Deep Freeze
    // Ascendant (S10 Hero Archetype)
    { BUILD_ASCENDANT, 40038, 4 },                  // Smite
    { BUILD_ASCENDANT, 40222, 4 },                  // Holy Fire
    { BUILD_ASCENDANT, 36613, 4 },                  // Holy Ascension
    { BUILD_ASCENDANT, 40071, 4 },                  // Exorcism
    { BUILD_ASCENDANT, 40669, 4 },                  // Holy Shock
    // Stormcaller (S10 Hero Archetype)
    { BUILD_STORMCALLER, 40025, 4 },                // Lightning Bolt
    { BUILD_STORMCALLER, 40201, 4 },                // Earth Shock
    { BUILD_STORMCALLER, 36572, 4 },                // Booming Thunder
    { BUILD_STORMCALLER, 40027, 4 },                // Chain Lightning
    { BUILD_STORMCALLER, 40472, 4 },                // Elemental Mastery
    // Fel Apostate (S10 Hero Archetype)
    { BUILD_FEL_APOSTATE, 40181, 4 },               // Rain of Fire
    { BUILD_FEL_APOSTATE, 40251, 4 },               // Incinerate
    { BUILD_FEL_APOSTATE, 37029, 4 },               // Beam of Hatred
    { BUILD_FEL_APOSTATE, 40189, 4 },               // Soul Fire
    { BUILD_FEL_APOSTATE, 40896, 4 },               // Chaos Bolt
    // White Walker (S10 Hero Archetype and community build)
    { BUILD_WHITE_WALKER, 40109, 4 },               // Sinister Strike
    { BUILD_WHITE_WALKER, 36679, 4 },               // Slice and Ice
    { BUILD_WHITE_WALKER, 36680, 4 },               // Winds of Winter
    { BUILD_WHITE_WALKER, 40867, 4 },               // Deep Freeze
    // Plague Caster (community build)
    { BUILD_PLAGUE_CASTER, 37576, 4 },              // Devour Mind
    { BUILD_PLAGUE_CASTER, 36877, 4 },              // Plague Swarm
    { BUILD_PLAGUE_CASTER, 40934, 4 },              // Haunt
    { BUILD_PLAGUE_CASTER, 40165, 4 },              // Wrath
    { BUILD_PLAGUE_CASTER, 37531, 4 },              // Wrath of the Plague
    { BUILD_PLAGUE_CASTER, 36876, 4 },              // Harbinger of Plague
    { BUILD_PLAGUE_CASTER, 40463, 3 },              // Shadowform
    { BUILD_PLAGUE_CASTER, 138260, 3 },             // Voice of Bwonsamdi
    { BUILD_PLAGUE_CASTER, 41102, 3 },              // Ebon Plaguebringer
    { BUILD_PLAGUE_CASTER, 37048, 3 },              // Natural Selection
    { BUILD_PLAGUE_CASTER, 40259, 2 },              // Avenging Wrath
    { BUILD_PLAGUE_CASTER, 41053, 2 },              // Improved Blood Presence
    { BUILD_PLAGUE_CASTER, 40456, 2 },              // Spirit Tap
    { BUILD_PLAGUE_CASTER, 40933, 2 },              // Improved Spirit Tap
    { BUILD_PLAGUE_CASTER, 41838, 2 },              // Runic Focus
};

using Weights = std::array<int8, 5>;

std::unordered_map<uint32, Weights> const& PickTable()
{
    static std::unordered_map<uint32, Weights> const table = []
    {
        std::unordered_map<uint32, Weights> weights;
        for (Pick const& pick : PICKS)
            for (uint32 entry = pick.first; entry <= (pick.last ? pick.last : pick.first); ++entry)
                for (uint8 archetype = 0; archetype < 5; ++archetype)
                    if (pick.archetypes & (1 << archetype))
                        weights[entry][archetype] = pick.weight;
        return weights;
    }();
    return table;
}

// A build's weight for each entry it holds.
std::unordered_map<uint32, std::vector<std::pair<uint8, int8>>> const& BuildTable()
{
    static std::unordered_map<uint32, std::vector<std::pair<uint8, int8>>> const table = []
    {
        std::unordered_map<uint32, std::vector<std::pair<uint8, int8>>> weights;
        for (BuildPick const& pick : BUILD_PICKS)
            weights[pick.entry].emplace_back(pick.build, pick.weight);
        return weights;
    }();
    return table;
}

int32 BuildWeight(uint32 entryId, uint8 build)
{
    if (build >= BUILD_COUNT)
        return 0;
    auto const& table = BuildTable();
    auto const found = table.find(entryId);
    if (found == table.end())
        return 0;
    for (auto const& [holder, weight] : found->second)
        if (holder == build)
            return weight;
    return 0;
}
}  // namespace

int32 WildcardPickWeight(uint32 entryId, WildcardArchetype archetype)
{
    auto const& table = PickTable();
    auto const found = table.find(entryId);
    return found == table.end() ? 0 : found->second[uint8(archetype)];
}

int32 WildcardPickWeight(uint32 entryId, WildcardArchetype archetype, uint8 build)
{
    return WildcardPickWeight(entryId, archetype) + BuildWeight(entryId, build);
}

uint8 WildcardBuildFor(uint32 guidCounter, WildcardArchetype archetype, std::vector<uint32> const& kept)
{
    std::vector<uint8> builds;
    for (uint8 build = 0; build < BUILD_COUNT; ++build)
        if (BUILDS[build].archetype == archetype)
            builds.push_back(build);
    if (builds.empty())
        return NO_WILDCARD_BUILD;

    // Another hash than the one of the fighting style, so that each style spreads over all of its builds.
    std::size_t const start = std::size_t((uint64(guidCounter) * 40503u) >> 9) % builds.size();
    uint8 best = builds[start];
    uint32 bestHeld = 0;
    for (std::size_t step = 0; step < builds.size(); ++step)
    {
        uint8 const build = builds[(start + step) % builds.size()];
        uint32 const held = uint32(std::count_if(kept.begin(), kept.end(),
            [build](uint32 entry) { return BuildWeight(entry, build) >= 4; }));
        if (held > bestHeld)
        {
            best = build;
            bestHeld = held;
        }
    }
    return best;
}

char const* WildcardBuildName(uint8 build)
{
    return build < BUILD_COUNT ? BUILDS[build].name : "no";
}

char const* WildcardArchetypeName(WildcardArchetype archetype)
{
    switch (archetype)
    {
        case WildcardArchetype::Tank:   return "tank";
        case WildcardArchetype::Heal:   return "healer";
        case WildcardArchetype::Melee:  return "melee";
        case WildcardArchetype::Ranged: return "ranged";
        default:                        return "caster";
    }
}
