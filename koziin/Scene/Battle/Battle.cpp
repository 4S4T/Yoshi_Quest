#include "Battle.h"
#include "../../Utility/InputControl.h"
#include "DxLib.h"
#include "../SceneManager.h"
#include "../../Utility/ResourceManager.h"
#include "../../Utility/PlayerData.h"
#include "../../Object/GameObjectManager.h"
#include <unordered_map>
#include <cmath>

// �� �ǉ��F���W�X�g���^�v�Z�^AI
#include "SpellRegistry.h"
#include "DamageCalculator.h"
#include "EnemyAI.h"

static const int SCREEN_W = 960;
static const int SCREEN_H = 720;




//--------------------------------------
// �R���X�g���N�^ / �f�X�g���N�^ / �Z�b�^�[
//--------------------------------------
BattleScene::BattleScene() {}
BattleScene::~BattleScene() {}
void BattleScene::SetPlayer(Player* p) { this->player = p; }

//--------------------------------------
// ���b�Z�[�W�i�蓮����j
//--------------------------------------
void BattleScene::enqueueMessage(const std::string& text) { messageQueue.push(text); }

void BattleScene::beginMessages(BattleState nextState) {
	messageNextState = nextState;
	battleState = BattleState::Message;
	currentMessage.clear();
	
}

void BattleScene::pumpMessageManual() {
	InputControl* input = Singleton<InputControl>::GetInstance();

	if (currentMessage.empty()) {
		if (!messageQueue.empty()) {
			currentMessage = messageQueue.front();
			messageQueue.pop();
		}
		else {
			battleState = messageNextState;
			return;
		}
	}
	if (input && input->GetKeyDown(KEY_INPUT_SPACE)) {
		currentMessage.clear();
	}
}

//--------------------------------------
// �G���[�e�B���e�B
//--------------------------------------
int BattleScene::livingEnemyCount() const {
	int c = 0;
	for (const auto& e : enemies)
		if (!e.defeated && e.getHp() > 0)
			c++;
	return c;
}
int BattleScene::firstLivingIndex() const {
	for (int i = 0; i < (int)enemies.size(); ++i)
		if (!enemies[i].defeated && enemies[i].getHp() > 0)
			return i;
	return -1;
}
int BattleScene::nextLivingIndex(int start, int dir) const {
	if (enemies.empty())
		return -1;
	int n = (int)enemies.size();
	int idx = start;
	for (int step = 0; step < n; ++step) {
		idx = (idx + dir + n) % n;
		if (!enemies[idx].defeated && enemies[idx].getHp() > 0)
			return idx;
	}
	return -1;
}
void BattleScene::onEnemyDefeated(EnemyHandle& e) {
	e.defeated = true;
	e.setVisible(false);
	totalEarnedExp += e.expValue;
	enqueueMessage(e.displayName + "�� ���������I");
}
void BattleScene::giveAllExpAndExit() {
	if (totalEarnedExp > 0) {
		PlayerData::GetInstance()->AddExperience(totalEarnedExp);
		totalEarnedExp = 0;
	}
}

//--------------------------------------
// �����F�d�ݒ��I �� �����i�����񂹁j�{ �d������A/B/C�c�t�^
//--------------------------------------
int BattleScene::chooseEnemyType() const {
	if (encounter.enemyTypeWeights.empty())
		return 0;
	int sum = 0;
	for (int i = 0; i < (int)encounter.enemyTypeWeights.size(); ++i) {
		int w = encounter.enemyTypeWeights[i];
		if (w < 0)
			w = 0;
		sum += w;
	}
	if (sum <= 0)
		return 0;
	int r = GetRand(sum - 1), acc = 0;
	for (int i = 0; i < (int)encounter.enemyTypeWeights.size(); ++i) {
		int w = encounter.enemyTypeWeights[i];
		if (w < 0)
			w = 0;
		acc += w;
		if (r < acc)
			return i;
	}
	return 0;
}

static std::string suffixFromIndex(int idx) {
	const char base = 'A' + (idx % 26);
	int cycle = idx / 26;
	if (cycle == 0)
		return std::string(1, base);
	return std::string(1, base) + std::to_string(cycle + 1);
}

void BattleScene::spawnEnemiesByEncounter() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	enemies.clear();

	int minC = encounter.minCount;
	if (minC < 0)
		minC = 0;
	int maxC = encounter.maxCount;
	if (maxC < minC)
		maxC = minC;

	int enemyCount = minC;
	if (maxC > minC)
		enemyCount = GetRand(maxC - minC) + minC;
	if (enemyCount <= 0)
		return;

	float baseX = 500.0f;
	float totalSpan = 260.0f;
	float spacing = (enemyCount <= 1) ? 0.0f : (totalSpan / (enemyCount - 1));
	float left = baseX - (spacing * (enemyCount - 1) * 0.5f);

	std::unordered_map<std::string, int> nameCounter;

	for (int i = 0; i < enemyCount; ++i) {
		int type = chooseEnemyType();
		float x = left + spacing * i;
		float y = 300.0f;

		EnemyHandle h;
		if (type == 0) {
			auto* pea = obj->CreateGameObject<peabird>(Vector2D(x, y));
			h.name = "�g���b�s�[";
			h.expValue = 100;
			h.getHp = [pea]() { return pea->GetHp(); };
			h.getAtk = [pea]() { return pea->GetAttack(); };
			h.getDef = [pea]() { return pea->GetDefense(); };
			h.applyDamage = [pea](int dmg) { pea->SetHp(dmg); };
			h.setBlink = [pea](float t) { pea->SetBlink(t); };
			h.setVisible = [pea](bool v) { pea->SetVisible(v); };
			h.ai = std::make_unique<SimpleAttackAI>();
		}
		else if (type == 1) {
			auto* tau = obj->CreateGameObject<Taurus>(Vector2D(x, y));
			h.name = "�^�E���X";
			h.expValue = 150;
			h.getHp = [tau]() { return tau->GetHp(); };
			h.getAtk = [tau]() { return tau->GetAttack(); };
			h.getDef = [tau]() { return tau->GetDefense(); };
			h.applyDamage = [tau](int dmg) { tau->SetHp(dmg); };
			h.setBlink = [tau](float t) { tau->SetBlink(t); };
			h.setVisible = [tau](bool v) { tau->SetVisible(v); };
			h.ai = std::make_unique<GuardWhenLowAI>(); // HP�ቺ�Ŗh��
		}
		else {
			auto* pea = obj->CreateGameObject<peabird>(Vector2D(x, y));
			h.name = "�g���b�s�[";
			h.expValue = 100;
			h.getHp = [pea]() { return pea->GetHp(); };
			h.getAtk = [pea]() { return pea->GetAttack(); };
			h.getDef = [pea]() { return pea->GetDefense(); };
			h.applyDamage = [pea](int dmg) { pea->SetHp(dmg); };
			h.setBlink = [pea](float t) { pea->SetBlink(t); };
			h.setVisible = [pea](bool v) { pea->SetVisible(v); };
			h.ai = std::make_unique<SimpleAttackAI>();
		}

		int seq = (nameCounter[h.name]++);
		h.displayName = h.name + " " + suffixFromIndex(seq);

		h.x = x;
		h.y = y;
		int hp0 = h.getHp();
		if (hp0 <= 0)
			hp0 = 1;
		h.maxHp = hp0;
		h.dispHp = hp0;

		enemies.push_back(std::move(h)); // �� ���[�u�Ŋi�[
	}
}

