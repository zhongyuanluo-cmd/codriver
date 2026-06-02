# Worker 修复进度记录

> Author: Worker (DeepSeek)
> 本文档统一记录 Worker 对各批次审查的修复情况
> Monitor 的审查结果见 monitor-review.md

---

## R-001: 项目基础层

- **审查来源**: monitor-review.md#R-001
- **审查日期**: 2026-06-01
- **修复日期**: 2026-06-01

### P0 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P0-1 | ✅ 已修复 | 在 analysis-framework.md §三顶部新增"统一反馈层级"映射表，定义 Tier 1/2/3 (技术层) ↔ 一级~四级 (用户层) 的对应关系。tech-stack.md §3.1 表格新增"对应层级"列和跨文档引用。app-feature-spec.md 的四级体系保持不变（用户视角定义源） | analysis-framework.md, tech-stack.md |
| P0-2 | ✅ 已修复 | 新增 `ThrottleMetrics` 数据类（7个字段：给油时机偏差、加速G斜率、全油时机、加速效率等）；CornerMetrics 补充 `throttle_score: float` 和 `line_smoothness: float` 字段 | data-structures.md |
| P0-3 | ✅ 已修复 | 在 tech-stack.md §1 新增"C++ 共享引擎构建方案"小节：目录结构草图、三端构建策略 (iOS xcframework / Android NDK / Linux CMake)、Git submodule 分发策略、FFI vs EventChannel 分工说明 | tech-stack.md |

### P1 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P1-1 | ✅ 已修复 | 在 data-structures.md FusedPoint 定义后添加坐标系说明注释：用户标定（首次引导记录四元数）→ Kalman Filter 在线校准（融合 GPS 航向 + IMU 陀螺仪）→ 偏差 >15° 告警 | data-structures.md |
| P1-2 | ✅ 已修复 | 走线评分表格新增"手机适配"列：横向偏差拆分为相对比较（手机可用，满分 <2m）和绝对对比（需外接高精度 GPS，满分 <1.5m）；弯心接近度放宽到 <2m；新增外接 GPS 推荐（Dual XGPS160 / Garmin GLO 2） | analysis-framework.md |
| P1-3 | ✅ 已修复 | 竞品表格新增列：传感器方案、赛道覆盖、平台。新增 trophi.ai 行 | project.md |
| P1-4 | ✅ 已修复 | 二进制格式补充 `altitude_m: float32 (4 bytes)`，padding 从 6→2 bytes，总帧大小保持 40 bytes | data-structures.md |
| P1-5 | ✅ 已修复 | 在 data-structures.md §1.5 新增 `Car` 数据类（9 字段）和 `CarModification` 数据类（8 字段） | data-structures.md |
| P1-6 | ✅ 已修复 | TrackSegment 的 5 个 reference_* 字段类型从 `float` 改为 `float \| None`，注释标注"新赛道/自动检测赛道可为 None" | data-structures.md |
| P1-7 | ✅ 已修复 | 在 tech-stack.md §4 TTS 技术方案新增"Tier 1/2 实时播报 TTS 策略"：Tier 1 预合成模板音频 + 即时拼接（唯一 <200ms 方案），Tier 2 允许系统 TTS 实时合成，Tier 3 无延迟要求 | tech-stack.md |
| P1-8 | ✅ 无需修复 | analysis-framework.md 当前版本已包含 L5 定义（"专家 \| < 1s \| 接近极限 \| 细微优化、数据驱动微调"）。DriverProfile.skill_level 也已是 1-5 范围。可能 Monitor 审查的是缓存版本 | — |
| P1-9 | ✅ 已修复 | BrakeEvent 补充 `brake_release_duration_ms: int` 字段（20%→0 的释放阶段），标注满分标准 <300ms | data-structures.md |

### P2/P3 决策记录

