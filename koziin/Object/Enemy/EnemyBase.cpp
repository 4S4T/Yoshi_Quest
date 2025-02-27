#include "EnemyBase.h"

EnemyBase::EnemyBase() 
{
}

EnemyBase::~EnemyBase()
{
}

void EnemyBase::Initialize() 
{
}

void EnemyBase::Draw(const Vector2D& screen_offset) const
{
	__super::Draw(screen_offset);
}

void EnemyBase::Finalize() 
{

}

std::vector<int> EnemyBase::GetAttckPatten()
{
	return std::vector<int>();
}
