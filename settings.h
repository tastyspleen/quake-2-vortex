#ifndef SETTINGS
#define SETTINGS

const char* s1;
const char* s2;

#define	CURRENT_DATE				s1 = Date()
#define CURRENT_TIME				s2 = Time()

#define VRX_VERSION "4.66"

#define RUNE_PICKUP_DELAY			2.0f	// time before another rune can be picked up

#define SENTRY_MAXIMUM				1		//Maximum number of sentry guns allowed

// chill effects for movement and attack refire speed
// note that player weapon refire is handled independently (50% effectiveness on players vs monsters)
#define CHILL_DEFAULT_BASE			0.0f
#define CHILL_DEFAULT_ADDON			0.1f

// berserker sprint
#define SPRINT_COST					4	// per frame ability charge cost
#define SPRINT_MAX_CHARGE			100	// maximum charge
#define SPRINT_CHARGE_RATE			20	// rate of charge per second while ability is unused

#define SHIELD_COST					2	// per frame ability charge cost
#define SHIELD_MAX_CHARGE			100	// maximum charge
#define SHIELD_CHARGE_RATE			10	// rate of charge per second while ability is unused
#define SHIELD_FRONT_PROTECTION		1.0f	// damage absorbed for front (power screen) protection
#define SHIELD_BODY_PROTECTION		0.8f	// damage absorbed for full-body protection
#define SHIELD_ABILITY_DELAY		0.3f	// seconds before shield can be re-activated

#define SMARTROCKET_LOCKFRAMES		3 // frames required for smart rocket to lock-on to a target
#define DAMAGE_ESCAPE_DELAY			0.2f // seconds before player can use tball/jetpack/boost/teleport/superspeed after being damaged
#define EXP_PLAYER_MONSTER				20
#define EXP_WORLD_MONSTER				20
#define AMMO_REGEN_DELAY			5.0f // seconds until next ammo regen tick
#define MAX_KNOCKBACK				300 // maximum knockback allowed for some attacks (e.g. rocket explosion)
#define CHAT_PROTECT_FRAMES			200
#define MAX_HOURS					24					//Maximum playing time per day (hours)
#define NEWBIE_BASHER_MAX 3 * AveragePlayerLevel()	//maximum level a newbie basher can be

#define	MAX_MINISENTRIES			2// number of minisentries you can have out at a time
#define MAX_CREDITS					1000000 // max credits someone can have; update this on next reset to unsigned long!
// General settings
#define CLOAK_DRAIN_TIME			1 //10 frames = 1 second
#define CLOAK_DRAIN_AMMO			1.0f
#define CLOAK_COST					50
#define CLOAK_ACTIVATE_TIME			2 // cloak after 3 seconds

#define LASER_TIMEUP				60
#define LASER_COST					25
#define LASER_CUTDAMAGE				15 // damage per frame
#define MAX_LASERS					6
#define LASER_SPAWN					3
#define SUPERSPEED_DRAIN_COST		3
#define RESPAWN_INVIN_TIME			20	//2.0 seconds

// force wall
#define FORCEWALL_WIDTH				256
#define FORCEWALL_HEIGHT			128
#define FORCEWALL_THICKNESS			16
#define FORCEWALL_DELAY				2.0
#define FORCEWALL_SOLID_COST		20
#define FORCEWALL_NOTSOLID_COST		50

#define MAX_PIPES					8
#define PTR_MAX_IDLE_FRAMES			600
#define MAX_IDLE_FRAMES				1800

// Sprees
#define SPREE_WARS			1
#define SPREE_START			6
#define SPREE_WARS_START	25

#define SPREE_WARS_BONUS		2
#define SPREE_BONUS				0.5
#define	SPREE_BREAKBONUS		25

