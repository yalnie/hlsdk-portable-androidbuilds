/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"
#include "UserMessages.h"

class CWorldItem : public CBaseEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	int m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

bool CWorldItem::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CWorldItem::Spawn()
{
	CBaseEntity* pEntity = NULL;

	switch (m_iType)
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create("item_battery", pev->origin, pev->angles);
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create("item_antidote", pev->origin, pev->angles);
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create("item_security", pev->origin, pev->angles);
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create("item_suit", pev->origin, pev->angles);
		break;
	}

	if (!pEntity)
	{
		ALERT(at_console, "unable to create world_item %d\n", m_iType);
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY(edict());
}


void CItem::Spawn()
{
	if (pev->spawnflags & SF_ITEMS_NOGRAVITY)
	{
		pev->movetype = MOVETYPE_NONE;
	}
	else
	{
		pev->movetype = MOVETYPE_TOSS;
	}

	if (FBitSet(pev->spawnflags, SF_ITEMS_DISABLED)) // if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_TRIGGER;

	SetTouch(&CItem::ItemTouch);
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CItem::ToggleUse);
	}

	if (!pev->spawnflags & SF_ITEMS_NOGRAVITY)
	{
		if (DROP_TO_FLOOR(ENT(pev)) == 0)
		{
			ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
			UTIL_Remove(this);
			return;
		}
	}
}

void CItem::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		SetTouch(NULL);

		// player grabbed the item.
		g_pGameRules->PlayerGotItem(pPlayer, this);
		if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_YES)
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove(this);
	}
}

CBaseEntity* CItem::Respawn()
{
	SetTouch(NULL);
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin(pev, g_pGameRules->VecItemRespawnSpot(this)); // blip to whereever you should respawn.

	SetThink(&CItem::Materialize);
	pev->nextthink = g_pGameRules->FlItemRespawnTime(this);
	return this;
}

void CItem::Materialize()
{
	if ((pev->effects & EF_NODRAW) != 0)
	{
		// changing from invisible state to visible.
		//EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CItem::ItemTouch);
}


//
// ToggleUse - If this is the USE function for a trigger, its state will toggle every time it's fired
//
void CItem::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->solid == SOLID_NOT)
	{
		pev->solid = SOLID_TRIGGER;
	}
	else
	{
		pev->solid = SOLID_NOT;
	}
	UTIL_SetOrigin(pev, pev->origin);
}

class CItemSuit : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_suit.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_suit.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		// Only for visual purposes.
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);



class CItemBattery : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_battery.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_battery.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		// Only for visual purposes.
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);


class CItemAntidote : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_antidote.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_antidote.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		pPlayer->SetSuitUpdate("!HEV_DET4", false, SUIT_NEXT_IN_1MIN);

		pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);

// Obsolete
class CItemSecurity : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_security.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_security.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		pPlayer->m_rgItems[ITEM_SECURITY] += 1;
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);

class CItemLongJump : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_longjump.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_longjump.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		// Only for visual purposes.
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);

//
// Half-Life: Insecure Items
//

class CItemArmorVest : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/barney_vest.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/barney_vest.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return false;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			pPlayer->HasSuit())
		{
			pPlayer->pev->armorvalue += 60;
			pPlayer->pev->armorvalue = V_min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_armorvest, CItemArmorVest);

class CItemHelmet : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/barney_helmet.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/barney_helmet.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return false;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			pPlayer->HasSuit())
		{
			pPlayer->pev->armorvalue += 40;
			pPlayer->pev->armorvalue = V_min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_helmet, CItemHelmet);

class CItemArmorPlate : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_plate.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_plate.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return false;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			pPlayer->HasSuit())
		{
			pPlayer->pev->armorvalue += 20;
			pPlayer->pev->armorvalue = V_min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_armorplate, CItemArmorPlate);

class CItemFlashlight : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_flashlight.mdl");

		m_pGlow = CSprite::SpriteCreate("sprites/glow03.spr", pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5), false);
		m_pGlow->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
		m_pGlow->SetScale(0.4);
		m_pGlow->SetAttachment(edict(), 1);

		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_flashlight.mdl");
		PRECACHE_MODEL("sprites/glow03.spr");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->HasFlashlight())
			return false;

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		if (m_pGlow)
			m_pGlow->pev->effects |= EF_NODRAW;

		pPlayer->SetFlashlight(true);
		return true;
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	CSprite* m_pGlow;
};

LINK_ENTITY_TO_CLASS(item_flashlight, CItemFlashlight);

class CItemKeycard : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_security.mdl");

		m_pGlow = CSprite::SpriteCreate("sprites/flare3.spr", pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5), false);
		m_pGlow->SetTransparency(kRenderGlow, 66, 135, 245, 255, kRenderFxNoDissipation);
		m_pGlow->SetAttachment(edict(), 1);

		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_security.mdl");
		PRECACHE_MODEL("sprites/flare3.spr");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->HasKeycard())
			return false;

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		if (m_pGlow)
			m_pGlow->pev->effects |= EF_NODRAW;

		pPlayer->ToggleKeycard(true);
		return true;
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	CSprite* m_pGlow;
};

LINK_ENTITY_TO_CLASS(item_keycard, CItemKeycard);

class CItemRedcard : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_security_red.mdl");

		m_pGlow = CSprite::SpriteCreate("sprites/flare3.spr", pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5), false);
		m_pGlow->SetTransparency(kRenderGlow, 250, 60, 60, 255, kRenderFxNoDissipation);
		m_pGlow->SetAttachment(edict(), 1);

		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_security_red.mdl");
		PRECACHE_MODEL("sprites/flare3.spr");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->HasRedKeycard())
			return false;

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		if (m_pGlow)
			m_pGlow->pev->effects |= EF_NODRAW;

		pPlayer->ToggleRedKeycard(true);
		return true;
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	CSprite* m_pGlow;
};

LINK_ENTITY_TO_CLASS(item_redcard, CItemRedcard);

class CItemC4 : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_c4.mdl");
		
		m_pGlow = CSprite::SpriteCreate("sprites/glow01.spr", pev->origin + Vector(0, 0, (pev->mins.z + pev->maxs.z) * 0.5), false);
		m_pGlow->SetTransparency(kRenderGlow, 255, 0, 0, 255, kRenderFxNoDissipation);
		m_pGlow->SetScale(0.10);
		m_pGlow->SetAttachment(edict(), 1);
		
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_c4.mdl");
		PRECACHE_MODEL("sprites/glow01.spr");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->HasC4())
			return false;

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		if (m_pGlow)
			m_pGlow->pev->effects |= EF_NODRAW;

		pPlayer->ToggleC4(true);
		return true;
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	CSprite* m_pGlow;
};

LINK_ENTITY_TO_CLASS(item_c4, CItemC4);

TYPEDESCRIPTION CItemC4::m_SaveData[] =
{
	DEFINE_FIELD(CItemC4, m_pGlow, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CItemC4, CBaseEntity);

TYPEDESCRIPTION CItemKeycard::m_SaveData[] =
{
	DEFINE_FIELD(CItemKeycard, m_pGlow, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CItemKeycard, CBaseEntity);

TYPEDESCRIPTION CItemRedcard::m_SaveData[] =
{
	DEFINE_FIELD(CItemRedcard, m_pGlow, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CItemRedcard, CBaseEntity);

TYPEDESCRIPTION CItemFlashlight::m_SaveData[] =
{
	DEFINE_FIELD(CItemFlashlight, m_pGlow, FIELD_CLASSPTR),
};
IMPLEMENT_SAVERESTORE(CItemFlashlight, CBaseEntity);