#pragma once

enum class ePlayerState : unsigned char
{
	IDLE = 0, // 停止状態
	RUN,	  // 移動状態
	NONE // 何もない
};

enum class PlayerCommandState {
	MainCommand, // 「たたかう」「にげる」など
	AttackTarget // 敵を選んで攻撃
};


extern PlayerCommandState commandState;
