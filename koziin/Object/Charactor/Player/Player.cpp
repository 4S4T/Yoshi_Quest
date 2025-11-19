#include "Player.h"
#include "../../../Utility/ResourceManager.h"
#include "DxLib.h"
#include "State/Factory/PlayerStateFactory.h"
#include "../../../Utility/InputControl.h"
#include "../../../Scene/Map/Map.h"
#include <cmath>

Player::Player() {}
Player::~Player() {}

void Player::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();

	//--- スプライトシート読み込み ---//
	spriteSheetHandle = LoadGraph("Resource/Images/yoshi.png");

	// 失敗時デバッグ用の絶対パス（必要なければ消してOK）
	if (spriteSheetHandle < 0) {
		spriteSheetHandle = LoadGraph(
			"C:\\Users\\kbc12305027\\Documents\\GitHub\\Yoshi_Quest\\koziin\\Resource\\Images\\yoshi.png");
	}

	animFrame = 1; // 真ん中のコマで待機
	direction = 0; // 下向き
	animTimer = 0.0f;

	// 状態
	state = PlayerStateFactory::Get((*this), ePlayerState::IDLE);
	next_state = ePlayerState::NONE;

	// ステータス
	data.SetHp(20);
	data.SetAttack(10);
	data.SetDefense(10);
	is_battle = false;
}

void Player::Finalize() {
	if (spriteSheetHandle >= 0) {
		DeleteGraph(spriteSheetHandle);
		spriteSheetHandle = -1;
	}
	PlayerStateFactory::Finalize();
}

void Player::Update(float delta_second) {
	// ステート切り替え
	if (next_state != ePlayerState::NONE) {
		state = PlayerStateFactory::Get((*this), next_state);
		next_state = ePlayerState::NONE;
	}

	// Idle / Run が velocity を決める
	state->Update(delta_second);

	// 向きとアニメ更新
	UpdateDirectionFromVelocity();
	UpdateAnimation(delta_second);

	// 位置更新 & 当たり判定
	Vector2D nextPosition = location + velocity;

	if (!mapReference || !mapReference->IsCollision(nextPosition.x, nextPosition.y)) {
		location = nextPosition;
	}
	else {
		velocity = Vector2D(0.0f, 0.0f);
	}
}

void Player::Draw(const Vector2D& screen_offset) const {
	if (is_battle) {
		return; // 戦闘中はここでは描画しない
	}

	// 読み込み失敗時のデバッグ表示
	if (spriteSheetHandle < 0) {
		DrawString(10, 10, "yoshi load fail", GetColor(255, 0, 0));
	}
	else {
		// ================================
		// 左上のキャラ 3×4 コマから 1コマだけ描画
		//
		//  タイルサイズ: 48×48
		//  列: 0〜2 (歩行のコマ)
		//  行: 0=下,1=左,2=右,3=上
		// ================================
		int col = animFrame; // 0,1,2
		int row = direction; // 0〜3

		// このキャラはシートの「一番左上」にいるので、
		// そのまま col,row を使えば OK
		int sx = FRAME_W * col;
		int sy = FRAME_H * row;

		DrawRectGraphF(
			location.x - FRAME_W / 2.0f,
			location.y - FRAME_H / 2.0f,
			sx, sy,
			FRAME_W, FRAME_H,
			spriteSheetHandle,
			TRUE);
	}

	// デバッグ用当たり判定ボックス
	DrawBox(
		location.x - 24, location.y - 24,
		location.x + 24, location.y + 24,
		GetColor(255, 0, 0),
		FALSE);
}

Vector2D& Player::GetLocation() {
	return this->location;
}

void Player::SetIsBattle(bool isbattle) {
	is_battle = isbattle;
}

void Player::SetNextState(ePlayerState next_state) {
	this->next_state = next_state;
}

void Player::OnHitCollision(GameObject* hit_object) {
	// ここはまだ何もしない
}

//--------------------------------------
// velocity から向きを決める
//--------------------------------------
void Player::UpdateDirectionFromVelocity() {
	if (velocity.x == 0.0f && velocity.y == 0.0f)
		return;

	if (std::fabs(velocity.x) > std::fabs(velocity.y)) {
		direction = (velocity.x > 0.0f) ? 2 : 1; // 右:2 左:1
	}
	else {
		direction = (velocity.y > 0.0f) ? 0 : 3; // 下:0 上:3
	}
}

//--------------------------------------
// 歩行アニメ（3コマ）
//--------------------------------------
void Player::UpdateAnimation(float delta_second) {
	const float WALK_INTERVAL = 0.15f; // 1コマ 0.15秒

	if (velocity.x == 0.0f && velocity.y == 0.0f) {
		animFrame = 1; // 止まっているときは真ん中
		animTimer = 0.0f;
		return;
	}

	animTimer += delta_second;
	if (animTimer >= WALK_INTERVAL) {
		animTimer -= WALK_INTERVAL;
		animFrame = (animFrame + 1) % 3; // 0→1→2→0…
	}
}
