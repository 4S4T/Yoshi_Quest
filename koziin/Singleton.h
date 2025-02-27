#pragma once

template <class Type>

class Singleton
{
protected:
	// クラスの実体をメンバ関数内でしか生成できないようにする
	Singleton() = default;
	// コピーガード
	Singleton(const Singleton& v) = delete;
	Singleton& operator=(const Singleton& v) = delete;
	// ~コピーガード

public:
	~Singleton() = default;

	static Type* GetInstance();
};

template <class Type>
inline Type* Singleton<Type>::GetInstance() {
	static Type* instance = nullptr;

	// インスタンスが生成されていない場合、生成する
	if (instance == nullptr) {
		instance = new Type();
	}

	// インスタンスのポインタを返却する
	return instance;
}
