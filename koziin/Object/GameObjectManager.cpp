#include "GameObjectManager.h"
#include "DxLib.h"

GameManager::GameManager() : create_objects(), destroy_objects(), game_objects(), screen_offset() {
}

GameManager::~GameManager() {
}

void GameManager::Initialize() {
}

void GameManager::Update(float delta_second) {
	CheckCreateObject();

	// 当たり判定確認処理
	for (int i = 0; i < object_list.size(); i++) {
		if (object_list[i]->GetMobility() == eMobilityType::Stationary) {
			continue;
		} 

		for (int j = 0; j < object_list.size(); j++) {

			if (i == j) {
				continue;
			}

			CheckCollision(object_list[i], object_list[j]);
		}
	}

	for (GameObject* object : object_list) {
		object->Update(delta_second);
	}

	CheckDestroyObject();
}

void GameManager::Draw() {
	// オブジェクトリスト内のオブジェクトを描画する
	for (GameObject* obj : object_list) {
		// obj->Initialize();
		obj->Draw(screen_offset);
	}
}

void GameManager::Finalize() {
	// オブジェクトリスト内のオブジェクトを破棄する
	DestoryAllObject();

	// 動的配列の解放
	if (!object_list.empty()) {
		object_list.clear();
	}
	/*if (!create_objects.empty())
	{
		create_objects.clear();
	}*/
}

void GameManager::CheckCreateObject() {
	// 生成するオブジェクトがあれば、オブジェクトリスト内に挿入する
	if (!create_objects.empty()) {
		for (GameObject* obj : create_objects) {
			// レイヤー情報を基に順番を入れ替える
			int z_layer = obj->GetZLayer();
			std::vector<GameObject*>::iterator itr = object_list.begin(); // オブジェクトリストの先頭
			// リストの末尾になるまで走査する
			for (; itr != object_list.end(); itr++) {
				// Zレイヤーが大きい場所に要素を追加する
				// 例 itr->ZLayer{1, 1, 2, 3}、z_layer = 2 の時
				//    itr->ZLayer{1, 1, 2, z_layer, 3}とする
				if (z_layer < (*itr)->GetZLayer()) {
					break;
				}
			}
			// リストの途中に挿入する
			object_list.insert(itr, obj);
		}
		// 生成リストを解放する
		create_objects.clear();
	}
}

void GameManager::CheckDestroyObject() {
	// 破棄リスト内が空でない場合、リスト内のオブジェクトを破棄する
	if (!destroy_objects.empty()) {
		for (GameObject* obj : destroy_objects) {
			std::vector<GameObject*>::iterator itr = object_list.begin(); // オブジェクトリストの先頭
			// リストの末尾になるまで走査する
			for (; itr != object_list.end(); itr++) {
				// リスト内にあれば、削除する
				if ((*itr) == obj) {
					object_list.erase(itr);
					obj->Finalize();
					delete obj;
					break;
				}
			}
		}
		// 動的配列の開放
		destroy_objects.clear();
	}
}
