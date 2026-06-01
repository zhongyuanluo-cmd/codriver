# CoDriver 项目 Monitor → Worker 协作开场白

---

你好，我是 **Monitor (GLM)**，你的审查搭档。我们已经建立了协作框架，现在正式开工。

## 我们是谁

| 角色 | 模型 | 职责 |
|------|------|------|
| **Worker** | DeepSeek | 撰写、修改、生成所有项目内容 |
| **Monitor** | GLM | 审查、验证、质询 Worker 的产出 |

**核心原则：职责不交叉。** 我不直接改你的文件，只出审查意见；你根据我的意见决定是否及如何修改。（合并时如需微调，我会记录在 MERGE-NOTES.md）

---

## 文件体系（各管各的）

```
赛道助手CoDriver/
├── openspec/                 ← 🟢 Worker 修改的项目文档
│   ├── project.md
│   ├── roadmap.md
│   └── ...
└── reviews/                  ← 审查工作区（所有审查相关文件）
    ├── COLLABORATION.md      ← 协作规范（公共）
    ├── REVIEW-PROTOCOL.md    ← 审查流程与标准（公共）
    ├── REVIEW-PLAN.md        ← 审查计划（公共）
    ├── monitor-review.md     ← 🟢 Monitor 的地盘：审查结果统一记录
    ├── worker-progress.md    ← 🔵 Worker 的地盘：修复进度统一记录
    ├── MERGE-NOTES.md        ← 🟢 Monitor 的地盘：合并调整记录
    ├── onboarding-for-worker.md  ← 本文件
    └── INDEX.md              ← 审查状态索引（Monitor 维护）
```

**规则很简单**：
- 🟢 **我写** `monitor-review.md` — 审查发现、评分、建议
- 🔵 **你写** `worker-progress.md` — 修复状态、修复方式、决策记录
- 🟢 **我写** `MERGE-NOTES.md` — 合并时如有代码调整，记录修改内容和下游影响
- 🟢 **你改** `openspec/` 下的项目文档 — 根据我的审查意见修改
- 🔵 **我不动** 你的任何文件，你也别动我的 `monitor-review.md` 和 `MERGE-NOTES.md`

---

## 工作流程（Git Branch 工作流）

```
Step 1: Worker 完成产出
   │    文档审查：直接修改 openspec/ 文件
   │    代码审查：在 fix/R-NNN 分支上工作
   │
   ▼
Step 2: Monitor 审查 → 写入 monitor-review.md（问题清单 P0~P3）
   │
   ▼
Step 3: Worker 读 monitor-review.md → 修复
   │    文档：直接修改 openspec/ 文件
   │    代码：在 fix/R-NNN 分支上修改 → git push
   │
   ▼
Step 4: Worker 写 worker-progress.md → 记录修了什么、怎么修的
   │
   ▼
Step 5: Monitor 闭环确认 → 更新 monitor-review.md
   │
   ▼
Step 6: Monitor 合并（代码审查阶段）
   │    git merge --squash origin/fix/R-NNN
   │    ★ 如有调整，修改暂存区代码
   │
   ▼
Step 7: 信息回流（如有合并调整）
        → Monitor 写 MERGE-NOTES.md
        → 更新 INDEX.md 为 ✅
        → 通知 Worker
```

---

## Worker 开工检查清单

**每次开始新一轮工作前，按顺序执行：**

```
1. git pull origin master             ← 同步最新代码
2. 阅读 MERGE-NOTES.md 最新章节       ← 了解 Monitor 合并时的调整
3. 阅读 INDEX.md                      ← 了解当前审查状态
4. 阅读最新 monitor-review.md 章节    ← 了解待处理问题
5. 阅读 worker-progress.md 上次修复   ← 了解自己的修复记录（避免重复）
6. 开始工作
```

> **为什么有这个清单？** 因为 LLM Agent 的上下文每次从零开始，不会自动感知 master 上的变化。MERGE-NOTES.md 是 Monitor → Worker 的信息回流通道，你必须在开工前阅读。

---

## Worker Git 操作速查

```bash
# 创建修复分支（代码审查阶段使用）
git checkout -b fix/R-NNN

# 在分支上工作
git add -A
git commit -m "fix(R-NNN): 描述"

# 推送分支
git push -u origin fix/R-NNN

# 同步 master 最新变化
git fetch origin
git merge origin/master
```

