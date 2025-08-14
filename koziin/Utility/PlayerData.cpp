#include "PlayerData.h"

PlayerData::PlayerData()
	: hp(30), maxHp(30), attack(10), defense(5), experience(0), level(1) {} // デフォルト値を設定（経験値とレベルも含む）

PlayerData::~PlayerData() {}

void PlayerData::SetAttack(int value) {
	attack = value;
}

void PlayerData::SetHp(int value) {
	   hp = value;
    if (hp > maxHp) hp = maxHp;
    if (hp <= 0) hp = 0;
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
	hp = maxHp;	  // レベルアップで HP を全回復
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

int PlayerData::GetMaxHp() const
{ return maxHp; 
}

// レベルアップに必要な経験値を計算する補助メソッド
int PlayerData::GetExperienceRequiredForLevel(int currentLevel) const {
	return currentLevel * 100; // 例: レベルアップに必要な経験値はレベル×100
}


// アイテム取得登録
void PlayerData::AddCollectedItem(int id, const std::string& name) {
    collectedItemsById[id] = std::make_pair(name, true);
}

// 取得済み判定
bool PlayerData::IsCollected(int id) const {
    auto it = collectedItemsById.find(id);
    return it != collectedItemsById.end() && it->second.second;
}

// 取得済みアイテム一覧取得（ID → (名前, 取得済みフラグ)）
const std::map<int, std::pair<std::string, bool>>& PlayerData::GetCollectedItemsById() const {
    return collectedItemsById;
}

// 名前ごとの個数を集計
std::map<std::string, int> PlayerData::GetCollectedItemCounts() const {
	std::map<std::string, int> counts;
	for (const auto& kv : collectedItemsById) {
		const auto& pair = kv.second;
		if (pair.second) {
			counts[pair.first]++;
		}
	}
	return counts;
}

void PlayerData::ClearCollectedItems() {
    collectedItemsById.clear();
}