#include "Slime.h"
#include"../../../Utility/ResourceManager.h"
#include"DxLib.h"
#include"../../../Scene/Battle/Battle.h"


Slime::Slime()
{

}

Slime::~Slime()
{

}

void Slime::Initialize()
{
	ResourceManager* rm = ResourceManager::GetInstance();
	image = rm->GetImages("Resource/Images/slime1.png", 1, 1, 1, 16, 16)[0];


	location = Vector2D(480, 360);
	attack = 5;
	hp = 10;
	defense = 5;


}

void Slime::Finalize() 
{

}

void Slime::Update(float delta_second) {
	if (hp <= 0) {
		hp = 0;
	}

	// �_�Ŏ��Ԃ̌���
	if (isBlinking) {
		blinkTimer -= delta_second;
		if (blinkTimer <= 0.0f) {
			isBlinking = false;
		}
	}
}

void Slime::Draw(const Vector2D& screen_offset) const
{
	DrawFormatString(0, 200, GetColor(255, 255, 255), "�̗́@: %d",hp );
	// �_�Œ���0.1�b���Ƃɔ�\��
	if (isBlinking && static_cast<int>(blinkTimer * 10) % 2 == 0) {
		// �����`�悵�Ȃ��i�_�Łj
		return;
	}

	__super::Draw(screen_offset);
}
