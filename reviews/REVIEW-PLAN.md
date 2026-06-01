# CoDriver 审查计划

> 最后更新: 2026-06-01
> Reviewer: Monitor (GLM)
> 基于 openspec 现有文档制定的全量审查计划

---

## 一、审查范围总览

当前 Worker (DeepSeek) 已完成的项目文档如下：

| 序号 | 文件 | 类型 | 审查优先级 |
|:---:|------|------|:---:|
| 1 | `项目概要.md` | 项目简介 | P2 |
| 2 | `openspec/config.yaml` | 项目配置 | P1 |
| 3 | `openspec/project.md` | 项目定义 | P0 |
| 4 | `openspec/roadmap.md` | 路线图 | P1 |
| 5 | `openspec/status.md` | 项目状态 | P1 |
| 6 | `openspec/design/tech-stack.md` | 技术选型 | P0 |
| 7 | `openspec/design/app-feature-spec.md` | 功能规格 | P0 |
| 8 | `openspec/design/analysis-framework.md` | 分析框架 | P0 |
| 9 | `openspec/design/data-structures.md` | 数据结构 | P0 |
| 10 | `openspec/design/reference-lap-collection.md` | 参考圈采集 | P1 |
| 11 | `openspec/changes/init-project-skeleton/proposal.md` | 变更提案 | P1 |
| 12 | `openspec/changes/init-project-skeleton/design.md` | Phase 0 设计 | P1 |
| 13 | `openspec/changes/init-project-skeleton/tasks.md` | Phase 0 任务 | P1 |
| 14 | `参考 - 赛道模式 Track mode 1.md` | 参考文档 | P2 |

---

## 二、审查批次与依赖

审查按批次进行，每批覆盖相关联的文档，确保跨文档一致性检查有效。

### 批次 1: 项目基础层 (P0)

**目标**：验证项目定位、核心假设、技术选型是否自洽

| 文件 | 审查重点 |
|------|------|
| `openspec/project.md` | 项目定位是否清晰；目标用户是否明确；竞品分析是否公正；核心卖点是否有依据 |
| `openspec/design/tech-stack.md` | 技术选型理由是否充分；架构是否可行；v1→v2 修订是否解决了评审问题；是否有遗留风险 |
| `openspec/design/analysis-framework.md` | 评分体系是否科学；规则引擎是否完备；实时反馈策略是否可行；教练进阶模型是否合理 |
| `openspec/design/data-structures.md` | 数据模型是否完整一致；字段定义是否精确；存储方案是否可行 |
| `openspec/design/app-feature-spec.md` | 功能规格是否覆盖全场景；与 project.md 的核心卖点是否对齐；功能优先级是否合理 |

**产出**：`monitor-review.md#R-001`

**关键审查点**：
1. tech-stack.md 中的架构与 analysis-framework.md 中的分层是否完全对应
2. data-structures.md 中的模型是否覆盖 app-feature-spec.md 中所有功能的数据需求
3. project.md 中承诺的核心卖点，在 design 文档中是否有完整的技术落地路径

### 批次 2: 执行计划层 (P1)

**目标**：验证路线图、Phase 0 规划与设计文档的对齐程度

| 文件 | 审查重点 |
|------|------|
| `openspec/roadmap.md` | Phase 划分是否合理；Phase 间依赖是否正确；交付物是否明确 |
| `openspec/status.md` | 状态记录是否准确；下一步是否与 roadmap 对齐；决策记录是否完整 |
| `openspec/config.yaml` | rules 是否与实际文档结构匹配；context 是否与 project.md 一致 |
| `openspec/changes/init-project-skeleton/proposal.md` | 提案是否与 design.md 对齐；Impact 是否完整 |
| `openspec/changes/init-project-skeleton/design.md` | 决策是否与 tech-stack.md 一致；风险是否已识别 |
| `openspec/changes/init-project-skeleton/tasks.md` | 任务是否覆盖 design 中所有决策；粒度是否可验证；工作量是否合理 |
| `openspec/design/reference-lap-collection.md` | 采集方案是否可行；成本估算是否现实；与 analysis-framework 的对接是否明确 |

**产出**：`monitor-review.md#R-002`

