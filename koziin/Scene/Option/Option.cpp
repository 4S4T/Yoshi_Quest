#include "Option.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"

Option::Option() 
{
}

Option::~Option()
{
}

void Option::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	OptionImage = rm->GetImages("Resource/Images/Option.png", 1, 1, 1, 32, 32)[0];
}

eSceneType Option::Update(float delta_second)
{
	// “ü—Í‹@”\‚ÌŽæ“¾
	InputControl* input = Singleton<InputControl>::GetInstance();


	if (input->GetKeyDown(KEY_INPUT_SPACE))
	{
		return eSceneType::eTitle;

	}
	return GetNowSceneType();
}

void Option::Draw() 
{
	DrawGraph(0, 0, OptionImage, TRUE);
}

void Option::Finalize() 
{
}

eSceneType Option::GetNowSceneType() const
{
	return eSceneType::eOption;
}
