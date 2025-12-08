#pragma once

#include "../SceneBase.h"

class TitleScene : public SceneBase {
private:
	int cursorSound = -1;

public:
	TitleScene();
	~TitleScene();

	// 初期化処理
	virtual void Initialize() override;
	int TitleImage;
	int Title_name;
	int select;
	int menu_cursor;

	// 更新処理
	virtual eSceneType Update(float delta_second) override;

	// 描画処理
	virtual void Draw() override;

	// 終了時処理
	virtual void Finalize() override;

	// 現在のシーンタイプ
	virtual eSceneType GetNowSceneType() const override;
};
