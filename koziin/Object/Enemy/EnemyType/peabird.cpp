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
	attack = 10;
	hp = 10;
	defense = 7;
}

void peabird::Finalize() {
}

void peabird::Update(float delta_second) 
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

void peabird::Draw(const Vector2D& screen_offset) const 
{
	if (!isVisible)
		return;

	DrawFormatString(0, 200, GetColor(255, 255, 255), "とり　: %d", hp);

	// 点滅中は0.1秒ごとに非表示
	if (isBlinking && static_cast<int>(blinkTimer * 10) % 2 == 0) {
		// 何も描画しない（点滅）
		return;
	}
	__super::Draw(screen_offset);

	
}
