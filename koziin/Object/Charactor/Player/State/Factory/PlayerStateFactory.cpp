#include "PlayerStateFactory.h"

#include"../Idle.h"
#include"../Run.h"

#include"../../Player.h"

PlayerStateFactory* PlayerStateFactory::instance = nullptr;

PlayerStateFactory::~PlayerStateFactory() {
}

void PlayerStateFactory::Initialize(Player& player) {
	idle = new IdleState(&player);
	run = new RunState(&player);
}

PlayerStateBase* PlayerStateFactory::Get(Player& player, ePlayerState state) 
{
	PlayerStateBase* ret = nullptr;

	if (instance == nullptr)
	{
		instance = new PlayerStateFactory();
		instance->Initialize(player);
	}

	

	switch (state)
	{
	case ePlayerState::IDLE:
		instance->idle->Initialize();
		ret = instance->idle;
		break;
	case ePlayerState::RUN:
		instance->run->Initialize();
		ret = instance->run;
		break;
	case ePlayerState::NONE: // •Ô‚·‚à‚Ì‚È‚µ
	default:
		break;
	}
	return ret;
}

void PlayerStateFactory::Finalize() {
	if (instance == nullptr)
		return;

	if (instance->idle != nullptr) {
		instance->idle->Finalize();
		delete instance->idle;
		instance->idle = nullptr; 
	}
	if (instance->run != nullptr) {
		instance->run->Finalize();
		delete instance->run;
		instance->run = nullptr; 
	}

	delete instance;	
	instance = nullptr; 
}
