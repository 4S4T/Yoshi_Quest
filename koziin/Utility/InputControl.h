#pragma once
#include "../Singleton.h"

#define D_KEYCODE_MAX 256

// 入力制御クラス
class InputControl : public Singleton<class Type> {
private:
	static InputControl* instance;

	char now_key[D_KEYCODE_MAX]; // 現在の入力値
	char old_key[D_KEYCODE_MAX]; // 前回の入力値

public:
	InputControl();
	// コピーガード
	InputControl(const InputControl&) = default;
	InputControl& operator=(const InputControl& v) = default;
	//~コピーガード

public:
	~InputControl();

	// インスタンス取得処理
	static InputControl* GetInstance();
	// インスタンス削除処理
	static void DeleteInstance();

	// 入力値の更新処理
	void Update();

	// キー取得
	bool GetKey(int key_code) const;
	bool GetKeyDown(int key_code) const;
	bool GetKeyUp(int key_code) const;

private:
	// キー範囲チェック
	bool CheckKeyCodeRange(int key_code) const;
};