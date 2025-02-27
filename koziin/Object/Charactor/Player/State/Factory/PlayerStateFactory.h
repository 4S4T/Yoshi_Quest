#pragma once
#include "../PlayerStateBase.h"


class PlayerStateFactory {
	// シングルトン用　ここから
private:
	static PlayerStateFactory* instance;

private:
	PlayerStateFactory() = default;
	PlayerStateFactory(const PlayerStateFactory&) = default;
	PlayerStateFactory& operator=(const PlayerStateFactory&) = default;
	// シングルトン用　ここまで

~PlayerStateFactory();

private:
	class IdleState* idle;
	class RunState* run;

private:
	void Initialize(class Player& player);

public:
	static PlayerStateBase* Get(class Player& player, ePlayerState state);
	static void Finalize();
};