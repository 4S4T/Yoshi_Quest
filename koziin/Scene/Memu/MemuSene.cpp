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
	OptionImage = rm->GetImages("Resource/Images/option.png", 1, 1, 1, 32, 32)[0];
	currentSelection = 0; // 最初の項目を選択状態に
	subMenuState = SubMenuState::None;
}

eSceneType Memu::Update(float delta_second) {

    InputControl* input = Singleton<InputControl>::GetInstance();

    if (subMenuState == SubMenuState::None) {
        // メニュー項目選択中
        if (input->GetKeyDown(KEY_INPUT_UP)) {
            currentSelection = (currentSelection - 1 + menuItemCount) % menuItemCount;
        }
        if (input->GetKeyDown(KEY_INPUT_DOWN)) {
            currentSelection = (currentSelection + 1) % menuItemCount;
        }
		if (input->GetKeyDown(KEY_INPUT_RETURN)||(input->GetKeyDown(KEY_INPUT_SPACE))) {
            switch (currentSelection) {
                case 0: // 設定
                    subMenuState = SubMenuState::Option;
                    break;
                case 1: // クレジット
                    subMenuState = SubMenuState::Credit;
                    break;
                case 2: // マップに戻る
                    return eSceneType::eMap;
            }
        }
    }
    else {
        // サブメニュー表示中
		if (input->GetKeyDown(KEY_INPUT_TAB) || input->GetKeyDown(KEY_INPUT_ESCAPE) || (input->GetKeyDown(KEY_INPUT_SPACE))) {
            // 戻るキーでサブメニュー終了
            subMenuState = SubMenuState::None;
        }
    }

    return GetNowSceneType();
}



void Memu::Draw() {

	if (subMenuState == SubMenuState::None) {
		// メニュー選択肢表示
		for (int i = 0; i < menuItemCount; ++i) {
			int color = (i == currentSelection) ? GetColor(255, 255, 0) : GetColor(255, 255, 255);
			DrawFormatString(120, 200 + i * 40, color, menuItems[i]);
		}
	}
	else if (subMenuState == SubMenuState::Option) {
		// オプション情報を描画
		DrawFormatString(120, 200, GetColor(255, 255, 255), "=== オプション画面 ===");
		DrawFormatString(120, 240, GetColor(255, 255, 255), "音量調整や操作設定をここに表示");
		DrawFormatString(120, 280, GetColor(255, 255, 255), "戻るには TAB または ESC を押してください");
	}
	else if (subMenuState == SubMenuState::Credit) {
		// クレジット情報を描画
		DrawFormatString(120, 200, GetColor(255, 255, 255), "=== クレジット ===");
		DrawFormatString(120, 240, GetColor(255, 255, 255), "制作：あなたの名前");
		DrawFormatString(120, 280, GetColor(255, 255, 255), "協力：〇〇〇〇");
		DrawFormatString(120, 320, GetColor(255, 255, 255), "戻るには TAB または ESC を押してください");
	}
}


void Memu::Finalize() {
}

eSceneType Memu::GetNowSceneType() const {
	return eSceneType::eOption;
}
