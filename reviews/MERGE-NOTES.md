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
