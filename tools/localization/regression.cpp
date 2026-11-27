#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include "VeoxDMA/Misc/Localization.h"
#include "VeoxDMA/Config/LootFilterData.h"
#include "expected_names.h"

namespace {
int failures = 0;
int checks = 0;
void Check(bool passed, const char* reason)
{
    ++checks;
    if (passed) return;
    ++failures;
    if (failures <= 12) std::printf("FAIL: %s\n", reason);
}
std::string Name(std::string_view text, std::string_view custom = {})
{
#ifdef HAS_ITEM_NAMES
    return Localization::TranslateItemName(text, custom);
#else
    return Localization::TranslateRuntime(std::string(custom.empty() ? text : custom));
#endif
}
bool Search(std::string_view query, std::string_view id, std::string_view name,
            std::string_view custom = {})
{
#ifdef HAS_ITEM_NAMES
    return Localization::ItemMatchesSearch(query, id, name, custom);
#else
    return id.find(query) != id.npos || name.find(query) != name.npos;
#endif
}
}

int main()
{
    const std::pair<const char*, const char*> labels[] = {
        {"Icon Size", "图标大小"}, {"Dot Size", "标记点大小"},
        {"Name > Rarity", "名称 > 稀有度"}, {"Rarity > Name", "稀有度 > 名称"},
        {"Price High-Low", "价格：从高到低"}, {"Sell price", "出售价格"},
        {"Buy price", "购买价格"}, {"Unlock price", "解锁价格"},
        {"Auth 1", "权限 1"}, {"Auth 2", "权限 2"}, {"Auth 3", "权限 3"},
        {"Auth 4", "权限 4"}, {"Auth 5", "权限 5"},
        {"Distance (Nearest)", "距离（最近）"}, {"Health (Lowest)", "生命值（最低）"},
        {"Name (A-Z)", "名称（A-Z）"},
        // Retain localization from the five existing commits and upstream merge.
        {"Chinese Language", "简体中文"}, {"Team Handling", "队友管理"},
        {"ABCurves Scale", "ABCurves 缩放"}, {"Role Counters", "角色人数统计"},
        {"COINS : $%d", "金币：$%d"}, {"Radar Size", "雷达尺寸"}
    };
    Check(!ConfigInstance.Overlay.ChineseLanguage, "English default");
    for (const auto& [en, zh] : kExpectedNames)
        Check(Name(en) == en, "English name preserved");
    for (const auto& [en, zh] : labels)
        Check(std::string(Localization::Tr(en)) == en, "English label preserved");
    for (const auto& row : LootFilter::kItemCatalog)
        Check(Name(row.id) == row.id, "English raw registry ID preserved");

    ConfigInstance.Overlay.ChineseLanguage = true;
    for (const auto& [en, zh] : labels)
        Check(std::string(Localization::Tr(en)) == zh, en);
    for (const auto* category : LootFilter::CategoryNames)
        Check(std::string(Localization::Tr(category)) != category, category);
    int vehicleRows = 0;
    for (const auto& row : LootFilter::kItemCatalog) {
        const auto expected = kExpectedNames.find(row.displayName);
        Check(expected != kExpectedNames.end(), "catalog name present in Chinese dictionary");
        if (expected == kExpectedNames.end()) continue;
        Check(Name(row.displayName) == expected->second, row.displayName);
        Check(Name(row.id) == expected->second, row.id);
        if (row.labelMatch[0]) Check(Name(row.labelMatch) == expected->second, row.labelMatch);
        if (row.category == LootFilter::Category::Vehicles) ++vehicleRows;
    }
    for (const auto& [en, zh] : kExpectedNames)
        Check(Name(en) == zh, "all dictionary names and SDK fallbacks");
    Check(Name("fukurou_spark_9") == "福枭·火花", "vehicle paint ID translated");
    Check(Name("alcohol_15") == "国王珍藏酒瓶", "UTF-8 apostrophe catalog entry");
    Check(Name("UMP45 | MK2") == "UMP45 冲锋枪 | MK2", "SDK weapon name");
    Check(Name("fukurou_spark_999999") == "fukurou_spark_999999", "no partial ID replacements");
    Check(Name("Unknown New Item") == "Unknown New Item", "unknown name preserved");
    Check(Name("Alcohol", "MEDIC Ammo Bush 玩家") == "MEDIC Ammo Bush 玩家", "custom names preserved");
    Check(Name("Alcohol") + Localization::Tr(" [empty]") == "酒 [空]", "ESP name with state suffix");
    Check(Search("酒", "alcohol_15", "King’s Reserve Bottle"), "Chinese item search");
    Check(Search("火花", "fukurou_spark_9", "Fukurou Spark"), "Chinese vehicle search");
    Check(Search("ALCOHOL_15", "alcohol_15", "King’s Reserve Bottle"), "ASCII ID search case fold");
    Check(Search("reserve", "alcohol_15", "King’s Reserve Bottle"), "English item search case fold");
    Check(Search("自定义", "alcohol_15", "King’s Reserve Bottle", "自定义酒瓶"), "custom label search");
    Check(Search("", "", ""), "empty query");
    Check(!Search("步枪", "alcohol_15", "King’s Reserve Bottle"), "unrelated query excluded");

    char text[128];
    std::snprintf(text, sizeof(text), Localization::Tr("Players On Map (%d / %d)###srv_roster"), 5, 5);
    Check(std::string(text) == "地图上的玩家（5 / 5）###srv_roster", "roster values and stable ImGui ID");
    std::snprintf(text, sizeof(text), Localization::Tr("%d alive | %d dead | %d party"), 5, 0, 0);
    Check(std::string(text) == "5 人存活 | 0 人死亡 | 0 名队友", "roster counts");
    std::snprintf(text, sizeof(text), Localization::Tr("%d alive | %d dead | %d party | %d spec"), 4, 1, 2, 3);
    Check(std::string(text) == "4 人存活 | 1 人死亡 | 2 名队友 | 3 人观战", "spectator counts");
    std::snprintf(text, sizeof(text), Localization::Tr("%d shown | %d enabled | %d sel"), 103, 103, 0);
    Check(std::string(text) == "103 项显示 | 103 项启用 | 0 项选中", "catalog counts");
    std::snprintf(text, sizeof(text), Localization::Tr("Range: %d m"), 500);
    Check(std::string(text) == "距离：500 米", "popup range");
    std::snprintf(text, sizeof(text), Localization::Tr("%d m"), 500);
    Check(std::string(text) == "500 米", "row range");
    Check(Localization::TranslateRuntime("[KD] [KO] [V]") == "[倒地] [昏迷] [车辆]", "existing ESP states");
    ConfigInstance.Overlay.ChineseLanguage = false;
    for (const auto& [en, zh] : kExpectedNames)
        Check(Name(en) == en, "switch back to English");
    Check(Name("fukurou_spark_9") == "fukurou_spark_9", "English raw vehicle ID restored");
    std::printf("%s: %d checks; %zu catalog rows; %d vehicle rows; %d failures\n",
        failures ? "FAIL" : "PASS", checks, LootFilter::kItemCatalog.size(), vehicleRows, failures);
    return failures ? 1 : 0;
}
