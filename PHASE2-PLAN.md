# Phase 2: 核心分析引擎 — 执行计划

> Worker: DeepSeek | 分支: feat/phase-2-core-analysis | 日期: 2026-06-02
> 上游: Phase 1 (EKF + corner_detector + root_cause + coach_template + LapTimer + FFI)
> 审查: Monitor (GLM) 待审核

---

## 1. 目标

实现完整的赛道分析 Pipeline，为 AI 教练提供精准的驾驶反馈基础。

## 2. 交付物

| # | 模块 | 描述 | 优先级 | 预估 |
|:---:|------|------|:---:|:---:|
| 2.1 | coord_transform 实现 | 手机→车身坐标系旋转（四元数标定 + 旋转矩阵） | P0 | ✅ 已完成 |
| 2.2 | 制动点检测 | 减速度峰值检测、制动点定位、与参考圈对比 | P0 | 待实现 |
| 2.3 | 弯速对比分析 | 当前圈 vs 参考圈各弯道速度对比 | P0 | 待实现 |
| 2.4 | 圈时差归因 Pipeline | EKF → corner_detector → root_cause → coach_template 联调 | P0 | 待实现 |
| 2.5 | 最快圈检测 | 多圈 LapTimer 数据中识别最快圈 | P1 | 待实现 |
| 2.6 | 速度曲线可视化 | Flutter fl_chart 展示当前圈 vs 参考圈速度曲线 | P1 | 待实现 |
| 2.7 | 后端分析 API | /api/sessions/{id}/analyze 编排分析 Pipeline | P1 | 待实现 |
| 2.8 | Session 统计摘要 | 圈数、最快圈、平均速度、一致性评分 | P2 | 待实现 |
| 2.9 | 测试覆盖 (L-10) | c_api/LapTimer/coord_transform 单元测试 | P2 | 待实现 |

## 3. 遗留项追踪

| 编号 | 内容 | 来源 | 本次处理 |
|:---:|------|:---:|:---:|
| L-5 | coord_transform calibrate/transform TODO | R-006 | ✅ 2.1 |
| L-10 | 测试覆盖不足（仅 2/2） | R-007 | 2.9 |

## 4. 执行顺序

```
2.1 coord_transform ──→ 2.2 制动点检测 ──→ 2.3 弯速对比
                              │                    │
                              └──→ 2.4 Pipeline 联调 ←──┘
                                       │
                        ┌──────────────┼──────────────┐
                        ↓              ↓              ↓
                   2.5 最快圈    2.7 后端 API    2.6 Flutter 可视化
                                       │
                                       ↓
                                  2.8 Session 摘要
                                       │
                                       ↓
                                  2.9 测试覆盖
```

## 5. 验收标准

- [ ] coord_transform 标定后可正确转换加速度到车身坐标系
- [ ] 制动点检测准确率 >80%（vs 模拟器参考数据）
- [ ] Pipeline 端到端延迟 < 圈后 3s
- [ ] 所有分析结果在 App AnalysisScreen 可视化
- [ ] 编译: MSVC Release 0 错误 | 测试: 新增 ≥4 个 | Dart analyze: 0 errors

## 6. 变更文件清单

| 文件 | 操作 | 说明 |
|------|:---:|------|
| `shared_engine/src/coord_transform.cpp` | 修改 | calibrate/transform 实现 |
| `shared_engine/include/codriver/coord_transform.h` | 可能修改 | 如需新增 API |
| `shared_engine/include/codriver/c_api.h` | 修改 | 新增 coord_transform C API |
| `shared_engine/src/c_api.cpp` | 修改 | 新增 coord_transform C API 实现 |
| `shared_engine/tests/test_main.cpp` | 修改 | 新增测试 |
| `app/lib/platform_bridge/engine_ffi.dart` | 修改 | 新增 coord_transform FFI 绑定 |
| 后续模块文件 | 新增 | 制动检测/弯速对比等 |
