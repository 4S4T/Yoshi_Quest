#pragma once
#include "../Singleton.h"
#include <string>
#include <map>
#include "../Object/Item/Item.h"

// ===========================================
// PlayerData�N���X
// �v���C���[�̃X�e�[�^�X�A�����i�A���������Ǘ�
// ===========================================
class PlayerData : public Singleton<PlayerData> {
public:
	PlayerData();
	~PlayerData();

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

	  // �A�C�e���Ǘ�
	void AddItem(const Item& item);							// �����i�ɒǉ�
	bool IsCollected(int id) const;							// �����m�F
	std::map<std::string, int> GetConsumableCounts() const; // ����A�C�e���̏�����
	std::map<std::string, int> GetAllItemCounts() const;	// �S�A�C�e���̏������i�����W�v�j

	// �����֘A
	void EquipItem(EquipCategory category, int itemId);		   // ������ύX
	std::string GetEquippedName(EquipCategory category) const; // �������A�C�e�����擾

	// �����A�C�e���ꗗ�i�ǂݎ���p�j�擾
	const std::map<int, Item>& GetOwnedItems() const;

	// �����i���X�g�N���A
	void ClearCollectedItems();

private:
	// �X�e�[�^�X
	int attack;
	int hp;
	int maxHp;
	int defense;
	int experience;
	int level;

	// �����A�C�e���ꗗ
	std::map<int, Item> ownedItems;

	// �������A�C�e��ID
	int equippedWeaponId;//����
	int equippedShieldId;//��
	int equippedArmorId;//����
	int equippedHelmetId;//��

	  // �������␳�̍��Z�w���p�[
	int GetEquipAttackBonus() const;  // �U���͂̑����␳�i���Z�j���v
	int GetEquipDefenseBonus() const; // �h��͂̑����␳�i���Z�j���v
	int GetEquipHpBonus() const;	  // �ő�HP�̑����␳�i���Z�j���v

	// ���x���A�b�v�ɕK�v�Ȍo���l�v�Z
	int GetExperienceRequiredForLevel(int currentLevel) const;
};
