#pragma once
#include <vector>
#include "../../Utility/PlayerData.h"

// 既存: 対象
enum class Targeting { SingleEnemy,
	AllEnemies };

// ★追加: 属性
enum class SpellElement { Neutral,
	Fire,
	Ice,
	Thunder,
	Heal };

struct SpellDef {
	const char* name;
	int mpCost = 0;
	bool isHealing = false;
	Targeting targeting = Targeting::SingleEnemy;

	// ★追加: 属性（演出と計算の拡張で使用）
	SpellElement element = SpellElement::Neutral;

	// ダメージ/回復の基礎（計算側ですでにあれば合わせる）
	int power = 10;
};

// 魔法検索
const SpellDef* FindSpell(PlayerData::MagicType t);
// レジストリ初期化（cpp側で埋め込み）
void InitSpellRegistryIfNeeded();
