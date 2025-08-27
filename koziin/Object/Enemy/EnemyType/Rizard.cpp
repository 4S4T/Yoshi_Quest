#include "Rizard.h"
#include "../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "../../../Scene/Battle/Battle.h"


Rizard::Rizard() {
}

Rizard::~Rizard() {
}

void Rizard::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	image = rm->GetImages("Resource/Images/lizard.png", 1, 1, 1, 16, 16)[0];


	location = Vector2D(480, 360);
	attack = 5;
	hp = 10;
	defense = 5;
}

void Rizard::Finalize() {
}

void Rizard::Update(float delta_second) 
{
	if (hp <= 0) {
		hp = 0;
	}

	// 点滅時間の減少
	if (isBlinking) {
		blinkTimer -= delta_second;
		if (blinkTimer <= 0.0f) {
			isBlinking = false;
		}
	}
}

void Rizard::Draw(const Vector2D& screen_offset) const 
{
	if (!isVisible)
		return;

	DrawFormatString(0, 200, GetColor(255, 255, 255), "体力　: %d", hp);

	// 点滅中は0.1秒ごとに非表示
	if (isBlinking && static_cast<int>(blinkTimer * 10) % 2 == 0) {
		// 何も描画しない（点滅）
		return;
	}
	
	__super::Draw(screen_offset);
}
