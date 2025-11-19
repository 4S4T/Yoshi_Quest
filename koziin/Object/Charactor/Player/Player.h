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
	void Draw(const Vector2D& screen_offset) const override;
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
	PlayerData data; // PlayerData をメンバ変数として追加

	// ================================
	// 分割スプライト用（左上のキャラ 3×4 コマ）
	// ================================
	int spriteSheetHandle = -1; // 1枚のシート画像ハンドル
	int animFrame = 1;			// 0〜2（横方向のコマ）
	int direction = 0;			// 0:下, 1:左, 2:右, 3:上
	float animTimer = 0.0f;

	// ★ 1コマ 48×48 （576/12, 384/8）
	static constexpr int FRAME_W = 48;
	static constexpr int FRAME_H = 48;

	void UpdateDirectionFromVelocity();
	void UpdateAnimation(float delta_second);
};

#endif // PLAYER_H