// Health and armor settings
#define INITIAL_HEALTH_SOLDIER		100
#define INITIAL_HEALTH_NECROMANCER	100
#define INITIAL_HEALTH_ENGINEER		100
#define INITIAL_HEALTH_VAMPIRE		100
#define INITIAL_HEALTH_POLTERGEIST	100
#define INITIAL_HEALTH_KNIGHT		100
#define INITIAL_HEALTH_CLERIC		100
#define INITIAL_HEALTH_MAGE			100
#define INITIAL_HEALTH_WEAPONMASTER	100
#define INITIAL_HEALTH_SHAMAN		100
#define INITIAL_HEALTH_ALIEN		100

#define LEVELUP_HEALTH_SOLDIER		1
#define LEVELUP_HEALTH_NECROMANCER	1
#define LEVELUP_HEALTH_ENGINEER		2
#define LEVELUP_HEALTH_VAMPIRE		5
#define LEVELUP_HEALTH_POLTERGEIST	5
#define LEVELUP_HEALTH_KNIGHT		1
#define LEVELUP_HEALTH_CLERIC		5
#define LEVELUP_HEALTH_MAGE			1
#define LEVELUP_HEALTH_WEAPONMASTER	1
#define	LEVELUP_HEALTH_SHAMAN		5
#define LEVELUP_HEALTH_ALIEN		1

#define INITIAL_ARMOR_SOLDIER		50
#define INITIAL_ARMOR_NECROMANCER	50
#define INITIAL_ARMOR_ENGINEER		100
#define INITIAL_ARMOR_VAMPIRE		50
#define INITIAL_ARMOR_POLTERGEIST	50
#define INITIAL_ARMOR_KNIGHT		100
#define INITIAL_ARMOR_CLERIC		50
#define INITIAL_ARMOR_MAGE			50
#define INITIAL_ARMOR_WEAPONMASTER	50
#define INITIAL_ARMOR_SHAMAN		50
#define INITIAL_ARMOR_ALIEN			50

#define LEVELUP_ARMOR_SOLDIER		1
#define LEVELUP_ARMOR_NECROMANCER	0
#define LEVELUP_ARMOR_ENGINEER		5
#define LEVELUP_ARMOR_VAMPIRE		0
#define LEVELUP_ARMOR_POLTERGEIST	0
#define LEVELUP_ARMOR_KNIGHT		10
#define LEVELUP_ARMOR_CLERIC		2
#define LEVELUP_ARMOR_MAGE			0
#define LEVELUP_ARMOR_WEAPONMASTER	1
#define	LEVELUP_ARMOR_SHAMAN		0
#define LEVELUP_ARMOR_ALIEN			0

#define INITIAL_POWERCUBES_SOLDIER			200
#define ADDON_POWERCUBES_SOLDIER			10
#define INITIAL_POWERCUBES_VAMPIRE			200
#define ADDON_POWERCUBES_VAMPIRE			10
#define INITIAL_POWERCUBES_NECROMANCER		200
#define ADDON_POWERCUBES_NECROMANCER		10
#define INITIAL_POWERCUBES_KNIGHT			200
#define ADDON_POWERCUBES_KNIGHT				10
#define INITIAL_POWERCUBES_MAGE				200
#define ADDON_POWERCUBES_MAGE				30
#define INITIAL_POWERCUBES_POLTERGEIST		200
#define ADDON_POWERCUBES_POLTERGEIST		10
#define INITIAL_POWERCUBES_CLERIC			200
#define ADDON_POWERCUBES_CLERIC				10
#define INITIAL_POWERCUBES_ALIEN			200
#define ADDON_POWERCUBES_ALIEN				10
#define INITIAL_POWERCUBES_SHAMAN			200
#define ADDON_POWERCUBES_SHAMAN				10
#define INITIAL_POWERCUBES_ENGINEER			200
#define ADDON_POWERCUBES_ENGINEER			10

// Respawning
#define TBALLS_RESPAWN		1
#define	POWERCUBES_RESPAWN	25

//Ammo pickups (also handled during respawning)
#define	SHELLS_PICKUP		10
#define	BULLETS_PICKUP		50
#define	GRENADES_PICKUP		5
#define	ROCKETS_PICKUP		5
#define	CELLS_PICKUP		50
#define	SLUGS_PICKUP		5

