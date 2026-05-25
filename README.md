# BOBAC3 Service Robot Simulation Workspace

BOBAC3（元宝三号）是一套基于 ROS 1 (Melodic/Noetic) 的自主导览服务机器人系统，集成语音交互、人脸识别、AR 标记追踪、自动充电与自主导航等核心功能，面向展馆/展厅等室内场景提供智能导览服务。

> 本项目由**兰州理工大学智能车协会**于 **2023 年**开发，基于 Reinovo（睿诺）BOBAC3 机器人平台。代码水平可能与当前竞赛新规及新选手方案存在差距，仅供参考与学习。

---

## 目录

- [功能概览](#功能概览)
- [系统架构](#系统架构)
- [功能包列表](#功能包列表)
- [环境要求](#环境要求)
- [编译与安装](#编译与安装)
- [快速上手](#快速上手)
  - [第一步：记录展位坐标](#第一步记录展位坐标)
  - [第二步：语音导航](#第二步语音导航日常使用)
  - [第三步：人脸识别](#第三步人脸识别可选)
  - [其他功能](#其他功能)
- [详细功能说明](#详细功能说明)
  - [自主导航](#自主导航)
  - [语音交互系统](#语音交互系统)
  - [人脸识别系统](#人脸识别系统)
  - [AR 标记与二次定位](#ar-标记与二次定位)
  - [自动充电](#自动充电)
  - [相对移动控制](#相对移动控制)
- [文件结构](#文件结构)
- [贡献指南](#贡献指南)
- [许可证](#许可证)

---

## 功能概览

| 功能 | 说明 |
|------|------|
| **自主导航** | 基于 Cartographer SLAM / AMCL 定位 + move_base (TEB 局部规划器) 的 2D 导航，支持仿真与实机 |
| **语音交互** | 唤醒词"元宝"，支持中文语音导航、运动控制、导览讲解，基于科大讯飞 AIUI 平台 |
| **人脸识别** | 基于 dlib / face_recognition 库的实时人脸识别，支持话题模式与服务模式 |
| **AR 标记追踪** | 基于 Alvar AR 码的视觉定位与姿态调整，支持精确接近对准 |
| **自动充电** | AR 码引导的充电桩自动对接 |
| **相对移动** | PID 控制的底盘相对位移（x/y/θ），支持差速与全向驱动 |
| **二次定位** | 利用 AR 标记进行视觉辅助的精细定位 |

---

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                      BOBAC3 Robot                           │
├──────────┬──────────┬───────────┬───────────┬───────────────┤
│  语音层  │  视觉层  │  导航层   │  运动层   │   感知层      │
│          │          │           │           │               │
│ robot_   │ face_rec │ bobac3_   │ relative_ │ ar_track_     │
│ audio    │          │ navigation│ move      │ alvar         │
│          │ face_rec_│           │           │               │
│ bobac3_  │ ex       │ (AMCL/    │ (PID      │ (marker       │
│ audio    │          │ Cartogra- │ control)  │ detection)    │
│          │          │ pher/move │           │               │
│          │          │ _base)    │ pid_lib   │               │
├──────────┴──────────┴───────────┴───────────┴───────────────┤
│              ar_pose (姿态调整 / 二次定位)                    │
├─────────────────────────────────────────────────────────────┤
│              auto_charging (自动充电对接)                     │
└─────────────────────────────────────────────────────────────┘
```

**依赖关系：**

```
pid_lib
  └── relative_move
        ├── ar_pose
        │     ├── auto_charging
        │     └── secondary_localization
        └── robot_audio (语音导航/运动控制)
              └── bobac3_audio

face_rec
  └── face_rec_ex

bobac3_navigation (独立导航栈)
```

---

## 功能包列表

### 1. bobac3_navigation — 导航栈

自主导航核心包，包含地图服务、定位（AMCL / Cartographer）、路径规划（move_base + TEB）及 RViz 配置。

| 子目录 | 内容 |
|--------|------|
| `launch/` | 导航启动文件（实机/仿真） |
| `param/` | move_base、TEB 规划器、代价地图参数 |
| `maps/` | PGM 地图文件及对应的 YAML 配置 |
| `rviz/` | RViz 预设配置 |

### 2. robot_audio — 语音基础设施

核心语音交互包，封装科大讯飞 AIUI SDK，实现音频采集（VAD）、语音唤醒、ASR/NLU、TTS 合成及语音驱动的导航/运动控制。

| 节点 | 功能 |
|------|------|
| `voice_collect_node` | 音频采集 + VAD 语音端点检测 |
| `robot_aiui_node` | 讯飞 AIUI 服务（TTS / ASR / NLU） |
| `voice_awake_node` | 唤醒词检测（"元宝"） |
| `voice_nav_node` | 语音导航（命名位置导航 + 导览模式） |
| `voice_control_node` | 语音运动控制（前进/后退/左移/右移/转向） |
| `voice_interaction_node` | 语音交互主控：采集 → 唤醒 → 指令 → 分发 |

### 3. bobac3_audio — 高层音频实验

基于 robot_audio 服务的上层 C++ 客户端，包含 TTS、语音采集、听写、语义分析及导游导航等实验节点。

### 4. face_rec — 人脸识别

基于 Python `face_recognition`（dlib）的实时人脸识别功能包。

| 节点 | 模式 |
|------|------|
| `face_rec_topic.py` | 话题模式：持续识别，发布标注画面和结果 |
| `face_rec_service.py` | 服务模式：按需识别（相机/图片） |

### 5. face_rec_ex — 人脸识别集成

结合人脸识别与语音交互的示例节点——识别到人脸后通过语音问答播报身份。

### 6. ar_pose — AR 姿态调整

订阅 `ar_track_alvar` 的标记检测结果，计算目标位姿，调用 `relative_move` 服务精确接近 AR 标记。

### 7. ar_track_alvar — AR 标记检测

vendored 版本的开源 `ar_track_alvar` (v0.7.1)，用于 AR 标签的检测与位姿估计。

### 8. relative_move — 相对移动

PID 控制的底盘相对位移服务，支持差速和全向驱动模型，通过 TF 坐标变换计算精确运动。

### 9. pid_lib — PID 控制器库

头文件级别的 C++ PID 控制器，支持 Kp/Ki/Kd 配置、积分限幅和输出钳制。

### 10. auto_charging — 自动充电

结合 AR 标记追踪与相对移动，实现充电桩自动对接。跟踪标记 #0，接近至 0.3m 后回退 0.1m 完成对接。

### 11. secondary_localization — 二次定位

利用 AR 标记进行视觉辅助的精细定位校正。

---

## 环境要求

### 操作系统

- Ubuntu 18.04 (ROS Melodic) 或 Ubuntu 20.04 (ROS Noetic)

### ROS 依赖

```bash
# 核心依赖
sudo apt install ros-${ROS_DISTRO}-amcl \
                 ros-${ROS_DISTRO}-map-server \
                 ros-${ROS_DISTRO}-move-base \
                 ros-${ROS_DISTRO}-move-base-msgs \
                 ros-${ROS_DISTRO}-gmapping \
                 ros-${ROS_DISTRO}-cartographer \
                 ros-${ROS_DISTRO}-cartographer-ros \
                 ros-${ROS_DISTRO}-robot-state-publisher \
                 ros-${ROS_DISTRO}-joint-state-publisher \
                 ros-${ROS_DISTRO}-gazebo-ros \
                 ros-${ROS_DISTRO}-usb-cam \
                 ros-${ROS_DISTRO}-ydlidar
```

### Python 依赖（人脸识别）

```bash
pip install dlib==19.19.0
pip install face_recognition
pip install Pillow    # 中文文本渲染
```

### 科大讯飞 AIUI SDK（语音交互）

- 需要注册科大讯飞开发者账号并获取 AIUI SDK 及 AppID
- SDK 文件放置于 `src/robot_audio/AIUI/` 目录

### 外部功能包

```bash
cd ~/catkin_ws/src
# BOBAC3 机器人描述包（URDF）
git clone https://gitee.com/reinovo/bobac3_description.git
```

---

## 编译与安装

```bash
# 1. 创建工作空间（如未创建）
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws/src

# 2. 克隆本仓库
git clone <repository-url> raicom-bobac3-simulation
mv raicom-bobac3-simulation/src/* .

# 3. 安装外部依赖（参见上方"外部功能包"）

# 4. 编译
cd ~/catkin_ws
catkin_make

# 5. 加载环境
source devel/setup.bash
```

---

## 快速上手

> **重要提示：** 以下每一步都需要在**新开的终端**中执行，且每个终端都要先运行 `source devel/setup.bash`。

### 第一步：记录展位坐标

把机器人推到（或遥控到）各个展位位置，记录坐标用于语音导航。

**终端 1 — 启动导航（加载地图）：**
```bash
roslaunch bobac3_navigation bobac3_nav_2d.launch
```

**终端 2 — 移动机器人到目标展位，读取当前坐标：**
```bash
rosrun tf tf_echo /map base_footprint
```

输出会不断刷新类似这样的内容：
```
Translation: [-5.823, 8.637, 0.000]
```

记下每个展位的 `(x, y)` 坐标，编辑 `src/robot_audio/AIUI/dist/position_info.txt`，按以下格式填入：

```
展位名称 x坐标 y坐标 朝向 描述文字
```

例如：
```
门口 -5.823 8.637 0 1 这里是门口，进入公司的地方
```

### 第二步：语音导航（日常使用）

坐标记录完成后，日常使用只需两个终端：

**终端 1 — 启动导航：**
```bash
roslaunch bobac3_navigation bobac3_nav_2d.launch
```

**终端 2 — 启动语音交互：**
```bash
roslaunch bobac3_audio nav.launch
```

唤醒词为**"元宝"**，唤醒后可以说：
- "带我去深圳厅" —— 导航到指定展位
- "带我转一转" —— 自动导览模式，依次访问所有展位

### 第三步：人脸识别（可选）

**终端 1 — 启动人脸识别服务：**
```bash
roslaunch face_rec face_rec_service.launch
```

**终端 2 — 启动识别 + 语音播报：**
```bash
roslaunch face_rec_ex nav.launch
```

添加新人脸：在 `src/face_rec/face_data/` 下新建以人名命名的文件夹，放入至少一张清晰正面照即可。

### 其他功能

```bash
# 仿真环境导航
roslaunch bobac3_navigation demo_nav_2d.launch

# AR 标记追踪（底座相机）
roslaunch ar_pose ar_base.launch

# 自动充电对接
roslaunch auto_charging auto_charging.launch
```

---

## 详细功能说明

### 自主导航

导航系统支持两种定位模式：

**AMCL 定位：**
- 基于粒子滤波的 2D 定位
- 配合激光雷达（YDLidar）数据
- 似然场观测模型

**Cartographer SLAM：**
- 加载预建的 `.pbstream` 地图文件
- 适用于高精度定位需求

**路径规划参数（TEB）：**

| 参数 | 值 |
|------|-----|
| 最大 x 线速度 | 0.5 m/s |
| 最大 y 线速度 | 0.4 m/s |
| 机器人半径 | 0.19 m |
| 目标容忍度 (xy) | 0.1 m |
| 控制器频率 | 5 Hz |
| 规划器频率 | 1 Hz |
| 障碍物膨胀半径 | 0.15 m |

### 语音交互系统

语音交互采用多节点协作架构：

```
麦克风 → voice_collect (VAD)
              ↓
         voice_awake (唤醒检测)
              ↓
         voice_collect (指令采集)
              ↓
         robot_aiui (ASR + NLU)
              ↓
    ┌─────────┼─────────┐
    ↓         ↓         ↓
 voice_nav  voice_control  TTS
 (导航)     (运动控制)    (播报)
```

**NLU 意图分类：**
- `robot_nav`：命名位置导航
- `robot_control`：运动控制（前进/后退/左移/右移/转向）
- `robot_guid`：导览模式

### 人脸识别系统

基于 dlib 的 128 维人脸特征向量编码，使用 k-近邻分类器进行身份识别。

- 支持中文姓名标注（PIL 渲染）
- 人脸数据库：`face_rec/face_data/<人名>/` 目录下放入照片
- 消息类型：`face_data`（名称 + 边界框）、`face_results`（检测列表）
- 服务类型：`recognition_results`（mode=1 相机 / mode=2 图片）

### AR 标记与二次定位

使用 Alvar AR 码（底座 5cm / 手部 3cm）进行视觉定位。`ar_pose` 节点实现多阶段状态机：

1. **搜索目标**：等待指定 ID 的 AR 标记进入视野
2. **粗略接近**：调用 `relative_move` 向标记移动
3. **精细调整**：微调位置和朝向对准标记
4. **完成**：发布最终对准位姿

### 自动充电

自动充电流程：
1. 启动底座相机，检测充电座上的 AR 标记 (#0)
2. 通过 `ar_pose` 计算目标位姿
3. 调用 `relative_move` 逐步接近至 0.3m
4. 回退 0.1m 完成充电桩对接

### 相对移动控制

`relative_move` 服务接收目标位移 (Δx, Δy, Δθ)，通过 TF 坐标变换计算里程计系下的目标点，使用 PID 控制器驱动底盘运动。

支持模型：
- **差速驱动**：仅 x 和 θ 控制
- **全向驱动**：同时支持 x、y 和 θ 控制

---

## 文件结构

```
raicom-bobac3-simulation/
├── .gitignore
├── README.md
├── LICENSE
├── docs/
│   ├── 服务组操作命令.docx
│   └── 服务组操作命令.odt
├── src/
│   ├── bobac3_navigation/       # 导航栈（地图/定位/规划）
│   │   ├── launch/              # 启动文件
│   │   ├── param/               # 导航参数
│   │   ├── maps/                # 地图文件 (.pgm/.yaml/.pbstream)
│   │   └── rviz/                # RViz 配置
│   ├── robot_audio/             # 语音交互核心
│   │   ├── launch/              # 语音节点启动文件
│   │   ├── src/                 # C++ 节点源码
│   │   └── AIUI/                # 讯飞 SDK 及配置
│   ├── bobac3_audio/            # 高层音频实验节点
│   ├── face_rec/                # 人脸识别
│   │   ├── scripts/             # Python 识别脚本
│   │   ├── face_data/           # 人脸数据库（.gitignore 排除）
│   │   ├── launch/
│   │   └── msg/ & srv/          # 消息/服务定义
│   ├── face_rec_ex/             # 人脸识别 + 语音集成
│   ├── ar_pose/                 # AR 姿态调整
│   ├── ar_track_alvar/          # AR 标记检测（vendored）
│   ├── relative_move/           # 相对移动控制
│   ├── pid_lib/                 # PID 控制器库
│   ├── auto_charging/           # 自动充电
│   └── secondary_localization/  # 二次定位
```

---

## 贡献指南

1. Fork 本仓库并创建功能分支
2. 提交前确保代码可通过 `catkin_make` 编译
3. 遵循现有代码风格（C++ 使用 ROS 编程惯例，Python 使用 PEP 8）
4. 提交 Pull Request 并描述变更内容
5. **请勿提交**包含个人隐私数据的文件（人脸照片、录音等）

---

## 许可证

本项目采用 [GNU General Public License v3.0](LICENSE) 授权。

```
Copyright (C) 2023 Lanzhou University of Technology - Intelligent Vehicle Association

本程序是自由软件：你可以根据自由软件基金会发布的 GNU 通用公共许可证
第三版的条款重新发布和/或修改本程序。

本程序按"原样"发布，不附带任何明示或暗示的担保，包括但不限于对
适销性和特定用途适用性的担保。详见 GNU 通用公共许可证。
```

简要权利说明：

| 行为 | 是否允许 |
|------|----------|
| 商业使用 | 允许（必须同样开源） |
| 修改 | 允许 |
| 分发 | 允许（必须同样开源） |
| 私人使用 | 允许 |
| 专利授权 | 有 |
| 闭源分发 | **不允许** |

详细条款请参阅 [LICENSE](LICENSE) 文件。
