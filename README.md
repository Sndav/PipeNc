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

## 使用方法

### 基本语法
```bash
pipenc.exe [选项] 管道名称
```

### 选项
- `-l` : 监听模式（创建命名管道服务器）
- `-t SECONDS` : 连接超时时间（默认：5秒）
- `-v` : 详细输出模式
- `-x` : 以 hex 模式显示
- `-h` : 显示帮助信息

## 许可证

本项目采用 MIT 许可证。

## 贡献

欢迎提交 issues 和 pull requests！
