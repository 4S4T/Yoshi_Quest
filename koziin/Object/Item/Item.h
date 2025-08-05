// Item.h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../../Utility/Vector2D.h"

class Item {
public:
    Item(const std::string& name, const Vector2D& pos);

    const std::string& GetName() const;
    const Vector2D& GetPosition() const;
    bool IsCollected() const;
    void Collect();

private:
    std::string itemName;
    Vector2D position;
    bool collected = false;
};

// ★ Map用：初期アイテム生成
std::vector<std::shared_ptr<Item>> GenerateMapItems();
