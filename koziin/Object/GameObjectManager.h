#pragma once
#include "../Singleton.h"
#include "../Object/GameObject.h"
#include <vector>
#include"../Utility/Collsion.h"

class GameManager : public Singleton<class Type> {
protected:
	std::vector<GameObject*> create_objects;
	std::vector<GameObject*> object_list;
	std::vector<GameObject*> destroy_objects;
	std::vector<GameObject*> game_objects;
	Vector2D screen_offset;

public:
	GameManager();
	~GameManager();

	void Initialize();
	void Update(float);
	void Draw();
	void Finalize();
	void CheckCreateObject();
	void CheckDestroyObject();

	// const std::vector<GameObject*>& const GetObjectsList();

	template <class OBJECT>
	OBJECT* CreateGameObject(const Vector2D& generate_location) {
		// 生成するゲームオブジェクトクラスを動的確保
		OBJECT* new_instance = new OBJECT();
		// GameObjectBaseを継承しているか確認
		GameObject* new_object = dynamic_cast<GameObject*>(new_instance);
		// エラーチェック
		if (new_object == nullptr) {
			// 動的確保したメモリの開放
			delete new_instance;
			return nullptr;
		}
		// シーン情報の設定
		new_object->SetOwnerScene(this);
		// オブジェクトの初期化
		new_object->Initialize();
		// 位置情報の設定
		new_object->SetLocation(generate_location);
		// 生成オブジェクトリストの末尾に格納する
		create_objects.push_back(new_object);
		// 生成したゲームオブジェクトのポインタを返す
		return new_instance;
	}

	/// <summary>
	/// オブジェクト破棄処理
	/// </summary>
	/// <param name="target">破棄を行うゲームオブジェクトのポインタ</param>
	void DestroyObject(GameObject* target) {
		// ヌルポチェック
		if (target == nullptr) {
			return;
		}

		// 破棄オブジェクトリスト内にあれば、追加しない
		for (GameObject* obj : destroy_objects) {
			if (obj == target) {
				return;
			}
		}

		// 破棄を行うオブジェクトリストに追加する
		destroy_objects.push_back(target);
	}

private:
	/// <summary>
	/// シーン内オブジェクト破棄処理
	/// </summary>
	void DestoryAllObject() {
		// オブジェクトリストが空なら処理を終了する
		if (object_list.empty()) {
			return;
		}
		// オブジェクトリスト内のオブジェクトを削除する
		for (GameObject* obj : object_list) {
			obj->Finalize();
			delete obj;
		}
		// 動的配列の解放
		object_list.clear();
	}

	/// <summary>
	/// 当たり判定確認処理
	/// </summary>
	/// <param name="target">1つ目のゲームオブジェクト</param>
	/// <param name="partner">2つ目のゲームオブジェクト</param>
	virtual void CheckCollision(GameObject* target, GameObject* partner) {
		// ヌルポチェック
		if (target == nullptr || partner == nullptr) {
			return;
		}

		// 当たり判定情報を取得
		Collision tc = target->GetCollision();
		Collision pc = partner->GetCollision();

		// 当たり判定が有効か確認する
		if (tc.IsCheckHitTarget(pc.object_type) || pc.IsCheckHitTarget(tc.object_type)) {

			// 線分の始点と終点を設定する
			tc.point[0] += target->GetLocation() - target->GetBoxSize();
			tc.point[1] += target->GetLocation() + target->GetBoxSize();
			pc.point[0] += partner->GetLocation() - partner->GetBoxSize();
			pc.point[1] += partner->GetLocation() + partner->GetBoxSize();

			// カプセル同士の当たり判定
			if (IsCheckCollision(tc, pc)) {
				// 当たっていることを通知する
				target->OnHitCollision(partner);
				partner->OnHitCollision(target);
			}
		}
	}
};

