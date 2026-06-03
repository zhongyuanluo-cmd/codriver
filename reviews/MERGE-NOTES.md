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

### R-008 — Phase 2.1 coord_transform (2025-07-12)

**合并方式**: `git merge fix/R-008-v2 --no-ff`
**合并提交**: `93b391f`
**Worker 分支**: `fix/R-008-v2` @ `89b48f7`

**Monitor 合并时微调**: 无
- Worker 的 R-008 修复全部通过验证，代码原样合入
- P0/P1 四项修复均已验证：P0-1 未标定零输出+return -1、P1-1 detectDrift C API/FFI、P1-2 calibrate 返回 bool+量程检查、P1-3 移除陀螺仪死参数
- P2/P3 三项标注为可接受级别，随代码入主分支

**已知遗留**:
| 项 | 说明 | 后续处理 |
|:---:|------|------|
| P2-1 | 180° 反平行轴选取可简化 | 保留，当前实现正确且安全 |
| P2-2 | gravity_mag 硬编码 9.81 | P0-1 修复后默认路径不再使用，影响极小 |
| P3-2 | Dart 未检查 transform 返回值 | FFI 接口已就绪，app 层包装待后续 phase 处理 |

**合入文件** (7 files, +261 -26):
- `PHASE2-PLAN.md` — 新增 Phase 2 计划文档
- `shared_engine/include/codriver/coord_transform.h` — API 调整
- `shared_engine/src/coord_transform.cpp` — P0/P1 修复 + 逻辑完善
- `shared_engine/include/codriver/c_api.h` — +1 C API 函数
- `shared_engine/src/c_api.cpp` — detectDrift 桥接实现
- `app/lib/platform_bridge/engine_ffi.dart` — +1 FFI 绑定 (29/29)
- `shared_engine/tests/test_main.cpp` — 测试 2→5

**验证**: build ✅ | test 5/5 ✅ | dart analyze 0 error ✅

---

（R-004~R-007 的合并未做额外修改，Worker 产出原样合入）

---

### R-010/R-011/R-012 — Phase 2.3~2.5 闭环验证 (2026-06-02)

**合并方式**: `git merge fix/R-010-012 --no-ff`
**合并提交**: `b338d63`
**Worker 分支**: `fix/R-010-012` @ `9d9616d`

**Monitor 合并时微调**: 无
- Worker 的 R-010/R-011/R-012 修复全部通过闭环验证，代码原样合入
- P1-5 `c_best_lap_record_sector` 已添加，但 `c_best_lap_get_lap` 标记为 MVP 可接受缺失
- Monitor 审查后将 `getLap` 缺失列为 ITER-2 迭代项

**已知遗留**:
| 项 | 说明 | 后续处理 |
|:---:|------|------|
| ITER-1 | Pipeline 缺少 `finalize()` 方法 | 已在后续合并中修复 |
| ITER-2 | `c_best_lap_get_lap` C API 未暴露 | 已在后续合并中修复 |
| ITER-3 | P2-2/P2-3 注释补充 | 待后续迭代 |
| ITER-4 | P1-4 精确 entry_speed | 待后续迭代 |

---

### ITER-1/ITER-2 — Monitor 直接修复 (2026-06-03)

**合并方式**: `git merge fix/ITER-1-2 --no-ff`
**合并提交**: `a188ade` + merge commit
**修复分支**: `fix/ITER-1-2` @ `a188ade`
**修复人**: **Monitor (GLM)** — 本次为 Monitor 直接修改代码，非 Worker 修复

**⚠️ Monitor 直接修改说明**: ITER-1 和 ITER-2 属于审查遗留迭代项，修改范围小且逻辑明确，由 Monitor 直接实现并合并。Worker 需知晓以下代码变更。

### 修改内容

| 文件 | 修改 | 原因 |
|------|------|------|
| `analysis_pipeline.h` | +`bool finalize();` 声明 | ITER-1: session 结束时 flush 最后 pending 弯道 |
| `analysis_pipeline.cpp` | +65 行 `finalize()` 实现 | IN_CORNER 状态时产出 PipelineResult 并重置为 STRAIGHT |
| `c_api.h` | +`CLapRecord` struct, +`c_best_lap_get_lap()`, +`c_pipeline_finalize()` | ITER-1+ITER-2: C API 桥接 |
| `c_api.cpp` | +`c_pipeline_finalize()`, +`c_best_lap_get_lap()` 实现 | bridge 实现，含 null/range guard |
| `engine_ffi.dart` | +`CLapRecord` struct, +`bestLapGetLap()`, +`pipelineFinalize()` | ITER-1+ITER-2: FFI 绑定 (46→48) |
| `test_main.cpp` | +Test 13 (finalize), +getLap 测试 | 验证修复正确性 |

### 下游影响

- [x] `AnalysisPipeline` 现在有 `finalize()` 方法 — **Worker 后续 Phase 中，session/lap 结束时必须调用 `finalize()`**，否则最后一个弯道会丢失
- [x] `BestLapFinder` 的 `getLap(index)` 已通过 C API + FFI 完整暴露 — Worker 可在 Dart 层直接使用 `bestLapGetLap()`
- [x] FFI 绑定总数从 46→48，Pipeline 5/5，BestLap 6/6
- [ ] ITER-3 (P2-2/P2-3 注释补充) 和 ITER-4 (精确 entry_speed) 仍未修复，Worker 后续可处理

### Worker 注意事项

1. **`pipelineFinalize()` 必须调用**: 在 Flutter app 中，当圈结束（lap timer 触发）或 session 结束时，调用 `EngineFFI.pipelineFinalize(handle)` 来获取最后一个未完成的弯道分析结果
2. **`bestLapGetLap()` 可用**: 现在可以通过 `EngineFFI.bestLapGetLap(handle, index, out)` 获取任意圈的 `LapRecord`（圈号、圈时、距离、平均速度），不再只有 `getBest()` 一种获取方式
3. **`monitor-review.md` 迭代项追踪表已更新**: ITER-1/ITER-2 标记为 ✅ 已修复
