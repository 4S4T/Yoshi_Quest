#pragma once

#include "../SceneBase.h"

class TitleScene2 : public SceneBase {
private:
public:
	TitleScene2();
	~TitleScene2();

	// 初期化処理
	virtual void Initialize() override;
	int TitleImage;
	int Title_name;
	int select;
	int menu_cursor;


	// 更新処理
	// 引数：1フレーム当たりの時間
	// 戻り値：次のシーンタイプ
	virtual eSceneType Update(float delta_second) override;

	// 描画処理
	virtual void Draw() override;

	// 終了時処理
	virtual void Finalize() override;

	// 現在のシーンタイプ（オーバーライド必須）
	virtual eSceneType GetNowSceneType() const override;
};