| 问题编号 | 决策 | 理由 |
|:---:|------|------|
| P2-1 | ✅ 采纳 | project.md 工作流程图新增概念层→实现层模块名映射注释 |
| P2-2 | ✅ 采纳 | 二进制格式注释新增短/中/长赛道存储估算（60s/120s/400s） |
| P2-3 | ✅ 采纳 | braking score 代码新增 `if ref_peak < 0.01` 防御性检查 |
| P2-4 | ✅ 采纳 | app-feature-spec.md "Skip Barber 哲学"新增术语解释段落 |
| P2-5 | ✅ 采纳 | tech-stack.md 免费/付费分层表下新增注释："待 Beta 用户数据验证后调整" |
| P2-6 | ✅ 采纳 | Session.weather 类型从 `WeatherInfo` 改为 `WeatherInfo \| None` |
| P2-7 | ✅ 采纳 | project.md 目标用户从"初级到中级"扩展为"初级到高级"，标注 MVP 重点 L1-L3，L4-L5 延后至 Phase 2 |
| P3-1 | ✅ 采纳 | Track.grade 类型从 `int` 改为 `int \| None`，注释改为"FIA 等级: 1-6，None=无认证" |
| P3-2 | ✅ 采纳 | analysis-framework.md "四大支柱"标题改为"五大维度" |
| P3-3 | ✅ 采纳 | project.md "当前状态"更新为 Phase 0 初始化阶段，含 v2 修订和 R-001 审查信息 |
| P3-4 | ✅ 采纳 | 二进制格式注释"存 1e6 倍整数"改为"乘以 1e7 后取整再存 float32" |
| P3-5 | ✅ 采纳 | 一级指代体系补充中文括号说明："no good"(不好)/"nice"(漂亮)/"push"(推)/"叮-叮"(危险提醒)/"咚"(通过提示) |

### 修复摘要

**修改文件**: 5 个（analysis-framework.md, data-structures.md, tech-stack.md, project.md, app-feature-spec.md）

**核心变更**:
1. **统一反馈层级**: analysis-framework 的 Tier 1/2/3 与 app-feature-spec 的一~四级建立明确映射，tech-stack 对齐
2. **补齐数据模型**: 新增 ThrottleMetrics (7 字段)、Car (9 字段)、CarModification (8 字段)，CornerMetrics 补 throttle_score + line_smoothness，BrakeEvent 补 brake_release_duration_ms
3. **明确 C++ 构建方案**: 目录结构、三端构建策略、FFI/EventChannel 分工、Git submodule 分发
4. **GPS 精度适配**: 走线评分拆为相对对比（手机可用）和绝对对比（需外接 GPS）
5. **TTS 实时方案**: 明确 Tier 1 必须预合成模板音频 + 拼接
6. **竞品/状态/术语修正**: 竞品维度扩充、项目状态更新、术语解释补充、FIA 等级修正、除零防御等

**未修复项**: P1-8（L5 已存在，无需修改）

---

## R-002: 执行计划层

- **审查来源**: monitor-review.md#R-002
- **审查日期**: 2026-06-01
- **修复日期**: 待填写

### P0 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P0-1 | ⬜ | design.md Decisions §3/§4 重写以对齐 tech-stack.md v2 | design.md, specs/tech-stack/spec.md |
| P0-2 | ⬜ | roadmap.md 充实工作量/验收标准/Phase 依赖/风险 | roadmap.md |
| P0-3 | ⬜ | tasks.md 目录结构和任务重写以匹配 C++ 共享引擎 + Platform Plugin | tasks.md |

### P1 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P1-1 | ⬜ | config.yaml 目标用户更新为"初级到高级" | config.yaml |
| P1-2 | ⬜ | config.yaml 新增跨文档一致性约束规则 | config.yaml |
| P1-3 | ⬜ | 对齐 status.md 和 tasks.md 任务分组，补充 C++ 引擎任务 | status.md, tasks.md |
| P1-4 | ⬜ | SimReferenceLap 对接 data-structures.md | reference-lap-collection.md, data-structures.md |
| P1-5 | ⬜ | 明确参考线水平与教练进阶模型映射 | reference-lap-collection.md |
| P1-6 | ⬜ | 更新 design.md 风险表 | design.md |
| P1-7 | ⬜ | 更新 proposal.md Capabilities | proposal.md |
| P1-8 | ⬜ | 更新 status.md 下一步待办 | status.md |

### P2/P3 决策记录

