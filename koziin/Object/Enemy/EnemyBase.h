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
	// ‰Šú‰»ˆ—
	virtual void Initialize() override;
	// •`‰æˆ—
	virtual void Draw(const Vector2D& screen_offset) const;
	// I—¹ˆ—
	virtual void Finalize();


	public:
	std::vector<int> GetAttckPatten();
	int GetDefense() const;
	// “_–Å§Œä—p
	bool isBlinking = false;
	float blinkTimer = 0.0f;
	void SetBlink(float duration);


	bool isVisible = true;

	void SetVisible(bool v) { isVisible = v; }
	bool IsVisible() const { return isVisible; }
};
