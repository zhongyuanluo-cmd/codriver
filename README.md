# CoDriver - AI 赛道驾驶教练

基于手机 App 的 AI 赛道驾驶教练。C++ 引擎实时分析驾驶表现，自然语言对话式赛后复盘。

## 项目结构

| 目录 | 说明 |
|------|------|
| `app/` | Flutter 移动端 (iOS + Android) |
| `shared_engine/` | C++ 共享引擎 (手机 + 车机复用) |
| `backend/` | Python 后端 (FastAPI + Supabase) |

## 快速开始

```bash
# Python 后端
cd backend && pip install -r requirements.txt && uvicorn app.main:app --reload

# C++ 引擎
cd shared_engine && cmake -B build && cmake --build build && ctest --test-dir build

# Flutter App (需安装 Flutter SDK)
cd app && flutter pub get && flutter run
```

## 文档

完整设计文档见 `../openspec/`

## 状态

🟡 Phase 0 — 项目骨架初始化
