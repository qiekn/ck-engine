# C# 脚本引擎 — 嵌入 Mono 运行时

对应 Hazel commits:
- [`b22a755`](https://github.com/TheCherno/Hazel/commit/b22a75502769fbc213318a74dea4fc93e6e7fc8b) — 初始 Mono 嵌入
- [`acfaac5`](https://github.com/TheCherno/Hazel/commit/acfaac5c1752499ce3162e46d95124adc2bd8aab) — Internal Calls + ScriptClass 重构

## 为什么需要脚本系统

游戏引擎的核心用 C++ 编写以获取性能，但游戏逻辑（角色行为、关卡规则等）需要快速迭代。C# 作为脚本语言的优势：

- 编译速度快，支持热重载
- 有垃圾回收，不用手动管理内存
- 生态成熟（Unity 也用 C#）

## Mono 是什么

[Mono](https://www.mono-project.com/) 是 .NET Framework 的开源实现，可以被嵌入到 C++ 应用中。Unity 引擎就是用 Mono 来运行 C# 代码的。

Mono 提供了一套 C API，让 C++ 宿主程序可以：

- 初始化 JIT 运行时
- 加载 C# 编译后的 DLL（assembly）
- 查找类和方法
- 创建对象、调用方法、传递参数

## 核心概念

### Domain

Mono 中有两层 Domain：

- **Root Domain** — `mono_jit_init()` 创建，是 JIT 编译器的入口，整个进程生命周期只有一个
- **App Domain** — `mono_domain_create_appdomain()` 创建，是隔离的运行环境。后续可以通过卸载/重建 App Domain 实现 C# 代码的热重载

### Assembly 加载流程

```
读取 DLL 文件到内存 (ReadBytes)
    → mono_image_open_from_data_full()  // 解析 PE image
    → mono_assembly_load_from_full()    // 加载 assembly
    → mono_assembly_get_image()         // 获取 image 引用
```

### 对象创建和方法调用

```cpp
// 查找类
MonoClass* klass = mono_class_from_name(image, "Namespace", "ClassName");

// 创建实例 + 调用构造函数
MonoObject* instance = mono_object_new(app_domain, klass);
mono_runtime_object_init(instance);

// 查找方法（第三个参数是参数个数）
MonoMethod* method = mono_class_get_method_from_name(klass, "MethodName", 0);

// 调用方法
mono_runtime_invoke(method, instance, nullptr, nullptr);
```

### 传递参数

参数通过 `void*` 数组传递，值类型传指针，引用类型（如 string）需要用 Mono API 创建：

```cpp
// 传 int
int value = 42;
void* args[] = { &value };
mono_runtime_invoke(method, instance, args, nullptr);

// 传 string
MonoString* str = mono_string_new(app_domain, "hello");
void* args[] = { str };
mono_runtime_invoke(method, instance, args, nullptr);
```

## 项目中的实现

### 文件结构

```
src/engine/scripting/
├── script_engine.h      // ScriptEngine + ScriptClass 声明
├── script_engine.cpp    // Mono 初始化、assembly 加载、ScriptClass 实现
├── script_glue.h        // ScriptGlue 声明
└── script_glue.cpp      // Internal Call 注册和 C++ 实现

CK-ScriptCore/
└── Source/Main.cs       // C# 侧：Entity 类、Vector3 结构体、InternalCalls

deps/mono/               // Vendored Mono 运行时
├── include/             // C 头文件（跨平台通用）
├── lib/
│   ├── libmonosgen-2.0.dll.a   // MinGW 导入库
│   ├── monosgen-2.0.lib        // MSVC 导入库
│   └── mono/4.5/mscorlib.dll   // .NET 核心运行时 assembly
└── bin/
    └── libmonosgen-2.0.dll     // 运行时 DLL
```

### 生命周期

```
Application 构造函数
  → ScriptEngine::Init()
    → InitMono()
      → mono_set_assemblies_path()
      → mono_jit_init()
    → LoadAssembly()
      → mono_domain_create_appdomain()
      → utils::LoadMonoAssembly()
      → mono_assembly_get_image()
    → ScriptGlue::RegisterFunctions()  // 注册 Internal Calls
    → ScriptClass("CK", "Entity")      // 查找 C# 类
    → ScriptClass::Instantiate()        // 创建实例

Application 析构函数
  → ScriptEngine::Shutdown()
    → ShutdownMono()
```

### 跨编译器支持

Mono 的 API 是纯 C 接口，DLL 本身可以跨编译器使用。但链接时需要的「导入库」格式不同：

| 编译器 | 导入库格式 | 文件 |
|--------|-----------|------|
| MinGW (GCC/Clang) | `.dll.a` | `libmonosgen-2.0.dll.a` |
| MSVC | `.lib` | `monosgen-2.0.lib` |

CMakeLists.txt 中通过 `if(MSVC)` 自动选择正确的导入库。MSVC 的 `.lib` 是从 DLL 用 `gendef` + `dlltool` 生成的。

### 平台差异

| 平台 | Mono 来源 | 系统依赖库 |
|------|----------|-----------|
| Windows | Vendored in `deps/mono/` | `ws2_32` `winmm` `version` `bcrypt` |
| Linux | `find_library(monosgen-2.0)` | `pthread` `dl` `m` |
| macOS | `find_library(monosgen-2.0)` | `pthread` |

Windows 上这四个系统库是 Mono 运行时的间接依赖：ws2_32（网络）、winmm（多媒体定时器）、version（系统版本查询）、bcrypt（加密/哈希）。

## 编译 C# DLL

```bash
# 用 Mono 编译器 (mcs) 编译
mono mcs.exe -target:library -out:assets/scripts/CK-ScriptCore.dll CK-ScriptCore/Source/Main.cs
```

## Internal Calls（C# 调用 C++）

Internal Call 是 Mono 提供的机制，允许 C# 代码直接调用 C++ 函数。这是游戏引擎脚本系统的核心——C# 脚本通过 Internal Call 访问引擎底层功能（获取位置、设置物理属性等）。

### 原理

C# 侧用 `[MethodImpl(MethodImplOptions.InternalCall)]` 标记一个 `extern` 方法，表示它的实现在运行时（宿主程序）中：

```csharp
public static class InternalCalls
{
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    internal extern static void NativeLog(string text, int parameter);
}
```

C++ 侧用 `mono_add_internal_call()` 注册对应的函数实现：

```cpp
static void NativeLog(MonoString* string, int parameter) {
    char* c_str = mono_string_to_utf8(string);
    std::string str(c_str);
    mono_free(c_str);
    std::cout << str << ", " << parameter << std::endl;
}

mono_add_internal_call("CK.InternalCalls::NativeLog", reinterpret_cast<const void*>(NativeLog));
```

关键点：
- 注册名格式为 `"命名空间.类名::方法名"`
- C# 的 `string` 对应 C++ 的 `MonoString*`，需要用 `mono_string_to_utf8()` 转换，转换后要 `mono_free()` 释放
- C# 的 `ref` 和 `out` 参数对应 C++ 的指针
- C++ 函数指针需要 `reinterpret_cast<const void*>` 转换（Clang 要求显式转换）

### 类型映射

| C# 类型 | C++ 类型 | 说明 |
|---------|---------|------|
| `int` | `int` | 直接对应 |
| `float` | `float` | 直接对应 |
| `string` | `MonoString*` | 需要 `mono_string_to_utf8()` 转换 |
| `ref Vector3` | `glm::vec3*` | 值类型按指针传递 |
| `out Vector3` | `glm::vec3*` | 输出参数也是指针 |

### ScriptGlue

`ScriptGlue` 类集中管理所有 Internal Call 的注册，通过宏简化注册代码：

```cpp
#define CK_ADD_INTERNAL_CALL(Name) \
    mono_add_internal_call("CK.InternalCalls::" #Name, reinterpret_cast<const void*>(Name))

void ScriptGlue::RegisterFunctions() {
    CK_ADD_INTERNAL_CALL(NativeLog);
    CK_ADD_INTERNAL_CALL(NativeLog_Vector);
    CK_ADD_INTERNAL_CALL(NativeLog_VectorDot);
}
```

`#Name` 是字符串化操作符，将 C++ 函数名转换为字符串，确保 C++ 函数名和 C# 方法名一致。

## ScriptClass 封装

`ScriptClass` 将 Mono 原始 API 封装为面向对象的接口：

```cpp
class ScriptClass {
public:
    ScriptClass(const std::string& class_namespace, const std::string& class_name);
    MonoObject* Instantiate();
    MonoMethod* GetMethod(const std::string& name, int parameter_count);
    MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
private:
    std::string class_namespace_;
    std::string class_name_;
    MonoClass* mono_class_ = nullptr;
};
```

使用示例：

```cpp
ScriptClass entity_class("CK", "Entity");
MonoObject* instance = entity_class.Instantiate();
MonoMethod* method = entity_class.GetMethod("PrintMessage", 0);
entity_class.InvokeMethod(instance, method);
```

相比直接调用 `mono_class_from_name` → `mono_object_new` → `mono_runtime_object_init` → `mono_class_get_method_from_name` → `mono_runtime_invoke` 的原始 API 链，ScriptClass 提供了更简洁的接口。后续会用它来管理每个 C# 脚本组件的类信息。

## ScriptEngine 重构

`Init()` 的职责被拆分：

| 方法 | 职责 |
|------|------|
| `InitMono()` | 设置 assembly 搜索路径、初始化 JIT root domain |
| `LoadAssembly()` | 创建 app domain、加载 DLL、获取 image |
| `InstantiateClass()` | 从 MonoClass 创建实例（供 ScriptClass 调用） |

工具函数被移到 `utils` 命名空间中：`ReadBytes`、`LoadMonoAssembly`、`PrintAssemblyTypes`。这让顶层的 ScriptEngine 方法更加清晰。
