# Monitor 合并修改记录

> Reviewer: Monitor (GLM)
> 本文档记录 Monitor 在合并 Worker 产出时所做的额外修改
> Worker 在开始新一轮工作前**必须阅读本文档最新章节**，了解 master 上的实际代码状态

---

## 使用说明

1. **Monitor**：每次 `git merge --squash` 后，如果对 Worker 代码做了调整（改参数、调结构、补逻辑等），必须在此追加一个章节
2. **Worker**：开始新工作前，执行以下"开工检查"：
   - `git pull origin master` — 同步最新代码
   - 阅读本文件最新章节 — 了解 Monitor 的修改
   - 阅读 `INDEX.md` — 了解当前审查状态
   - 阅读最新 `monitor-review.md` 章节 — 了解待处理问题

---

## 合并记录

（暂无 — R-004 的合并未做额外修改，Worker 产出原样合入）
