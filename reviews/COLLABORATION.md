# CoDriver Monitor / Worker 协作规范

> 最后更新: 2026-06-02
> 本文档定义 DeepSeek (Worker) 与 GLM (Monitor) 的分工、协作流程和文件约定
> v2: 新增 Git Branch 工作流、MERGE-NOTES 信息回流机制

---

## 一、角色定义

### Worker — DeepSeek

| 属性 | 说明 |
|------|------|
| **职责** | 撰写、修改、生成所有项目内容 |
| **产出** | 设计文档、代码、规格说明、计划、分析报告 |
| **工作模式** | 主动创作，按 roadmap 和 phase 推进 |
| **Git 权限** | 只能推送到 `fix/R-*` 或 `feat/*` 分支，**禁止直接推送 master** |
| **标记** | 文件头部标注 `Author: Worker (DeepSeek)` |

### Monitor — GLM

| 属性 | 说明 |
|------|------|
| **职责** | 审查、验证、质询 Worker 的产出 |
| **产出** | 审查报告、问题清单、改进建议、一致性检查 |
| **工作模式** | 被动触发，收到 Worker 产出后进行审查 |
| **Git 权限** | 唯一能合并到 master 的角色（owner + branch protection） |
| **标记** | 文件头部标注 `Reviewer: Monitor (GLM)` |

---

## 二、核心原则

1. **权限隔离**：Worker 永远无法 push 到 master，只有 Monitor 能合并。这是安全保障的基石
2. **审查留痕**：所有审查意见和修改决策必须记录在 `reviews/` 目录，不可口头协商不留记录
3. **问题分级**：审查发现的问题分为 P0~P3 四级，Worker 必须处理 P0/P1，P2/P3 可选
4. **闭环确认**：Worker 修改后，Monitor 需对修改结果进行二次确认，形成闭环
5. **信息回流**：Monitor 合并时如果修改了 Worker 的代码，必须记录在 `MERGE-NOTES.md`，确保 Worker 后续工作基于正确的代码状态

---

## 三、文件体系

```
赛道助手CoDriver/
├── codriver/                    ← Git 仓库（master 分支受 Branch Protection）
│   ├── shared_engine/           ← C++ 共享引擎
│   ├── app/                     ← Flutter App
│   ├── backend/                 ← Python FastAPI
│   └── docs/                    ← 开发文档
├── openspec/                    ← 项目规范文档（Worker 产出）
│   ├── project.md
│   ├── roadmap.md
│   ├── status.md
│   ├── config.yaml
│   ├── design/
│   └── changes/
└── reviews/                     ← 审查工作区（所有审查相关文件）
    ├── COLLABORATION.md         ← 本文件：协作规范
    ├── REVIEW-PROTOCOL.md       ← 审查流程与标准
    ├── REVIEW-PLAN.md           ← 基于当前 openspec 的审查计划
    ├── monitor-review.md        ← Monitor 审查结果统一记录（Monitor 维护）
    ├── worker-progress.md       ← Worker 修复进度统一记录（Worker 维护）
    ├── MERGE-NOTES.md           ← Monitor 合并修改记录（★ 信息回流关键文件）
    ├── onboarding-for-worker.md ← Worker 开场白
    └── INDEX.md                 ← 审查状态索引
```

---

## 四、审查流程（Git Branch 工作流）

### 完整一轮审查（R-NNN）的 7 个步骤

```
Step 1: Worker 完成产出
   │    代码：在 feat/phase-N 或 fix/R-NNN 分支上工作
   │    文档：直接修改（文档审查阶段无需分支）
   │
   ▼
Step 2: Monitor 审查
   │    代码：git fetch → git diff master..origin/fix/R-NNN
   │    文档：read_file 逐文件审查
   │    → 写 monitor-review.md#R-NNN
   │
   ▼
Step 3: Worker 修复
   │    代码：在 fix/R-NNN 分支上修改
   │    git commit -m "fix(R-NNN): 描述"
   │    git push -u origin fix/R-NNN
   │
   ▼
Step 4: Worker 记录修复
   │    → 写 worker-progress.md#R-NNN
   │
   ▼
Step 5: Monitor 闭环确认
   │    逐项 read_file 验证修复
   │    → 追加闭环确认到 monitor-review.md#R-NNN
   │
   ▼
Step 6: Monitor 合并（★ 关键步骤）
   │    git checkout master
   │    git merge --squash origin/fix/R-NNN
   │    # ★ 如需调整代码，此时修改
   │    git commit -m "fix(R-NNN): approved by Monitor"
   │    git push origin master
   │
   ▼
Step 7: 信息回流（★ 如有 Monitor 修改）
        → 写 MERGE-NOTES.md#R-NNN
        → 更新 INDEX.md 状态为 ✅
        → 通知 Worker
```

