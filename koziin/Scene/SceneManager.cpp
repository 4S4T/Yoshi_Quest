#include "SceneManager.h"
#include "../Utility/InputControl.h"
#include "DxLib.h"
#include"Title/Title.h"
#include"Title/Title2.h"
#include"Option/Option.h"
#include"Map/Map.h"
#include"Battle/Battle.h"

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
	// 解放忘れ防止
	Finalize();
}

void SceneManager::DeleteInstance() {
	static SceneManager* instance;

	if (instance != nullptr) {
		delete instance;
		instance = nullptr;
	}
}

/// <summary>
/// 初期化処理
/// </summary>
void SceneManager::Initialize() {
}

bool SceneManager::Update(float delta_second) {
	// シーンの更新処理
	eSceneType next_scene_type = current_scene->Update(delta_second);

	if (next_scene_type == eSceneType::eEnd)
	{
		return true;
	}

	// シーンの描画処理
	Draw();

	// シーン切り替え
	if (next_scene_type != current_scene->GetNowSceneType()) {
		ChangeScene(next_scene_type);
	}

	return false;
}

/// <summary>
/// 終了時処理
/// </summary>
void SceneManager::Finalize() {
	// シーン情報が生成されていれば、削除する
	if (current_scene != nullptr) {
		current_scene->Finalize();
		delete current_scene;
		current_scene = nullptr;
	}

	// 入力機能のインスタンスを削除
	InputControl::DeleteInstance();

	// DXライブラリの終了処理
	DxLib_End();
}

/// <summary>
/// 描画処理
/// </summary>
void SceneManager::Draw() const {
	// シーンの描画処理
	current_scene->Draw();
}

/// <summary>
/// シーン切替処理
/// </summary>
/// <param name="type">シーン情報</param>
void SceneManager::ChangeScene(eSceneType type) {
	SceneBase* new_scene = CreateScene(type);
	if (new_scene == nullptr) {
		throw("シーンが生成できませんでした\n");
	}

	if (type == eSceneType::eBattle) {
		Map* mapScene = dynamic_cast<Map*>(current_scene);
		BattleScene* battleScene = dynamic_cast<BattleScene*>(new_scene);

		if (!mapScene) {
			printf("Error: current_scene is not a Map instance!\n");
			return;
		}
		if (!battleScene) {
			printf("Error: new_scene is not a BattleScene instance!\n");
			return;
		}

		if (!mapScene->player) {
			printf("Error: mapScene->player is nullptr! Initializing new player.\n");
			mapScene->player = new Player(); // 仮の初期化（メモリ管理に注意）
		}

		battleScene->SetPlayerPosition(mapScene->player->GetLocation());
		battleScene->SetPlayer(mapScene->player);
	}


	if (current_scene != nullptr) {
		current_scene->Finalize();
		current_scene = new_scene;
	}

	new_scene->Initialize();
	current_scene = new_scene;
}



/// <summary>
/// シーン生成処理
/// </summary>
/// <param name="type">シーン情報</param>
/// <returns>シーンクラスのポイント</returns>
SceneBase* SceneManager::CreateScene(eSceneType type) {

	// シーン情報によって、生成するシーンを切り替える
	switch (type) {
	case eTitle:
		return dynamic_cast<SceneBase*>(new TitleScene());
	case eTitle2:
		return dynamic_cast<SceneBase*>(new TitleScene2());
	case eMap:
		return dynamic_cast<SceneBase*>(new Map());
	case eOption:
		return dynamic_cast<SceneBase*>(new Option());
	case eBattle:
		return dynamic_cast<SceneBase*>(new BattleScene());

	default:
		return nullptr;
	}
}