> 文档审查阶段（R-001~R-003）不需要分支，直接修改即可。代码审查阶段（R-004+）**必须**使用分支。

---

## 问题分级（务必注意）

| 级别 | 含义 | 你的义务 |
|:---:|------|------|
| **P0** | 严重问题，会导致实现歧义或无法落地 | **必须修复**，不可跳过 |
| **P1** | 重要问题，影响功能正确性或一致性 | **必须修复**，不可跳过 |
| **P2** | 改进建议，提升文档质量 | 建议修复，**可跳过但必须记录理由** |
| **P3** | 微小问题，措辞/格式/细节 | 可选修复，**可跳过但必须记录理由** |

**P2/P3 跳过不丢人，但不记录理由就是违规。** 在 `worker-progress.md` 的 P2/P3 决策记录表中写清"不采纳"及理由即可。

---

## 当前任务：R-001 项目基础层

### 审查概况

- **审查对象**：5 个核心设计文档（project.md, tech-stack.md, analysis-framework.md, data-structures.md, app-feature-spec.md）
- **审查结论**：⚠️ 修改后通过
- **发现问题**：3 个 P0 + 9 个 P1 + 7 个 P2 + 5 个 P3

### P0 严重问题（必须修复）

| # | 问题 | 涉及文件 |
|:---:|------|------|
| P0-1 | **反馈层级定义不一致**：analysis-framework 定义"三层反馈"，app-feature-spec 定义"四级信息体系"，tech-stack 有 5 个场景。需统一为一套体系 | analysis-framework.md, app-feature-spec.md, tech-stack.md |
| P0-2 | **油门评分+评分子维度数据缺失**：analysis-framework 定义了 22 个评分子维度，data-structures 只存了 3 个总分，油门评分字段完全缺失 | data-structures.md |
| P0-3 | **C++ 共享引擎构建方案缺失**：跨平台构建、包管理、FFI/EventChannel 分工全未说明 | tech-stack.md |

### P1 重要问题（必须修复）

| # | 问题 | 涉及文件 |
|:---:|------|------|
| P1-1 | 手机→车身坐标转换策略未说明 | data-structures.md / tech-stack.md |
| P1-2 | GPS 3-5m 精度与走线评分 1m 标准矛盾 | analysis-framework.md |
| P1-3 | 竞品对比缺少传感器精度等关键维度 | project.md |
| P1-4 | 二进制格式缺 altitude_m 字段 | data-structures.md |
| P1-5 | 车辆管理数据模型（Car/CarModification）完全缺失 | data-structures.md |
| P1-6 | TrackSegment 参考数据不可空，新赛道无法使用 | data-structures.md |
| P1-7 | 实时 TTS 延迟方案缺失（<200ms 目标无实现路径） | tech-stack.md |
| P1-8 | L5 水平定义缺失 | analysis-framework.md |
| P1-9 | BrakeEvent 缺制动释放指标 brake_release_duration_ms | data-structures.md |

### 你的操作步骤

1. **阅读** `monitor-review.md` 的 R-001 章节，了解每个问题的详细描述和建议
2. **修改** `openspec/` 下对应的文档（P0 先修，P1 次之，P2/P3 自行判断）
3. **更新** `worker-progress.md` 的 R-001 章节：
   - P0/P1 修复记录：逐条填写修复状态、修复方式、修改文件
   - P2/P3 决策记录：采纳写修复方式，不采纳写理由
   - 修复摘要：整体说明改了什么
4. **通知我**修复完成，我会读取 `worker-progress.md` 进行闭环确认

---

## 注意事项

1. **修改 openspec 文档时，保持文档自身结构和风格的一致性**，不要因为修复某个问题而破坏文档整体
2. **P0-1 反馈层级统一**是最高优先级，建议先修这个，因为其他文档的术语要跟着变
3. **遇到审查意见你不认同的，可以在 worker-progress.md 中写明理由**，有理有据的不采纳完全 OK，我们会讨论
4. **所有修改记录必须留痕**，不要在聊天中说"我改了"就完事，要写进 worker-progress.md

---

开工吧。先读 `monitor-review.md` 的 R-001 部分，从 P0-1 开始修。
