#include "PlayerData.h"

PlayerData::PlayerData()
	: hp(30), attack(10), defense(5) {} // 🔹 デフォルト値を設定


PlayerData::~PlayerData() {
}

void PlayerData::SetAttack(int value) 
{
	attack = value;
}

void PlayerData::SetHp(int value) 
{
	hp = value;
	if (hp <= 0)
	{
		hp = 0;
	}
}

void PlayerData::SetDefense(int value)
{
	defense = value;
}

int PlayerData::GetHp() const 
{
	return hp;
}

int PlayerData::GetAttack() const {
	return attack;
}

int PlayerData::GetDefense() const {
	return defense;
}


