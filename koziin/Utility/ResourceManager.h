#pragma once

#include <map>
#include <string>
#include <vector>

/// <summary>
/// リソース管理クラス
/// </summary>
class ResourceManager {
public:
	// 自クラスのポインタ（実体をアドレスの先で保有）
	static ResourceManager* instance;

	std::map<std::string, std::vector<int>> images_container; // 画像コンテナ
	std::map<std::string, int> sounds_container;			  // 音源コンテナ

private:
	// クラスの実体をメンバ関数内でしか生成できないようにする
	ResourceManager() = default;

	// コピーガード
	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;
	// ~コピーガード

public:
	~ResourceManager() = default;

	/// <summary>
	/// インスタンス取得処理
	/// </summary>
	/// <returns>インスタンスのポインタを返却する</returns>
	static ResourceManager* GetInstance();

	/// <summary>
	/// インスタンス削除処理
	/// </summary>
	static void DeleteInstance();

public:
	/// <summary>
	/// 画像取得処理
	/// </summary>
	/// <param name="file_name">ファイルパス</param>
	/// <param name="all_num">画像の分割総数</param>
	/// <param name="num_x">横の分割</param>
	/// <param name="num_y">縦の分割</param>
	/// <param name="size_x">横のサイズ(px)</param>
	/// <param name="size_y">縦のサイズ(px)</param>
	/// <returns>読み込んだ画像ハンドルのvector配列</returns>
	const std::vector<int>& GetImages(std::string file_name, int all_num = 1, int num_x = 1, int num_y = 1, int size_x = 0, int size_y = 0);
	/// <summary>
	/// 画像取得処理
	/// </summary>
	/// <param name="file_name">ファイルパス</param>
	/// <param name="all_num">画像の分割総数</param>
	/// <param name="num_x">横の分割</param>
	/// <param name="num_y">縦の分割</param>
	/// <param name="size_x">横のサイズ(px)</param>
	/// <param name="size_y">縦のサイズ(px)</param>
	/// <returns>読み込んだ画像ハンドルのvector配列</returns>
	const std::vector<int>& GetImages(const char* file_name, int all_num = 1, int num_x = 1, int num_y = 1, int size_x = 0, int size_y = 0);

	/// <summary>
	/// 音源取得処理
	/// </summary>
	/// <param name="file_path">音源のファイルパス</param>
	/// <returns>音源ハンドルデータ</returns>
	int GetSounds(std::string file_path);
	/// <summary>
	/// 音源取得処理
	/// </summary>
	/// <param name="file_path">音源のファイルパス</param>
	/// <returns>音源ハンドルデータ</returns>
	int GetSounds(const char* file_path);

	/// <summary>
	/// 画像解放処理
	/// </summary>
	void UnLoadImages();

	/// <summary>
	/// 音源解放処理
	/// </summary>
	void UnLoadSounds();

private:
	/// <summary>
	/// 画像読み込み処理
	/// </summary>
	/// <param name="file_path">ファイルパス</param>
	void CreateImagesResource(std::string file_path);

	/// <summary>
	/// 画像分割読み込み処理
	/// </summary>
	/// <param name="file_name">ファイルパス</param>
	/// <param name="all_num">画像の分割総数</param>
	/// <param name="num_x">横の分割</param>
	/// <param name="num_y">縦の分割</param>
	/// <param name="size_x">横のサイズ(px)</param>
	/// <param name="size_y">縦のサイズ(px)</param>
	void CreateImagesResource(std::string file_name, int all_num, int num_x, int num_y, int size_x, int size_y);

	// 音源読み込み処理
	void CreateSoundsResource(std::string file_path);
};