### 文档审查 vs 代码审查

| 阶段 | 审查对象 | Worker 工作位置 | Monitor 合并方式 | 需要分支？ |
|------|------|------|------|:---:|
| **文档审查** (R-001~R-003) | 设计文档、规格说明 | 项目目录直接修改 | 直接 commit master | ❌ |
| **代码审查** (R-004+) | 源代码实现 | `fix/R-*` 或 `feat/*` 分支 | `git merge --squash` | ✅ |

> 文档修改零风险，可在 master 上直接操作。代码修改有破坏性，**必须**用分支隔离。

### Monitor 合并时的调整流程

Monitor 在 `git merge --squash` 后、`git commit` 前，如果发现需要微调（参数调整、结构优化、补遗漏逻辑），执行以下步骤：

1. **修改代码**：直接编辑暂存区的文件
2. **提交**：`git commit -m "fix(R-NNN): approved by Monitor + [调整说明]"`
3. **写 MERGE-NOTES.md**：追加一个章节，记录：
   - 修改了什么（具体到文件和行）
   - 为什么修改
   - 对后续工作的影响（哪些模块/接口受影响）
   - Worker 下一步需要注意什么

### MERGE-NOTES.md 格式

```markdown
## R-NNN: [合并调整标题]

- **合并日期**: YYYY-MM-DD
- **原分支**: fix/R-NNN
- **Monitor**: GLM

### 修改内容

| 文件 | 修改位置 | 原值 | 新值 | 原因 |
|------|------|------|------|------|
| types.h | TrackSegment.difficulty | `int` | `int (1-5)` | 添加范围约束注释 |

### 下游影响

- [ ] Worker 需在后续 Phase 中使用新参数范围
- [ ] Python schemas.py 需同步更新

### Worker 注意事项

- TrackSegment.difficulty 现在有 1-5 的范围约束，请确保 FFI 层传递时做范围校验
```

---

## 五、问题分级标准

| 级别 | 定义 | 处理要求 | 示例 |
|:---:|------|------|------|
| **P0** | 严重错误，会导致项目方向偏离或核心功能失败 | **必须修复** | 技术选型自相矛盾、架构存在根本缺陷 |
| **P1** | 重要问题，影响可行性或质量 | **必须修复** | 数据模型不一致、缺失关键边界条件 |
| **P2** | 改进建议，可提升质量但不影响核心功能 | 建议修复，记录决策 | 命名不统一、文档表述可优化 |
| **P3** | 微小问题或风格偏好 | 可选 | 排版、措辞、注释风格 |

---

## 六、审查维度

Monitor 审查时，需覆盖以下六个维度：

| 维度 | 关注点 |
|------|------|
| **1. 一致性** | 文档间引用是否一致？术语是否统一？数据模型是否前后对应？ |
| **2. 完整性** | 是否有遗漏的场景、边界条件、错误处理？需求是否全覆盖？ |
| **3. 可行性** | 技术方案是否可实现？工作量估算是否合理？依赖是否真实存在？ |
| **4. 准确性** | 技术描述是否正确？数据/公式/引用是否准确？ |
| **5. 安全性** | 是否存在安全风险？数据隐私是否合规？ |
| **6. 清晰性** | 表述是否明确无歧义？读者能否准确理解意图？ |

---

## 七、信息回流机制

### 问题：Monitor 修改了 Worker 的代码，Worker 怎么知道？

