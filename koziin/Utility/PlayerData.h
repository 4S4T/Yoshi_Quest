#pragma once

#include "../Singleton.h"
#include <string>
#include <vector>

// �v���C���[�̃f�[�^�ۑ��N���X
class PlayerData : public Singleton<PlayerData> {
public:
	PlayerData();
	~PlayerData();

	void SetAttack(int value);
	void SetHp(int value);
	void SetDefense(int value);
	void AddExperience(int value); // �o���l��ǉ����郁�\�b�h
	void LevelUp();				   // ���x���A�b�v���\�b�h

	int GetHp() const;
	int GetAttack() const;
	int GetDefense() const;
	int GetExperience() const; // �o���l�̃Q�b�^�[
	int GetLevel() const;	   // ���x���̃Q�b�^�[

	// �擾�ς݃A�C�e����ǉ�
	void AddCollectedItem(const std::string& itemName);
	// �S�擾�ς݃A�C�e�����擾
	const std::vector<std::string>& GetCollectedItems() const;
	// �S���X�g���N���A����i�K�v�ȏꍇ�j
	void ClearCollectedItems();


private:
	int attack;
	int hp;
	int defense;
	int experience; // �o���l
	int level;		// ���x��

	int GetExperienceRequiredForLevel(int currentLevel) const; // �o���l���v�Z���郁�\�b�h
	std::vector<std::string> collectedItems;				   // �� �擾�ς݃A�C�e�������X�g
};
