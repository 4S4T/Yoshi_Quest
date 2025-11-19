#pragma once
#include "SpellRegistry.h" // SpellDef / SpellElement / Targeting

// ---- 依存しない簡易ヘルパ ----
static inline int IMAX(int a, int b) { return (a > b) ? a : b; }

// -----------------------------
// ダメージ計算時の入力構造
// -----------------------------
struct CalcContext {
	int attackerAtk = 0;
	int defenderDef = 0;
	int attackerLv = 1;
	bool critical = false; // 物理のみで使用
};

// -----------------------------
// 物理ダメージ
// -----------------------------
inline int CalcPhysicalDamage(const CalcContext& c) {
	// 基礎：攻撃 ? 防御/2（下限1）
	int base = c.attackerAtk - (c.defenderDef / 2);
	if (base < 1)
		base = 1;

	// 会心：1.5倍（端数切り上げ）
	if (c.critical) {
		base = (base * 3 + 1) / 2;
	}
	return IMAX(1, base);
}

// -----------------------------
// 魔法ダメージ
// -----------------------------
// SpellDef.power を“魔法の素の強さ”として利用。
// LV / ATK も少しだけ影響するようにして手触りを出す。
inline int CalcSpellDamage(const SpellDef& def, const CalcContext& c) {
	int dmg = def.power + (c.attackerLv * 2) // レベルで微増
			  + (c.attackerAtk / 4)			 // 魔力が攻撃力と一体なら少し寄与
			  - (c.defenderDef / 3);		 // 敵防御で軽減
	if (dmg < 1)
		dmg = 1;
	return dmg;
}

// -----------------------------
// 回復量（Heal 用）
// -----------------------------
// SpellDef.power を“回復強さ”として利用。
inline int CalcHealingAmount(const SpellDef& def, int playerLv) {
	int heal = def.power + playerLv * 3;
	if (heal < 1)
		heal = 1;
	return heal;
}
