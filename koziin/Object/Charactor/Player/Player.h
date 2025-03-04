#pragma once
#ifndef PLAYER_H
#define PLAYER_H


#include "../Charactor.h"
#include "State/Enum/PlayerState.h"
#include "../../../Utility/Vector2D.h"
#include "../../../Utility/PlayerData.h"
#include <vector>

class Player : public Character {
private:
	class PlayerStateBase* state = nullptr;
	std::vector<std::vector<char>> mapData;
	class Map* mapReference = nullptr;

public:
	Player();
	~Player();

	bool is_battle;

	void SetIsBattle(bool isbattle);



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

	void SetHp(int value) { PlayerData::GetInstance()->SetHp(value); }
	void SetAttack(int value) { PlayerData::GetInstance()->SetAttack(value); }
	void SetDefense(int value) { PlayerData::GetInstance()->SetDefense(value); }

	int GetHp() const { return PlayerData::GetInstance()->GetHp(); }
	int GetAttack() const { return PlayerData::GetInstance()->GetAttack(); }
	int GetDefense() const { return PlayerData::GetInstance()->GetDefense(); }


private:
	bool CanMoveTo(const Vector2D& newPosition) const;
	PlayerData data; // ?? PlayerData ‚ğƒƒ“ƒo•Ï”‚Æ‚µ‚Ä’Ç‰Á

};

#endif // PLAYER_H