#include "Rock_Lizard.h"
#include "../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "../../../Scene/Battle/Battle.h"


Rock_Lizard::Rock_Lizard() {
}

Rock_Lizard::~Rock_Lizard() {
}

void Rock_Lizard::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	image = rm->GetImages("Resource/Images/Rock_Lizard.png", 1, 1, 1, 16, 16)[0];


	location = Vector2D(480, 360);
	attack = 6;
	hp = 12;
	defense = 7;
}

void Rock_Lizard::Finalize() {
}

void Rock_Lizard::Update(float delta_second) {
	if (hp <= 0) {
		hp = 0;
	}
}

void Rock_Lizard::Draw(const Vector2D& screen_offset) const {
	DrawFormatString(0, 200, GetColor(255, 255, 255), "�̗́@: %d", hp);
	__super::Draw(screen_offset);
}
