#pragma once

#include "../Singleton.h"
#include <string>
#include<vector>
#include <map>

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

	    // --- �A�C�e���Ǘ� ---
	void AddCollectedItem(int id, const std::string& name);
	bool IsCollected(int id) const;

	// �擾�ς݃A�C�e��ID�Ɩ��O�̃}�b�v�擾�i�ǂݎ���p�j
	const std::map<int, std::pair<std::string, bool>>& GetCollectedItemsById() const;

	// ���O���Ƃ̎擾�����W�v���ĕԂ�
	std::map<std::string, int> GetCollectedItemCounts() const;

	// �N���A
	void ClearCollectedItems();


private:
	int attack;
	int hp;
	int defense;
	int experience; // �o���l
	int level;		// ���x��

	int GetExperienceRequiredForLevel(int currentLevel) const; // �o���l���v�Z���郁�\�b�h

	// �A�C�e���� �� ��
	std::map<int, std::pair<std::string, bool>> collectedItemsById;
};
