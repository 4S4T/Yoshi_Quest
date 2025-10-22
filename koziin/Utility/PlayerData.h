#pragma once
#include "../Singleton.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include "../Object/Item/Item.h"

// ===========================================
// PlayerDataクラス
// プレイヤーのステータス、所持品、装備情報を管理
// ===========================================
class PlayerData : public Singleton<PlayerData> {
public:
	PlayerData();
	~PlayerData();

	// ★ 魔法の種類
	enum class MagicType {
		Fire,	 // 単体攻撃
		Heal,	 // 自己回復
		Ice,	 // 氷
		Thunder, // 雷
		Flare	 // ★ 追加: 全体攻撃
	};


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

	// ★ MP アクセサ
	void SetMp(int value);
	int GetMp() const;
	int GetMaxMp() const;	  // 装備補正を乗せたい場合はこの中で加算
	bool ConsumeMp(int cost); // cost を支払えたら true
	bool HasMp(int cost) const;

	// アイテム管理
	void AddItem(const Item& item); // 所持品に追加（在庫）
	bool IsCollected(int id) const; // 所持確認（在庫）
	bool UseItem(int itemId);		// 消費アイテムを使用（在庫を減らす）

	std::map<std::string, int> GetConsumableCounts() const; // 消費アイテムの所持数
	std::map<std::string, int> GetAllItemCounts() const;	// 全アイテムの所持数（同名集計）

	// 装備関連
	void EquipItem(EquipCategory category, int itemId);		   // 装備を変更
	std::string GetEquippedName(EquipCategory category) const; // 装備中アイテム名取得

	// 所持アイテム一覧（読み取り専用）取得
	const std::map<int, Item>& GetOwnedItems() const;

	// 所持アイテムリストクリア（在庫と装備のみ）
	void ClearCollectedItems();

	// ★ 拾得“実績”（在庫とは独立）
	bool HasEverCollected(int id) const;
	void MarkMapPickupCollected(int id);

	// ★ 魔法の習得状態
	bool HasMagic(MagicType t) const;
	void LearnMagic(MagicType t);
	std::vector<MagicType> GetLearnedMagics() const;

private:
	// ステータス
	int attack;
	int hp;
	int maxHp;
	// ★ MP
	int mp;
	int maxMp;
	int defense;
	int experience;
	int level;

	// 所持アイテム一覧（在庫）
	std::map<int, Item> ownedItems;

	// 装備中アイテムID
	int equippedWeaponId; // 武器
	int equippedShieldId; // 盾
	int equippedArmorId;  // 防具
	int equippedHelmetId; // 頭

	// ★ 装備補正の合算ヘルパー
	int GetEquipAttackBonus() const;  // 攻撃力の装備補正（加算）合計
	int GetEquipDefenseBonus() const; // 防御力の装備補正（加算）合計
	int GetEquipHpBonus() const;	  // 最大HPの装備補正（加算）合計

	// レベルアップに必要な経験値計算
	int GetExperienceRequiredForLevel(int currentLevel) const;

	// ★ 拾得“実績”のセット（ID保持）
	std::set<int> collectedItemIds;

	// ★ 習得済み魔法
	std::set<MagicType> learnedMagics;
};