// Ammo upgrade skill
#define AMMO_UP_BASE		1.0
#define AMMO_UP_MULT		0.1	//10%

// CTF
#define CTF_CAPTURE_BASE					ctf_capture_base->value
#define CTF_CAPTURE_BONUS					0.5
#define CTF_TEAM_BONUS						1
#define CTF_RETURN_FLAG_ASSIST_BONUS		1.2
#define CTF_FRAG_CARRIER_ASSIST_BONUS		1.2
#define CTF_MIN_CAPTEAM						ctf_min_capteam->value

#define CTF_RECOVERY_BONUS					5
#define CTF_FLAG_BONUS						0

#define CTF_FRAG_MODIFIER					ctf_frag_modifier->value
#define CTF_TEAMMATE_FRAGAWARD				ctf_teammate_fragaward->value
#define CTF_FRAG_CARRIER_BONUS				1.3
#define CTF_CARRIER_DANGER_PROTECT_BONUS	1.2
#define CTF_CARRIER_PROTECT_BONUS			1.1
#define CTF_FLAG_DEFENSE_BONUS				1.2

#define CTF_TARGET_PROTECT_RADIUS			400
#define CTF_ATTACKER_PROTECT_RADIUS			400
#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT	8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT		10
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT		10
#define CTF_AUTO_FLAG_RETURN_TIMEOUT		30
#define CTF_FORCE_EVENTEAMS					ctf_force_eventeams->value
#define CTF_TECH_TIMEOUT					30
#define CTF_FLAG_HOLDTIME					300


// Points
#define START_NEXTLEVEL			500
#define NEXTLEVEL_MULT			1.5
#define BONUS_2FER				4

#define BONUS_ACCURACY			0
#define BONUS_ACCURACY_START	100

#define EXP_PLAYER_BASE			50
#define EXP_OTHER_BASE			5
#define EXP_INIT				2
#define EXP_MONSTER				5
#define EXP_LOW					4
#define EXP_SAME				6
#define EXP_HIGH				8
#define EXP_LOSE_LOW			0
#define EXP_LOSE_SAME			1
#define EXP_LOSE_HIGH			2
#define EXP_LOSE_SUICIDE		5
#define EXP_MINIBOSS			100

#define CREDITS_PLAYER_BASE		10
#define	CREDITS_OTHER_BASE		10
#define CREDIT_LOW				1
#define CREDIT_SAME				2
#define CREDIT_HIGH				3

// Weapons
#define SABRE_INITIAL_DAMAGE	100
#define SABRE_ADDON_DAMAGE		5
#define SABRE_ADDON_HEATDAMAGE	2
#define SABRE_INITIAL_RANGE		32
#define SABRE_ADDON_RANGE		3.2
#define SABRE_INITIAL_KICK		100
#define SABRE_ADDON_KICK		0

#define BLASTER_INITIAL_DAMAGE_MIN	10
#define BLASTER_INITIAL_DAMAGE_MAX	40
#define BLASTER_ADDON_DAMAGE_MIN	2
#define BLASTER_ADDON_DAMAGE_MAX	3
#define BLASTER_INITIAL_HASTE		50
#define BLASTER_ADDON_HASTE			14
#define BLASTER_INITIAL_SPEED		1000
#define BLASTER_ADDON_SPEED			50

// 20mm cannon fires every 0.2 seconds
#define WEAPON_20MM_INITIAL_DMG		35
#define WEAPON_20MM_ADDON_DMG		1.5
//#define WEAPON_20MM_INITIAL_DMG_MIN 30
//#define WEAPON_20MM_INITIAL_DMG_MAX 50
//#define WEAPON_20MM_ADDON_DMG_MIN	1.5
//#define WEAPON_20MM_ADDON_DMG_MAX	2.5
#define WEAPON_20MM_INITIAL_RANGE	500
#define WEAPON_20MM_ADDON_RANGE		25

