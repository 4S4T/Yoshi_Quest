#include "Rock_Rizard.h"
#include "../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "../../../Scene/Battle/Battle.h"


Rock_Rizard::Rock_Rizard() {
}

Rock_Rizard::~Rock_Rizard() {
}

void Rock_Rizard::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	image = rm->GetImages("Resource/Images/Rock_Lizard.png", 1, 1, 1, 16, 16)[0];


	location = Vector2D(480, 360);
	attack = 4;
	hp = 12;
	defense = 7;

}

void Rock_Rizard::Finalize() {
}

void Rock_Rizard::Update(float delta_second) 
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

void Rock_Rizard::Draw(const Vector2D& screen_offset) const 
{
	if (!isVisible)
		return;

	DrawFormatString(0, 200, GetColor(255, 255, 255), "‘Ì—Í@: %d", hp);
	__super::Draw(screen_offset);
}