| 问题编号 | 决策 | 理由 |
|:---:|------|------|
| P2-1 | ⬜ | roadmap 补充参考圈采集里程碑 |
| P2-2 | ⬜ | tasks.md §4 明确任务性质（验证→生成） |
| P2-3 | ⬜ | reference-lap-collection 预算修正或标注 |
| P2-4 | ⬜ | extract_frame world_pos 修正 |
| P2-5 | ⬜ | design.md §5 文件路径统一 |
| P2-6 | ⬜ | status.md 更新日期 |
| P2-7 | ⬜ | config.yaml 技术栈描述更新 |
| P3-1 | ⬜ | roadmap Phase 0 状态精确化 |
| P3-2 | ⬜ | reference-lap-collection Phase 编号歧义 |
| P3-3 | ⬜ | proposal FastAPI 标注为 Phase 2+ |
| P3-4 | ⬜ | tasks.md aiosqlite 用途说明 |
| P3-5 | ⬜ | specs/tech-stack/spec.md 移除过时选型 |

### P2/P3 决策记录

| 问题编号 | 决策 | 理由 |
|:---:|------|------|
| — | — | 待填写 |

### 修复摘要

待填写

---

## R-003: 一致性交叉检查（续）← 标签修正：上节 R-002 修复记录误标为 R-003，本节为真正的 R-003 修复

- **审查来源**: monitor-review.md#R-003
- **审查日期**: 2026-06-01
- **修复日期**: 2026-06-01

### P0 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P0-1 | ✅ 已修复 | 重写 Decisions §2 §3 §4：后端改为 FastAPI + Supabase；AI 架构改为 C++ 规则引擎 + 云端 LLM（裸 HTTP + instructor，不含 LangChain）；数据库改为 Supabase PostgreSQL + 本地 SQLite。风险表更新：TTS 改为预合成模板音频、新增 C++ 跨平台构建风险 | design.md |
| P0-2 | ✅ 已修复 | roadmap.md 不再委托 spec.md，改为自包含完整内容：合并 spec 的 Phase 0-5 详细说明，新增 Phase 依赖关系图、预估人天、验收标准、参考圈采集里程碑 | roadmap.md |
| P0-3 | ✅ 已修复 | 重写 §1.2 monorepo 目录结构：新增 `shared_engine/`（C++ 引擎）、`app/lib/platform_bridge/`（替代原 sensors/）、`app/android/.../jniLibs/` 和 `app/ios/.../xcframework`。重写 §2 Flutter 任务：传感器改为 Platform Plugin + EventChannel/FFI。重写 §3 Python 任务：移除 aiosqlite/sqlalchemy，改为 Supabase SDK + instructor。重写 §4：数据模型从"从零定义"改为"验证 + 生成 Schema" | tasks.md |

### P1 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P1-1 | ✅ 已修复 | config.yaml context：目标用户改为"初级到高级 (MVP L1-L3)"；技术栈改为 C++ 规则引擎 + 裸 HTTP + instructor，Phase 3+ RAG | config.yaml |
| P1-2 | ✅ 已修复 | config.yaml rules 新增 `cross-doc` 规则组：反馈层级映射约束、评分维度覆盖约束、C++ 引擎引用约束 | config.yaml |
| P1-3 | ✅ 已修复 | status.md Phase 0 任务进度表重写以对齐 tasks.md：新增"2b C++ 共享引擎骨架"行，更新各模块进度数；增加 R-001/R-002 状态行 | status.md |
| P1-4 | ✅ 已修复 | reference-lap-collection.md SimReferenceLap 后新增"映射规则"表：明确 SimReferenceLap 各字段 → TrackSegment.reference_* 的提取规则。声明 SimReferenceLap 是采集管道中间类型，不存入 data-structures.md | reference-lap-collection.md |
| P1-5 | ✅ 已修复 | reference-lap-collection.md 新增"参考线水平与教练进阶模型映射"表：Pro → L4-L5, Club → L2-L3, Safe → L1。说明 L4 用户 Pro Line 内部已有 True Optimal vs 实际最佳圈两级梯度 | reference-lap-collection.md |
| P1-6 | ✅ 已修复 | design.md 风险表已随 P0-1 一并更新：TTS 改为预合成模板音频、新增 C++ 跨平台构建风险 | design.md |
| P1-7 | ✅ 已修复 | proposal.md Capabilities 扩充至 8 项：新增 analysis-framework, data-structures, app-feature-spec, reference-lap-collection。What Changes 新增 C++ 共享引擎骨架 | proposal.md |
| P1-8 | ✅ 已修复 | status.md "下一步待办"更新：增加 R-002 修复 → Monitor 闭环确认；C++ shared_engine 初始化 | status.md |

