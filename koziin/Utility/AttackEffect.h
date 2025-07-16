#pragma once
#include"Vector2D.h"
#include "DxLib.h"

class AttackEffect {
public:
	Vector2D position; // エフェクトの位置
	float duration;	   // エフェクトの持続時間
	int currentFrame;  // 現在のフレーム
	int totalFrames;   // エフェクトのフレーム数
	int effectImage;   // エフェクトの画像
	bool isActive;	   // エフェクトがアクティブかどうか

	AttackEffect(Vector2D pos) {
		position = pos;
		duration = 0.5f; // エフェクトが続く時間（例えば0.5秒）
		currentFrame = 0;
		totalFrames = 5;												// 例えば5フレームのスプライト
		effectImage = LoadGraph("Resource/Images/attack_effect_%.png"); // スプライトシートのパスを指定
		isActive = true;
	}

	void Update(float delta_second) {
		if (isActive) {
			duration -= delta_second;
			if (duration <= 0.0f) {
				isActive = false; // エフェクト終了
			}
			else {
				// アニメーションを進める
				currentFrame++;
				if (currentFrame >= totalFrames) {
					currentFrame = totalFrames - 1; // 最後のフレームで止まる
				}
			}
		}
	}

	void Draw() {
		if (isActive) {
			// スプライトシートから現在のフレームを表示
			DrawGraph(position.x, position.y, effectImage + currentFrame, TRUE);
		}
	}
};
