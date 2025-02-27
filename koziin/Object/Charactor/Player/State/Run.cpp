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
	// ���x��0�ɂ���
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
	// ���͏����擾
	InputControl* input = Singleton<InputControl>::GetInstance();

	// �ړ�����
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
		// �L�[���͂��Ȃ��ꍇ�A���x��0�ɂ���
		this->player->velocity.x = 0.0f;
		player->SetNextState(ePlayerState::IDLE);
	}

	// �O����W�̍X�V
	old_location = player->GetLocation();
	AnimationControl(delta_second);
}

void RunState::Draw() const {
	DrawRotaGraphF(player->GetLocation().x, player->GetLocation().y, 1.5, 0.0, image, TRUE);
}

void RunState::AnimationControl(float delta_second) {
	// �t���[���J�E���g�����Z����
	animation_count++;

	// 60�t���[���ڂɓ��B������
	if (animation_count >= 30) {
		// �J�E���g���Z�b�g
		animation_count = 0;

		// �摜�̐ؑ�
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
