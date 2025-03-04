#pragma once
#include "SceneBase.h"
#include "../Singleton.h"
#include"../Object/Charactor/Player/Player.h"

class SceneManager : public Singleton<class Type> {
private:
	SceneBase* current_scene; // 現在のシーン情報のポインタ
	/*static SceneManager* instance;*/
private:
	Player* player;


public:
	SceneManager();
	~SceneManager();

	static void DeleteInstance();

	// 初期化処理
	void Initialize();

	// 更新処理
	bool Update(float delta_second);

	// 終了時処理
	void Finalize();

	// シーン切替処理
	void ChangeScene(eSceneType);

public:
	// 描画処理
	void Draw() const;


	// シーン切替処理
	SceneBase* CreateScene(eSceneType type);
};