//--------------------------------------
// ����
//--------------------------------------
void BattleScene::attemptEscape() {
	PlayerData* pd = PlayerData::GetInstance();

	int playerSpd = pd->GetAttack();
	if (playerSpd < 1)
		playerSpd = 1;

	int sumAtk = 0, cnt = 0;
	for (int i = 0; i < (int)enemies.size(); ++i) {
		const auto& e = enemies[i];
		if (!e.defeated && e.getHp() > 0) {
			sumAtk += e.getAtk();
			cnt += 1;
		}
	}
	int avgEnemyAtk = 1;
	if (cnt > 0) {
		avgEnemyAtk = sumAtk / cnt;
		if (avgEnemyAtk < 1)
			avgEnemyAtk = 1;
	}

	int rate = 40 + (playerSpd - avgEnemyAtk) / 2;
	if (rate < 20)
		rate = 20;
	if (rate > 95)
		rate = 95;

	int r = GetRand(99);
	if (r < rate || escapePity >= 2) {
		escapePity = 0;
		escapedSuccessfully = true; // �� ��������
		enqueueMessage("������[�� �ɂ��������I");
		beginMessages(BattleState::PlayerCommand);
	}
	else {
		escapePity += 1;
		enqueueMessage("������ �܂�肱�܂�Ă��܂����I");
		enqueueMessage("�ɂ����Ȃ������I");
		beginMessages(BattleState::EnemyTurn);
	}
}

//--------------------------------------
// ��_�����o
//--------------------------------------
void BattleScene::triggerPlayerHit(int damage) {
	lastDamage = damage;
	hitFlashTimer = hitFlashDuration;
	uiShakeTimer = uiShakeDuration;

	DamagePopup p;
	p.timer = 0.6f;
	p.amount = damage;
	p.x = 90.0f;
	p.y = 80.0f;
	popups.push_back(p);

	// �Ԃ��X�v���b�V��
	HitSplash s;
	s.timer = s.duration;
	s.x = p.x + 10.0f;
	s.y = p.y + 10.0f;
	splashes.push_back(s);
}

void BattleScene::updateHitEffects(float dt) {
	if (hitFlashTimer > 0.0f) {
		hitFlashTimer -= dt;
		if (hitFlashTimer < 0.0f)
			hitFlashTimer = 0.0f;
	}
	if (uiShakeTimer > 0.0f) {
		uiShakeTimer -= dt;
		if (uiShakeTimer < 0.0f)
			uiShakeTimer = 0.0f;
	}
	if (worldShakeTimer > 0.0f) {
		worldShakeTimer -= dt;
		if (worldShakeTimer < 0.0f)
			worldShakeTimer = 0.0f;
	}

	for (int i = 0; i < (int)popups.size(); ++i) {
		popups[i].timer -= dt;
		popups[i].y -= 40.0f * dt;
	}
	std::vector<DamagePopup> next;
	next.reserve(popups.size());
	for (int i = 0; i < (int)popups.size(); ++i)
		if (popups[i].timer > 0.0f)
			next.push_back(popups[i]);
	popups.swap(next);

	// �X�v���b�V���X�V
	std::vector<HitSplash> ns;
	ns.reserve(splashes.size());
	for (auto& s : splashes) {
		s.timer -= dt;
		if (s.timer > 0.0f)
			ns.push_back(s);
	}
	splashes.swap(ns);

	if (attackFlashTimer > 0.0f) {
		attackFlashTimer -= dt;
		if (attackFlashTimer < 0.0f)
			attackFlashTimer = 0.0f;
	}
	if (burstTimer > 0.0f) {
		burstTimer -= dt;
		if (burstTimer < 0.0f)
			burstTimer = 0.0f;
	}

	// �U���Y�[������
	if (attackZoomT > 0.0f) {
		attackZoomT -= dt * attackZoomDecay;
		if (attackZoomT < 0.0f)
			attackZoomT = 0.0f;
	}
}