// fires every 1.0 seconds
#define SHOTGUN_INITIAL_DAMAGE		8
#define SHOTGUN_ADDON_DAMAGE		0.2//0.3
#define SHOTGUN_INITIAL_BULLETS		10
#define SHOTGUN_ADDON_BULLETS		0.5

// fires every 1.2 seconds
#define SUPERSHOTGUN_INITIAL_DAMAGE		10
#define SUPERSHOTGUN_ADDON_DAMAGE		0.4//0.3
#define SUPERSHOTGUN_INITIAL_BULLETS	15
#define SUPERSHOTGUN_ADDON_BULLETS		0.5

#define MACHINEGUN_INITIAL_DAMAGE		10
#define MACHINEGUN_ADDON_DAMAGE			0.5
#define MACHINEGUN_ADDON_TRACERDAMAGE	3

// note: CG fires 40 bullets/sec
#define CHAINGUN_INITIAL_DAMAGE			5
#define CHAINGUN_ADDON_DAMAGE			0.2
#define CHAINGUN_ADDON_TRACERDAMAGE		5

#define GRENADE_INITIAL_DAMAGE			200
#define GRENADE_ADDON_DAMAGE			10
#define GRENADE_INITIAL_RADIUS_DAMAGE	200
#define GRENADE_ADDON_RADIUS_DAMAGE		10
#define GRENADE_INITIAL_RADIUS			100
#define GRENADE_ADDON_RADIUS			2.5

// fires every 7-8 frames
#define GRENADELAUNCHER_INITIAL_DAMAGE			100
#define GRENADELAUNCHER_ADDON_DAMAGE			5
#define GRENADELAUNCHER_INITIAL_RADIUS_DAMAGE	100
#define GRENADELAUNCHER_ADDON_RADIUS_DAMAGE		5
#define GRENADELAUNCHER_INITIAL_RADIUS			100
#define GRENADELAUNCHER_ADDON_RADIUS			2.5
#define GRENADELAUNCHER_INITIAL_SPEED			600
#define GRENADELAUNCHER_ADDON_SPEED				30

// fires every 7-8 frames
#define ROCKETLAUNCHER_INITIAL_DAMAGE			100
#define ROCKETLAUNCHER_ADDON_DAMAGE				3.5//2.5
#define ROCKETLAUNCHER_INITIAL_SPEED			650
#define ROCKETLAUNCHER_ADDON_SPEED				25
#define ROCKETLAUNCHER_INITIAL_RADIUS_DAMAGE	100
#define ROCKETLAUNCHER_ADDON_RADIUS_DAMAGE		2.5
#define ROCKETLAUNCHER_INITIAL_DAMAGE_RADIUS	100
#define ROCKETLAUNCHER_ADDON_DAMAGE_RADIUS		2.5

// hyperblaster fires every other frame, 5 bolts/sec
#define HYPERBLASTER_INITIAL_DAMAGE		30
#define HYPERBLASTER_ADDON_DAMAGE		1.5 //1
#define HYPERBLASTER_INITIAL_SPEED		1500
#define HYPERBLASTER_ADDON_SPEED		50

// fires every 13-14 frames
#define RAILGUN_INITIAL_DAMAGE		100
#define RAILGUN_ADDON_DAMAGE		8//10
#define RAILGUN_ADDON_HEATDAMAGE	1.0

#define BFG10K_INITIAL_DAMAGE		30
#define BFG10K_ADDON_DAMAGE			2.0
#define BFG10K_INITIAL_SPEED		650
#define BFG10K_ADDON_SPEED			35
#define BFG10K_RADIUS				150
#define BFG10K_INITIAL_DURATION		1.0
#define BFG10K_ADDON_DURATION		0.05
#define BFG10K_DEFAULT_DURATION		1.5
#define BFG10K_DEFAULT_SLIDE		0

