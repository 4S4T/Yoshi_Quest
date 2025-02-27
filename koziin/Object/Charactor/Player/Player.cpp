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

	// ?? “–‚½‚è”»’èŠm”F
	if (!mapReference->IsCollision(nextPosition.x, nextPosition.y)) {
		location = nextPosition;
	}
	else {
		velocity = Vector2D(0.0f, 0.0f);
	}
}

void Player::Draw(const Vector2D& screen_offset) const {
	__super::Draw(screen_offset);
	DrawBox(location.x - 24, location.y - 24, location.x + 24, location.y + 24, GetColor(255, 0, 0), FALSE);
	state->Draw();
}

Vector2D& Player::GetLocation() {
	return this->location;
}

void Player::SetNextState(ePlayerState next_state) {
	this->next_state = next_state;
}

void Player::OnHitCollision(GameObject* hit_object) {}
