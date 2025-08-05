#pragma once
#include"../../Application.h"
#include"../GameObject.h"
#include"DxLib.h"

class EnemyBase : public GameObject
{
protected:
	std::vector<int> attck;
	
	EnemyBase();
	~EnemyBase();

	public:
	// ����������
	virtual void Initialize() override;
	// �`�揈��
	virtual void Draw(const Vector2D& screen_offset) const;
	// �I��������
	virtual void Finalize();


	public:
	std::vector<int> GetAttckPatten();
	int GetDefense() const;
	// �_�Ő���p
	bool isBlinking = false;
	float blinkTimer = 0.0f;
	void SetBlink(float duration);


	bool isVisible = true;

	void SetVisible(bool v) { isVisible = v; }
	bool IsVisible() const { return isVisible; }
};
