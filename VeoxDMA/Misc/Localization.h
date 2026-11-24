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
		{"Weapon", "武器"}, {"Weapons", "武器"}, {"Player", "玩家"}, {"Players", "玩家"}, {"Vehicle", "载具"}, {"Vehicles", "载具"}, {"Police", "警察"}, {"Police Officer", "警察"}, {"Criminal", "罪犯"}, {"Civilian", "平民"}, {"Hubber", "居民"},
		{"Head", "头部"}, {"Neck", "颈部"}, {"Chest", "胸部"}, {"Pelvis", "骨盆"}, {"Closest", "最近"}, {"Head & Body", "头部与身体"}, {"Crosshair FOV", "准星视野"}, {"3D Distance", "三维距离"}, {"Lowest Health", "最低生命值"}, {"Hardware Aim", "硬件瞄准"}, {"Memory Aim", "内存瞄准"}, {"Linear", "线性"}, {"Bezier", "贝塞尔"},
		{"Target", "目标"}, {"Bone", "骨骼"}, {"Key", "按键"}, {"Aim Deadzone", "瞄准死区"}, {"Random Offset", "随机偏移"}, {"Soft Entry & Falloff", "平滑进入与衰减"}, {"Profile Bone", "方案骨骼"}, {"Profile Curve", "方案曲线"}, {"Profile Trigger", "方案扳机"}, {"Allow Trigger", "允许扳机"}, {"Override Active", "启用覆盖"}, {"Reset Profile", "重置方案"}, {"Reset To Native", "恢复原生设置"},
		{"Dot Radius", "标记半径"}, {"Max Distance (Aim)", "最大瞄准距离"}, {"Max Distance (Player)", "最大玩家距离"}, {"Font Size (Player)", "玩家字体大小"}, {"Min Cash / Value", "最低现金/价值"}, {"Category##size", "类别##size"}, {"Use Global Distance", "使用全局距离"}, {"Share or reset item rules.", "分享或重置物品规则。"}, {"Search...", "搜索..."}, {"Scrollable list", "可滚动列表"},
		{"On", "开启"}, {"Yes", "是"}, {"No", "否"}, {"Up", "按下"}, {"Down", "抬起"}, {"held", "按下"}, {"up", "抬起"}, {"Auto", "自动"}, {"Head", "头部"}, {"Body", "身体"}, {"Camera", "镜头"}, {"Screen", "屏幕"}, {"Bottom", "底部"}, {"Center", "中心"}, {"Top", "顶部"}, {"Crosshair", "准星"},
		{"Right Shift", "右 Shift"}, {"Insert", "插入键"}, {"Delete", "删除键"}, {"Home", "Home 键"}, {"End", "End 键"}, {"Off", "关闭"}, {"Off-Screen Indicators", "屏幕外指示"}, {"Match", "战局"}, {"Active", "运行中"}, {"Waiting", "等待中"}, {"Not attached", "未连接"}, {"Attached", "已连接"}, {"Menu", "菜单"},
		{"RANGE", "距离"}, {"DISPLAY", "显示"}, {"ITEM", "物品"}, {"Entities", "实体"}, {"Cash sources", "现金来源"}, {"Top value", "最高价值"}, {"Inventory", "物品栏"}, {"Slots", "槽位"}, {"Catalog", "目录"}, {"Skeletons", "骨骼数"}, {"Retries", "重试次数"}, {"Read failures", "读取失败"}, {"Overlay", "叠加层"}, {"FPS", "帧率"}, {"ENT", "实体"},
		{"High Contrast Menu", "高对比度菜单"}, {"Accent Colour", "强调色"}, {"Status Panel", "状态面板"}, {"Frame Cap", "帧率上限"}, {"Display Monitor", "显示器"}, {"Panic Mode", "紧急停止模式"}, {"Panic Key", "紧急停止键"}, {"Re-attach Key", "重新连接键"}, {"Show Backdrop", "显示背景"}, {"Config Profiles", "配置方案"}, {"Connection Status", "连接状态"}, {"Re-attach Game", "重新连接游戏"}, {"Match: Active (%zu entities | %u skeletons | %u failures)", "战局：运行中（%zu 个实体 | %u 个骨骼 | %u 次失败）"}, {"Match: Waiting for entities (menu / loading)", "战局：等待实体（菜单/加载中）"}, {"Match: Not attached", "战局：未连接"}, {"Overlay: %.0f FPS", "叠加层：%.0f 帧/秒"}, {"Menu Key: %s", "菜单键：%s"}, {"ROB RUN External DMA Overlay", "ROB RUN 外置 DMA 叠加层"}, {"External DMA reader - never injects into the game process. Rows marked MEM WRITE modify game memory over DMA.", "外置 DMA 读取器，不会注入游戏进程。标记为内存写入的项目会通过 DMA 修改游戏内存。"}, {"%s to close", "按 %s 关闭"},
		{"Waiting for in-game map state...", "正在等待游戏地图状态..."}, {"Waiting for game snapshot...", "正在等待游戏数据..."}, {"Live Map State", "实时地图状态"}, {"No matching options.", "没有匹配的选项。"}, {"No items match the current filter.", "没有物品符合当前筛选条件。"}, {"No world snapshot. Live stats appear in-raid.", "暂无世界数据，进入战局后将显示实时统计。"}, {"No inventory snapshot yet (enable while in a raid).", "暂无物品栏数据（请在战局中启用）。"}, {"No gamepad detected (Makcu/Ferrum controller & host XInput).", "未检测到手柄（Makcu/Ferrum 控制器或主机 XInput）。"}, {"No COM ports found.", "未发现串口。"}, {"Choose a specific key to activate battle mode.", "选择一个按键以启用战斗模式。"}, {"Suppresses World ESP, Radar & Money HUD so only Player ESP draws.", "隐藏世界透视、雷达和资金 HUD，仅显示玩家透视。"}, {"Master Switch is OFF - all MEM WRITE features below are turned off and locked.", "内存写入总开关已关闭，下方所有内存写入功能均已关闭并锁定。"}, {"Master Switch is OFF - enable Mem Write Master Switch to unlock.", "内存写入总开关已关闭，请先开启以解除锁定。"},
		{"Aim", "瞄准"}, {"Visuals", "视觉"}, {"World", "世界"}, {"Radar", "雷达"}, {"Misc", "杂项"}, {"Settings", "设置"}, {"Chinese Language", "简体中文"},
		{"IP address", "IP 地址"}, {"Network port", "网络端口"}, {"Device ID", "设备 ID"}, {"COM3", "COM3"},
		{"Custom label (blank = default)", "自定义标签（留空使用默认）"}, {"Search items...", "搜索物品..."},
		{"Filter name, clan, weapon...", "筛选名称、战队、武器..."}, {"New name...", "新名称..."}, {"Config name...", "配置名称..."},
		{"Scrollable", "可滚动"}, {"SHOW", "显示"}, {"PRICE", "价格"}, {"AIMING AT YOU", "正在瞄准你"},
		{"MEM WRITE WARNING", "内存写入警告"}, {"You're enabling the Mem Write Master Switch, do you wish to proceed?", "你正在启用内存写入总开关，是否继续？"},
		{"Yes, Enable", "是，启用"}, {"No, Don't Enable", "否，不启用"}, {"This unlocks MEM WRITE toggles so you can enable them. Turning the Master Switch off immediately turns off, locks, and restores all active writes.", "这将解锁内存写入开关。关闭总开关会立即关闭、锁定并恢复所有活动写入。"},
		{"Super switch for all MEM WRITE features. When OFF, all write toggles are turned off and locked.", "所有内存写入功能的总开关。关闭后所有写入开关都会关闭并锁定。"},
		{"Panic instantly blanks the overlay and halts targeting.", "紧急模式会立即清空叠加层并停止瞄准。"},
		{"Read-only DMA overlay silhouette over character limbs.", "在角色肢体上绘制只读 DMA 轮廓。"},
		{"Loot Filter owns item visibility & range. Turn Loot Filter off to use legacy switches.", "战利品筛选控制物品显示与距离。关闭战利品筛选后可使用旧版开关。"},
		{"No items match the current filter.", "没有物品符合当前筛选条件。"}, {"No matching options.", "没有匹配的选项。"},
		{"No COM ports found.", "未发现串口。"}, {"Waiting for in-game map state...", "正在等待游戏地图状态..."}, {"Waiting for game snapshot...", "正在等待游戏数据..."},
		{"IP address", "IP 地址"}, {"Network port", "网络端口"}, {"Device ID", "设备 ID"}, {"COM3", "COM3"},
		{"Custom label (blank = default)", "自定义标签（留空使用默认）"}, {"Search items...", "搜索物品..."},
		{"Filter name, clan, weapon...", "筛选名称、战队、武器..."}, {"New name...", "新名称..."}, {"Config name...", "配置名称..."},
		{"Scrollable", "可滚动"}, {"Scrollable list", "可滚动列表"}, {"SHOW", "显示"}, {"PRICE", "价格"}, {"AIMING AT YOU", "正在瞄准你"},
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
		{"Edit", "编辑"}, {"Global", "全局"}, {"Alive", "存活"}, {"Hide Party", "隐藏队伍"}, {"Hide Self", "隐藏自己"}, {"YOU", "自己"}, {"PARTY", "队伍"}, {"DEAD", "已淘汰"}, {"KNOCKED", "已击倒"}, {"AIMING AT YOU", "正在瞄准你"}, {"TRUSTED", "可信"}, {"Trust", "信任"}, {"Untrust", "取消信任"}, {"UNKNOWN", "未知"}, {"Weapon: ", "武器："}, {"[Vehicle]", "[载具]"},
		{"Enable Gamepad Input", "启用手柄输入"}, {"Dual Gamepad Key", "双手柄按键"}, {"Auto (0-3)", "自动（0-3）"}, {"Pad 1", "手柄 1"}, {"Pad 2", "手柄 2"}, {"Pad 3", "手柄 3"}, {"Pad 4", "手柄 4"}, {"Manual COM Port", "手动指定串口"}, {"Ferrum Controller", "Ferrum 控制器"}, {"Protocol verified.", "协议已验证。"}, {"Retrying in %u ms", "%u 毫秒后重试"}, {"Enabled", "已启用"}, {"Disabled", "已禁用"}, {"None", "无"}, {"Default", "默认"}, {"Inherit", "继承"}, {"Any / Closest", "任意 / 最近"}, {"Rifle", "步枪"}, {"SMG", "冲锋枪"}, {"Shotgun", "霰弹枪"}, {"Pistol", "手枪"}, {"Sniper", "狙击枪"}, {"DMR", "精确射手步枪"}, {"DMR / Sniper", "精确射手步枪 / 狙击枪"}, {"Heavy", "重型武器"}, {"Melee", "近战武器"}, {"Unknown", "未知"}, {"Mouse 4", "鼠标 4"}, {"Mouse 5", "鼠标 5"}, {"Caps Lock", "大写锁定"}, {"Space", "空格"}, {"Tab", "Tab 键"}, {"Gamepad LT", "手柄 LT"}, {"Gamepad RT", "手柄 RT"}, {"Gamepad LB", "手柄 LB"}, {"Gamepad RB", "手柄 RB"}, {"Gamepad L3", "手柄 L3"}, {"Gamepad R3", "手柄 R3"}, {"Gamepad A", "手柄 A"}, {"Gamepad B", "手柄 B"}, {"Gamepad X", "手柄 X"}, {"Gamepad Y", "手柄 Y"}, {"Gamepad D-Up", "手柄方向键上"}, {"Gamepad D-Down", "手柄方向键下"}, {"Gamepad D-Left", "手柄方向键左"}, {"Gamepad D-Right", "手柄方向键右"}, {"Gamepad Back", "手柄返回键"}, {"Gamepad Start", "手柄开始键"}, {"Gamepad Stick", "手柄摇杆"},
		{"Aim disabled", "瞄准未启用"}, {"device idle", "设备空闲"}, {"device ready", "设备就绪"}, {"Trigger: off", "扳机：关闭"}, {"Key", "按键"}, {"Candidates", "候选目标"}, {"target", "目标"}, {"Write", "写入"}, {"fail", "失败"}, {"ok", "成功"}, {"override", "覆盖"}, {"Class", "类别"}, {"Equipped", "已装备"}, {"No weapon", "无武器"}, {"waiting", "等待中"}, {"ready", "就绪"}, {"idle", "空闲"}, {"Status:", "状态："}, {"Endpoint:", "端点："}, {"Pad:", "手柄："}, {"LT", "LT"}, {"RT", "RT"}, {"RS", "RS"}, {"Aim:", "瞄准："}, {"Curve:", "曲线："}, {"FOV:", "视野："}, {"Weapon:", "武器："}, {"Trigger:", "扳机："}, {"Candidates ", "候选目标 "}, {"target ", "目标 "}, {"Yaw", "偏航"}, {"Pitch", "俯仰"},
		{"Right Mouse", "鼠标右键"}, {"Left Mouse", "鼠标左键"}, {"Middle Mouse", "鼠标中键"}, {"Left Alt", "左 Alt"}, {"Right Alt", "右 Alt"}, {"Left Ctrl", "左 Ctrl"}, {"Right Ctrl", "右 Ctrl"}, {"Left Shift", "左 Shift"}, {"Page Up", "上一页"}, {"Page Down", "下一页"}, {"FOV", "视野"}, {"EMA", "EMA"}, {"XInput", "XInput"}, {"Kmbox USB", "Kmbox USB"}, {"Kmbox Net", "Kmbox Net"}, {"Makcu Controller", "Makcu 控制器"}, {"Enable Gamepad Input", "启用手柄输入"}, {"Key up", "按键未按下"}, {"Key held", "按键已按下"}, {"device not ready", "设备未就绪"}, {"aim disabled", "瞄准未启用"}, {"aim enabled", "瞄准已启用"}, {"no target", "无目标"}, {"No target", "无目标"}, {"None (Default)", "无（默认）"}, {"Trigger: off", "扳机：关闭"}, {"Trigger: on", "扳机：开启"}, {"Trigger: idle", "扳机：待机"},
		{"Zeroes weapon vectors over DMA or applies dynamic pattern compensation.", "通过 DMA 清零武器向量，或应用动态压枪补偿。"}, {"Trusted Players: %d (manage in Misc -> Server)", "可信玩家：%d（在“杂项 -> 服务器”中管理）"}, {"Key %s (%s) | %s", "按键 %s（%s）| %s"}, {"Weapon: %s (%s%s) | Trigger: %s", "武器：%s（%s%s）| 扳机：%s"}, {"Candidates %d / %d | target %d", "候选目标 %d / %d | 目标 %d"}, {"Write %s (%s) | yaw %.1f pitch %.1f | %.0fm", "写入 %s（%s）| 偏航 %.1f 俯仰 %.1f | %.0f 米"}, {"Equipped: %s (Class: %s)", "已装备：%s（类别：%s）"}, {"No weapon", "无武器"}, {"No target", "无目标"}, {"aim disabled | Curve: %s | FOV: %d deg", "瞄准未启用 | 曲线：%s | 视野：%d 度"}, {"Status: %s", "状态：%s"}, {"Endpoint: %s", "端点：%s"}, {"Pad: %s | LT %.2f RT %.2f | RS (%.2f, %.2f)", "手柄：%s | LT %.2f RT %.2f | RS（%.2f，%.2f）"}, {"Read-only DMA overlay silhouette over character limbs.", "在角色肢体上绘制只读 DMA 轮廓。"}, {"Weapon skin data not found (offsets changed?).", "未找到武器皮肤数据（偏移是否已变化？）。"}, {"Switch weapons once if a chosen skin does not apply immediately.", "如果选择的皮肤没有立即生效，请切换一次武器。"}, {"Client-side only. Saved per Male/Female and restored when off or closed.", "仅客户端生效。按男性/女性分别保存，关闭或退出时恢复。"}, {"Status: Disconnected", "状态：未连接"}, {"Status: Connected", "状态：已连接"}, {"Status: Connecting", "状态：连接中"}, {"Status: Recovering", "状态：恢复中"}, {"Status: Ready", "状态：就绪"},
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
		std::pair{"[FIRE]", "[开火]"}, std::pair{"[RLD]", "[换弹]"}, std::pair{"[STM", "[体力"},
		std::pair{"AIMING AT YOU", "正在瞄准你"}, std::pair{"Criminal", "罪犯"}, std::pair{"Police Officer", "警察"},
		std::pair{"Civilian", "平民"}, std::pair{"Hubber", "居民"}, std::pair{"Bush", "灌木"}, std::pair{"Weed", "杂草"},
		std::pair{"Raided", "已被袭击"}, std::pair{"Not raided", "未被袭击"}, std::pair{"No fuses", "无保险丝"},
		std::pair{"Fuses armed", "保险丝已启动"}, std::pair{"Doors:", "门："}, std::pair{" open", " 已打开"},
		std::pair{"PID ", "进程号 "}, std::pair{" FPS", " 帧/秒"}, std::pair{" ENT", " 实体"}, std::pair{"Uncapped", "不限制"},
		std::pair{"CBAG", "警包"}, std::pair{"RBAG", "匪包"}, std::pair{"ME", "我"},
		std::pair{"Aim: ", "瞄准："}, std::pair{"Curve: ", "曲线："}, std::pair{"FOV: ", "视野："},
		std::pair{"Menu: ", "菜单："}, std::pair{"Key: ", "按键："}, std::pair{"Entities: ", "实体："},
		std::pair{"Candidates: ", "候选目标："}, std::pair{"Candidates ", "候选目标 "}, std::pair{"Target: ", "目标："},
		std::pair{"Delta: ", "偏差："}, std::pair{"Step: ", "步进："}, std::pair{"Lead: ", "提前量："},
		std::pair{"Pred: ", "预测："}, std::pair{"Type: ", "类型："}, std::pair{"Device: ", "设备："},
		std::pair{"Write: ", "写入："}, std::pair{"yaw ", "偏航 "}, std::pair{"pitch ", "俯仰 "},
		std::pair{" dist", " 距离"}, std::pair{"Reason: ", "原因："}, std::pair{"Skip: ", "跳过："},
		std::pair{"ESP cam: ", "ESP 镜头："}, std::pair{"native ", "原生镜头 "}, std::pair{"pose=", "姿态="},
		std::pair{"not found", "未找到"}, std::pair{"closed", "已关闭"},
		std::pair{"瞄准：on  曲线：", "瞄准：开启  曲线："}, std::pair{"瞄准：off  曲线：", "瞄准：关闭  曲线："},
		std::pair{"菜单：open", "菜单：已打开"}, std::pair{"菜单：closed", "菜单：已关闭"},
		std::pair{"按键：held", "按键：已按下"}, std::pair{"按键：up", "按键：未按下"},
		std::pair{"Trigger: off", "扳机：关闭"}, std::pair{"Trigger: idle", "扳机：待机"},
		std::pair{"Write: off", "写入：关闭"}, std::pair{"Pred: off", "预测：关闭"},
		std::pair{"device idle", "设备空闲"}, std::pair{"device ready", "设备就绪"},
		std::pair{"memory", "内存"}, std::pair{"hardware", "硬件"}, std::pair{"not ready", "未就绪"},
		std::pair{"waiting for target", "等待目标"}, std::pair{"no entities", "无实体"},
		std::pair{"no target in FOV", "视野范围内无目标"}, std::pair{"no camera origin", "无镜头原点"},
		std::pair{"zero aim vector", "瞄准向量为零"}, std::pair{"in deadzone", "位于死区内"},
		std::pair{"inside deadzone", "位于死区内"}, std::pair{"on target", "已对准目标"},
		std::pair{"moving (cursor)", "光标移动中"}, std::pair{"moving (hardware)", "硬件设备移动中"},
		std::pair{"hardware device not ready", "硬件设备未就绪"}, std::pair{"hardware submit rejected", "硬件提交被拒绝"},
		std::pair{"memory write unavailable", "内存写入不可用"}, std::pair{"writing angles (memory)", "正在写入角度（内存）"},
		std::pair{"硬件 device 未就绪", "硬件设备未就绪"}, std::pair{"硬件 submit rejected", "硬件提交被拒绝"},
		std::pair{"内存 write unavailable", "内存写入不可用"}, std::pair{"writing angles (内存)", "正在写入角度（内存）"},
		std::pair{"Ammo", "弹药"}, std::pair{"Armor", "护甲"}, std::pair{"Breaching tools", "破门工具"},
		std::pair{"Car upgrades", "车辆升级"}, std::pair{"Cargo", "货物"}, std::pair{"Containers", "容器"},
		std::pair{"Cumulative items", "累计物品"}, std::pair{"Documents", "文件"}, std::pair{"Furniture & objects", "家具与物件"},
		std::pair{"Grenades", "手榴弹"}, std::pair{"Home & plot", "住宅与地块"}, std::pair{"Keys", "钥匙"},
		std::pair{"Laboratory", "实验室"}, std::pair{"Loot Nodes", "战利品点"}, std::pair{"Medicine", "医疗用品"},
		std::pair{"Misc", "杂项"}, std::pair{"NPC", "NPC"}, std::pair{"Plot buildings", "地块建筑"},
		std::pair{"Security", "安保"}, std::pair{"Tools", "工具"}, std::pair{"Valuables", "贵重物品"},
		std::pair{"Vehicles", "载具"}, std::pair{"Weapon maintenance", "武器维护"}, std::pair{"Weapons", "武器"},
		std::pair{"Workbenches & upgrades", "工作台与升级"}, std::pair{"World Loot", "世界战利品"},
		std::pair{"Name > Rarity", "名称 > 稀有度"}, std::pair{"Rarity > Name", "稀有度 > 名称"},
		std::pair{"Price High-Low", "价格：从高到低"}, std::pair{"Sell price", "出售价格"}, std::pair{"Buy price", "购买价格"},
		std::pair{"Unlock price", "解锁价格"}, std::pair{"Auth 1", "权限 1"}, std::pair{"Auth 2", "权限 2"},
		std::pair{"Auth 3", "权限 3"}, std::pair{"Auth 4", "权限 4"}, std::pair{"Auth 5", "权限 5"},
		std::pair{"Uncommon+", "罕见及以上"}, std::pair{"Rare+", "稀有及以上"}, std::pair{"Epic+", "史诗及以上"},
		std::pair{"Legendary+", "传说及以上"}, std::pair{"Mythical", "神话"}, std::pair{"Hold", "按住"},
		std::pair{"Toggle", "切换"}, std::pair{"Distance (Nearest)", "距离（最近）"},
		std::pair{"Health (Lowest)", "生命值（最低）"}, std::pair{"Name (A-Z)", "名称（A-Z）"} })
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