**关键审查点**：
1. tasks.md 中的任务是否覆盖了 tech-stack.md 中的所有技术选型决策
2. Phase 0 的 Non-Goals 是否与 Phase 1 的 scope 有清晰边界
3. reference-lap-collection 与 analysis-framework 中"参考圈"概念是否一致
4. status.md 中记录的 v2 修订决策是否全部在 design 文档中落地

### 批次 3: 一致性交叉检查 (跨批次)

**目标**：在批次 1 和 2 完成后，进行全项目范围的跨文档一致性检查

| 检查项 | 说明 |
|------|------|
| 术语一致性 | 所有文档中的关键术语（如"赛道分段""参考圈""Tier"）是否定义统一 |
| 数据流一致性 | 从传感器采集 → 分析 → AI 教练 → 用户交互的数据流是否在所有文档中描述一致 |
| 优先级一致性 | 功能优先级（免费/付费分层、Phase 优先级、MVP 范围）是否在所有文档中一致 |
| 版本一致性 | v1 → v2 的修订是否在所有相关文档中同步更新，是否存在残留的旧方案描述 |

**产出**：`monitor-review.md#R-003`

---

## 三、审查时间表

| 批次 | 预计审查项 | 产出 |
|:---:|:---:|------|
| 批次 1 | 5 个核心设计文档 | monitor-review.md#R-001 |
| 批次 2 | 7 个执行计划文档 | monitor-review.md#R-002 |
| 批次 3 | 全项目交叉检查 | monitor-review.md#R-003 |

---

## 四、已知风险与关注点

基于对现有文档的初步浏览，以下问题需在审查中重点关注：

### 4.1 技术层面

| # | 关注点 | 涉及文档 |
|:---:|------|------|
| 1 | Flutter Platform Plugin + C++ FFI 的构建复杂度可能被低估 | tech-stack.md, design.md |
| 2 | GPS 精度问题（手机 3-5m）对制动点分析的影响是否有充分缓解方案 | analysis-framework.md, tech-stack.md |
| 3 | Kalman Filter 在手机端的 CPU/电池消耗是否有实测数据支撑 | tech-stack.md |
| 4 | LLM 调用裸 HTTP + instructor 方案的错误处理和重试策略未详细说明 | tech-stack.md |

### 4.2 产品层面

| # | 关注点 | 涉及文档 |
|:---:|------|------|
| 5 | 免费/付费分层是否足够驱动付费转化 | app-feature-spec.md, tech-stack.md |
| 6 | 赛道数据库的冷启动问题（30 条赛道的人工标注工作量） | reference-lap-collection.md |
| 7 | 用户众包数据质量的保障机制缺失 | reference-lap-collection.md |

### 4.3 一致性层面

| # | 关注点 | 涉及文档 |
|:---:|------|------|
| 8 | data-structures.md 中的 FusedPoint 是否覆盖 analysis-framework.md 所有分析维度所需字段 | data-structures.md, analysis-framework.md |
| 9 | app-feature-spec.md 中"四级信息体系"与 analysis-framework.md 中"三层反馈策略"是否完全对齐 | app-feature-spec.md, analysis-framework.md |
| 10 | tech-stack.md v2 修订砍掉的功能（LangChain/Celery/本地 LLM）是否在所有文档中都已同步更新 | 全部 |

---

## 五、审查执行方式

1. **逐批次执行**：按批次 1→2→3 顺序，Monitor 每批次在 `monitor-review.md` 中写入审查结果
2. **问题即时记录**：审查过程中发现的问题立即记录到对应批次审查报告
3. **Worker 统一修复记录**：Worker 完成修复后，在 `worker-progress.md` 中统一记录所有批次的修复内容（按审查报告编号分区），包括：
   - P0/P1 修复状态、修复方式、修改文件
   - P2/P3 采纳/不采纳决策及理由
   - 修复摘要
4. **Monitor 闭环确认**：Monitor 读取 `worker-progress.md`，一次性确认各批次修复情况，在 `reviews/INDEX.md` 标记闭环
5. **灵活调整**：如果批次 1 发现重大问题，可在批次 2 中追加相关审查点

---

## 六、修订记录

| 日期 | 修订人 | 内容 |
|------|------|------|
| 2026-06-01 | GLM (Monitor) | 初始版本，基于现有 openspec 文档制定 |
