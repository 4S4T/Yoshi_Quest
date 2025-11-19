#pragma once
#include "PlayerStateBase.h"
#include "../../../../Utility/Vector2D.h"
#include <vector>

/// <summary>
/// 移動状態
/// </summary>
class RunState : public PlayerStateBase {
private:
	std::vector<int> player_animation; // 今は未使用でもOK
	int animation[2];				   // 今は未使用でもOK
	Vector2D old_location;
	float animation_time;
	int animation_count;

	const int animation_num[1] = {
		0
	};

public:
	RunState(class Player* p);
	virtual ~RunState();

public:
	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void Update(float delta_second) override;
	virtual void Draw() const override;
	void AnimationControl(float delta_second);

	virtual ePlayerState GetState() const override;
};
