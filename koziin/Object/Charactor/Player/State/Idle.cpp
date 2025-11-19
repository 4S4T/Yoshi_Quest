#include "Idle.h"
#include "../../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "../../../../Utility/InputControl.h"
#include "../Player.h"
#include "../../../GameObjectManager.h"

IdleState::IdleState(Player* p)
	: PlayerStateBase(p),
	  input(nullptr) {
}

IdleState::~IdleState() {
}

void IdleState::Initialize() {
	// 速度を0にする
	player->velocity = 0.0f;
	// 画像は Player 側で描画するので、ここでは読み込まない
}

void IdleState::Finalize() {
}

void IdleState::Update(float delta_second) {
	// 入力情報を取得
	InputControl* input = Singleton<InputControl>::GetInstance();

	// どれかキーが押されたら RUN 状態へ
	if (input->GetKey(KEY_INPUT_A) ||
		input->GetKey(KEY_INPUT_D) ||
		input->GetKey(KEY_INPUT_W) ||
		input->GetKey(KEY_INPUT_S)) {
		player->SetNextState(ePlayerState::RUN);
	}
}

void IdleState::Draw() const {
	// ※プレイヤーの描画は Player::Draw で行うので何もしない
}

ePlayerState IdleState::GetState() const {
	return ePlayerState::IDLE;
}
