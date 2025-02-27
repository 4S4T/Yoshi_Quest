#include "ResourceManager.h"
#include "DxLib.h"

// 静的メンバ変数定義
ResourceManager* ResourceManager::instance = nullptr; // 自クラスのポインタ

/// <summary>
/// インスタンス取得処理
/// </summary>
/// <returns>インスタンスのポインタを返却する</returns>
ResourceManager* ResourceManager::GetInstance() {
	// インスタンスが生成されていない場合、生成する
	if (instance == nullptr) {
		instance = new ResourceManager();
	}

	return instance;
}

/// <summary>
/// インスタンス削除処理
/// </summary>
void ResourceManager::DeleteInstance() {
	// インスタンスが生成されている場合、削除する
	if (instance != nullptr) {
		// 画像リソースの解放
		instance->UnLoadImages();
		// 音源リソースの解放
		instance->UnLoadSounds();
		// インスタンスを削除する
		delete instance;
		instance = nullptr;
	}
}

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
const std::vector<int>& ResourceManager::GetImages(std::string file_name, int all_num, int num_x, int num_y, int size_x, int size_y) {
	// コンテナ内に画像データが無い場合、画像読み込みを行う
	if (images_container.count(file_name) == NULL) {
		// 分割読み込みを行う？行わない？
		if (all_num == 1) {
			// 分割読み込みを行わない
			CreateImagesResource(file_name);
		}
		else {
			// 分割読み込みを行う
			CreateImagesResource(file_name, all_num, num_x, num_y, size_x, size_y);
		}
	}

	// コンテナ内のデータを返す
	return images_container[file_name];
}
const std::vector<int>& ResourceManager::GetImages(const char* file_name, int all_num, int num_x, int num_y, int size_x, int size_y) {
	return GetImages(std::string(file_name), all_num, num_x, num_y, size_x, size_y);
}

/// <summary>
/// 音源取得処理
/// </summary>
/// <param name="file_path">音源のファイルパス</param>
/// <returns>音源ハンドルデータ</returns>
int ResourceManager::GetSounds(std::string file_path) {
	// コンテナ内に指定ファイルが無ければ、生成する
	if (sounds_container.count(file_path) == NULL) {
		// 音源の読み込みを行う
		CreateSoundsResource(file_path);
	}

	// コンテナ内のデータを返す
	return sounds_container[file_path];
}
int ResourceManager::GetSounds(const char* file_path) {
	return GetSounds(std::string(file_path));
}

/// <summary>
/// 画像解放処理
/// </summary>
void ResourceManager::UnLoadImages() {
	// コンテナ内に画像が無ければ、処理を終了する
	if (images_container.size() == NULL) {
		return;
	}

	// 全ての画像を削除
	for (std::pair<std::string, std::vector<int>> value : images_container) {
		DeleteSharingGraph(value.second[0]);
		value.second.clear();
	}

	// コンテナを解放
	images_container.clear();
}

/// <summary>
/// 音源解放処理
/// </summary>
void ResourceManager::UnLoadSounds() {
	// コンテナ内に音源が無ければ、処理を終了する
	if (sounds_container.size() == NULL) {
		return;
	}

	// 全ての画像を削除
	for (std::pair<std::string, int> value : sounds_container) {
		DeleteSoundMem(value.second);
	}

	// コンテナを解放
	sounds_container.clear();
}

/// <summary>
/// 画像読み込み処理
/// </summary>
/// <param name="file_path">ファイルパス</param>
void ResourceManager::CreateImagesResource(std::string file_path) {
	// 指定された画像ファイル読み込む
	int handle = LoadGraph(file_path.c_str());

	// エラーチェック
	if (handle == -1) {
		throw(file_path + "がありません\n");
	}

	// コンテナに読み込んだ画像を追加する
	images_container[file_path].push_back(handle);
}
void ResourceManager::CreateImagesResource(std::string file_name, int all_num, int num_x, int num_y, int size_x, int size_y) {
	// 分割する画像分だけメモリを確保する
	int* handle = new int[all_num];

	// 指定された画像ファイルを分割で読み込む
	int result = LoadDivGraph(file_name.c_str(), all_num, num_x, num_y, size_x, size_y, handle);

	// エラーチェック
	if (result == -1) {
		throw(file_name + "がありません\n");
	}

	// コンテナに読み込んだ画像を追加する
	for (int i = 0; i < all_num; i++) {
		images_container[file_name].push_back(handle[i]);
	}

	// 確保したメモリを解放する
	delete[] handle;
}

void ResourceManager::CreateSoundsResource(std::string file_path) {
	// 指定されたファイルを読み込む
	int handle = LoadSoundMem(file_path.c_str());

	// エラーチェック
	if (handle == -1) {
		throw(file_path + "がありません\n");
	}

	// コンテナに読み込んだ音源を追加する
	sounds_container[file_path] = handle;
}
