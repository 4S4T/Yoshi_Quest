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

	// “_–ÅŽžŠÔ‚ÌŒ¸­
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

	DrawFormatString(0, 200, GetColor(255, 255, 255), "‘Ì—Í@: %d", hp);
	__super::Draw(screen_offset);
}
