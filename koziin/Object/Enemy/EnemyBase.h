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
};
