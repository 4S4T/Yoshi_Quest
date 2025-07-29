#pragma once

#include "../SceneBase.h"

enum class SubMenuState {
	None,	// 何も選択されてないメニュー状態
	Option, // オプション表示中
	Credit	// クレジット表示中
};

class Memu : public SceneBase {
private:
	int currentSelection;
	const char* menuItems[3] = { "設定", "クレジット", "マップに戻る" };
	const int menuItemCount = 3;
	SubMenuState subMenuState; // サブメニュー状態

public:
	Memu();
	~Memu();

	// 初期化処理
	virtual void Initialize() override;
	int OptionImage;


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