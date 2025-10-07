#include"../Title/Title2.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"

TitleScene2::TitleScene2() {
}

TitleScene2::~TitleScene2() {
}

void TitleScene2::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	TitleImage = rm->GetImages("Resource/Images/Title.jpg", 1, 1, 1, 32, 32)[0];
	Title_name = rm->GetImages("Resource/Images/2.png", 1, 1, 1, 32, 32)[0];
	select = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32)[0];
	menu_cursor = 1;

	int x = 380;
	int y = 620;
	const char* text = "冒険を終わる";

	// 縁の色（黒）
	int edgeColor = GetColor(0, 0, 0);
	// 本体の色（白）
	int textColor = GetColor(255, 255, 255);

	// 縁を上下左右＋斜めに描画（8方向）
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			if (dx != 0 || dy != 0) {
				DrawFormatString(x + dx, y + dy, edgeColor, text);
			}
		}
	}

}

eSceneType TitleScene2::Update(float delta_second) {
	// 入力機能の取得
	InputControl* input = Singleton<InputControl>::GetInstance();

	if (input->GetKeyDown(KEY_INPUT_DOWN)) {
		menu_cursor++;

		if (menu_cursor > 4) {
			menu_cursor = 1;
		}
	}

	if (input->GetKeyDown(KEY_INPUT_UP)) {
		menu_cursor--;

		if (menu_cursor < 1) {
			menu_cursor = 4;
		}
	}

	if (input->GetKeyDown(KEY_INPUT_SPACE)) {
		if (menu_cursor == 1) {
			return eSceneType::eMap;
		}

		if (menu_cursor == 3) {
			return eSceneType::eOption;
		}

		if (menu_cursor == 4) {
			return eSceneType::eEnd;
		}
	}

	return GetNowSceneType();
}

void TitleScene2::Draw() {
	DrawGraph(0, 0, TitleImage, TRUE);
	DrawRotaGraph(500, 250, 0.5f, 0.0, Title_name, TRUE);
	DrawRotaGraph(350, 430 + menu_cursor * 50, 0.1, 0, select, TRUE);

	// 現在の経過時間（ミリ秒）
	int time = GetNowCount();

	//点滅（0.5秒ON、0.5秒OFF）
	bool blink = (time / 500) % 2 == 0;

	SetFontSize(16);

	// 各メニュー項目を描画
	for (int i = 1; i <= 4; i++) {
		int y = 470 + (i - 1) * 50;
		const char* text = "";

		switch (i) {
		case 1:
			text = "冒険を始める";
			break;
		case 2:
			text = "チュートリアル";
			break;
		case 3:
			text = "オプション";
			break;
		case 4:
			text = "冒険を終わる";
			break;
		}

		// 選択中の項目を点滅させる
		if (i == menu_cursor) {
			if (blink) {
				DrawFormatString(380, y, GetColor(255, 255, 0), text); // 黄色で点滅
			}
		}
		else {
			DrawFormatString(380, y, GetColor(255, 255, 255), text);
		}
	}
}


void TitleScene2::Finalize() {
}

eSceneType TitleScene2::GetNowSceneType() const {
	return eSceneType::eTitle2;
}