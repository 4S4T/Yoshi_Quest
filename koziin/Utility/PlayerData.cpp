#include "PlayerData.h"

PlayerData::PlayerData()
	: hp(30), attack(10), defense(5), experience(0), level(1) {} // デフォルト値を設定（経験値とレベルも含む）

PlayerData::~PlayerData() {}

void PlayerData::SetAttack(int value) {
	attack = value;
}

void PlayerData::SetHp(int value) {
	hp = value;
	if (hp <= 0) {
		hp = 0;
	}
}

void PlayerData::SetDefense(int value) {
	defense = value;
}

void PlayerData::AddExperience(int exp) {
	if (exp > 0) {
		experience += exp;
		// 経験値が足りている場合、レベルアップをチェック
		while (experience >= GetExperienceRequiredForLevel(level)) {
			experience -= GetExperienceRequiredForLevel(level);
			LevelUp();
		}
	}
}

void PlayerData::LevelUp() {
	++level;
	attack += 5;  // レベルアップ時に攻撃力を上げる例
	hp += 10;	  // レベルアップ時にHPを上げる例
	defense += 3; // レベルアップ時に防御力を上げる例
}

int PlayerData::GetHp() const {
	return hp;
}

int PlayerData::GetAttack() const {
	return attack;
}

int PlayerData::GetDefense() const {
	return defense;
}

int PlayerData::GetExperience() const {
	return experience;
}

int PlayerData::GetLevel() const {
	return level;
}

// レベルアップに必要な経験値を計算する補助メソッド
int PlayerData::GetExperienceRequiredForLevel(int currentLevel) const {
	return currentLevel * 100; // 例: レベルアップに必要な経験値はレベル×100
}


//取得済みアイテムを追加
void PlayerData::AddCollectedItem(const std::string& itemName) {
	// 重複追加を避ける（任意）
	for (const auto& name : collectedItems) {
		if (name == itemName)
			return;
	}
	collectedItems.push_back(itemName);
}

// 全取得済みアイテムを取得
const std::vector<std::string>& PlayerData::GetCollectedItems() const {
	return collectedItems;
}

// 全リストをクリアする
void PlayerData::ClearCollectedItems() {
	collectedItems.clear();
}