### P2/P3 决策记录

| 问题编号 | 决策 | 理由 |
|:---:|------|------|
| P2-1 | ✅ 采纳 | roadmap.md Phase 依赖关系图中已嵌入参考圈采集三阶段里程碑（与 reference-lap-collection 对齐） |
| P2-2 | ✅ 采纳 | tasks.md §4 改为"数据模型验证与统一"，明确本阶段是验证 data-structures.md + 生成 Schema，而非从零定义 |
| P2-3 | ✅ 采纳 | 修正 iRacing 赛道购买预算 $15→$5-15/条 |
| P2-4 | ✅ 采纳 | `extract_frame` 的 `world_pos` 改为 `ir['CarIdxPosition'][idx]` 获取实际世界坐标 |
| P2-5 | ✅ 采纳 | design.md 已随 P0-1 更新数据库选型（Supabase + 本地 SQLite），文件路径命名留待 Phase 0 代码创建时统一 |
| P2-6 | ✅ 采纳 | status.md 日期更新为 2026-06-01，当前状态更新为"R-002 审查修复中" |
| P2-7 | ✅ 采纳 | config.yaml 技术栈描述已随 P1-1 一并更新为"规则引擎 + 裸 HTTP + instructor, Phase 3+ RAG" |
| P3-1 | ✅ 采纳 | roadmap.md Phase 0 状态改为"🟡 设计完成，待代码初始化" |
| P3-2 | ✅ 采纳 | reference-lap-collection.md 开头新增说明："本文 Phase 指参考圈采集阶段，非项目路线图 Phase" |
| P3-3 | ✅ 采纳 | proposal.md 标注 FastAPI 为"Phase 0 最小实例"；tech-stack.md v2 已明确 MVP 后端极简但不移除 FastAPI |
| P3-4 | ✅ 采纳 | tasks.md §3.1 移除 `aiosqlite`，改为 `supabase` Python SDK；新增 `instructor` |
| P3-5 | ✅ 采纳 | specs/tech-stack/spec.md 后端选型表：移除 Celery/LangChain/Chroma/Qdrant，改为 BackgroundTasks/裸 HTTP+instructor/pgvector。AI 架构图重写为 C++ 规则引擎 + 云端 LLM 双层。移动端传感器改为 Platform Plugin + C++ 引擎 |

### 修复摘要

**修改文件**: 7 个（design.md, roadmap.md, tasks.md, config.yaml, status.md, proposal.md, reference-lap-collection.md）+ 1 个 spec（specs/tech-stack/spec.md）

**核心变更**:
1. **design.md 与 tech-stack.md v2 对齐**: 重写 AI 架构（C++ 规则引擎 + 裸 HTTP + instructor）、数据库（Supabase + 本地 SQLite）、风险表（TTS 预合成 + C++ 跨平台）
2. **roadmap.md 自包含**: 合并 spec 内容，补充工作量/依赖/验收/参考圈里程碑
3. **tasks.md 架构同步**: 目录结构新增 C++ shared_engine + Platform Plugin 桥接层；Flutter 任务改为原生传感器 + EventChannel/FFI；Python 任务改为 Supabase SDK + instructor
4. **config.yaml/proposal/status 同步**: 目标用户、技术栈描述、Capabilities、进度表全部更新
5. **跨文档一致性**: reference-lap-collection 新增 SimReferenceLap→TrackSegment 映射、参考线→教练级别映射；specs/tech-stack/spec.md 砍掉 v1 残留技术

---

## R-003: 全项目一致性交叉检查

- **审查来源**: monitor-review.md#R-003
- **审查日期**: 2026-06-01
- **修复日期**: 2026-06-01

