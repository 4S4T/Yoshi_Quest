#pragma once
#include <memory>

// バトル側からAIに渡す最小の状況情報
struct EnemyView {
	int selfHp = 1, selfMaxHp = 1, selfAtk = 1, selfDef = 0;
	int playerHp = 1, playerMaxHp = 1, playerAtk = 1, playerDef = 0;
	int livingEnemyCount = 1;
};

// 行動決定IF：ここでは「最終ダメージ」を返す簡易版
class IEnemyAI {
public:
	virtual ~IEnemyAI() = default;
	// baseDamage を元に最終ダメージを返す（0以下なら“防御/様子見”扱い）
	virtual int decideDamage(const EnemyView& v, int baseDamage) = 0;
};

// いつも攻撃（そのまま）
class SimpleAttackAI : public IEnemyAI {
public:
	int decideDamage(const EnemyView&, int baseDamage) override {
		return baseDamage;
	}
};

// 体力が半分以下なら防御（= 0ダメージ）、それ以外は攻撃
class GuardWhenLowAI : public IEnemyAI {
public:
	int decideDamage(const EnemyView& v, int baseDamage) override {
		if (v.selfHp * 2 <= v.selfMaxHp)
			return 0; // 防御/様子見
		return baseDamage;
	}
};
