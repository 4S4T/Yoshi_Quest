#include "SpellRegistry.h"
#include <optional>

static bool g_inited = false;
static SpellDef g_fire, g_ice, g_flare, g_heal;

static void initOnce() {
	if (g_inited)
		return;

	// Fire（単体・炎）
	g_fire.name = "ファイア";
	g_fire.mpCost = 4;
	g_fire.isHealing = false;
	g_fire.targeting = Targeting::SingleEnemy;
	g_fire.element = SpellElement::Fire;
	g_fire.power = 22;

	// Ice（単体・氷）
	g_ice.name = "アイス";
	g_ice.mpCost = 5;
	g_ice.isHealing = false;
	g_ice.targeting = Targeting::SingleEnemy;
	g_ice.element = SpellElement::Ice;
	g_ice.power = 24;

	// Flare（全体・雷/光）
	g_flare.name = "フレア";
	g_flare.mpCost = 8;
	g_flare.isHealing = false;
	g_flare.targeting = Targeting::AllEnemies;
	g_flare.element = SpellElement::Thunder; // 雷系演出
	g_flare.power = 18;

	// Heal（回復）
	g_heal.name = "ヒール";
	g_heal.mpCost = 5;
	g_heal.isHealing = true;
	g_heal.targeting = Targeting::SingleEnemy; // 自分対象扱い
	g_heal.element = SpellElement::Heal;
	g_heal.power = 28;

	g_inited = true;
}

void InitSpellRegistryIfNeeded() { initOnce(); }

const SpellDef* FindSpell(PlayerData::MagicType t) {
	initOnce();
	switch (t) {
	case PlayerData::MagicType::Fire:
		return &g_fire;
	case PlayerData::MagicType::Ice:
		return &g_ice;
	case PlayerData::MagicType::Flare:
		return &g_flare;
	case PlayerData::MagicType::Heal:
		return &g_heal;
	default:
		return nullptr;
	}
}
