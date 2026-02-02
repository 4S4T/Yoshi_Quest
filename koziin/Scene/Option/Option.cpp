#include "Option.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include"../../Utility/SoundManager.h"


Option::Option() 
{
}

Option::~Option()
{
}

void Option::Initialize()
{
	SoundManager::GetInstance().PlayBGM("Resource/Sounds/jo-jokyoku.mp3");
	Option_SE = LoadSoundMem("Resource/Sounds/音/選択/cancel.mp3");

	ResourceManager* rm = ResourceManager::GetInstance();
	OptionImage = rm->GetImages("Resource/Images/Titel.png", 1, 1, 1, 32, 32)[0];
}

eSceneType Option::Update(float delta_second)
{
	// 入力機能の取得
	InputControl* input = Singleton<InputControl>::GetInstance();


	if (input->GetKeyDown(KEY_INPUT_SPACE)) {
		PlaySoundMem(Option_SE, DX_PLAYTYPE_BACK);
		return eSceneType::eTitle2;
	}
	return GetNowSceneType();
}

void Option::Draw() 
{
	DrawGraph(0, 0, OptionImage, TRUE);
	
		// 背景（古文書）
		DrawGraph(0, 0, OptionImage, TRUE);

		SetFontSize(28);

		int textColor = GetColor(255, 255, 255);
		int edgeColor = GetColor(0, 0, 0);

		auto DrawEdgeText = [&](int x, int y, const char* text) {
			for (int dx = -1; dx <= 1; dx++) {
				for (int dy = -1; dy <= 1; dy++) {
					if (dx == 0 && dy == 0)
						continue;
					DrawFormatString(x + dx, y + dy, edgeColor, text);
				}
			}
			DrawFormatString(x, y, textColor, text);
		};

		DrawEdgeText(350, 180, "―― 操作方法 ――");

		DrawEdgeText(300, 250, "W/A/S/D");

		DrawEdgeText(450, 250, "移動");

		DrawEdgeText(300, 300, "TAB");
		DrawEdgeText(450, 300, "メニュー");

		DrawEdgeText(300, 350, "SPACE");
		DrawEdgeText(450, 350, "決定");

		DrawEdgeText(600, 600, "SPACEで戻る");

}


void Option::Finalize()
{

}


eSceneType Option::GetNowSceneType() const
{
	return eSceneType::eOption;
}
