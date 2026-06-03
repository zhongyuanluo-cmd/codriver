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

---

### R-013/R-014 — Phase 2.6~2.7 闭环验证 (2026-06-03)

**合并方式**: `git merge fix/R-013-014 --no-ff`
**合并提交**: merge commit (parent: master @ b534b16)
**修复分支**: `fix/R-013-014` @ `0be356a`
**Worker 分支**: `fix/R-013-014` @ `20a74b6` (fix) + Monitor @ `0be356a` (closure docs)
**审查来源**: `monitor-review.md` R-013 / R-014 章节

**Monitor 合并时微调**: 无
- Worker 修复全部通过闭环验证，代码原样合入
- Monitor 仅追加了 `monitor-review.md` 闭环确认章节（commit `0be356a`）

**本次合入内容**:

Phase 2.6 (Flutter 可视化) + Phase 2.7 (Backend API) + R-013/R-014 修复，3 个分支一次合入：

| 来源分支 | 描述 | commit |
|----------|------|--------|
| `feat/phase-2.6-flutter-viz` | Flutter fl_chart 速度曲线 + AnalysisScreen | `ade30ae` |
| `feat/phase-2.7-backend-api` | FastAPI analysis endpoint + schemas | `812b2a8` |
| `fix/R-013-014` | R-013 弯道高亮 + R-014 router/schema 修复 | `20a74b6` → `0be356a` |

**R-013 修复 (Flutter)**:

| # | 修复 | 文件 |
|---|------|------|
| P0-1 | `SpeedChart._buildCornerLines()` + `extraLinesData` 渲染弯道橙色虚线标注 | `speed_chart.dart` |
| P1-1 TODO | SpeedPoint/CornerZone 标注 Phase 3 替换 | `speed_chart.dart` |
| P1-2 TODO | analysis_screen 标注 Phase 3 API 对接 | `analysis_screen.dart` |

**R-014 修复 (Backend)**:

| # | 修复 | 文件 |
|---|------|------|
| P0-1 | Router prefix `/api/sessions` → `/api/analysis`，端点 → `/sessions/{session_id}` | `analysis.py` |
| P1-1 | LapRecord 恢复 `is_valid: bool = True` + `is_personal_best: bool = False` | `schemas.py` |
| P1-2 | CornerAnalysis +`brake_distance_m`, `brake_peak_g`, `speed_drop_kmh`, `feedback_tier` | `schemas.py` |
| P1-3 | 移除 `if not session_id` 死代码，替换为 404 TODO | `analysis.py` |

**已知遗留**:
| 项 | 说明 | 后续处理 |
|:---:|------|------|
| P1-4 | 3 个 stub 端点仍返回 200 (应 501) | Phase 3 API 实现时统一升级 |
| Mock | `_mock_corner_analysis()` 未填充新制动字段 | Phase 3 FFI 集成时替换 |
| P1-1 TODO | SpeedPoint/CornerZone 需替换为 API 模型 | Phase 3 API 对接 |
| P1-2 TODO | Mock 数据需替换为 Backend API 调用 | Phase 3 API 对接 |

**合入文件** (9 files, +1359 -131):
- `app/lib/main.dart` — AnalysisScreen 替换 placeholder
- `app/lib/ui/screens/analysis_screen.dart` — 新增 (174 行)
- `app/lib/ui/widgets/speed_chart.dart` — 新增 (181 行)
- `app/pubspec.lock` — fl_chart/riverpod 依赖
- `app/pubspec.yaml` — riverpod 修正
- `backend/app/api/analysis.py` — 新增 (135 行)
- `backend/app/main.py` — 注册 analysis router
- `backend/app/models/schemas.py` — +CornerAnalysis/LapAnalysis 等 schemas
- `reviews/monitor-review.md` — R-013/R-014 审查 + 闭环确认

### Worker 注意事项

1. **`SpeedChart` 现在支持弯道标注**: `corners` 参数已被使用（P0-1 修复），传入 `List<CornerZone>` 即可渲染橙色虚线 + 标签
2. **Backend analysis 端点路径已变更**: `GET /api/analysis/sessions/{session_id}`（非旧的 `/api/sessions/{session_id}/analyze`）
3. **`LapRecord` 已恢复 `is_valid` / `is_personal_best`**: Flutter 侧可使用这两个字段过滤无效圈/标记最快圈
4. **`CornerAnalysis` 新增 4 个制动字段**: Phase 3 FFI 映射时使用 `brake_distance_m`, `brake_peak_g`, `speed_drop_kmh`, `feedback_tier`
5. **所有 TODO(Phase 3) 标注**: Flutter 侧 SpeedPoint/CornerZone/mock 数据均有 Phase 3 替换标注，Phase 3 API 对接时统一处理
