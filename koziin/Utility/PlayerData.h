#pragma once

#include "../Singleton.h"
#include <string>

// プレイヤーのデータ保存クラス
class PlayerData : public Singleton<PlayerData> {
public:
	PlayerData();
	~PlayerData();

	void SetAttack(int value);
	void SetHp(int value);
	void SetDefense(int value);
	void AddExperience(int value); // 経験値を追加するメソッド
	void LevelUp();				   // レベルアップメソッド

	int GetHp() const;
	int GetAttack() const;
	int GetDefense() const;
	int GetExperience() const; // 経験値のゲッター
	int GetLevel() const;	   // レベルのゲッター

private:
	int attack;
	int hp;
	int defense;
	int experience; // 経験値
	int level;		// レベル

	int GetExperienceRequiredForLevel(int currentLevel) const; // 経験値を計算するメソッド
};
