# PipeNc - Windows Named Pipe Netcat-like Tool

PipeNc 是一个类似于 netcat 的工具，专门用于 Windows 命名管道通信。它支持服务器和客户端模式，可以在命名管道上进行双向通信。

## 特性

- 🔌 支持 Windows 命名管道通信
- 🖥️ 服务器模式（监听）和客户端模式（连接）
- ⚡ 类似于 netcat 的简单接口
- 🔄 双向实时通信
- ⏱️ 可配置连接超时
- 📝 详细的错误信息和调试输出
- 🛠️ 使用 MinGW 交叉编译，支持 Windows

## 编译要求

### 安装 MinGW-w64

**macOS:**
```bash
brew install mingw-w64
```

**Ubuntu/Debian:**
```bash
sudo apt-get install mingw-w64
```

**Fedora/CentOS:**
```bash
sudo dnf install mingw64-gcc
```

## 编译

### 检查 MinGW 安装
```bash
make check-mingw
```

### 编译所有版本
```bash
make all
```

### 编译特定版本
```bash
# 简单版本（无外部依赖）
make simple

# 高级版本（支持更多选项）
make advanced
```

### 创建发布包
```bash
make dist
```

## 使用方法

### 基本语法
```bash
pipenc [选项] 管道名称
```

### 选项
- `-l` : 监听模式（创建命名管道服务器）
- `-t SECONDS` : 连接超时时间（默认：5秒）
- `-v` : 详细输出模式
- `-h` : 显示帮助信息

### 管道名称格式
Windows 本地命名管道应该以 `\\\\.\\pipe\\` 开头，例如：
- `\\\\.\\pipe\\test`
- `\\\\.\\pipe\\myapp`
- `\\\\.\\pipe\\communication`

## 使用示例

### 示例 1：基本通信

**终端 1（服务器）：**
```bash
./build/pipenc_simple.exe -l \\\\.\\pipe\\test
```

**终端 2（客户端）：**
```bash
./build/pipenc_simple.exe \\\\.\\pipe\\test
```

### 示例 2：带详细输出的通信

**终端 1（服务器）：**
```bash
./build/pipenc.exe -l -v -t 10 \\\\.\\pipe\\myapp
```

**终端 2（客户端）：**
```bash
./build/pipenc.exe -v -t 10 \\\\.\\pipe\\myapp
```

### 示例 3：应用程序间通信

可以将 PipeNc 用作调试工具，与其他使用命名管道的 Windows 应用程序通信：

```bash
# 监听应用程序创建的管道
./build/pipenc.exe \\\\.\\pipe\\MyApplication

# 创建管道供应用程序连接
./build/pipenc.exe -l \\\\.\\pipe\\MyDebugPipe
```

## 实际使用场景

### 1. 应用程序调试
当你需要调试使用命名管道的 Windows 应用程序时：
```bash
# 拦截应用程序的管道通信
pipenc.exe \\\\.\\pipe\\MyApp_Debug
```

### 2. 进程间通信测试
测试不同进程之间的通信：
```bash
# 进程 A
pipenc.exe -l \\\\.\\pipe\\ProcessComm

# 进程 B  
pipenc.exe \\\\.\\pipe\\ProcessComm
```

### 3. 脚本自动化
在批处理脚本中使用：
```batch
@echo off
echo Starting pipe server...
start pipenc.exe -l \\\\.\\pipe\\automation
timeout /t 2
echo Connecting client...
echo "Hello from script" | pipenc.exe \\\\.\\pipe\\automation
```

## 编译选项

### 调试版本
```bash
make debug
```

### 发布版本（优化）
```bash
make release
```

### 查看配置
```bash
make config
```

### 清理编译文件
```bash
make clean
```

## 故障排除

### 常见错误

1. **管道名称错误**
   ```
   Error: Failed to create named pipe (Error code: 123)
   ```
   确保管道名称以 `\\\\.\\pipe\\` 开头

2. **权限问题**
   ```
   Error: Access denied (Error code: 5)
   ```
   尝试以管理员权限运行

3. **连接超时**
   ```
   Error: Timeout waiting for named pipe
   ```
   增加超时时间或确保服务器正在运行

4. **MinGW 编译器未找到**
   ```bash
   make check-mingw
   ```
   检查 MinGW 安装

### 调试技巧

1. **使用详细输出**
   ```bash
   pipenc.exe -v -l \\\\.\\pipe\\debug
   ```

2. **检查管道状态**
   使用 Windows 的 `handle.exe` 工具：
   ```cmd
   handle.exe -a | findstr "pipe"
   ```

3. **监控系统日志**
   查看 Windows 事件查看器中的应用程序日志

## 兼容性

- ✅ Windows 7/8/10/11
- ✅ Windows Server 2008R2+
- ✅ 32位和64位系统
- ✅ 管理员和普通用户权限

## 许可证

本项目采用 MIT 许可证。

## 贡献

欢迎提交 issues 和 pull requests！

## 更新日志

### v1.0.0
- 初始版本
- 支持基本的命名管道通信
- 服务器和客户端模式
- MinGW 交叉编译支持
