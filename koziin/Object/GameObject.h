#pragma once
#include "../Utility/Vector2D.h"
#include"../Utility/Collsion.h"

/// <summary>
/// 可動性の種類
/// </summary>
enum eMobilityType {
	Stationary, // 固定
	Movable,	// 可動
};

class GameObject {

protected:
	class GameManager* owner_scene;
	Vector2D location;
	unsigned char z_layer;
	int image;
	int image2;
	Vector2D box_size;
	Collision collision;
	eMobilityType mobility; // 可動性
	bool MapCollision(int x, int y);
	int attack;
	int defense;
	int hp;
	bool alive;


public:
	GameObject();
	virtual ~GameObject();;

	/// <summary>
	/// 初期化処理
	/// </summary>
	virtual void Initialize();
	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="delta_second">1フレームあたりの時間</param>
	virtual void Update(float delta_second);
	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="screen_offset">オフセット値</param>
	virtual void Draw(const Vector2D& screen_offset) const;
	/// <summary>
	/// 終了時処理
	/// </summary>
	virtual void Finalize();

public:
	/// <summary>
	/// 当たり判定通知処理
	/// </summary>
	/// <param name="hit_object">当たったゲームオブジェクトのポインタ</param>
	virtual void OnHitCollision(GameObject* hit_object);

public:
	/// <summary>
	/// 所有シーン情報の設定
	/// </summary>
	/// <param name="scene">所有シーン情報</param>
	void SetOwnerScene(class GameManager* scene);

	/// <summary>
	/// 位置座標取得処理
	/// </summary>
	/// <returns>位置座標情報</returns>
	const Vector2D& GetLocation() const;

	const int GetAttack();
	void SetAttack(const int attack);

	const int GetDefense();


	void SetDefense(const int defense);
	const int GetAHp();
	void SetHp(const int hp);

	/// <summary>
	/// 位置情報変更処理
	/// </summary>
	/// <param name="location">変更したい位置情報</param>
	void SetLocation(const Vector2D& location);


	/// <summary>
	/// Ｚレイヤー情報取得処理
	/// </summary>
	/// <returns>Ｚレイヤー情報</returns>
	const int GetZLayer() const;

	Collision GetCollision() const;

	/// <summary>
	/// 可動性情報の取得処理
	/// </summary>
	/// <returns>可動性情報</returns>
	const eMobilityType GetMobility() const;

	Vector2D GetBoxSize();
};