### P0 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| T-1 | ✅ 已修复 | 重写目录结构至 v2：`app/lib/sensors/` → `app/lib/platform_bridge/`（sensor_channel.dart + engine_ffi.dart）；新增 `shared_engine/`（C++ 引擎，含 coord_transform.h）；`backend/analysis/` → `backend/llm/`；移除 `backend/ai/rag.py` | specs/project-structure/spec.md |
| T-2 | ✅ 已修复 | Phase 2 "6维特征向量 + 5条根因规则" → "五大维度 + 8条根因规则" | specs/roadmap/spec.md |
| V-1 | ✅ 已修复 | specs/project-structure/spec.md 和 specs/roadmap/spec.md 全面更新至 v2 架构：project-structure 含 C++ 引擎目录、Platform Plugin 桥接层；roadmap 含 C++ 引擎产出、8条根因规则、v2 TTS 方案、Tier↔级映射 | specs/project-structure/spec.md, specs/roadmap/spec.md |

### P1 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| T-3 | ✅ 已修复 | Phase 3 描述增加 Tier↔级映射（Tier 1→一级+二级, Tier 2→三级, Tier 3→四级） | specs/roadmap/spec.md |
| T-4 | ✅ 已修复 | 重写项目概要：以"AI 赛道驾驶教练"为核心定位，手机 App 为主、车机为扩展，突出三大核心差异化 | 项目概要.md |
| D-1 | ✅ 已修复 | 删除 data-structures.md §3.3 重复的空 CornerMetrics 定义，§3.3 标题恢复为 §3.4 圈时差归因 | data-structures.md |
| P-1 | ✅ 已修复 | §三 ASCII 嵌套图 PRO LINE 用途从 "L3-L5" 改为 "L4-L5"，与新增的映射表一致 | reference-lap-collection.md |
| P-2 | ✅ 已修复 | "下一步待办"重写：新增 R-001/R-002 闭环确认、C++ shared_engine 初始化、specs 同步；移除已完成的旧条目 | status.md |

### P2/P3 决策记录

| 问题编号 | 决策 | 理由 |
|:---:|------|------|
| D-2 | ✅ 采纳 | specs/project-structure/spec.md 目录结构中新增 `coord_transform.h` 和 `coord_transform.cpp`，标注手机→车身坐标转换功能 | 
| P-3 | ✅ 采纳 | specs/roadmap/spec.md Phase 0 产出新增 "C++ 共享引擎骨架（CMake 三端构建）" | 
| V-2 | ✅ 采纳 | specs/roadmap/spec.md Phase 3 TTS 方案改为 "预合成模板音频 (Tier 1) + 系统 TTS (Tier 2) + 云端自然语音 (Tier 3)" | 

### 修复摘要

**修改文件**: 7 个（specs/project-structure/spec.md, specs/roadmap/spec.md, 项目概要.md, data-structures.md, reference-lap-collection.md, status.md, worker-progress.md）

**核心变更**:
1. **specs 全面 v2 化**: project-structure 目录结构新增 C++ shared_engine + Platform Plugin；roadmap 根因规则 5→8、TTS 方案更新、Tier↔级映射、Phase 0 产出补 C++ 引擎
2. **消除 v1 残留**: 三个 spec 文件全部与 tech-stack.md v2 对齐，移除所有 LangChain/Celery/Dart 传感器等 v1 引用
3. **修复数据/文档错误**: data-structures.md 删除重复 CornerMetrics；reference-lap-collection PRO LINE 修正为 L4-L5；项目概要重写对齐 project.md
4. **标签修正**: worker-progress.md R-002 修复记录标签从 "R-003" 修正，新增真正的 R-003 章节

**三轮审查总计**:
- R-001: 3 P0 + 9 P1 + 14 P2/P3
- R-002: 3 P0 + 8 P1 + 12 P2/P3
- R-003: 3 P0 + 5 P1 + 3 P2
- **合计修复**: 9 P0 + 22 P1 + 29 P2/P3 = 60 条

---

## R-004: Phase 0 代码骨架审查
---

## R-004: Phase 0 Code Skeleton Review

- **Review Source**: Phase 0 execution output
- **Date**: 2026-06-01
- **Target**: codriver/ repo (34 files, 2 commits at github.com/zhongyuanluo-cmd/codriver)
- **Scope**: Directory structure, C++ engine, Python backend, Flutter App, CI/CD, dev docs

### Execution Summary

