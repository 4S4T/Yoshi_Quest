#pragma once

#include <string>
#include "Scene/SceneBase.h"

class Application : public Singleton<class Type> {
private:
	SceneBase* curent_scene;
	bool end;

public:
	Application();
	~Application();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void WakeUp();

	/// <summary>
	///  実行処理
	/// </summary>
	void Run();

	/// <summary>
	/// 終了時処理
	/// </summary>
	void Shutdown();
};

/**********************************************************
 * マクロ定義
 ***********************************************************/
#define D_SUCCESS (0)  // 成功
#define D_FAILURE (-1) // 失敗

#define D_WIN_MAX_X (672) // スクリーンサイズ（幅）
#define D_WIN_MAX_Y (864) // スクリーンサイズ（高さ）
#define D_COLOR_BIT (32)  // カラービット

/**********************************************************
 * プロトタイプ宣言
 ***********************************************************/
/// <summary>
/// フレーム制御
/// </summary>
void FreamControl();

/// <summary>
/// 1フレーム当たりの時間を取得する
/// </summary>
/// <returns>1フレーム当たりの時間</returns>
const float& GetDeltaSecond();

/// <summary>
/// エラー内容を出力する
/// </summary>
/// <returns>終了状態の値</returns>
int ErrorThrow(std::string error_log);
