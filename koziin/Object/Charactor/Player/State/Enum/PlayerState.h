#pragma once

enum class ePlayerState : unsigned char
{
	IDLE = 0, // ’â~ó‘Ô
	RUN,	  // ˆÚ“®ó‘Ô
	NONE // ‰½‚à‚È‚¢
};

enum class PlayerCommandState {
	MainCommand, // í‚¤E“¦‚°‚é“™‚ğ‘I‚Ô
	EnemySelect	 // “G‚ğ‘I‚Ô
};

PlayerCommandState commandState;