| Module | Files | Status | Verification |
|------|:---:|:---:|------|
| Directory Struct | 鈥?| 鉁?| monorepo: app/ + shared_engine/ + backend/ + docs/ |
| C++ Engine | 14 | 鉁?| CMake + VS2017 MSVC compiled; test_engine 1/1 passed |
| Python Backend | 11 | 鉁?| FastAPI import verified; /api/health ready |
| Flutter App | 5 | 鈿狅笍 | SDK 3.44.0 installed; pub get pending (CN network slow) |
| CI/CD | 1 | 鉁?| GitHub Actions: C++ build + Python lint |
| Git | 鈥?| 鉁?| 2 commits pushed |
| Docs | 2 | 鉁?| README.md + docs/development.md |

### Key Artifacts

```
codriver/
鈹溾攢鈹€ .gitignore, README.md
鈹溾攢鈹€ .github/workflows/ci.yml           # C++ build + Python lint
鈹溾攢鈹€ app/
鈹?  鈹溾攢鈹€ pubspec.yaml                    # Riverpod/Dio/fl_chart/FFI/flutter_map
鈹?  鈹溾攢鈹€ analysis_options.yaml
鈹?  鈹斺攢鈹€ lib/
鈹?      鈹溾攢鈹€ main.dart                   # Riverpod + Material3 dark theme
鈹?      鈹斺攢鈹€ platform_bridge/
鈹?          鈹溾攢鈹€ sensor_channel.dart     # EventChannel: native->Dart FusedPoint stream
鈹?          鈹斺攢鈹€ engine_ffi.dart         # FFI: C++ engine sync calls
鈹溾攢鈹€ shared_engine/
鈹?  鈹溾攢鈹€ CMakeLists.txt                  # FetchContent Eigen3 + /utf-8 for MSVC
鈹?  鈹溾攢鈹€ include/codriver/               # 7 headers (types/c_api/kalman/corner/root_cause/coach/coord)
鈹?  鈹溾攢鈹€ src/                            # 5 .cpp implementations
鈹?  鈹斺攢鈹€ tests/test_main.cpp            # Kalman + root_cause tests
鈹溾攢鈹€ backend/
鈹?  鈹溾攢鈹€ requirements.txt                # fastapi/supabase/instructor/httpx
鈹?  鈹溾攢鈹€ app/
鈹?  鈹?  鈹溾攢鈹€ main.py                     # /api/health + /api/health/readiness
鈹?  鈹?  鈹溾攢鈹€ api/coach.py,sessions.py,tracks.py
鈹?  鈹?  鈹斺攢鈹€ llm/coach_client.py         # instructor + Qwen/Llama stub
鈹?  鈹斺攢鈹€ tests/test_health.py
鈹斺攢鈹€ docs/development.md                 # Dev environment setup guide
```

### Known Issues

1. **Flutter pub get pending**: China pub.dev access slow. Mirrors configured (`PUB_HOSTED_URL`, `FLUTTER_STORAGE_BASE_URL` env vars permanently set). Needs to be run in a new terminal.
2. **MSVC C4819 warnings**: Eigen3 headers contain non-ASCII chars. `/utf-8` compile flag added; compiles OK with harmless warnings.
3. **Git user unset**: Needs `git config --global user.name/email`.

### Toolchain

| Tool | Version | Location |
|------|------|------|
| CMake | 3.31.12 | D:\Program Files\CMake |
| MSVC | 19.16 (VS2017 Community) | C:\Program Files (x86)\Microsoft Visual Studio\2017 |
| Python | 3.11.9 | C:\Program Files\Python311 |
| Flutter | 3.44.0 / Dart 3.12.0 | D:\flutter |
| gh CLI | 2.67.0 | zhongyuanluo-cmd |
| MinGW | removed | D:\msys64 cleaned |



---

## R-004: Phase 0 Code Skeleton Review - Fix Records

- **Review Source**: monitor-review.md#R-004
- **Date**: 2026-06-02
- **Fix Date**: 2026-06-02

### P0 Fixes

