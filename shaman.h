#ifndef SHAMAN_H
#define SHAMAN_H

#define SHAMAN_CURSE_RADIUS_BASE	512
#define SHAMAN_CURSE_RADIUS_BONUS	0

#define CURSE_DEFAULT_INITIAL_RADIUS	256
#define CURSE_DEFAULT_ADDON_RADIUS		0
#define CURSE_DEFAULT_INITIAL_RANGE		512
#define CURSE_DEFAULT_ADDON_RANGE		0

//************************************************************
//			Mind Absorb (passive skill)
//************************************************************
#define MIND_ABSORB_RADIUS_BASE		256
#define MIND_ABSORB_RADIUS_BONUS	0
#define MIND_ABSORB_AMOUNT_BASE		0
#define MIND_ABSORB_AMOUNT_BONUS	10
//************************************************************
//			Lower Resist (Curse)
//************************************************************
#define LOWER_RESIST_INITIAL_RANGE			512
#define LOWER_RESIST_ADDON_RANGE			0
#define LOWER_RESIST_INITIAL_RADIUS			256
#define LOWER_RESIST_ADDON_RADIUS			0
#define LOWER_RESIST_INITIAL_DURATION		0
#define LOWER_RESIST_ADDON_DURATION			1.0
#define LOWER_RESIST_COST					50
#define LOWER_RESIST_DELAY					1.0
#define LOWER_RESIST_INITIAL_FACTOR			0.25
#define LOWER_RESIST_ADDON_FACTOR			0.025
//************************************************************
//			Amp Damage (Curse)
//************************************************************
#define AMP_DAMAGE_DELAY			2
#define AMP_DAMAGE_DURATION_BASE	0
#define AMP_DAMAGE_DURATION_BONUS	1.0
#define AMP_DAMAGE_COST				50
#define AMP_DAMAGE_MULT_BASE		1.5
#define AMP_DAMAGE_MULT_BONUS		0.1
//************************************************************
//			Weaken (Curse)
// Weaken causes target to take extra damage, deal less damage
// and slows them down
//************************************************************
#define WEAKEN_DELAY			2
#define WEAKEN_DURATION_BASE	0
#define WEAKEN_DURATION_BONUS	1.0
#define WEAKEN_COST				50
#define WEAKEN_MULT_BASE		1.25
#define WEAKEN_MULT_BONUS		0.025
#define WEAKEN_SLOW_BASE		0
#define WEAKEN_SLOW_BONUS		0.1
/*
 ************************************************************
 *			Iron Maiden (Curse)
 ************************************************************
#define IRON_MAIDEN_DELAY			2
#define IRON_MAIDEN_DURATION_BASE	2
#define IRON_MAIDEN_DURATION_BONUS	0.5
#define IRON_MAIDEN_COST			50
#define IRON_MAIDEN_BONUS			0.66	//damage multiplier
*/
//************************************************************
//			Life Drain (Curse)
//************************************************************
#define LIFE_DRAIN_DELAY			2
#define LIFE_DRAIN_COST				50
#define LIFE_DRAIN_HEALTH			10
#define LIFE_DRAIN_DURATION_BASE	0
#define LIFE_DRAIN_DURATION_BONUS	1.0
#define LIFE_DRAIN_RANGE			256
#define LIFE_DRAIN_UPDATETIME		1.0
//************************************************************
//			Amnesia (Curse)
//************************************************************
#define AMNESIA_DELAY			2
#define AMNESIA_DURATION_BASE	0
#define AMNESIA_DURATION_BONUS	2
#define AMNESIA_COST			50
//************************************************************
//			Curse (Curse)
//************************************************************
#define CURSE_DELAY				2
#define CURSE_DURATION_BASE		0
#define CURSE_DURATION_BONUS	1
#define CURSE_COST				25
//************************************************************
//			Healing (Blessing)
//************************************************************
#define HEALING_DELAY			2
#define HEALING_DURATION_BASE	10.5	//allow for 10 "ticks"
#define HEALING_DURATION_BONUS	0
#define HEALING_COST			50
#define HEALING_HEAL_BASE		0
#define HEALING_HEAL_BONUS		1
//************************************************************
//			Bless (Blessing)
//************************************************************
#define BLESS_DELAY				0
#define BLESS_DURATION_BASE		0
#define BLESS_DURATION_BONUS	0.5
#define BLESS_COST				50
#define BLESS_BONUS				1.5		//Damage / Defense
#define BLESS_MAGIC_BONUS		1.5
//************************************************************
//			Deflect (Blessing)
//************************************************************
#define DEFLECT_INITIAL_DURATION			0
#define DEFLECT_ADDON_DURATION				1.0	
#define DEFLECT_COST						50
#define DEFLECT_DELAY						2.0
#define DEFLECT_INITIAL_HITSCAN_CHANCE		0.1
#define DEFLECT_ADDON_HITSCAN_CHANCE		0.023
#define DEFLECT_INITIAL_PROJECTILE_CHANCE	0.1
#define DEFLECT_ADDON_PROJECTILE_CHANCE		0.023
#define DEFLECT_MAX_PROJECTILE_CHANCE		0.33
#define DEFLECT_MAX_HITSCAN_CHANCE			0.33
#define DEFLECT_HITSCAN_ABSORB_BASE			1.0
#define DEFLECT_HITSCAN_ABSORB_ADDON		0
#define DEFLECT_HITSCAN_ABSORB_MAX			1.0
//************************************************************

void Cmd_AmpDamage(edict_t *ent);
void Cmd_Weaken(edict_t *ent);
//void Cmd_Slave(edict_t *ent);
void Cmd_Amnesia(edict_t *ent);
void Cmd_Curse(edict_t *ent);
void Cmd_Bless(edict_t *ent);
void Cmd_Healing(edict_t *ent);
void MindAbsorb(edict_t *ent);
void CursedPlayer (edict_t *ent);
void CurseEffects (edict_t *self, int num, int color);
void Cmd_LifeDrain(edict_t *ent);
void LifeDrain (edict_t *ent);
void Bleed (edict_t *curse);
void Cmd_LowerResist (edict_t *ent);
qboolean curse_add(edict_t* target, edict_t* caster, int type, int curse_level, float duration);//4.4
void CurseMessage(edict_t* caster, edict_t* target, int type, float duration, qboolean isCurse);//4.4

#endif