#include "Run.h"

#include "DxLib.h"
#include "../../../../Utility/InputControl.h"
#include "../Player.h"
#include "../../../../Utility/ResourceManager.h"


RunState::RunState(Player* p) : PlayerStateBase(p),
								old_location(Vector2D(0.0f)),
								animation_count(0),
								animation_time(0.f)
{
}

RunState::~RunState() {
}

void RunState::Initialize() {
	// 速度を0にする
	old_location = 0.0f;
	animation_time = 0.0f;
	animation_count = 0;
	ResourceManager* rm = ResourceManager::GetInstance();
	animation[0] = LoadGraph("Resource/Images/yossi.png");
	animation[1] = LoadGraph("Resource/Images/yossi_ikiri.png");
	/*image = rm->GetImages("Resource/Images/Sheep_0.png", 1, 1, 1, 16, 16)[0];*/
	/*player_animation = rm->GetImages("Resource/Images/sheep.png", 1, 1, 1, 16, 16);
	image = player_animation[1];*/
}

void RunState::Finalize() {
}

void RunState::Update(float delta_second)
{
	// 入力情報を取得
	InputControl* input = Singleton<InputControl>::GetInstance();

	// 移動処理
	if (input->GetKey(KEY_INPUT_D)) {
		this->player->velocity.x = 1.0f;
	}
	else if (input->GetKey(KEY_INPUT_A)) {
		this->player->velocity.x = -1.0f;
	}
	else if (input->GetKey(KEY_INPUT_W))
	{
		this->player->velocity.y = -1.0f;
	}
	else if (input->GetKey(KEY_INPUT_S)) {
		this->player->velocity.y = 1.0f;
	}
	else {
		// キー入力がない場合、速度を0にする
		this->player->velocity.x = 0.0f;
		player->SetNextState(ePlayerState::IDLE);
	}

	// 前回座標の更新
	old_location = player->GetLocation();
	AnimationControl(delta_second);
}

void RunState::Draw() const {
	DrawRotaGraphF(player->GetLocation().x, player->GetLocation().y, 1.5, 0.0, image, TRUE);
}

void RunState::AnimationControl(float delta_second) {
	// フレームカウントを加算する
	animation_count++;

	// 60フレーム目に到達したら
	if (animation_count >= 30) {
		// カウントリセット
		animation_count = 0;

		// 画像の切替
		if (image == animation[0]) {
			image = animation[1];
		}
		else {
			image = animation[0];
		}
	}
}

ePlayerState RunState::GetState() const {
	return ePlayerState::RUN;
}