//4.1 (new ability constants)
//Totems (general)
#define TOTEM_MAX_RANGE				512
#define TOTEM_COST					25
#define TOTEM_HEALTH_BASE			100
#define TOTEM_HEALTH_MULT			15
#define TOTEM_REGEN_BASE			10
#define TOTEM_REGEN_MULT			0
#define TOTEM_MASTERY_MULT			2
#define TOTEM_REGEN_FRAMES			100
#define TOTEM_REGEN_DELAY			10

//Fury
#define FURY_INITIAL_REGEN			0.05
#define FURY_ADDON_REGEN			0.005
#define FURY_MAX_REGEN				0.1
#define FURY_INITIAL_FACTOR			1.5
#define FURY_ADDON_FACTOR			0.05
#define FURY_FACTOR_MAX				2.0
#define FURY_DURATION_BASE			0
#define FURY_DURATION_BONUS			1.0
//Fire totem
#define FIRETOTEM_DAMAGE_BASE		0
#define FIRETOTEM_DAMAGE_MULT		3
#define FIRETOTEM_REFIRE_BASE		2.0
#define FIRETOTEM_REFIRE_MULT		0.0		//make this negative to reduce the "cooldown".
//Water totem
//Don't make this duration too short. The ice talent will do too much damage if you do.
#define WATERTOTEM_DURATION_BASE	3.0
#define WATERTOTEM_DURATION_MULT	0.0
#define WATERTOTEM_REFIRE_BASE		1.0
#define WATERTOTEM_REFIRE_MULT		0.0		//make this negative to reduce the "cooldown".
//Air totem
#define AIRTOTEM_RESIST_BASE		1.0
#define AIRTOTEM_RESIST_MULT		0.2
//Earth totem
#define EARTHTOTEM_RESIST_MULT		0.05
#define EARTHTOTEM_DAMAGE_MULT		0.1
//Nature totem
#define NATURETOTEM_HEALTH_BASE		0
#define NATURETOTEM_HEALTH_MULT		5
#define NATURETOTEM_ARMOR_BASE		0
#define NATURETOTEM_ARMOR_MULT		2.5f
#define NATURETOTEM_REFIRE_BASE		5.0f
#define NATURETOTEM_REFIRE_MULT		0.0f		//make this negative to reduce the "cooldown".
//Darkness totem
#define DARKNESSTOTEM_VAMP_MULT		0.033f
#define DARKNESSTOTEM_MAX_MULT		0.1f

//Talent: Precision Tuning
#define PRECISION_TUNING_COST_FACTOR	0.1
#define PRECISION_TUNING_DELAY_FACTOR	0.1
#define PRECISION_TUNING_SKILL_FACTOR	0.1

// Sentries
#define SENTRY_INITIAL_HEALTH		100
#define SENTRY_ADDON_HEALTH			10
#define SENTRY_INITIAL_ARMOR		150
#define SENTRY_ADDON_ARMOR			15
#define SENTRY_INITIAL_AMMO			100
#define SENTRY_ADDON_AMMO			10

#define MINISENTRY_INITIAL_HEALTH	50
#define MINISENTRY_ADDON_HEALTH		10
#define MINISENTRY_INITIAL_ARMOR	50
#define MINISENTRY_ADDON_ARMOR		30
#define MINISENTRY_MAX_HEALTH		200
#define MINISENTRY_MAX_ARMOR		350
#define MINISENTRY_INITIAL_AMMO		50
#define MINISENTRY_ADDON_AMMO		5

#define MINISENTRY_INITIAL_BULLET	10
#define MINISENTRY_ADDON_BULLET		1
#define MINISENTRY_INITIAL_ROCKET	50
#define MINISENTRY_ADDON_ROCKET		15//4.4 - raised from 10
#define MINISENTRY_MAX_BULLET		100
#define MINISENTRY_MAX_ROCKET		1000