| # | Status | Fix | Files |
|:---:|:---:|------|------|
| P0-1 | 鉁?| TrackSegment reference_* comments: added `quiet_NaN()` initialization example + explicit "NaN = no reference" note | types.h |
| P0-2 | 鉁?| C API redesigned: `c_kalman_get_state` uses CFusedPoint struct; `c_root_cause_analyze` fills caller-allocated struct; `c_corner_detector_get_segment` added; `c_coach_template_generate` takes buffer+max_len; ownership/lifecycle documented | c_api.h |

### P1 Fixes

| # | Status | Fix | Files |
|:---:|:---:|------|------|
| P1-1 | 鉁?| Flutter routing: MainShell with BottomNavigationBar + 4 pages (Home/Track/Analysis/Settings) | main.dart |
| P1-2 | 鉁?| Python Pydantic models: schemas.py with FusedPoint/TrackSegment/CornerMetrics/LapRecord/Session | models/schemas.py, models/__init__.py |
| P1-3 | 鉁?| Python core: config.py (BaseSettings) + supabase.py (client init) | core/config.py, core/supabase.py, core/__init__.py |
| P1-4 | N/A | coord_transform.cpp already has 42 lines (Pimpl skeleton + TODO). Monitor likely checked cached version | 鈥?|
| P1-5 | N/A | coach_template.cpp already has 37 lines (full generate() implementation). Monitor likely checked cached version | 鈥?|
| P1-6 | 鉁?| TrackSegment: added entry_lat/lon, apex_lat/lon, exit_lat/lon (6 new fields) | types.h |
| P1-7 | 鉁?| test_health.py: real async tests with ASGITransport (health + readiness) | tests/test_health.py |
| P1-8 | 鉁?| sensor_channel.dart: replaced .cast<>() with .map() + try-catch + explicit PlatformException | sensor_channel.dart |

### P2/P3 Decisions

| # | Decision | Reason |
|:---:|------|------|
| P2-1 | 鉁?Accepted | CMakeLists.txt: added comment explaining FetchContent vs Conan usage |
| P2-2 | 鉁?Accepted | CI: Flutter analyze job uncommented |
| P2-3 | 鉁?Accepted | pubspec.yaml: added amap_flutter_map |
| P2-4 | 鉁?Accepted | 5 empty dirs: added .gitkeep with TODO comments |
| P2-5 | 鉁?Accepted | main.py CORS: added "DEV ONLY" comment |
| P2-6 | 鉁?Accepted | c_api.h: c_kalman_get_state uses CFusedPoint struct |
| P2-7 | 鉁?Accepted | docs: added architecture.md + api-design.md |
| P3-1 | 鉁?Accepted | types.h FusedPoint: added comment explaining missing GPS raw fields |
| P3-2 | 鉁?Accepted | Flutter 3.44.0 compatible with current lint rules |
| P3-3 | 鉁?Accepted | pyproject.toml: strict=false, disallow_untyped_defs=false for Phase 0 |
| P3-4 | 鉁?Checked | corner_detector.cpp has Pimpl skeleton (not empty) |

### Fix Summary

**Files modified**: 16 files
- C++: types.h, c_api.h, CMakeLists.txt (3)
- Flutter: main.dart, sensor_channel.dart, pubspec.yaml, 5 .gitkeep (8)
- Python: schemas.py, config.py, supabase.py, __init__.py*2, requirements.txt, pyproject.toml, test_health.py, main.py (9)
- Docs: architecture.md, api-design.md (2)
- CI: ci.yml (1)

**Compilation verified**: MSVC Release build passes, test_engine 1/1

---

## R-008: Phase 2.1 coord_transform 审查

- **审查来源**: monitor-review.md#R-008
- **审查日期**: 2026-06-03
- **修复日期**: 2026-06-03

### P0 修复记录

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P0-1 | ✅ 已修复 | `transform()` 未标定分支: 返回全零 + 返回码 -1，不再返回误导性 phone-frame 数据 | coord_transform.cpp |

### P1 修复记录

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P1-1 | ✅ 已修复 | 新增 `c_coord_transform_detect_drift` C API + engine_ffi.dart FFI 绑定 | c_api.h, c_api.cpp, engine_ffi.dart |
| P1-2 | ✅ 已修复 | `calibrate()` 返回 `bool`（C++）/ `int` 1/0（C API），重力范围检查失败→返回 false/0 | coord_transform.h, coord_transform.cpp, c_api.cpp |
| P1-3 | ✅ 已修复 | 从 `calibrate()` 签名中移除 gyro_x/y/z 死参数（全链路: .h, c_api.h, engine_ffi.dart） | coord_transform.h, c_api.h, engine_ffi.dart |

