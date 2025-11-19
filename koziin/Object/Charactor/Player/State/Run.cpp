#include "Run.h"

#include "DxLib.h"
#include "../../../../Utility/InputControl.h"
#include "../Player.h"
#include "../../../../Utility/ResourceManager.h"

RunState::RunState(Player* p)
	: PlayerStateBase(p),
	  old_location(Vector2D(0.0f)),
	  animation_time(0.f),
	  animation_count(0) {
}

RunState::~RunState() {
}

void RunState::Initialize() {
	old_location = player->GetLocation();
	animation_time = 0.0f;
	animation_count = 0;
	// 画像は Player 側で管理するので、ここでは読み込まない
}

void RunState::Finalize() {
}

void RunState::Update(float delta_second) {
	// 入力情報を取得
	InputControl* input = Singleton<InputControl>::GetInstance();

	Vector2D v(0.0f, 0.0f);

	if (input->GetKey(KEY_INPUT_D)) {
		v.x += 1.0f;
	}
	if (input->GetKey(KEY_INPUT_A)) {
		v.x -= 1.0f;
	}
	if (input->GetKey(KEY_INPUT_W)) {
		v.y -= 1.0f;
	}
	if (input->GetKey(KEY_INPUT_S)) {
		v.y += 1.0f;
	}

	player->velocity = v;

	// どのキーも押されていなければ Idle へ戻る
	if (v.x == 0.0f && v.y == 0.0f) {
		player->SetNextState(ePlayerState::IDLE);
	}

	old_location = player->GetLocation();
}

void RunState::Draw() const {
	// ※プレイヤーの描画は Player::Draw で行うので、ここでは何もしない
}

void RunState::AnimationControl(float delta_second) {
	// Player 側でアニメ制御しているので空でOK
}

ePlayerState RunState::GetState() const {
	return ePlayerState::RUN;
}
