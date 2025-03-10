#include "Idle.h"
#include "../../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "../../../../Utility/InputControl.h"
#include "../Player.h"
#include "../../../GameObjectManager.h"

IdleState::IdleState(Player* p) : PlayerStateBase(p),
								  input(nullptr) {
}

IdleState::~IdleState() {
}

void IdleState::Initialize()
{
	// 速度を0にする
	player->velocity = 0.0f;
	ResourceManager* rm = ResourceManager::GetInstance();
	image = rm->GetImages("Resource/Images/yossi.png", 1, 1, 1, 16, 16)[0];
}

void IdleState::Finalize() 
{

}

void IdleState::Update(float delta_second) {
	// 入力情報を取得
	InputControl* input = Singleton<InputControl>::GetInstance();
	// 移動処理
	if (input->GetKey(KEY_INPUT_A))
	{
		player->SetNextState(ePlayerState::RUN);
	}
	if (input->GetKey(KEY_INPUT_D))
	{
		player->SetNextState(ePlayerState::RUN);
	}
	if (input->GetKey(KEY_INPUT_W)) {
		player->SetNextState(ePlayerState::RUN);
	}
	if (input->GetKey(KEY_INPUT_S)) {
		player->SetNextState(ePlayerState::RUN);
	}
}



void IdleState::Draw() const {
	DrawRotaGraphF(player->GetLocation().x, player->GetLocation().y, 1.5, 0.0, image, TRUE);
}

ePlayerState IdleState::GetState() const {
	return ePlayerState::IDLE;
}
