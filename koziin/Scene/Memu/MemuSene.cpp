#include "MemuSene.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../Battle/Battle.h"
#include "../../Utility/ResourceManager.h"

Memu::Memu() {
}

Memu::~Memu() {
}


void Memu::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	OptionImage = rm->GetImages("Resource/Images/˜g.png", 1, 1, 1, 32, 32)[0];
}

eSceneType Memu::Update(float delta_second) {
	// “ü—Í‹@”\‚ÌŽæ“¾
	InputControl* input = Singleton<InputControl>::GetInstance();


	if (input->GetKeyDown(KEY_INPUT_TAB)) {
		return eSceneType::eMap;
	}
	return GetNowSceneType();
}

void Memu::Draw() {
	DrawGraph(100,100, OptionImage, TRUE);
}

void Memu::Finalize() {
}

eSceneType Memu::GetNowSceneType() const {
	return eSceneType::eOption;
}
