#pragma once

#include "../Singleton.h"
#include <string>
#include<vector>
#include <map>

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

	    // --- アイテム管理 ---
	void AddCollectedItem(int id, const std::string& name);
	bool IsCollected(int id) const;

	// 取得済みアイテムIDと名前のマップ取得（読み取り専用）
	const std::map<int, std::pair<std::string, bool>>& GetCollectedItemsById() const;

	// 名前ごとの取得個数を集計して返す
	std::map<std::string, int> GetCollectedItemCounts() const;

	// クリア
	void ClearCollectedItems();


private:
	int attack;
	int hp;
	int defense;
	int experience; // 経験値
	int level;		// レベル

	int GetExperienceRequiredForLevel(int currentLevel) const; // 経験値を計算するメソッド

	// アイテム名 → 個数
	std::map<int, std::pair<std::string, bool>> collectedItemsById;
};