LLM Agent 和人类开发者不同——人类会习惯性 `git pull` + 看差异，但 LLM Agent 的上下文是每次对话从零开始的，不会自动感知 master 上的变化。

### 解决方案：MERGE-NOTES.md + Worker 开工检查清单

**MERGE-NOTES.md** 是 Monitor → Worker 的单向信息回流通道。Monitor 每次合并时如果做了调整，必须在此记录。

**Worker 开工检查清单**（每次开始新一轮工作前执行）：

```
1. git pull origin master             ← 同步最新代码
2. 阅读 MERGE-NOTES.md 最新章节       ← 了解 Monitor 的修改
3. 阅读 INDEX.md                      ← 了解当前审查状态
4. 阅读最新 monitor-review.md 章节    ← 了解待处理问题
5. 阅读 worker-progress.md 上次修复   ← 了解自己的修复记录（避免重复）
6. 开始工作
```

### 信息流总览

```
Worker 产出 ──→ Monitor 审查 ──→ Worker 修复 ──→ Monitor 闭环
     │                                              │
     │              ┌────────────────┐              │
     │              │  MERGE-NOTES   │◄─────────────┘
     │              │  (Monitor 写)   │   合并时如有调整
     │              └───────┬────────┘
     │                      │
     │  ◄───────────────────┘
     │  Worker 开工时阅读
     │
     ▼
  下一轮工作
```

---

## 八、沟通约定

1. **审查触发**：Worker 完成阶段性产出后，主动声明需要审查的范围和文件
2. **审查时限**：Monitor 在收到审查请求后，应在同一次会话内完成审查
3. **争议处理**：Worker 对 P0/P1 审查意见有异议时，需在审查报告回复区阐述理由，Monitor 重新评估
4. **版本标注**：每次修改后，文件头部的更新日期必须更新

---

## 九、与 OpenSpec 的关系

- OpenSpec 的 `changes/` 目录由 Worker 维护，Monitor 审查
- 审查意见统一放入 `reviews/` 目录，不侵入 OpenSpec 体系
- Worker 的修复记录统一放入 `worker-progress.md`，方便 Monitor 一次性确认
- `status.md` 中可引用审查报告编号，如 `Phase 0 — 审查中 (R-001)`
- Monitor 的审查结论不直接修改 `status.md`，由 Worker 根据闭环结果更新

---

## 十、Git Branch Protection 配置

### 当前配置（zhongyuanluo-cmd/codriver）

| 规则 | 值 | 说明 |
|------|------|------|
| Require PR before merge | ✅ | 合并到 master 必须通过 PR |
| Required approvals | 1 | 需要 1 人批准 |
| Dismiss stale reviews | ✅ | 新 push 后旧 review 自动失效 |
| Allow force pushes | ❌ | 禁止 force push |
| Allow deletions | ❌ | 禁止删除分支 |
| Enforce admins | ❌ | Owner 可直接 push（Monitor 就是 owner） |

### 分支命名约定

```
fix/R-NNN           ← Worker 审查修复分支（审查驱动）
feat/phase-N        ← Worker 功能开发分支（路线图驱动）
```

### Worker Git 操作速查

```bash
# 创建分支
git checkout -b feat/phase-1

# 工作中
git add -A
git commit -m "feat(phase-1): 描述"

# 推送
git push -u origin feat/phase-1

# 同步 master 最新变化
git fetch origin
git merge origin/master
```

### Monitor Git 操作速查

```bash
# 审查分支差异
git fetch origin
git diff master..origin/fix/R-005

# 合并（squash merge 保持历史干净）
git checkout master
git merge --squash origin/fix/R-005
# ★ 如需调整代码，此时编辑
git commit -m "fix(R-005): approved by Monitor [+ 调整说明]"
git push origin master

# 清理分支
git branch -d fix/R-005
git push origin --delete fix/R-005
```

---

## 十一、修订记录

| 日期 | 修订人 | 内容 |
|------|------|------|
| 2026-06-02 | GLM (Monitor) | v2: 新增 Git Branch 工作流、MERGE-NOTES 信息回流机制、Branch Protection 配置 |
| 2026-06-01 | GLM (Monitor) | 初始版本 |