//GHz: Changed to damage multiplier
#define SENTRY_LEVEL1_DAMAGE		0.5
#define SENTRY_LEVEL2_DAMAGE		0.75
#define SENTRY_LEVEL3_DAMAGE		1.0
#define SENTRY_INITIAL_BULLETDAMAGE 10
#define SENTRY_ADDON_BULLETDAMAGE	2//4.4 - raised from 1 to better compete with tank/monsters
#define SENTRY_INITIAL_ROCKETDAMAGE	50
#define SENTRY_ADDON_ROCKETDAMAGE	15
#define SENTRY_INITIAL_ROCKETSPEED	650
#define SENTRY_ADDON_ROCKETSPEED	35

#define SENTRY_COST					50
#define SENTRY_MAX					1
#define SENTRY_UPGRADE				100
#define DELAY_SENTRY_SCAN			3
#define DELAY_SENTRY				3

#define SPIKER_MAX_COUNT			4
#define GASSER_MAX_COUNT			4
#define OBSTACLE_MAX_COUNT			6
#define SPIKEBALL_MAX_COUNT			3
#define MAX_MONSTERS				3
#define DELAY_MONSTER_THINK			3
#define DELAY_MONSTER				3

// Monster cost

#define M_FLYER_COST			25
#define M_INSANE_COST			25
#define M_SOLDIERLT_COST		25
#define M_SOLDIER_COST			25
#define M_GUNNER_COST			25
#define M_CHICK_COST			25
#define M_PARASITE_COST			25
#define M_MEDIC_COST			25
#define M_BRAIN_COST			25
#define M_TANK_COST				50
#define M_HOVER_COST			25
#define M_SUPERTANK_COST		300
#define	M_COMMANDER_COST		300
#define M_MUTANT_COST			25
#define M_DEFAULT_COST			25

// Monster control cost

#define M_FLYER_CONTROL_COST			1
#define M_INSANE_CONTROL_COST			1
#define M_SOLDIERLT_CONTROL_COST		1
#define M_SOLDIER_CONTROL_COST			1
#define M_GUNNER_CONTROL_COST			1
#define M_CHICK_CONTROL_COST			1
#define M_PARASITE_CONTROL_COST			1
#define M_MEDIC_CONTROL_COST			1
#define M_TANK_CONTROL_COST				2
#define M_BRAIN_CONTROL_COST			1
#define M_HOVER_CONTROL_COST			1
#define M_SUPERTANK_CONTROL_COST		3
#define M_COMMANDER_CONTROL_COST		3
#define M_MUTANT_CONTROL_COST			1
#define M_DEFAULT_CONTROL_COST			1

// Misc Delays

#define DELAY_QUAD					10
#define DELAY_INVULN				10
#define DELAY_AMMOSTEAL				2.0
#define DELAY_FREEZE				5
#define DELAY_BOOST					3.0
#define DELAY_BLOODSUCKER			3
#define DELAY_SALVATION				1
#define DELAY_CORPSEEXPLODE			0.5
#define DELAY_LASER					1
#define DELAY_DECOY					2
#define DELAY_BOMB					1
#define DELAY_THORNS				1
#define DELAY_HOLYFREEZE			1

// Spell Stuff

#define COST_FOR_QUAD				400
#define QUAD_TIME					50 //in frames 10 frames = 1 second
#define COST_FOR_INVIN				400
#define INVIN_TIME					50//in frames 10 frames = 1 second
#define COST_FOR_STEALER			25
#define COST_FOR_FREEZER			50
#define FREEZE_DURATION				5
#define BURN_DAMAGE					2
#define COST_FOR_BOOST				0
#define COST_FOR_BLOODSUCKER		100
#define COST_FOR_SALVATION			1
#define COST_FOR_CORPSEEXPLODE		20
#define COST_FOR_DECOY				100
#define COST_FOR_HOLYSHOCK			1
#define COST_FOR_HOLYFREEZE			1
#define COST_FOR_HOOK				10
#define MAX_BOMB_RANGE				768
#define COST_FOR_BOMB				25
#define METEOR_COST					25
#define ICEBOLT_COST				25
#define FIREBALL_COST				25
#define NOVA_COST					25
#define BOLT_COST					25
#define CLIGHTNING_COST				25
#define LIGHTNING_COST				25

