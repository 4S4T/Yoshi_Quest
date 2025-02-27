#pragma once

#include "../Charactor.h"
#include "State/Enum/PlayerState.h"
#include "../../../Utility/Vector2D.h"
#include <vector>

class Player : public Character {
private:
	class PlayerStateBase* state = nullptr;
	std::vector<std::vector<char>> mapData;
	class Map* mapReference = nullptr;

public:
	Player();
	~Player();

	ePlayerState next_state = ePlayerState::NONE;
	ePlayerState now_state = ePlayerState::NONE;

	void Initialize() override;
	void Finalize();
	void Update(float delta_second);
	void Draw(const Vector2D& screen_offset) const;
	Vector2D& GetLocation();
	void SetNextState(ePlayerState next_state);
	void OnHitCollision(GameObject* hit_object) override;

	void SetMapData(const std::vector<std::vector<char>>& data) { mapData = data; }
	void SetMapReference(Map* map) { mapReference = map; }

private:
	bool CanMoveTo(const Vector2D& newPosition) const;
};
