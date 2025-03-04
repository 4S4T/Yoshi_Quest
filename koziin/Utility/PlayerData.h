#pragma once

#include "../Singleton.h"
#include <string>

// �v���C���[�̃f�[�^�ۑ��N���X
class PlayerData : public Singleton<PlayerData>
{
public:
	PlayerData();
	~PlayerData();

	void SetAttack(int value);
	void SetHp(int value);
	void SetDefense(int value);

	int GetHp() const;
	int GetAttack() const;
	int GetDefense() const;



private:
	int attack;
	int hp;
	int defense;
};
