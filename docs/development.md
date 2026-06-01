# CoDriver 开发环境搭建指南

## 前置条件

| 工具 | 版本要求 | Windows 安装 |
|------|------|------|
| Git | 2.x+ | 已安装 ✅ |
| CMake | 3.20+ | 已安装 ✅ (D:\Program Files\CMake) |
| MSVC (C++ 编译器) | VS2017+ | 已安装 ✅ (VS2017 Community) |
| Python | 3.12+ | 已安装 ✅ (3.11.9) |
| Flutter | 3.x | ⬜ [下载安装](https://docs.flutter.dev/get-started/install/windows) |

> **注意**: 不需要 MinGW。C++ 引擎用 VS2017 MSVC 编译，CI 上的 Linux 构建由 GitHub Actions Ubuntu 自带 gcc 完成。

## 快速开始

```powershell
# 克隆仓库
git clone https://github.com/zhongyuanluo-cmd/codriver.git
cd codriver
```

### 1. Python 后端

```powershell
cd backend
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
uvicorn app.main:app --reload
# 访问 http://localhost:8000/api/health
```

### 2. C++ 共享引擎

```powershell
cd shared_engine
# 配置 (VS2017)
cmake -B build -G "Visual Studio 15 2017" -A x64
# 编译
cmake --build build --config Release
# 测试
ctest --test-dir build -C Release --output-on-failure
```

### 3. Flutter App (需先安装 Flutter SDK)

```powershell
cd app
flutter pub get
flutter run
# 或在模拟器上运行:
# flutter run -d windows  (Windows 桌面)
# flutter run -d android  (Android 模拟器)
```

## 目录结构

```
codriver/
├── app/                    # Flutter 移动端 (iOS + Android)
│   ├── lib/
│   │   ├── main.dart
│   │   └── platform_bridge/  # Platform Plugin 桥接层
│   └── pubspec.yaml
├── shared_engine/          # C++ 共享引擎 (手机 + 车机复用)
│   ├── CMakeLists.txt
│   ├── include/codriver/   # 7 个头文件
│   └── src/                # 5 个实现文件
├── backend/                # Python FastAPI 后端
│   ├── app/
│   │   ├── main.py         # 入口
│   │   ├── api/            # coach / sessions / tracks
│   │   └── llm/            # LLM 调用 (instructor + httpx)
│   └── requirements.txt
├── docs/                   # 文档
├── openspec/               # 项目规范
└── .github/workflows/      # CI/CD
```

## 常用命令

```powershell
# C++ 引擎: 清理重建
cd shared_engine && Remove-Item -Recurse -Force build && cmake -B build -G "Visual Studio 15 2017" -A x64 && cmake --build build --config Release

# Python: 类型检查 + lint
cd backend && mypy app/ && ruff check .

# 全量测试
cd shared_engine && ctest --test-dir build -C Release
cd backend && pytest
```

## 环境变量

复制 `backend/.env.example` 为 `backend/.env`，填写实际值：

```env
SUPABASE_URL=https://xxxxx.supabase.co
SUPABASE_ANON_KEY=eyJ...
QWEN_API_KEY=sk-...
LLAMA_API_KEY=...
```

> MVP 阶段这些可选——后端 `main.py` 的 `/api/health` 不需要任何外部服务即可运行。
