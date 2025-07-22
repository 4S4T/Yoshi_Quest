#pragma once
#include "../Object/GameObjectManager.h"

enum eSceneType {
	eTitle,
	eTitle2,
	eMap,
	eResult,
	eOption,
	eBattle,
	eEnd,
};

class SceneBase {
public:
	SceneBase() {}
	~SceneBase() {}

	// 初期化処理
	virtual void Initialize() {}

	// 更新処理
	// 引数：1フレーム当たりの時間
	// 戻り値：次のシーンタイプ
	virtual eSceneType Update(float aaaa) {
		return GetNowSceneType();
	}

	// 描画処理
	virtual void Draw() 
	{
		GameManager* obj = Singleton<GameManager>::GetInstance();
		obj->Draw();
	}

	// 終了時処理
	virtual void Finalize() 
	{

	}

	// 現在のシーンタイプ（オーバーライド必須）
	virtual eSceneType GetNowSceneType() const = 0;
};