#pragma once

template <class Type>

class Singleton
{
protected:
	// �N���X�̎��̂������o�֐����ł��������ł��Ȃ��悤�ɂ���
	Singleton() = default;
	// �R�s�[�K�[�h
	Singleton(const Singleton& v) = delete;
	Singleton& operator=(const Singleton& v) = delete;
	// ~�R�s�[�K�[�h

public:
	~Singleton() = default;

	static Type* GetInstance();
};

template <class Type>
inline Type* Singleton<Type>::GetInstance() {
	static Type* instance = nullptr;

	// �C���X�^���X����������Ă��Ȃ��ꍇ�A��������
	if (instance == nullptr) {
		instance = new Type();
	}

	// �C���X�^���X�̃|�C���^��ԋp����
	return instance;
}
