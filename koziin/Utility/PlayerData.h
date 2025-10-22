#pragma once
#include "../Singleton.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include "../Object/Item/Item.h"

// ===========================================
// PlayerData�N���X
// �v���C���[�̃X�e�[�^�X�A�����i�A���������Ǘ�
// ===========================================
class PlayerData : public Singleton<PlayerData> {
public:
	PlayerData();
	~PlayerData();

	// �� ���@�̎��
	enum class MagicType {
		Fire,	 // �P�̍U��
		Heal,	 // ���ȉ�
		Ice,	 // �X
		Thunder, // ��
		Flare	 // �� �ǉ�: �S�̍U��
	};


	// �X�e�[�^�X�ݒ�E�擾
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

	// �� MP �A�N�Z�T
	void SetMp(int value);
	int GetMp() const;
	int GetMaxMp() const;	  // �����␳���悹�����ꍇ�͂��̒��ŉ��Z
	bool ConsumeMp(int cost); // cost ���x�������� true
	bool HasMp(int cost) const;

	// �A�C�e���Ǘ�
	void AddItem(const Item& item); // �����i�ɒǉ��i�݌Ɂj
	bool IsCollected(int id) const; // �����m�F�i�݌Ɂj
	bool UseItem(int itemId);		// ����A�C�e�����g�p�i�݌ɂ����炷�j

	std::map<std::string, int> GetConsumableCounts() const; // ����A�C�e���̏�����
	std::map<std::string, int> GetAllItemCounts() const;	// �S�A�C�e���̏������i�����W�v�j

	// �����֘A
	void EquipItem(EquipCategory category, int itemId);		   // ������ύX
	std::string GetEquippedName(EquipCategory category) const; // �������A�C�e�����擾

	// �����A�C�e���ꗗ�i�ǂݎ���p�j�擾
	const std::map<int, Item>& GetOwnedItems() const;

	// �����A�C�e�����X�g�N���A�i�݌ɂƑ����̂݁j
	void ClearCollectedItems();

	// �� �E���g���сh�i�݌ɂƂ͓Ɨ��j
	bool HasEverCollected(int id) const;
	void MarkMapPickupCollected(int id);

	// �� ���@�̏K�����
	bool HasMagic(MagicType t) const;
	void LearnMagic(MagicType t);
	std::vector<MagicType> GetLearnedMagics() const;

private:
	// �X�e�[�^�X
	int attack;
	int hp;
	int maxHp;
	// �� MP
	int mp;
	int maxMp;
	int defense;
	int experience;
	int level;

	// �����A�C�e���ꗗ�i�݌Ɂj
	std::map<int, Item> ownedItems;

	// �������A�C�e��ID
	int equippedWeaponId; // ����
	int equippedShieldId; // ��
	int equippedArmorId;  // �h��
	int equippedHelmetId; // ��

	// �� �����␳�̍��Z�w���p�[
	int GetEquipAttackBonus() const;  // �U���͂̑����␳�i���Z�j���v
	int GetEquipDefenseBonus() const; // �h��͂̑����␳�i���Z�j���v
	int GetEquipHpBonus() const;	  // �ő�HP�̑����␳�i���Z�j���v

	// ���x���A�b�v�ɕK�v�Ȍo���l�v�Z
	int GetExperienceRequiredForLevel(int currentLevel) const;

	// �� �E���g���сh�̃Z�b�g�iID�ێ��j
	std::set<int> collectedItemIds;

	// �� �K���ςݖ��@
	std::set<MagicType> learnedMagics;
};
