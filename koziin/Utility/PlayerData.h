#pragma once
#include "../Singleton.h"
#include <string>
#include <map>
#include "../Object/Item/Item.h"

// ===========================================
// PlayerDataクラス
// プレイヤーのステータス、所持品、装備情報を管理
// ===========================================
class PlayerData : public Singleton<PlayerData> {
public:
	PlayerData();
	~PlayerData();

	// ステータス設定・取得
	void SetAttack(int value);
	void SetHp(int value);
	void SetDefense(int value);
	void AddExperience(int value);
	void LevelUp();

	int GetHp() const;
	int GetAttack() const;
	int GetDefense() const;
	int GetExperience() const;
	int GetLevel() const;
	int GetMaxHp() const;
	int GetEquippedId(EquipCategory category) const;

	  // アイテム管理
	void AddItem(const Item& item);							// 所持品に追加
	bool IsCollected(int id) const;							// 所持確認
	std::map<std::string, int> GetConsumableCounts() const; // 消費アイテムの所持数
	std::map<std::string, int> GetAllItemCounts() const;	// 全アイテムの所持数（同名集計）

	// 装備関連
	void EquipItem(EquipCategory category, int itemId);		   // 装備を変更
	std::string GetEquippedName(EquipCategory category) const; // 装備中アイテム名取得

	// 所持アイテム一覧（読み取り専用）取得
	const std::map<int, Item>& GetOwnedItems() const;

	// 所持品リストクリア
	void ClearCollectedItems();

private:
	// ステータス
	int attack;
	int hp;
	int maxHp;
	int defense;
	int experience;
	int level;

	// 所持アイテム一覧
	std::map<int, Item> ownedItems;

	// 装備中アイテムID
	int equippedWeaponId;//武器
	int equippedShieldId;//盾
	int equippedArmorId;//道具
	int equippedHelmetId;//頭

	  // ★装備補正の合算ヘルパー
	int GetEquipAttackBonus() const;  // 攻撃力の装備補正（加算）合計
	int GetEquipDefenseBonus() const; // 防御力の装備補正（加算）合計
	int GetEquipHpBonus() const;	  // 最大HPの装備補正（加算）合計

	// レベルアップに必要な経験値計算
	int GetExperienceRequiredForLevel(int currentLevel) const;
};
