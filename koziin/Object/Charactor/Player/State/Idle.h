#pragma once
#include "PlayerStateBase.h"

/// <summary>
/// í‚é~èÛë‘
/// </summary>
class IdleState : public PlayerStateBase {
private:
	class InputControl* input;

public:
	IdleState(class Player* p);
	virtual ~IdleState();

public:
	virtual void Initialize() override;
	virtual void Finalize() override;
	virtual void Update(float delta_second) override;
	virtual void Draw() const override;

	virtual ePlayerState GetState() const override;
};