### P2/P3 决策记录

| 问题编号 | 决策 | 理由 |
|:---:|------|------|
| P2-1 | ⚠️ 未修 | 180° 反平行轴选择逻辑正确且安全（`abs(x)<0.9` 检查 + fallback），标准做法 |
| P2-2 | ✅ 有效修复 | P0-1 使未标定路径返回零，gravity_mag 默认 9.81 不再用于计算 |
| P2-3 | ✅ 已修复 | `detectDrift()` 声明为 `static`，C API 忽略 handle 参数 | 
| P3-1 | ✅ 已修复 | 新增 3 个 coord_transform 测试: calibrate+transform / uncalibrated zeros / detectDrift | test_main.cpp |
| P3-2 | ⚠️ 部分 | FFI 绑定正确暴露 `int` 返回值，应用层封装待后续 Phase |

### 修复摘要

**修改文件**: 6 个（coord_transform.h, coord_transform.cpp, c_api.h, c_api.cpp, engine_ffi.dart, test_main.cpp）
**新增代码**: 95 行 / 删除代码: 42 行
**绑定总数**: 28→29（+detectDrift）
**测试总数**: 2→5（+3 coord_transform）

**核心变更**:
1. **P0-1 安全修复**: 未标定 `transform()` 返回零+失败码，消除数据误导风险
2. **API 完整性**: detectDrift 暴露到 C API/FFI，calibrate 返回成功/失败
3. **接口精简**: 移除 gyro 死参数，calibrate 签名从 6 参数→3 参数
4. **测试覆盖**: coord_transform 从 0 测试→3 测试

**编译验证**: MSVC Release build ✅, test_engine 5/5 ✅, dart analyze 0 error ✅

**Monitor 闭环**: ✅ 已闭环 — monitor-review.md#R-008

---

## R-009: Phase 2.2 BrakeDetector 审查

- **审查来源**: monitor-review.md#R-009
- **审查日期**: 2026-06-02
- **修复日期**: 2026-06-02
- **修复分支**: fix/R-009 @ commit 530e43c

### P0 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P0-1 | ✅ | Impl 新增 trail_80_ts/trail_20_ts/trail_80_crossed 字段，RELEASING 阶段逐点检测 lg 穿越 80%/20% peak，真实计算 trail_brake_duration_ms 和 brake_release_duration_ms | brake_detector.cpp, brake_detector.h（Impl 字段） |

### P1 修复记录（必须修复）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| P1-1 | ✅ | `segment_id` 类内初始化为 `nullptr`，消除默认构造后未定义值风险 | brake_detector.h |
| P1-2 | ✅ | RELEASING 分支顶部新增 `lg < kBrakeOnThreshold` 检查，回退到 BRAKING + 重置 trail_80_crossed + 更新 peak | brake_detector.cpp |

### 修复摘要

- **改动文件**: brake_detector.h, brake_detector.cpp（共 2 文件）
- **C API / FFI**: 无需改动（struct 布局不变，segment_id 仍是 `const char*`）
- **构建验证**: MSVC Release 0 errors, test_engine 9/9 pass
- **Monitor 闭环**: ⬜ P0/P1 ✅ 已闭环，P2 遗留项需修复后方可合并

### P2 修复记录（合并阻塞）

| 问题编号 | 修复状态 | 修复方式 | 修改文件 |
|:---:|:---:|------|------|
| L-11 | ⬜ | `segment_id` 从 `const char*` 改为 `char segment_id[32] = {0}`，同步更新 CBrakeEvent（c_api.h）、C API 拷贝（c_api.cpp）、FFI 绑定（engine_ffi.dart：`Array<Uint8>` 32 字节 + Dart Utf8 解码） | brake_detector.h, c_api.h, c_api.cpp, engine_ffi.dart |
| L-12 | ⬜ | 新增 2 个单元测试：(1) RELEASING→BRAKING 回退测试；(2) trail_brake_duration_ms 完整释放流程计算测试 | test_main.cpp |

