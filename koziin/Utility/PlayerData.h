#pragma once

#include "../Singleton.h"
#include <string>
#include <vector>

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

	// 取得済みアイテムを追加
	void AddCollectedItem(const std::string& itemName);
	// 全取得済みアイテムを取得
	const std::vector<std::string>& GetCollectedItems() const;
	// 全リストをクリアする（必要な場合）
	void ClearCollectedItems();


private:
	int attack;
	int hp;
	int defense;
	int experience; // 経験値
	int level;		// レベル

	int GetExperienceRequiredForLevel(int currentLevel) const; // 経験値を計算するメソッド
	std::vector<std::string> collectedItems;				   // ★ 取得済みアイテム名リスト
};
