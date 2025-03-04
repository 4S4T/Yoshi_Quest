#include "Player.h"
#include "../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "State/Factory/PlayerStateFactory.h"
#include "../../../Utility/InputControl.h"
#include "../../../Scene/Map/Map.h"

Player::Player() {}
Player::~Player() {}



void Player::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	state = PlayerStateFactory::Get((*this), ePlayerState::IDLE);
	next_state = ePlayerState::NONE;
	// ?? PlayerData を使ってステータスを設定
	data.SetHp(20);
	data.SetAttack(10);
	data.SetDefense(10);
	is_battle = false;
}

void Player::Finalize() {
	PlayerStateFactory::Finalize();
}

void Player::Update(float delta_second) {
	if (next_state != ePlayerState::NONE) {
		state = PlayerStateFactory::Get((*this), next_state);
		next_state = ePlayerState::NONE;
	}

	state->Update(delta_second);

	Vector2D nextPosition = location + velocity;

	// ?? 当たり判定確認
	if (!mapReference->IsCollision(nextPosition.x, nextPosition.y))
	{
		location = nextPosition;
	}
	else {
		velocity = Vector2D(0.0f, 0.0f);
	}
}

void Player::Draw(const Vector2D& screen_offset) const {
	if (is_battle == false)
	{
	__super::Draw(screen_offset);
	DrawBox(location.x - 24, location.y - 24, location.x + 24, location.y + 24, GetColor(255, 0, 0), FALSE);
	state->Draw();
	}
	
}

Vector2D& Player::GetLocation() {
	return this->location;
}

void Player::SetIsBattle(bool isbattle)
{
	is_battle = isbattle;
}

void Player::SetNextState(ePlayerState next_state) {
	this->next_state = next_state;
}


void Player::OnHitCollision(GameObject* hit_object) {}