// ��_�����o�̕`��i�ԃt���b�V���^�_���[�W�|�b�v�^�X�v���b�V���j
void BattleScene::drawHitEffects() {
	// 1) ��ʐԃt���b�V���i��e���j
	if (hitFlashTimer > 0.0f) {
		float t = hitFlashTimer / hitFlashDuration; // 1.0 -> 0.0
		int a = (int)(t * 160);						// ����
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
		DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 64, 64), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// 2) �_���[�W���l�|�b�v
	if (!popups.empty()) {
		int font = CreateFontToHandle("�l�r �S�V�b�N", 18, 4);
		for (const auto& p : popups) {
			// �c�莞�Ԃɉ����ăt�F�[�h
			float life = (p.timer <= 0.6f) ? (p.timer / 0.6f) : 1.0f; // 0.6�b�
			if (life < 0.0f)
				life = 0.0f;
			if (life > 1.0f)
				life = 1.0f;
			int a = (int)(life * 255);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
			std::string s = std::to_string(p.amount);
			DrawStringToHandle((int)p.x, (int)p.y, s.c_str(), GetColor(255, 220, 220), font);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		DeleteFontToHandle(font);
	}

	// 3) �Ԃ��~�X�v���b�V���i�����w���p�j
	drawHitSplashes();
}


// �ԃX�v���b�V���`��
void BattleScene::drawHitSplashes() {
	for (const auto& s : splashes) {
		float t = 1.0f - (s.timer / s.duration); // 0��1
		if (t < 0)
			t = 0;
		if (t > 1)
			t = 1;
		float r = s.r0 + (s.r1 - s.r0) * t;
		int a = (int)((1.0f - t) * 180);
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		SetDrawBlendMode(DX_BLENDMODE_ADD, a);
		DrawCircle((int)s.x, (int)s.y, (int)r, GetColor(255, 32, 32), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void BattleScene::startAttackEffect() {
	worldShakeTimer = worldShakeDuration + 0.05f; // ��������
	attackFlashTimer = attackFlashDuration;
	burstTimer = burstDuration;
	attackZoomT = 1.0f; // �Y�[���C���J�n�i���X�ɖ߂�j
}
void BattleScene::drawProceduralAttackEffects() {
	if (attackFlashTimer > 0.0f) {
		float t = attackFlashTimer / attackFlashDuration;
		int a = (int)(t * 180);
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
		DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	if (burstTimer > 0.0f) {
		float t = 1.0f - (burstTimer / burstDuration);
		float radius = 20.0f + 120.0f * t;
		int a = (int)((1.0f - t) * 200);
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
		DrawCircle((int)effectPos.x, (int)effectPos.y, (int)radius, GetColor(255, 255, 255), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_ADD, a);
		for (int i = 0; i < burstLines; ++i) {
			float ang = (6.2831853f * i) / (float)burstLines;
			float len = radius + 30.0f;
			int x1 = (int)effectPos.x, y1 = (int)effectPos.y;
			int x2 = (int)(effectPos.x + cosf(ang) * len);
			int y2 = (int)(effectPos.y + sinf(ang) * len);
			DrawLine(x1, y1, x2, y2, GetColor(255, 255, 255));
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

// ===============================
// �� �����G�t�F�N�g�i�X�e�b�v3�j
// ===============================
void BattleScene::startSpellEffect(SpellElement elem, const Vector2D& pos, bool aoe) {
	spellFxActive = true;
	spellFxElement = elem;
	spellFxAoE = aoe;
	spellFxTimer = spellFxDur;
	spellFxPos = pos;
	bolts.clear();
	shards.clear();

	// �����X�|�[���i���E�X�j
	if (elem == SpellElement::Thunder) {
		const int n = spellFxAoE ? 6 : 3;
		for (int i = 0; i < n; ++i) {
			Bolt b;
			b.life = 0.20f + 0.06f * (GetRand(3)); // 0.2-0.38
			b.t = b.life;
			float ox = (float)(GetRand(80) - 40);
			b.x1 = spellFxPos.x + ox;
			b.y1 = -20.0f;
			b.x2 = spellFxPos.x + (float)(GetRand(40) - 20);
			b.y2 = spellFxPos.y - 10.0f + (float)(GetRand(30) - 15);
			bolts.push_back(b);
		}
	}
	else if (elem == SpellElement::Ice) {
		const int n = spellFxAoE ? 24 : 12;
		for (int i = 0; i < n; ++i) {
			Shard s;
			s.life = 0.40f + 0.25f * (GetRand(4) / 4.0f);
			s.t = s.life;
			s.x = spellFxPos.x + (float)(GetRand(80) - 40);
			s.y = spellFxPos.y - (float)(GetRand(40));
			s.vx = (float)(GetRand(60) - 30) * 0.8f;
			s.vy = (float)(GetRand(50) + 40) * -1.0f; // ��ɔ��
			shards.push_back(s);
		}
	}
}

void BattleScene::updateSpellEffects(float dt) {
	if (!spellFxActive)
		return;
	spellFxTimer -= dt;
	if (spellFxTimer <= 0.0f) {
		spellFxActive = false;
		bolts.clear();
		shards.clear();
		return;
	}
	// ��/�X�̎����i�s
	for (auto& b : bolts)
		b.t -= dt;
	for (auto& s : shards) {
		s.t -= dt;
		s.x += s.vx * dt;
		s.y += s.vy * dt;
		s.vy += 220.0f * dt; // �d�͂��ۂ�
	}
	// �f�X�|��
	if (!bolts.empty()) {
		std::vector<Bolt> nb;
		nb.reserve(bolts.size());
		for (auto& b : bolts)
			if (b.t > 0.0f)
				nb.push_back(b);
		bolts.swap(nb);
	}
	if (!shards.empty()) {
		std::vector<Shard> ns;
		ns.reserve(shards.size());
		for (auto& s : shards)
			if (s.t > 0.0f)
				ns.push_back(s);
		shards.swap(ns);
	}
}

void BattleScene::drawSpellEffects() {
	if (!spellFxActive)
		return;

	float t = 1.0f - (spellFxTimer / spellFxDur); // 0��1
	if (t < 0)
		t = 0;
	if (t > 1)
		t = 1;

	// �ėp�̉~�����O
	auto ring = [&](int r, int g, int b, float radius, float alpha) {
		int a = (int)(alpha * 200);
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		SetDrawBlendMode(DX_BLENDMODE_ADD, a);
		DrawCircle((int)spellFxPos.x, (int)spellFxPos.y, (int)radius, GetColor(r, g, b), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	};

	switch (spellFxElement) {
	case SpellElement::Fire: {
		int a = (int)((1.0f - t) * 180);
		if (a < 0)
			a = 0;
		if (a > 255)
			a = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
		DrawBox(0, 0, 960, 720, GetColor(60, 10, 6), TRUE); // �g�F�g�[��
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		float base = spellFxAoE ? 160.0f : 80.0f;
		ring(255, 96, 48, base * t, 1.0f);
		ring(255, 160, 64, base * 0.5f * t, 0.8f);
		break;
	}
	case SpellElement::Ice: {
		float base = spellFxAoE ? 150.0f : 90.0f;
		ring(120, 200, 255, base * t, 1.0f);
		ring(80, 180, 255, base * 0.6f * t, 0.8f);
		for (auto& s : shards) {
			float k = s.t / s.life; // 1��0
			int a = (int)(k * 200);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
			int x1 = (int)(s.x), y1 = (int)(s.y);
			int x2 = (int)(s.x + s.vx * 0.05f), y2 = (int)(s.y + s.vy * 0.05f);
			DrawLine(x1, y1, x2, y2, GetColor(180, 230, 255));
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		break;
	}
	case SpellElement::Thunder: {
		int a = (int)((1.0f - t) * 220);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
		DrawBox(0, 0, 960, 720, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		for (auto& b : bolts) {
			float k = b.t / b.life;
			if (k < 0)
				k = 0;
			if (k > 1)
				k = 1;
			int aa = (int)(k * 255);
			SetDrawBlendMode(DX_BLENDMODE_ADD, aa);
			DrawLine((int)b.x1, (int)b.y1, (int)b.x2, (int)b.y2, GetColor(255, 255, 120));
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		float base = spellFxAoE ? 140.0f : 70.0f;
		ring(255, 255, 160, base * t, 1.0f);
		break;
	}
	case SpellElement::Heal: {
		float base = spellFxAoE ? 140.0f : 80.0f;
		ring(120, 255, 140, base * t, 1.0f);
		ring(80, 220, 120, base * 0.55f * t, 0.8f);
		int a = (int)((1.0f - t) * 120);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
		DrawBox(0, 0, 960, 720, GetColor(10, 30, 10), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		break;
	}
	default:
		break;
	}
}

//--------------------------------------
// �I�t�X�N���[���Ǘ�
//--------------------------------------
void BattleScene::ensureOffscreen() {
	if (sceneScreen <= 0) {
		sceneScreen = MakeScreen(SCREEN_W, SCREEN_H, TRUE);
	}
}
void BattleScene::releaseOffscreen() {
	if (sceneScreen > 0) {
		DeleteGraph(sceneScreen);
		sceneScreen = -1;
	}
}

//--------------------------------------
// Initialize
//--------------------------------------
void BattleScene::Initialize() {
	ResourceManager* rm = ResourceManager::GetInstance();
	GameManager* obj = Singleton<GameManager>::GetInstance();

	generate_location = Vector2D(480.0f, 360.0f);

	{
		auto imgs = rm->GetImages("Resource/Images/select.png", 1, 1, 1, 32, 32);
		selectImg = imgs.empty() ? -1 : imgs[0];
	}

	ensureOffscreen();

	if (!player) {
		printf("Error: player is nullptr in BattleScene::Initialize()\n");
		return;
	}
	player->SetIsBattle(true);

	// �����e�[�u����
	encounter.minCount = 1;
	encounter.maxCount = 3;
	encounter.enemyTypeWeights.clear();
	encounter.enemyTypeWeights.push_back(70);
	encounter.enemyTypeWeights.push_back(30);

	spawnEnemiesByEncounter();

	int firstIdx = firstLivingIndex();
	if (firstIdx < 0)
		firstIdx = 0;
	targetCursor = firstIdx;
	commandCursor = 0;

	battleState = BattleState::PlayerCommand;
	while (!messageQueue.empty())
		messageQueue.pop();
	currentMessage.clear();

	totalEarnedExp = 0;
	escapePity = 0;

	escapedSuccessfully = false;

	// �G�t�F�N�g������
	hitFlashTimer = 0.0f;
	uiShakeTimer = 0.0f;
	lastDamage = 0;
	popups.clear();
	splashes.clear();
	attackZoomT = 0.0f;
	blinkClock = 0.0f;

	enemyTurnQueue.clear();
	enemyTurnCursor = 0;

	// �t�F�[�h�{�Y�[��
	fadeAlpha = 255;
	fadeState = 1;
	introZoomT = 0.0f;

	// �t�B�j�b�V�����o
	victoryTimer = 0.0f;
	finishZoomT = 0.0f;

	attackFlashTimer = 0.0f;
	burstTimer = 0.0f;

	effectPos = Vector2D(600.0f, 340.0f);

	isPlayerDefending = false;

	// ���U���g������
	resultInitialized = false;
	resultLines.clear();
	resultLineIndex = 0;
	resultCharIndex = 0;
	resultTypeTimer = 0.0f;
	resultGrantDone = false;

	// �s�k������
	defeatInitialized = false;
	defeatLines.clear();
	defeatLineIndex = 0;
	defeatCharIndex = 0;
	defeatTypeTimer = 0.0f;
	defeatDarkT = 0.0f;

		//  ���@
	magicCursor = 0;
	availableMagics = PlayerData::GetInstance()->GetLearnedMagics();

	// ���A�C�e���i�o�g���J�n���̓J�[�\���ƃ��X�g���N���A�j
	itemCursor = 0;
	battleItemIds.clear();
}


//--------------------------------------
// �Y�[���l�擾�i�����~�t�B�j�b�V���~�U���j
//--------------------------------------
float BattleScene::getCurrentZoom() const {
	float introZoom = 1.0f;
	if (introZoomT < 1.0f) {
		float t = introZoomT;
		if (t < 0.0f)
			t = 0.0f;
		if (t > 1.0f)
			t = 1.0f;
		introZoom = 1.15f - 0.15f * t;
	}
	float finishZoom = 1.0f;
	if (finishZoomT > 0.0f) {
		float t = finishZoomT;
		if (t > 1.0f)
			t = 1.0f;
		finishZoom = 1.0f + 0.2f * t;
	}
	float atkEase = attackZoomT * attackZoomT;
	float atkZoom = 1.0f + attackZoomAmp * atkEase;

	return introZoom * finishZoom * atkZoom;
}

//--------------------------------------
// �������U���g�i�^�C�v���C�^�j
//--------------------------------------
void BattleScene::initResultScreen() {
	if (resultInitialized)
		return;
	resultInitialized = true;

	resultLines.clear();
	resultLines.push_back("�܂��̂� ��������I");
	resultLines.push_back(std::string("�������� ") + std::to_string(totalEarnedExp) + " �� �����I");

	resultLineIndex = 0;
	resultCharIndex = 0;
	resultTypeTimer = 0.0f;

	while (!messageQueue.empty())
		messageQueue.pop();
	currentMessage.clear();

	victoryTimer = 1.0f;
	finishZoomT = 0.0001f;

	battleState = BattleState::Result;
}

void BattleScene::updateResult(float dt) {
	InputControl* input = Singleton<InputControl>::GetInstance();

	if (resultLineIndex < (int)resultLines.size()) {
		resultTypeTimer += dt;
		int add = (int)(resultTypeTimer * resultTypeSpeed);
		if (add > 0) {
			resultTypeTimer -= (float)add / resultTypeSpeed;
			resultCharIndex += add;
			int len = (int)resultLines[resultLineIndex].size();
			if (resultCharIndex > len)
				resultCharIndex = len;
		}

		if (input && input->GetKeyDown(KEY_INPUT_SPACE)) {
			int len = (int)resultLines[resultLineIndex].size();
			if (resultCharIndex < len) {
				resultCharIndex = len; // �S���\��
			}
			else {
				resultLineIndex++;
				resultCharIndex = 0;
				resultTypeTimer = 0.0f;

				if (resultLineIndex >= (int)resultLines.size()) {
					if (!resultGrantDone) {
						bool rebuilt = grantExpAndMaybeRebuildResultLines();
						if (rebuilt) {
							resultGrantDone = true;
							resultLineIndex = 0;
							resultCharIndex = 0;
							resultTypeTimer = 0.0f;
							return;
						}
					}
					return;
				}
			}
		}
	}
	else {
		if (!resultGrantDone) {
			bool rebuilt = grantExpAndMaybeRebuildResultLines();
			if (rebuilt) {
				resultGrantDone = true;
				resultLineIndex = 0;
				resultCharIndex = 0;
				resultTypeTimer = 0.0f;
				return;
			}
		}
	}
}

void BattleScene::drawResult() {
	int LargeFont = CreateFontToHandle("�l�r �S�V�b�N", 22, 6);

	int w = 720, h = 220;
	int x = (SCREEN_W - w) / 2;
	int y = (SCREEN_H - h) / 2;
	drawWindow(x, y, w, h, 0, 0, 64);

	int drawY = y + 24;
	for (int i = 0; i < resultLineIndex; ++i) {
		DrawStringToHandle(x + 24, drawY, resultLines[i].c_str(), GetColor(255, 255, 255), LargeFont);
		drawY += 36;
	}
	if (resultLineIndex < (int)resultLines.size()) {
		const std::string& line = resultLines[resultLineIndex];
		std::string sub = line.substr(0, (size_t)resultCharIndex);
		DrawStringToHandle(x + 24, drawY, sub.c_str(), GetColor(255, 255, 255), LargeFont);
		if ((int)sub.size() == (int)line.size()) {
			DrawStringToHandle(x + w - 40, y + h - 32, "��", GetColor(200, 200, 200), LargeFont);
		}
	}

	DeleteFontToHandle(LargeFont);
}

bool BattleScene::grantExpAndMaybeRebuildResultLines() {
	PlayerData* pd = PlayerData::GetInstance();
	int beforeLv = pd->GetLevel();

	if (totalEarnedExp > 0) {
		pd->AddExperience(totalEarnedExp);
		totalEarnedExp = 0;
	}
	int afterLv = pd->GetLevel();

	if (afterLv > beforeLv) {
		std::vector<std::string> lvLines;
		lvLines.push_back("���x���� ���������I");
		lvLines.push_back(std::string("���x�� ") + std::to_string(afterLv) + " �� �Ȃ����I");
		resultLines = lvLines;
		return true;
	}
	return false;
}

//--------------------------------------
// ���s�k���U���g
//--------------------------------------
void BattleScene::initDefeatScreen() {
	if (defeatInitialized)
		return;
	defeatInitialized = true;

	defeatLines.clear();
	defeatLines.push_back("������[�� ����������c");
	defeatLines.push_back("�߂̂܂��� �܂������ �Ȃ����I");

	defeatLineIndex = 0;
	defeatCharIndex = 0;
	defeatTypeTimer = 0.0f;
	defeatDarkT = 0.0f;

	while (!messageQueue.empty())
		messageQueue.pop();
	currentMessage.clear();

	battleState = BattleState::Defeat;
}

void BattleScene::updateDefeat(float dt) {
	InputControl* input = Singleton<InputControl>::GetInstance();

	if (defeatDarkT < 1.0f) {
		defeatDarkT += dt * defeatDarkSpeed;
		if (defeatDarkT > 1.0f)
			defeatDarkT = 1.0f;
	}

	if (defeatLineIndex < (int)defeatLines.size()) {
		defeatTypeTimer += dt;
		int add = (int)(defeatTypeTimer * defeatTypeSpeed);
		if (add > 0) {
			defeatTypeTimer -= (float)add / defeatTypeSpeed;
			defeatCharIndex += add;
			int len = (int)defeatLines[defeatLineIndex].size();
			if (defeatCharIndex > len)
				defeatCharIndex = len;
		}

		if (input && input->GetKeyDown(KEY_INPUT_SPACE)) {
			int len = (int)defeatLines[defeatLineIndex].size();
			if (defeatCharIndex < len) {
				defeatCharIndex = len;
			}
			else {
				defeatLineIndex++;
				defeatCharIndex = 0;
				defeatTypeTimer = 0.0f;
			}
		}
	}
}

void BattleScene::drawDefeat() {
	int a = (int)(defeatDarkT * 200.0f);
	if (a < 0)
		a = 0;
	if (a > 255)
		a = 255;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
	DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(80, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int LargeFont = CreateFontToHandle("�l�r �S�V�b�N", 22, 6);
	int w = 720, h = 220;
	int x = (SCREEN_W - w) / 2;
	int y = (SCREEN_H - h) / 2;
	drawWindow(x, y, w, h, 16, 0, 32);

	int drawY = y + 24;
	for (int i = 0; i < defeatLineIndex; ++i) {
		DrawStringToHandle(x + 24, drawY, defeatLines[i].c_str(), GetColor(255, 200, 200), LargeFont);
		drawY += 36;
	}
	if (defeatLineIndex < (int)defeatLines.size()) {
		const std::string& line = defeatLines[defeatLineIndex];
		std::string sub = line.substr(0, (size_t)defeatCharIndex);
		DrawStringToHandle(x + 24, drawY, sub.c_str(), GetColor(255, 220, 220), LargeFont);
		if ((int)sub.size() == (int)line.size()) {
			DrawStringToHandle(x + w - 40, y + h - 32, "��", GetColor(220, 180, 180), LargeFont);
		}
	}

	DeleteFontToHandle(LargeFont);
}

//--------------------------------------
// Update
//--------------------------------------
eSceneType BattleScene::Update(float delta_second) {
	GameManager* gm = Singleton<GameManager>::GetInstance();
	gm->Update(delta_second);

	InputControl* input = Singleton<InputControl>::GetInstance();
	PlayerData* pd = PlayerData::GetInstance();

	if (livingEnemyCount() == 0 &&
		battleState != BattleState::Result &&
		battleState != BattleState::Message &&
		battleState != BattleState::Defeat) {
		initResultScreen();
	}

	if (fadeState == 1) {
		fadeAlpha -= (int)(delta_second * 400.0f);
		if (fadeAlpha <= 0) {
			fadeAlpha = 0;
			fadeState = 0;
		}
	}
	if (introZoomT < 1.0f) {
		introZoomT += (delta_second / introZoomDur);
		if (introZoomT > 1.0f)
			introZoomT = 1.0f;
	}
	if (finishZoomT > 0.0f) {
		finishZoomT += (delta_second / finishZoomDur);
		if (finishZoomT > 1.0f)
			finishZoomT = 1.0f;
	}
	if (victoryTimer > 0.0f) {
		victoryTimer -= delta_second;
		if (victoryTimer < 0.0f)
			victoryTimer = 0.0f;
	}

	blinkClock += delta_second;

	switch (battleState) {
	case BattleState::PlayerCommand: {
		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			commandCursor = (commandCursor + 1) % 5;
		}
		if (input->GetKeyDown(KEY_INPUT_UP)) {
			commandCursor = (commandCursor + 4) % 5;
		}
		if (input->GetKeyDown(KEY_INPUT_SPACE)) {
			if (commandCursor == 0) { // ��������
				if (livingEnemyCount() > 0) {
					battleState = BattleState::AttackSelect;
					int firstIdx = firstLivingIndex();
					if (firstIdx < 0)
						firstIdx = 0;
					targetCursor = firstIdx;
				}
				else {
					initResultScreen();
				}
			}
			else if (commandCursor == 1) {
				battleState = BattleState::MagicMenu;
			}
			else if (commandCursor == 2) { // �A�C�e��
				// �o�g���p�̏���A�C�e���ꗗ�����
				buildBattleItemList();

				// 1���Ȃ��Ȃ烁�b�Z�[�W���o���ăR�}���h�ɖ߂�
				if (battleItemIds.empty()) {
					enqueueMessage("������ �ǂ��� �� �Ȃ��I");
					beginMessages(BattleState::PlayerCommand);
				}
				else {
					itemCursor = 0;
					battleState = BattleState::ItemMenu;
				}
			}

			else if (commandCursor == 3) {
				isPlayerDefending = true;
				enqueueMessage("������[�� �݂� �܂����Ă���B");
				beginMessages(BattleState::EnemyTurn);
			}
			else {
				attemptEscape();
			}
		}
		break;
	}

	case BattleState::MagicMenu: {
		const int magicCount = static_cast<int>(availableMagics.size());
		if (magicCount == 0) {
			enqueueMessage("������ �܂� �Ȃɂ� ���ڂ��Ă��Ȃ��I");
			beginMessages(BattleState::PlayerCommand);
			break;
		}

		if (input->GetKeyDown(KEY_INPUT_DOWN))
			magicCursor = (magicCursor + 1) % magicCount;
		if (input->GetKeyDown(KEY_INPUT_UP))
			magicCursor = (magicCursor + magicCount - 1) % magicCount;

		if (input->GetKeyDown(KEY_INPUT_ESCAPE))
			battleState = BattleState::PlayerCommand;

		if (input->GetKeyDown(KEY_INPUT_SPACE)) {
			auto magic = availableMagics[magicCursor];
			pendingMagic = magic;

			const SpellDef* def = FindSpell(pendingMagic);
			if (!def) {
				enqueueMessage("������ �Ȃɂ� ������Ȃ������I");
				beginMessages(BattleState::PlayerCommand);
				break;
			}
			int cost = def->mpCost;

			PlayerData* pd = PlayerData::GetInstance();
			if (def->isHealing) {
				if (!pd->HasMp(cost)) {
					enqueueMessage("MP�� ����Ȃ��I");
					beginMessages(BattleState::PlayerCommand);
					break;
				}
				pd->ConsumeMp(cost);
				enqueueMessage("MP�� " + std::to_string(cost) + " ���傤�Ђ����I");
				enqueueMessage(std::string("������[�� ") + def->name + "�� �ƂȂ����I");

				int heal = CalcHealingAmount(*def, pd->GetLevel());
				pd->SetHp(pd->GetHp() + heal);

				// �� �񕜂̑����G�t�F�N�g
				startSpellEffect(def->element, Vector2D(90.0f, 80.0f), /*aoe=*/false);

				enqueueMessage("HP�� �����ӂ������I");
				beginMessages(BattleState::EnemyTurn);
				break;
			}

			if (livingEnemyCount() <= 0) {
				initResultScreen();
				break;
			}
			int firstIdx = firstLivingIndex();
			if (firstIdx < 0)
				firstIdx = 0;
			targetCursor = firstIdx;
			battleState = BattleState::MagicTarget;
		}
		break;
	}

	case BattleState::MagicTarget: {
		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			int next = nextLivingIndex(targetCursor, +1);
			if (next >= 0)
				targetCursor = next;
		}
		if (input->GetKeyDown(KEY_INPUT_UP)) {
			int prev = nextLivingIndex(targetCursor, -1);
			if (prev >= 0)
				targetCursor = prev;
		}
		if (input->GetKeyDown(KEY_INPUT_ESCAPE)) {
			battleState = BattleState::MagicMenu;
			break;
		}
		if (input->GetKeyDown(KEY_INPUT_SPACE)) {
			PlayerData* pd = PlayerData::GetInstance();

			const SpellDef* def = FindSpell(pendingMagic);
			if (!def) {
				enqueueMessage("������ �Ȃɂ� ������Ȃ������I");
				beginMessages(BattleState::PlayerCommand);
				break;
			}
			int cost = def->mpCost;

			if (!pd->HasMp(cost)) {
				enqueueMessage("MP�� ����Ȃ��I");
				beginMessages(BattleState::PlayerCommand);
				break;
			}
			pd->ConsumeMp(cost);
			enqueueMessage("MP�� " + std::to_string(cost) + " ���傤�Ђ����I");
			enqueueMessage(std::string("������[�� ") + def->name + "�� �ƂȂ����I");

			CalcContext ctx;
			ctx.attackerAtk = pd->GetAttack();
			ctx.attackerLv = pd->GetLevel();

			if (def->targeting == Targeting::AllEnemies) {
				for (auto& en : enemies) {
					if (en.defeated || en.getHp() <= 0)
						continue;
					ctx.defenderDef = en.getDef();
					int actual = CalcSpellDamage(*def, ctx);

					// �� �K�[�h���Ȃ甼��
					if (en.isGuarding) {
						actual = (actual + 1) / 2;
						enqueueMessage(en.displayName + "�� �K�[�h���Ă���I �_���[�W�� �ւ����I");
					}

					en.applyDamage(actual);
					enqueueMessage(en.displayName + "�� " + std::to_string(actual) + " �� �_���[�W�I");
					if (en.getHp() <= 0)
						onEnemyDefeated(en);
				}
				// ���o�i�S�́j
				startAttackEffect();
				effectPos = Vector2D(600.0f, 340.0f);
				startSpellEffect(def->element, effectPos, /*aoe=*/true);

				beginMessages(BattleState::EnemyTurn);
				break;
			}

			if (targetCursor < 0 || targetCursor >= (int)enemies.size())
				targetCursor = firstLivingIndex();
			if (targetCursor >= 0) {
				auto& e = enemies[targetCursor];
				ctx.defenderDef = e.getDef();
				int actual = CalcSpellDamage(*def, ctx);

				// �� �K�[�h���Ȃ甼��
				if (e.isGuarding) {
					actual = (actual + 1) / 2;
					enqueueMessage(e.displayName + "�� �K�[�h���Ă���I �_���[�W�� �ւ����I");
				}

				e.applyDamage(actual);
				enqueueMessage(e.displayName + "�� " + std::to_string(actual) + " �� �_���[�W�I");
				if (e.getHp() <= 0)
					onEnemyDefeated(e);

				// ���o�i�P�́j
				startAttackEffect();
				effectPos = Vector2D((float)e.x, (float)(e.y - 10.0f));
				startSpellEffect(def->element, effectPos, /*aoe=*/false);
			}
			beginMessages(BattleState::EnemyTurn);
		}
		break;
	}

		case BattleState::ItemMenu: {
		const int itemCount = static_cast<int>(battleItemIds.size());

		// �O�̂��߁A���X�g����Ȃ瑦�I��
		if (itemCount == 0) {
			enqueueMessage("������ �ǂ��� �� �Ȃ��I");
			beginMessages(BattleState::PlayerCommand);
			break;
		}

		// �㉺�ŃJ�[�\���ړ�
		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			itemCursor = (itemCursor + 1) % itemCount;
		}
		if (input->GetKeyDown(KEY_INPUT_UP)) {
			itemCursor = (itemCursor + itemCount - 1) % itemCount;
		}

		// ESC �ŃR�}���h�ɖ߂�
		if (input->GetKeyDown(KEY_INPUT_ESCAPE)) {
			battleState = BattleState::PlayerCommand;
			break;
		}

		// SPACE �ŃA�C�e���g�p
		if (input->GetKeyDown(KEY_INPUT_SPACE)) {
			PlayerData* pd = PlayerData::GetInstance();
			const auto& owned = pd->GetOwnedItems();

			// ���݃J�[�\�����w���Ă���A�C�e��ID
			int id = battleItemIds[itemCursor];

			// �O�̂��ߏ����`�F�b�N
			auto it = owned.find(id);
			if (it == owned.end()) {
				buildBattleItemList();
				enqueueMessage("���� �ǂ��� �� ���� �Ȃ��I");
				beginMessages(BattleState::PlayerCommand);
				break;
			}

			const Item& item = it->second;
			const std::string name = item.GetName();
			const int heal = item.GetHealAmount();

			bool used = pd->UseItem(id);
			if (used) {
				char buf[128];
#if defined(_MSC_VER)
				sprintf_s(buf, "%s �� �������I", name.c_str());
#else
				snprintf(buf, sizeof(buf), "%s �� �������I", name.c_str());
#endif
				enqueueMessage(buf);

				if (heal > 0) {
					char buf2[128];
#if defined(_MSC_VER)
					sprintf_s(buf2, "HP�� %d �����ӂ������I", heal);
#else
					snprintf(buf2, sizeof(buf2), "HP�� %d �����ӂ������I", heal);
#endif
					enqueueMessage(buf2);
				}

				// �A�C�e����������̂Ń��X�g����蒼��
				buildBattleItemList();

				// �^�[���I�� �� �G�^�[����
				beginMessages(BattleState::EnemyTurn);
			}
			else {
				// HP���^���Ȃǂ� UseItem �� false �̏ꍇ
				enqueueMessage("���܂� ���� �ǂ��� �� �����Ȃ��I");
				beginMessages(BattleState::PlayerCommand);
			}
		}

		break;
	}


	case BattleState::AttackSelect: {
		if (input->GetKeyDown(KEY_INPUT_DOWN)) {
			int next = nextLivingIndex(targetCursor, +1);
			if (next >= 0)
				targetCursor = next;
		}
		if (input->GetKeyDown(KEY_INPUT_UP)) {
			int prev = nextLivingIndex(targetCursor, -1);
			if (prev >= 0)
				targetCursor = prev;
		}
		if (input->GetKeyDown(KEY_INPUT_SPACE) && targetCursor >= 0 && targetCursor < (int)enemies.size()) {
			EnemyHandle& tgt = enemies[targetCursor];
			if (!tgt.defeated && tgt.getHp() > 0) {
				CalcContext cx;
				cx.attackerAtk = pd->GetAttack();
				cx.defenderDef = tgt.getDef();
				cx.critical = (GetRand(99) < criticalRatePercent);

				int actualDamage = CalcPhysicalDamage(cx);

				// �� �K�[�h���Ȃ甼��
				if (tgt.isGuarding) {
					actualDamage = (actualDamage + 1) / 2;
					enqueueMessage(tgt.displayName + "�� �K�[�h���Ă���I �_���[�W�� �ւ����I");
				}

				if (cx.critical) {
					enqueueMessage("��������� ���������I�I");
				}

				tgt.applyDamage(actualDamage);
				tgt.setBlink(1.0f);
				enqueueMessage("������[�� ���������I");
				enqueueMessage(tgt.displayName + "�� " + std::to_string(actualDamage) + " �� �_���[�W�I");

				startAttackEffect();
				effectPos = Vector2D((float)tgt.x, (float)(tgt.y - 10.0f));
				// �����͒����F���~������Έȉ���L����
				// startSpellEffect(SpellElement::Neutral, effectPos, false);

				if (tgt.getHp() <= 0 && !tgt.defeated)
					onEnemyDefeated(tgt);

				if (livingEnemyCount() > 0) {
					enemyTurnQueue.clear();
					enemyTurnCursor = 0;
					for (int i = 0; i < (int)enemies.size(); ++i) {
						const auto& e = enemies[i];
						if (e.defeated || e.getHp() <= 0)
							continue;
						int dmg = e.getAtk() / 3;
						if (dmg < 0)
							dmg = 0;

						if (e.ai) {
							EnemyView view;
							view.selfHp = e.getHp();
							view.selfMaxHp = e.maxHp;
							view.selfAtk = e.getAtk();
							view.selfDef = e.getDef();

							auto* pdView = PlayerData::GetInstance();
							view.playerHp = pdView->GetHp();
							view.playerMaxHp = pdView->GetMaxHp();
							view.playerAtk = pdView->GetAttack();
							view.playerDef = pdView->GetDefense();
							view.livingEnemyCount = livingEnemyCount();

							dmg = e.ai->decideDamage(view, dmg);
						}

						EnemyAction act;
						act.enemyIndex = i;
						act.damage = dmg;
						enemyTurnQueue.push_back(act);
					}
					beginMessages(BattleState::EnemyTurn);
				}
				else {
					initResultScreen();
				}
			}
		}
		if (input->GetKeyDown(KEY_INPUT_ESCAPE)) {
			battleState = BattleState::PlayerCommand;
		}
		break;
	}

	case BattleState::EnemyTurn: {
		// �� ���̃^�[���ŐV���ɍs�������߂�O�ɁA�S���̃K�[�h������
		if (enemyTurnQueue.empty()) {
			for (auto& e : enemies)
				e.isGuarding = false;

			for (int i = 0; i < (int)enemies.size(); ++i) {
				const auto& e = enemies[i];
				if (e.defeated || e.getHp() <= 0)
					continue;

				int dmg = e.getAtk() / 3;
				if (dmg < 0)
					dmg = 0;

				if (e.ai) {
					EnemyView view;
					view.selfHp = e.getHp();
					view.selfMaxHp = e.maxHp;
					view.selfAtk = e.getAtk();
					view.selfDef = e.getDef();

					auto* pdView = PlayerData::GetInstance();
					view.playerHp = pdView->GetHp();
					view.playerMaxHp = pdView->GetMaxHp();
					view.playerAtk = pdView->GetAttack();
					view.playerDef = pdView->GetDefense();
					view.livingEnemyCount = livingEnemyCount();

					dmg = e.ai->decideDamage(view, dmg);
				}

				EnemyAction act;
				act.enemyIndex = i;
				act.damage = dmg;
				enemyTurnQueue.push_back(act);
			}
			enemyTurnCursor = 0;
		}

		if (enemyTurnCursor < (int)enemyTurnQueue.size()) {
			EnemyAction act = enemyTurnQueue[enemyTurnCursor];
			if (act.enemyIndex >= 0 && act.enemyIndex < (int)enemies.size()) {
				EnemyHandle& e = enemies[act.enemyIndex];

				int dmg = act.damage;
				if (isPlayerDefending) {
					dmg = (dmg + 1) / 2; // �v���C���[�h�䒆�͔���
				}

				if (dmg <= 0) {
					// �� �h��^�l�q���F���̃v���C���[�s���܂Ŕ�_������
					e.isGuarding = true;
					enqueueMessage(e.displayName + "�� �݂� �܂����Ă���B");
				}
				else {
					enqueueMessage(e.displayName + "�� ���������I");
					enqueueMessage("������[�� " + std::to_string(dmg) + " �� �_���[�W�I");
					pd->SetHp(pd->GetHp() - dmg);
					if (dmg > 0)
						triggerPlayerHit(dmg);
				}
			}
			enemyTurnCursor += 1;

			if (pd->GetHp() <= 0) {
				enemyTurnQueue.clear();
				enemyTurnCursor = 0;
				initDefeatScreen();
			}
			else {
				beginMessages(BattleState::EnemyTurn);
			}
		}
		else {
			enemyTurnQueue.clear();
			enemyTurnCursor = 0;
			isPlayerDefending = false;
			beginMessages(BattleState::PlayerCommand);
		}
		break;
	}

	case BattleState::Message: {
		pumpMessageManual();
		break;
	}

	case BattleState::Result: {
		updateResult(delta_second);
		if (resultGrantDone && resultLineIndex >= (int)resultLines.size()) {
			return eSceneType::eMap;
		}
		break;
	}

	case BattleState::Defeat: {
		updateDefeat(delta_second);
		if (defeatLineIndex >= (int)defeatLines.size()) {
			PlayerData* p = PlayerData::GetInstance();
			if (p->GetHp() <= 0)
				p->SetHp(1);
			return eSceneType::eMap;
		}
		break;
	}
	}

	// �U��/��_�����o�Ȃ�
	updateHitEffects(delta_second);
	// �� �����G�t�F�N�g�X�V
	updateSpellEffects(delta_second);

	// �GHP�\���̒Ǐ]
	updateEnemyHpDisplays(delta_second);

	// �����������F���b�Z�[�W�𔲂����瑦�}�b�v��
	if (escapedSuccessfully && battleState != BattleState::Message) {
		return eSceneType::eMap;
	}

	Draw();
	return GetNowSceneType();
}

// �GHP ���o�Ǐ]
void BattleScene::updateEnemyHpDisplays(float dt) {
	for (auto& e : enemies) {
		int real = e.getHp();
		if (real < 0)
			real = 0;

		if (e.dispHp > real) {
			int diff = e.dispHp - real;
			int step = (diff / 6 > 1) ? (diff / 6) : 1; // ����1
			e.dispHp -= step;
			if (e.dispHp < real)
				e.dispHp = real;
		}
		else {
			e.dispHp = real;
		}
	}
}

//--------------------------------------
// Draw�i�I�t�X�N���[���ɕ`���Ă���g��k���j
//--------------------------------------
void BattleScene::drawWindow(int x, int y, int w, int h, int fillR, int fillG, int fillB) {
	DrawBox(x, y, x + w, y + h, GetColor(fillR, fillG, fillB), TRUE);
	DrawBox(x, y, x + w, y + h, GetColor(255, 255, 255), FALSE);
}

void BattleScene::Draw() {
	GameManager* gm = Singleton<GameManager>::GetInstance();

	// ===== �܂��I�t�X�N���[�� =====
	ensureOffscreen();
	SetDrawScreen(sceneScreen);
	ClearDrawScreen();

	gm->Draw();

	// �GHP�o�[���^�[�Q�b�g����
	for (int i = 0; i < (int)enemies.size(); ++i) {
		const auto& e = enemies[i];
		if (e.defeated || e.getHp() <= 0)
			continue;

		// �^�[�Q�b�g�Ԙg�i�_�Łj
		if ((battleState == BattleState::AttackSelect || battleState == BattleState::MagicTarget) && i == targetCursor) {
			float s = (sinf(blinkClock * 8.0f) * 0.5f + 0.5f);
			int a = (int)(120 + 120 * s);
			if (a > 255)
				a = 255;
			if (a < 0)
				a = 0;
			SetDrawBlendMode(DX_BLENDMODE_ADD, a);
			int rx = (int)e.x, ry = (int)e.y;
			int w = 70, h = 70;
			DrawBox(rx - w / 2, ry - h / 2, rx + w / 2, ry + h / 2, GetColor(255, 64, 64), FALSE);
			DrawCircle(rx, ry, 36, GetColor(255, 48, 48), FALSE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		// HP�o�[
		const int w = 56;
		const int h = 6;
		const int x0 = (int)e.x - w / 2;
		const int y0 = (int)e.y - 40;

		float rate = 0.0f;
		if (e.maxHp > 0) {
			rate = (float)e.dispHp / (float)e.maxHp;
			if (rate < 0.0f)
				rate = 0.0f;
			if (rate > 1.0f)
				rate = 1.0f;
		}

		int fillW = (int)(w * rate);
		if (fillW < 0)
			fillW = 0;
		if (fillW > w)
			fillW = w;

		DrawBox(x0, y0, x0 + fillW, y0 + h, GetColor(50, 255, 50), TRUE);
		DrawBox(x0, y0, x0 + w, y0 + h, GetColor(255, 255, 255), FALSE);

		// �i�C�Ӂj�K�[�h���\��
		if (e.isGuarding) {
			DrawString((int)e.x - 20, (int)e.y - 58, "GUARD", GetColor(180, 220, 255));
		}
	}

	// UI�h��
	int uiOx = 0, uiOy = 0;
	if (uiShakeTimer > 0.0f) {
		uiOx = (GetRand(4) - 2);
		uiOy = (GetRand(4) - 2);
	}

	int LargeFont = CreateFontToHandle("�l�r �S�V�b�N", 18, 6);

	if (battleState == BattleState::Result) {
		drawResult();
	}
	else if (battleState == BattleState::Defeat) {
		drawDefeat();
	}
	else {
		// ����F�X�e�[�^�X�g
		drawWindow(20 + uiOx, 20 + uiOy, 180, 190, 0, 0, 64);
		{
			bool dangerBlink = (hitFlashTimer > 0.0f);
			int plateColor = dangerBlink ? GetColor(160, 32, 32) : GetColor(255, 255, 255);
			DrawBox(37 + uiOx, 10 + uiOy, 112 + uiOx, 40 + uiOy, plateColor, true);
			DrawString(38 + uiOx, 20 + uiOy, "������[", GetColor(0, 0, 0));

			PlayerData* pd = PlayerData::GetInstance();
			DrawFormatStringToHandle(30 + uiOx, 60 + uiOy, GetColor(255, 255, 255), LargeFont, "Lv  : %d", pd->GetLevel());
			DrawFormatStringToHandle(30 + uiOx, 90 + uiOy, GetColor(255, 255, 255), LargeFont, "HP  : %d", pd->GetHp());
			DrawFormatStringToHandle(30 + uiOx, 120 + uiOy, GetColor(180, 220, 255), LargeFont, "MP  : %d/%d", pd->GetMp(), pd->GetMaxMp());

			// MP�o�[
			{
				const int bx = 30 + uiOx;
				const int by = 150 + uiOy;
				const int bw = 120;
				const int bh = 8;
				float mpr = 0.0f;
				const int maxmp = pd->GetMaxMp();
				if (maxmp > 0) {
					mpr = (float)pd->GetMp() / (float)maxmp;
					if (mpr < 0.0f)
						mpr = 0.0f;
					if (mpr > 1.0f)
						mpr = 1.0f;
				}
				int fill = (int)(bw * mpr);
				if (fill < 0)
					fill = 0;
				if (fill > bw)
					fill = bw;

				DrawBox(bx, by, bx + bw, by + bh, GetColor(40, 60, 90), TRUE);
				DrawBox(bx, by, bx + fill, by + bh, GetColor(80, 160, 255), TRUE);
				DrawBox(bx, by, bx + bw, by + bh, GetColor(255, 255, 255), FALSE);
			}

			if (isPlayerDefending) {
				DrawStringToHandle(30 + uiOx, 170 + uiOy, "�ڂ����咆", GetColor(200, 200, 255), LargeFont);
			}
		}

		// �E�F�R�}���h�g
		drawWindow(720 + uiOx, 460 + uiOy, 220, 230, 0, 0, 64);
		{
			const char* cmds[5] = { "��������", "�������", "�ǂ���", "�ڂ�����", "�ɂ���" };
			for (int i = 0; i < 5; ++i) {
				int y = 480 + i * 40;
				DrawStringToHandle(750 + uiOx, y + uiOy, cmds[i], GetColor(255, 255, 255), LargeFont);
				if (battleState == BattleState::PlayerCommand && i == commandCursor && selectImg >= 0) {
					DrawRotaGraph(730 + uiOx, y + 12 + uiOy, 0.05, 0, selectImg, TRUE);
				}
			}
		}

		// �����F���b�Z�[�W�g
		drawWindow(20 + uiOx, 520 + uiOy, 680, 170, 0, 0, 64);
		{
			if (battleState == BattleState::AttackSelect) {
				int y = 540;
				for (int i = 0; i < (int)enemies.size(); ++i) {
					const auto& e = enemies[i];
					if (e.defeated || e.getHp() <= 0)
						continue;
					DrawStringToHandle(50 + uiOx, y + uiOy, (e.displayName + " �� ��������").c_str(), GetColor(255, 255, 255), LargeFont);
					if (i == targetCursor && selectImg >= 0) {
						DrawRotaGraph(30 + uiOx, y + 12 + uiOy, 0.05, 0, selectImg, TRUE);
					}
					y += 40;
				}
			}
			else if (battleState == BattleState::MagicMenu) {
				PlayerData* pd = PlayerData::GetInstance();

				char mpbuf[64];
#if defined(_MSC_VER)
				sprintf_s(mpbuf, "MP: %d / %d", pd->GetMp(), pd->GetMaxMp());
#else
				snprintf(mpbuf, sizeof(mpbuf), "MP: %d / %d", pd->GetMp(), pd->GetMaxMp());
#endif
				DrawStringToHandle(50 + uiOx, 540 + uiOy, "�������� �����ł��������B  SPACE: ���� / ESC: ���ǂ�", GetColor(200, 200, 200), LargeFont);
				DrawStringToHandle(50 + uiOx + 520, 540 + uiOy, mpbuf, GetColor(180, 220, 255), LargeFont);

				int x = 50 + uiOx;
				int y = 580 + uiOy;
				int lineH = 28;

				if (availableMagics.empty()) {
					DrawStringToHandle(x, y, "������ �܂� �Ȃɂ� ���ڂ��Ă��Ȃ��I", GetColor(200, 200, 255), LargeFont);
				}
				else {
					for (int i = 0; i < (int)availableMagics.size(); ++i) {
						PlayerData::MagicType t = availableMagics[i];
						const SpellDef* def = FindSpell(t);
						const char* name = def ? def->name : "�H�H�H";
						int cost = def ? def->mpCost : 0;

						char line[128];
#if defined(_MSC_VER)
						sprintf_s(line, "%s  (%dMP)", name, cost);
#else
						snprintf(line, sizeof(line), "%s  (%dMP)", name, cost);
#endif
						int color = GetColor(255, 255, 255);
						if (i == magicCursor) {
							color = GetColor(255, 255, 0);
						}
						else if (!pd->HasMp(cost)) {
							color = GetColor(160, 160, 160);
						}
						DrawStringToHandle(x + 20, y + i * lineH, line, color, LargeFont);
						if (i == magicCursor && selectImg >= 0) {
							DrawRotaGraph(x, y + i * lineH + 12, 0.05, 0, selectImg, TRUE);
						}
					}
				}
			}
			else if (battleState == BattleState::MagicTarget) {
				int y = 540;
				for (int i = 0; i < (int)enemies.size(); ++i) {
					const auto& e = enemies[i];
					if (e.defeated || e.getHp() <= 0)
						continue;
					DrawStringToHandle(50 + uiOx, y + uiOy, (e.displayName + " �� �������").c_str(), GetColor(255, 255, 255), LargeFont);
					if (i == targetCursor && selectImg >= 0) {
						DrawRotaGraph(30 + uiOx, y + 12 + uiOy, 0.05, 0, selectImg, TRUE);
					}
					y += 40;
				}
				DrawStringToHandle(50 + uiOx, 670 + uiOy, "SPACE: �����Ă� / ESC: ���ǂ�", GetColor(200, 200, 200), LargeFont);
			}
			else if (battleState == BattleState::ItemMenu) {
				PlayerData* pd = PlayerData::GetInstance();
				const auto& owned = pd->GetOwnedItems();

				// ���o��
				DrawStringToHandle(
					50 + uiOx,
					540 + uiOy,
					"�ǂ���  SPACE: ���� / ESC: ���ǂ�",
					GetColor(200, 200, 200),
					LargeFont);

				int x = 50 + uiOx;
				int y = 580 + uiOy;
				int lineH = 28;

				if (battleItemIds.empty()) {
					DrawStringToHandle(
						x, y,
						"������ �ǂ��� �� �Ȃ�",
						GetColor(200, 200, 255),
						LargeFont);
				}
				else {
					int n = static_cast<int>(battleItemIds.size());
					for (int i = 0; i < n; ++i) {
						int id = battleItemIds[i];
						auto it = owned.find(id);
						const char* name = (it != owned.end()) ? it->second.GetName().c_str() : "???";

						char line[128];
#if defined(_MSC_VER)
						sprintf_s(line, "%s", name);
#else
						snprintf(line, sizeof(line), "%s", name);
#endif

						// �I�𒆂��ǂ���
						int color = GetColor(255, 255, 255);
						DrawStringToHandle(x + 20, y + i * lineH, line, color, LargeFont);

						if (i == itemCursor && selectImg >= 0) {
							DrawRotaGraph(
								x,
								y + i * lineH + 12,
								0.05,
								0,
								selectImg,
								TRUE);
						}
					}
				}
			}

			else if (battleState == BattleState::Message && !currentMessage.empty()) {
				DrawString(50 + uiOx, 540 + uiOy, currentMessage.c_str(), GetColor(255, 255, 255));
				DrawString(620 + uiOx, 670 + uiOy, "��", GetColor(200, 200, 200));
			}
			else if (battleState == BattleState::PlayerCommand) {
				DrawStringToHandle(50 + uiOx, 540 + uiOy, "�R�}���h�� �����ł��������B", GetColor(200, 200, 200), LargeFont);
			}
		}

		// ��_������
		drawHitEffects();

		// �摜�Ȃ��̍U�����o
		drawProceduralAttackEffects();

		// �� �����G�t�F�N�g�`��
		drawSpellEffects();

		// �����e�L�X�g�i�t�F�[�h�j
		if (victoryTimer > 0.0f) {
			int a = (int)(victoryTimer * 255);
			if (a > 255)
				a = 255;
			if (a < 0)
				a = 0;
			int bigFont = CreateFontToHandle("�l�r �S�V�b�N", 36, 10);
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
			DrawStringToHandle(SCREEN_W / 2 - 80, SCREEN_H / 2 - 20, "���傤��I", GetColor(255, 255, 255), bigFont);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DeleteFontToHandle(bigFont);
		}
	}

	// ===== �Y�[���]�� =====
	SetDrawScreen(DX_SCREEN_BACK);

	int wOx = 0, wOy = 0;
	if (worldShakeTimer > 0.0f) {
		wOx = (GetRand(5) - 2);
		wOy = (GetRand(5) - 2);
	}

	float zoom = getCurrentZoom();
	DrawRotaGraph(SCREEN_W / 2 + wOx, SCREEN_H / 2 + wOy, zoom, 0.0, sceneScreen, TRUE, FALSE);

	if (fadeAlpha > 0) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
		DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	DeleteFontToHandle(LargeFont);
}

//--------------------------------------
// Finalize / GetNowSceneType / SetPlayerPosition
//--------------------------------------
void BattleScene::Finalize() {
	GameManager* obj = Singleton<GameManager>::GetInstance();
	obj->Finalize();
	releaseOffscreen();
}
eSceneType BattleScene::GetNowSceneType() const { return eSceneType::eBattle; }
void BattleScene::SetPlayerPosition(const Vector2D& position) { playerPosition = position; }


//--------------------------------------
// �o�g���p�F�g�������A�C�e���̈ꗗ�����
//--------------------------------------
void BattleScene::buildBattleItemList() {
	battleItemIds.clear();

	PlayerData* pd = PlayerData::GetInstance();
	const auto& owned = pd->GetOwnedItems();

	for (const auto& kv : owned) {
		const Item& item = kv.second;
		// ����A�C�e���������X�g�ɓ����
		if (item.GetType() == ItemType::Consumable) {
			battleItemIds.push_back(kv.first); // id ��������
		}
	}

	if (battleItemIds.empty()) {
		itemCursor = 0;
	}
	else {
		if (itemCursor < 0)
			itemCursor = 0;
		if (itemCursor >= static_cast<int>(battleItemIds.size()))
			itemCursor = static_cast<int>(battleItemIds.size()) - 1;
	}
}
