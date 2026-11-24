#pragma once

#include "ConfigInstance.h"
#include <string>
#include <string_view>
#include <unordered_map>

namespace Localization {

inline bool IsChinese()
{
	return ConfigInstance.Overlay.ChineseLanguage;
}

inline const char* Tr(const char* text)
{
	if (!text || !IsChinese()) return text ? text : "";
	static const std::unordered_map<std::string_view, const char*> words = {
		{"Aim", "瞄准"}, {"Visuals", "视觉"}, {"World", "世界"}, {"Radar", "雷达"}, {"Misc", "杂项"}, {"Settings", "设置"}, {"Chinese Language", "简体中文"},
		{"General", "常规"}, {"Targets & Recoil", "目标与后坐力"}, {"Triggerbot", "自动扳机"}, {"Hardware & Pad", "硬件与手柄"},
		{"Max Distance", "最大距离"}, {"Font Size", "字体大小"}, {"Dynamic Recoil Curve", "动态后坐力曲线"}, {"Curve Strength", "曲线强度"}, {"Bone Thickness", "骨骼粗细"}, {"Fill Opacity", "填充不透明度"}, {"Limb Thickness", "肢体粗细"}, {"Reload BVH", "重新加载 BVH"}, {"BVH Draw Dist", "BVH 绘制距离"}, {"Vis Ray Count", "可见性射线数"}, {"Vis Max Dist", "可见性最大距离"}, {"Hidden Alpha", "隐藏时透明度"}, {"Vis Ring", "可见性环"}, {"State Tags", "状态标签"}, {"Blend Alpha", "混合透明度"}, {"House Range", "房屋范围"}, {"House / Legacy Font", "房屋/旧版字体"}, {"Base Dot Size", "标记点基础尺寸"}, {"ESP Font Size", "透视字体大小"}, {"ESP Icon Size", "透视图标大小"}, {"Global Max Distance", "全局最大距离"}, {"High Value Pulse (Radar)", "高价值雷达脉冲"}, {"Per-Category Sizing", "按类别调整尺寸"}, {"Min Price", "最低价格"}, {"Mid Value", "中等价值"}, {"Web Port", "网页端口"}, {"Auto Map Select", "自动选择地图"}, {"Icon Scale", "图标缩放"}, {"Cargo", "货物"}, {"Camera Zoom", "镜头缩放"}, {"Camera Distance", "镜头距离"}, {"FOV Changer", "视野修改"}, {"Camera FOV", "镜头视野"}, {"Instant Loot", "快速搜刮"}, {"Instant Safe & Door Unlock", "快速开启保险箱与门"}, {"Mem Write Master Switch", "内存写入总开关"}, {"Hide Default Crosshair", "隐藏默认准星"}, {"Battle Mode Key", "战斗模式按键"}, {"Battle Mode Action", "战斗模式动作"}, {"Read Item Catalog", "读取物品目录"}, {"Dump Item Prices", "导出物品价格"}, {"Probe Money Models", "探测货币模型"}, {"Enable Skin Changer", "启用皮肤修改"}, {"Weapon Skins", "武器皮肤"}, {"Players On Map", "地图上的玩家"}, {"Trusted Players", "可信玩家"}, {"Slot", "槽位"}, {"Profile", "配置方案"}, {"Device", "设备"}, {"COM Port", "串口"}, {"Baud Rate", "波特率"}, {"Host Pad Slot", "主机手柄槽位"}, {"Gamepad Aim Key", "手柄瞄准键"}, {"Trigger Threshold", "扳机阈值"}, {"Stick Deadzone", "摇杆死区"}, {"Require Stick Move", "需要摇杆输入"}, {"Stick Output Scale", "摇杆输出缩放"}, {"Show Role", "显示身份"}, {"Off", "关闭"}, {"Page Up", "上一页"}, {"Page Down", "下一页"},
		{"Player ESP", "玩家透视"}, {"Skeleton & VisCheck", "骨骼与可见性检测"}, {"States & Colours", "状态与颜色"},
		{"World & Loot", "世界与战利品"}, {"Value & Sizing", "价值与尺寸"}, {"Catalog", "目录"}, {"2D & Web", "二维与网页雷达"}, {"In-Game Map", "游戏内地图"},
		{"Gameplay & Tools", "玩法与工具"}, {"Skins", "皮肤"}, {"Server", "服务器"}, {"Overlay & Hotkeys", "叠加层与快捷键"}, {"Configs & Status", "配置与状态"},
		{"Aimbot", "瞄准辅助"}, {"Memory Aim", "内存瞄准"}, {"Tuning & Prediction", "调校与预测"}, {"Target Filters", "目标筛选"}, {"Aim Visuals", "瞄准显示"}, {"Recoil & Spread", "后坐力与散布"}, {"Live State", "实时状态"},
		{"Weapon Profiles", "武器配置"}, {"Hardware Device", "硬件设备"}, {"Controller & Input", "控制器与输入"}, {"Weapons & Range", "武器与距离"}, {"Flags & HUD", "标记与 HUD"}, {"Skeleton & Chams", "骨骼与模型高亮"}, {"Visibility & BVH", "可见性与 BVH"}, {"State Indicators", "状态指示"}, {"Name & Role Colours", "名称与身份颜色"}, {"Rendering", "渲染"},
		{"Loot Filter Display", "战利品筛选显示"}, {"Value Tiers", "价值等级"}, {"Filter Presets", "筛选预设"}, {"Per-Category Sizing", "分类尺寸"}, {"Live World Stats", "世界实时统计"}, {"Loot Catalog", "战利品目录"}, {"2D Radar", "二维雷达"}, {"Web Radar", "网页雷达"}, {"Map Selection & Bounds", "地图选择与边界"}, {"Overlay Modes", "地图叠加模式"}, {"Map Markers", "地图标记"}, {"Gameplay", "玩法"}, {"Battle Mode", "战斗模式"}, {"Diagnostics & Tools", "诊断与工具"}, {"Session Telemetry", "会话遥测"}, {"Skin Changer", "皮肤修改"}, {"Cosmetics", "外观"}, {"Weapon Skins", "武器皮肤"}, {"Players On Map", "地图上的玩家"}, {"Overlay & Display", "叠加层与显示"}, {"Hotkeys & Safety", "快捷键与安全"}, {"Config Profiles", "配置方案"}, {"Connection Status", "连接状态"}, {"About", "关于"},
		{"Enable Aim", "启用瞄准"}, {"Aim Type", "瞄准类型"}, {"Target Bone", "目标骨骼"}, {"Target Priority", "目标优先级"}, {"Dynamic Bone Fallback", "动态骨骼回退"}, {"Aim Key", "瞄准键"}, {"Write Camera Angles", "写入镜头角度"}, {"Cursor Fallback", "光标回退"}, {"Response", "响应"}, {"Aim FOV", "瞄准视野"}, {"Smoothing Curve", "平滑曲线"}, {"Smoothness", "平滑度"}, {"Bezier Arc", "贝塞尔曲线"}, {"Curve Speed", "曲线速度"}, {"Hardware Gain", "硬件增益"}, {"Predict Targets", "预测目标"}, {"Hard Lock", "强锁定"}, {"Switch Delay", "切换延迟"}, {"Prediction & Lock", "预测与锁定"},
		{"Visible Bones Only", "仅可见骨骼"}, {"Ignore Teammates", "忽略队友"}, {"Skip Knocked Players", "跳过倒地玩家"}, {"Skip Police targets", "跳过警察目标"}, {"Skip Criminal targets", "跳过罪犯目标"}, {"Skip Civilian targets", "跳过平民目标"}, {"Show FOV Circle", "显示视野圈"}, {"Aim Debug HUD", "瞄准调试 HUD"}, {"No Recoil", "无后坐力"}, {"No Spread", "无子弹散布"}, {"Dynamic Recoil Curve", "动态后坐力曲线"}, {"Enable Triggerbot", "启用自动扳机"}, {"Trigger Key", "扳机键"}, {"Trigger Bone", "扳机骨骼"}, {"Min Delay", "最小延迟"}, {"Max Delay", "最大延迟"}, {"Burst Hold", "连发保持"}, {"Shot Cooldown", "射击冷却"}, {"Hitbox Scale", "命中框缩放"}, {"Max Range", "最大范围"}, {"Visible Only", "仅可见目标"}, {"Enable Weapon Profiles", "启用武器配置"}, {"Weapon Class", "武器类别"},
		{"Hide Teammates", "隐藏队友"}, {"Hidden Player ESP", "隐藏玩家透视"}, {"Box ESP", "方框透视"}, {"Health", "生命值"}, {"Distance", "距离"}, {"Snaplines", "指向线"}, {"Snapline Origin", "指向线起点"}, {"Off-Screen Indicators", "屏幕外指示"}, {"Head Hitbox Dot", "头部命中点"}, {"Weapon Name", "武器名称"}, {"Weapon Sprite", "武器图标"}, {"Looking At You", "正在看你"}, {"Looking Alert", "注视警报"}, {"Spectator Alert", "观战者警报"}, {"Show Role", "显示身份"}, {"Color-code by role", "按身份着色"}, {"Money & Value HUD", "资金与价值 HUD"}, {"Draw Skeleton", "绘制骨骼"}, {"Head Circle", "头部圆圈"}, {"3D Capsule Chams", "3D 胶囊模型高亮"}, {"Character Chams", "角色模型高亮"}, {"Enable VisCheck", "启用可见性检测"}, {"Enable WallCheck", "启用墙体检测"}, {"Show BVH", "显示 BVH"}, {"Car Cabin Solid", "车辆驾驶舱实心"}, {"Status Ring", "状态环"}, {"Firing", "开火中"}, {"Reloading", "换弹中"}, {"Armor", "护甲"}, {"Armor Value", "护甲值"}, {"Stamina", "体力"}, {"Tag Colour", "标签颜色"}, {"Color names by role", "按身份为名称着色"}, {"Pose Smoothing", "姿态平滑"}, {"Native Camera", "原生镜头"},
		{"Enable World ESP", "启用世界透视"}, {"House Info", "房屋信息"}, {"Show Empty Tag", "显示空置标签"}, {"Defense Blocks", "防御方块"}, {"Legacy Mode (Filter Off)", "旧版模式（筛选关闭）"}, {"Show Distance", "显示距离"}, {"Security Alarms", "安全警报"}, {"First Aid Cabinets", "急救柜"}, {"Drug Labs", "制药台"}, {"Enable Loot Filter", "启用战利品筛选"}, {"Show on Map", "在地图显示"}, {"Show on Minimap", "在小地图显示"}, {"Hide Empty Containers", "隐藏空容器"}, {"Hide Looted Items", "隐藏已搜刮物品"}, {"Read Vehicle / Table Inventories", "读取车辆/桌面物品"}, {"Filter By Min Price", "按最低价格筛选"}, {"High Value", "高价值"}, {"High Value Color", "高价值颜色"}, {"Mid Value", "中等价值"}, {"Mid Value Color", "中等价值颜色"}, {"Export JSON", "导出 JSON"}, {"Import JSON", "导入 JSON"}, {"Copy Code", "复制代码"}, {"Paste Code", "粘贴代码"}, {"Show Category", "显示类别"}, {"Hide Category", "隐藏类别"}, {"Show Selected", "显示选中项"}, {"Hide Selected", "隐藏选中项"}, {"Clear", "清除"}, {"All", "全部"}, {"None", "无"}, {"Default", "默认"}, {"Display", "显示"}, {"ITEM", "物品"}, {"RANGE", "范围"}, {"DISPLAY", "显示"}, {"Enable Radar", "启用雷达"}, {"Show Players", "显示玩家"}, {"Show Vehicles", "显示车辆"}, {"Show Loot", "显示战利品"}, {"Rotate With Camera", "随镜头旋转"}, {"Full Map Window", "全屏地图窗口"}, {"Web Radar Server", "网页雷达服务"}, {"Pop Out Web Radar", "弹出网页雷达"}, {"Active Map", "当前地图"}, {"Calibrate to Map", "校准至地图"}, {"Top-Left Minimap", "左上角小地图"}, {"Fullscreen Map (M)", "全屏地图（M）"}, {"Hide 3D ESP on Full Map", "全屏地图时隐藏 3D 透视"}, {"Player Arrows", "玩家箭头"}, {"Vehicle Icons", "车辆图标"}, {"Player Names (Full Map)", "全屏地图显示玩家名称"}, {"Dropped Bags", "掉落背包"}, {"Valuables & Loot", "贵重物品与战利品"}, {"Min Loot Rarity", "最低战利品稀有度"}, {"Min Cargo Rarity", "最低货物稀有度"},
		{"Transparent", "透明"}, {"Click-Through When Closed", "关闭时鼠标穿透"}, {"Always On Top", "窗口置顶"}, {"High Contrast Menu", "高对比度菜单"}, {"Accent Colour", "强调色"}, {"Show FPS", "显示帧率"}, {"Status Panel", "状态面板"}, {"Font", "字体"}, {"Frame Cap", "帧率上限"}, {"Display Monitor", "显示器"}, {"Menu Key", "菜单键"}, {"Panic Mode", "紧急停止模式"}, {"Panic Key", "紧急停止键"}, {"Re-attach Key", "重新连接键"}, {"Show Backdrop", "显示背景"}, {"Save", "保存"}, {"Load", "加载"}, {"Rename Selected", "重命名所选项"}, {"New Profile", "新建方案"}, {"Rename", "重命名"}, {"Create", "创建"}, {"Delete", "删除"}, {"Confirm?", "确认？"}, {"Apply", "应用"}, {"Search...", "搜索..."}, {"Re-attach", "重新连接"}, {"Connect", "连接"}, {"Disconnect", "断开连接"}, {"Refresh Ports", "刷新端口"}, {"Test Movement", "测试移动"}, {"Locked - enable Mem Write Master Switch first.", "已锁定，请先启用内存写入总开关。"}, {"No matching options.", "没有匹配的选项。"}, {"No items match the current filter.", "没有物品符合当前筛选条件。"}, {"Waiting for in-game map state...", "正在等待游戏地图状态..."}, {"Waiting for game snapshot...", "正在等待游戏数据..."}, {"No world snapshot. Live stats appear in-raid.", "暂无世界数据，进入战局后将显示实时统计。"},
		{"Criminal", "罪犯"}, {"Police Officer", "警察"}, {"Civilian", "平民"}, {"Hubber", "居民"}, {"Male", "男性"}, {"Female", "女性"}, {"Connected", "已连接"}, {"Offline", "离线"}, {"Uncapped", "不限制"}, {"DMA Offline", "DMA 未连接"}, {"Waiting for Game", "等待游戏"}, {"In Match", "战局中"}, {"Attached (Menu)", "已连接（菜单中）"}, {"No COM ports found.", "未发现串口。"}, {"No gamepad detected (Makcu/Ferrum controller & host XInput).", "未检测到手柄（Makcu/Ferrum 控制器或主机 XInput）。"}, {"No player entities detected on the map. Join a match or attach DMA reader.", "地图上未检测到玩家实体。请进入战局或连接 DMA 读取器。"}, {"No players match the current filter.", "没有玩家符合当前筛选条件。"}, {"No reader snapshot available.", "暂无读取器快照。"}, {"Join a match to pick cosmetics.", "进入战局后可选择外观。"}, {"Join a match to pick weapon skins.", "进入战局后可选择武器皮肤。"}, {"No snapshot yet. Waiting for the game process.", "暂无快照，正在等待游戏进程。"}, {"No inventory snapshot yet (enable while in a raid).", "暂无物品栏快照（请在战局中启用）。"},
		{"No owner", "无所有者"}, {"Raided", "已被袭击"}, {"Not raided", "未被袭击"}, {"No fuses", "无保险丝"}, {"Fuses armed", "保险丝已启动"}, {"Raid locked", "袭击锁定"}, {"Loot - placement ", "战利品 - 放置点 "}, {"[empty]", "[空]"}, {"[loot]", "[战利品]"}, {"[armed]", "[已启动]"}, {"DEFENCE", "防御"}, {"Doors", "门"}, {"BUILDINGS", "建筑"}, {"is looking at you!", "正在看着你！"}, {"COINS : $%d", "金币：$%d"}, {"CRYPTO : %d", "加密币：%d"}, {"BAG VALUE : $%d (Cash $%d / Items $%d)", "背包价值：$%d（现金 $%d / 物品 $%d）"}, {"EQUIP : $%d  |  VEH : $%d", "装备：$%d  |  车辆：$%d"}, {"Looking Alert", "注视警报"}, {"Radar", "雷达"}, {"Full Map", "全屏地图"},
	};
	auto it = words.find(text);
	return it == words.end() ? text : it->second;
}

inline std::string TranslateRuntime(std::string text)
{
	if (!IsChinese()) return text;
	const auto replaceAll = [&](std::string_view from, std::string_view to) {
		std::size_t pos = 0;
		while ((pos = text.find(from, pos)) != std::string::npos) {
			text.replace(pos, from.size(), to);
			pos += to.size();
		}
	};
	if (text.ends_with(" is looking at you!")) {
		text.resize(text.size() - sizeof(" is looking at you!") + 1);
		text += "正在看着你！";
		return text;
	}
	for (const auto& pair : { std::pair{" [empty]", " [空]"}, std::pair{" [loot]", " [战利品]"},
		std::pair{" [armed]", " [已启动]"}, std::pair{"[no fuses]", "[无保险丝]"},
		std::pair{"RAIDED", "已被袭击"}, std::pair{"doors:", "门："}, std::pair{" open", " 已打开"},
		std::pair{"Loot - placement ", "战利品 - 放置点 "}, std::pair{"No owner", "无所有者"},
		std::pair{"BUILDINGS", "建筑"}, std::pair{"Slot ", "槽位 "}, std::pair{"Entity ", "实体 "},
		std::pair{"[VIS]", "[可见]"}, std::pair{"[WALL]", "[墙后]"}, std::pair{"[ARM]", "[护甲]"},
		std::pair{"[AIMING]", "[瞄准]"}, std::pair{"[KD]", "[倒地]"}, std::pair{"[KO]", "[昏迷]"},
		std::pair{"[SUR]", "[投降]"}, std::pair{"[HC]", "[铐住]"}, std::pair{"[HIDING]", "[隐藏]"},
		std::pair{"[CBAG:", "[警包："}, std::pair{"[RBAG:", "[匪包："}, std::pair{"[LOOKING]", "[注视]"},
		std::pair{"[V]", "[车辆]"}, std::pair{"[C]", "[蹲伏]"}, std::pair{"[S]", "[奔跑]"},
		std::pair{"CBAG", "警包"}, std::pair{"RBAG", "匪包"}, std::pair{"ME", "我"} })
		replaceAll(pair.first, pair.second);
	for (std::size_t i = 1; i < text.size(); ++i) {
		if (text[i] == 'm' && text[i - 1] >= '0' && text[i - 1] <= '9') {
			text.replace(i, 1, "米");
			i += std::string("米").size() - 1;
		}
	}
	return std::string(Tr(text.c_str()));
}

} // namespace Localization