// Spell Radii
#define RADIUS_AMMOSTEAL			512
#define RADIUS_FREEZE				512
#define RADIUS_BLOODSUCKER			512
#define RADIUS_CORPSEEXPLODE		1024
#define RADIUS_BOMB					512

//3.0 Armoury
#define ARMORY_ITEMS				28	//Number of available items

#define ARMORY_RUNE_UNIQUE_PRICE	150000
#define ARMORY_RUNE_APOINT_PRICE	5000
#define ARMORY_RUNE_WPOINT_PRICE	2500
#define ARMORY_MAX_RUNES			10

#define ARMORY_PRICE_WEAPON			5
#define ARMORY_PRICE_AMMO			5
#define ARMORY_PRICE_TBALLS			10
#define ARMORY_PRICE_POWERCUBE		1
//#define ARMORY_PRICE_RESPAWNS		100
#define ARMORY_PRICE_HEALTH			5
#define ARMORY_PRICE_ARMOR			10
#define ARMORY_PRICE_POTIONS		50
#define ARMORY_PRICE_ANTIDOTES		100
#define ARMORY_PRICE_GRAVITYBOOTS	150
#define ARMORY_PRICE_FIRE_RESIST	75
#define ARMORY_PRICE_AUTO_TBALL		250
#define ARMORY_PRICE_RESET			2500 // per level price (so level 10 would be 25k)

//#define ARMORY_QTY_RESPAWNS			1000
#define ARMORY_QTY_POTIONS			5
#define ARMORY_QTY_ANTIDOTES		10
#define ARMORY_QTY_GRAVITYBOOTS		25
#define ARMORY_QTY_FIRE_RESIST		25
#define ARMORY_QTY_AUTO_TBALL		3

//4.5 player combat preferences
#define HOSTILE_PLAYERS				0x00000001			
#define HOSTILE_MONSTERS			0x00000002

//3.0 Special Flags
#define SFLG_NONE					0x00000000
#define SFLG_MATRIXJUMP				0x00000001
#define SFLG_UNDERWATER				0x00000002
#define SFLG_PARTIAL_INWATER		0x00000004
#define SFLG_AUTO_TBALLED			0x00000008
#define SFLG_DOUBLEJUMP				0x00000010

//Other flag combinations
#define SFLG_TOUCHING_WATER		SFLG_UNDERWATER | SFLG_PARTIAL_INWATER

//Matrix jump
#define MJUMP_VELOCITY				350
#define MJUMP_GRAVITY_MULT			2.5

// Cheat Stuff
#define CLIENT_GL_MODULATE			1
#define CLIENT_GL_DYNAMIC			2
#define CLIENT_SW_DRAWFLAT			3
#define CLIENT_GL_SHOWTRIS			4
#define	CLIENT_R_FULLBRIGHT			5
#define CLIENT_TIMESCALE			6
#define CLIENT_GL_LIGHTMAP			7
#define	CLIENT_GL_SATURATELIGHTING	8
#define	CLIENT_R_DRAWFLAT			9
#define	CLIENT_CL_TESTLIGHTS		10
#define	CLIENT_FIXEDTIME			11

// class numbers
#define CLASS_SOLDIER			1
#define CLASS_POLTERGEIST		2
#define CLASS_VAMPIRE			3
#define CLASS_MAGE				4
#define CLASS_ENGINEER			5
#define CLASS_KNIGHT			6
#define CLASS_CLERIC			7
#define CLASS_NECROMANCER		8
#define CLASS_SHAMAN			9
#define CLASS_ALIEN				10
#define CLASS_WEAPONMASTER		11

#define CLASS_MAX				10		//Number of classes to choose from

//Trade stuff
#define TRADE_MAX_DISTANCE		512		//Trade distance
#define TRADE_MAX_PLAYERS		5		//Max # of players to choose when trading

#endif
