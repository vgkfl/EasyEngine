# EasyEngine

A modular C++ game engine prototype focused on protocol-oriented architecture, ECS-driven runtime systems, and clear one-way dependency boundaries.

一个以**协议分层架构**、**ECS 运行时系统**和**单向依赖边界**为核心设计目标的 C++ 游戏引擎原型项目。

---

## 项目定位

EasyEngine 不是一个单纯堆功能的课程作业，而是一个持续演进中的引擎原型。  
这个项目的重点不只是“把渲染和动画跑起来”，更在于：

- 设计清晰的模块边界
- 控制运行时依赖方向
- 探索协议化、可扩展的引擎组织方式
- 为后续接入不同渲染后端、编辑工具和项目层逻辑预留空间

当前版本主要围绕以下方向展开：

- ECS 风格的实体与组件组织
- Transform 层级与运行时更新
- OpenGL 渲染链路
- 动画 / 骨骼相关基础设施
- ImGui 编辑器面板
- FBX 资源导入方向探索

---

## 当前重点能力

### 运行时架构
- 基于协议分层组织引擎模块
- 强调单向依赖，避免高层逻辑反向侵入底层
- 通过系统（System）统一调度运行时更新

### 场景与实体
- 实体创建、注册、组件挂载
- Transform 本地数据、层级关系与世界变换更新
- 基于 ECS 风格的批量遍历与系统处理

### 渲染
- OpenGL 渲染基础链路
- Mesh / Material / Shader 相关基础设施
- 向前渲染方向的原型实现
- 编辑器与运行时共享部分渲染能力

### 动画与资源
- FBX 导入方向探索
- 骨骼、姿态、动画播放相关基础设施
- Root Motion / Character 相关实验性逻辑

### 工具与编辑器
- 基于 ImGui 的编辑器面板原型
- World Hierarchy / Inspector 等基础面板
- 为后续编辑器模式扩展预留结构

---

## 核心设计思路

这个项目最核心的部分不是某一个功能点，而是**模块如何协作**。

EasyEngine 当前采用分层协议化组织方式，主要划分为以下几层：

### 1. DataProtocol
纯数据层。  
负责跨模块传递和持久化所需的数据结构，例如：

- 数学类型
- Mesh / Material / Animation 等资产数据
- 可序列化、可交换的数据表达

### 2. BaseProtocol
基础运行时数据层。  
负责系统运行时真正挂在实体上的基础组件和通用状态，例如：

- LocalTransform
- TransformHierarchy
- Camera / MeshRenderer 等基础组件

### 3. ControlProtocol
控制器协议层。  
负责提供对世界上下文、管理器和运行时资源的访问入口。  
这一层更偏“访问门面”和“控制入口”，而不是系统更新逻辑本身。

### 4. System
系统层。  
负责真正的运行时规则与更新流程，例如：

- Transform 更新
- Render 更新
- Animation 更新

### 5. Launcher
启动器层。  
负责组织项目启动流程、运行时上下文、系统注册和主循环。

### 6. Tool
工具层。  
负责编辑器、导入器、调试工具等非核心运行时能力。

---

## 为什么这样分层

这个项目希望尽量避免以下问题：

- 控制器层膨胀成“万能类”
- 工具逻辑反向污染运行时核心
- 系统规则散落在各个 manager / panel / script 中
- 项目代码和引擎代码相互缠绕

因此，EasyEngine 更关注：

- **谁持有数据**
- **谁解释规则**
- **谁负责更新**
- **谁只做访问，不做调度**

这也是我在这个项目里重点练习和验证的部分。

---

## 当前目录结构（简化）

```text
EasyEngine/
├─ EZEngine/
│  ├─ module/
│  │  ├─ DataProtocol/
│  │  ├─ BaseProtocol/
│  │  ├─ ControlProtocol/
│  │  ├─ System/
│  │  ├─ Launcher/
│  │  └─ Tool/
│  ├─ Project/
│  │  └─ Test01/
│  └─ third_party/
├─ EasyEngine.sln
└─ README.md