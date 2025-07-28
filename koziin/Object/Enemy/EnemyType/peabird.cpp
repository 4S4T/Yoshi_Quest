#include "peabird.h"
#include "../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "../../../Scene/Battle/Battle.h"


peabird::peabird() {
}

peabird::~peabird() {
}

void peabird::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	image = rm->GetImages("Resource/Images/peabird.png", 1, 1, 1, 16, 16)[0];


	location = Vector2D(480, 360);
	attack = 5;
	hp = 10;
	defense = 5;
}

void peabird::Finalize() {
}

void peabird::Update(float delta_second) {
	if (hp <= 0) {
		hp = 0;
	}
}

void peabird::Draw(const Vector2D& screen_offset) const {
	DrawFormatString(0, 200, GetColor(255, 255, 255), "�̗́@: %d", hp);
	__super::Draw(screen_offset);
}
