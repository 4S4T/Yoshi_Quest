#pragma once
#include "../../Utility/Vector2D.h"
#include <string>
#include <vector>

class NCP {
public:
	NCP();
	NCP(const Vector2D& pos, const std::string& name, const std::vector<std::string>& lines);
	~NCP();

	void SetPosition(const Vector2D& pos);
	Vector2D GetPosition() const;

	void SetName(const std::string& name);
	void SetDialog(const std::vector<std::string>& lines);

	// ★ 画像を手動セット（村人だけに使う）
	void SetImage(const std::string& path);
	void ClearImage();

	void BeginTalk();
	void AdvanceTalk();
	void EndTalk();
	bool IsTalking() const;

	bool IsPlayerInRange(const Vector2D& playerPos, float range) const;

	void Update(float delta);
	void Draw() const;

	// 追加した場所   9/2

	void SetDrawSize(float px);
	float GetDrawSize() const;

private:
	void ReloadImage();

private:
	Vector2D position;
	std::string ncpName;
	std::vector<std::string> dialogLines;
	int currentLine;
	bool talking;

	// 画像まわり
	int imageHandle;	   // -1: 未ロード / 失敗
	std::string imagePath; // 現在の画像パス
	float drawSize;		   // 目安表示サイズ（px）
};
