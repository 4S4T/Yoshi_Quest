#include "InputControl.h"
#include "DxLib.h"

// 静的メンバ変数
InputControl* InputControl::instance = nullptr;

InputControl::InputControl() {
	memset(now_key, false, (sizeof(char) * D_KEYCODE_MAX));
	memset(old_key, false, (sizeof(char) * D_KEYCODE_MAX));
}

InputControl::~InputControl() {
	instance = nullptr;
}

// インスタンス取得処理
InputControl* InputControl::GetInstance() {
	// インスタンスが生成されていない場合、生成する
	if (instance == nullptr) {
		instance = new InputControl();
	}
	return instance;
}

void InputControl::DeleteInstance() {
	if (instance != nullptr) {
		delete instance;
	}
}

// 入力値の更新処理
void InputControl::Update() {
	// 前回入力値の更新
	memcpy(old_key, now_key, (sizeof(char) * D_KEYCODE_MAX));
	// 現在入力値の更新
	GetHitKeyStateAll(now_key);
}


bool InputControl::GetKey(int key_code) const {
	return (CheckKeyCodeRange(key_code) && (now_key[key_code] == TRUE));
}


// キーが押された瞬間か確認する
bool InputControl::GetKeyDown(int key_code) const {
	return (CheckKeyCodeRange(key_code) && (now_key[key_code] == TRUE && old_key[key_code] == FALSE));
}

// キーが離された瞬間か確認する
bool InputControl::GetKeyUp(int key_code) const {
	return (CheckKeyCodeRange(key_code) && (now_key[key_code] == FALSE && old_key[key_code] == TRUE));
}

// キーコードの範囲内か確認する処理
bool InputControl::CheckKeyCodeRange(int key_code) const {
	return (0 <= key_code && key_code < D_KEYCODE_MAX);
}
