#include "GameObject.h"
#include "../Scene/SceneBase.h"
#include "DxLib.h"




GameObject::GameObject() : owner_scene(nullptr), location(0.0f), image(NULL), z_layer(0) {
}

GameObject::~GameObject() {
}

void GameObject::Initialize() {
}

void GameObject::Update(float delta_second) {
}

void GameObject::Draw(const Vector2D& screen_offset) const {
	// オフセット値を基に画像の描画を行う
	Vector2D graph_location = this->location + screen_offset;
	Vector2D enemy_location = this->location + screen_offset;

	DrawRotaGraphF(graph_location.x, graph_location.y, 1.5, 0.0, image, TRUE);

}

void GameObject::Finalize() {
}

void GameObject::OnHitCollision(GameObject* hit_object) {
}

void GameObject::SetOwnerScene(GameManager* scene) {
	this->owner_scene = scene;
}

const Vector2D& GameObject::GetLocation() const {
	return location;
}

const int GameObject::GetAttack() {
	printf("Debug: GetAttack called, Attack = %d\n", attack);
	return attack;
}

void GameObject::SetAttack(const int attack) {
	this->attack = attack;
}

const int GameObject::GetDefense() {
	return defense;
}

void GameObject::SetDefense(const int defense) {
	this->defense = defense;
}

const int GameObject::GetAHp() {
	return hp;
}

void GameObject::SetHp(const int damage) {
	this->hp -= damage;
	if (this->hp < 0) {
		this->hp = 0; // HPが負の値にならないようにする
	}
}


void GameObject::SetLocation(const Vector2D& location) {
	this->location = location;
}

const int GameObject::GetZLayer() const {
	return z_layer;
}

Collision GameObject::GetCollision() const {
	return collision;
}

const eMobilityType GameObject::GetMobility() const {
	return mobility;
}

Vector2D GameObject::GetBoxSize() {
	return this->box_size;
}
