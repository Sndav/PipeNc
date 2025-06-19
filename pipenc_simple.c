#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#define BUFFER_SIZE 4096
#define PIPE_TIMEOUT 5000

// 程序选项结构
typedef struct {
    char* pipe_name;
    int listen_mode;
    int verbose;
    int timeout;
} program_options_t;

// 函数声明
void print_usage(const char* program_name);
int create_named_pipe_server(const char* pipe_name, int timeout);
int connect_to_named_pipe(const char* pipe_name, int timeout);
void handle_pipe_communication(HANDLE pipe_handle);
void print_error(const char* message);
int parse_arguments(int argc, char* argv[], program_options_t* options);

void print_usage(const char* program_name) {
    printf("PipeNc - Windows Named Pipe Netcat-like Tool\n");
    printf("Usage: %s [OPTIONS] PIPE_NAME\n\n", program_name);
    printf("Options:\n");
    printf("  -l              Listen mode (create named pipe server)\n");
    printf("  -t SECONDS      Connection timeout in seconds (default: 5)\n");
    printf("  -v              Verbose output\n");
    printf("  -h              Show this help message\n\n");
    printf("Examples:\n");
    printf("  Server mode: %s -l \\\\.\\pipe\\test\n", program_name);
    printf("  Client mode: %s \\\\.\\pipe\\test\n", program_name);
    printf("\nNote: Pipe names should start with \\\\.\\pipe\\ for local pipes\n");
}

void print_error(const char* message) {
    DWORD error = GetLastError();
    fprintf(stderr, "Error: %s (Error code: %lu)\n", message, error);
    
    LPSTR error_text = NULL;
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                   NULL, error, 0, (LPSTR)&error_text, 0, NULL);
    
    if (error_text) {
        fprintf(stderr, "System error: %s", error_text);
        LocalFree(error_text);
    }
}

int parse_arguments(int argc, char* argv[], program_options_t* options) {
    int i;
    
    // 初始化默认值
    memset(options, 0, sizeof(program_options_t));
    options->timeout = 5;
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            options->listen_mode = 1;
        } else if (strcmp(argv[i], "-v") == 0) {
            options->verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return -1;
        } else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -t option requires a timeout value\n");
                return -1;
            }
            options->timeout = atoi(argv[++i]);
            if (options->timeout <= 0) {
                fprintf(stderr, "Error: Invalid timeout value\n");
                return -1;
            }
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            return -1;
        } else {
            // 这应该是管道名称
            if (options->pipe_name == NULL) {
                options->pipe_name = argv[i];
            } else {
                fprintf(stderr, "Error: Multiple pipe names specified\n");
                return -1;
            }
        }
    }
    
    if (options->pipe_name == NULL) {
        fprintf(stderr, "Error: Pipe name is required\n");
        return -1;
    }
    
    return 0;
}

int create_named_pipe_server(const char* pipe_name, int timeout) {
    HANDLE pipe_handle;
    BOOL connected;
    
    printf("Creating named pipe server: %s\n", pipe_name);
    
    // 创建命名管道
    pipe_handle = CreateNamedPipeA(
        pipe_name,                    // 管道名称
        PIPE_ACCESS_DUPLEX,          // 双向访问
        PIPE_TYPE_BYTE |             // 字节类型
        PIPE_READMODE_BYTE |         // 字节读模式
        PIPE_WAIT,                   // 阻塞模式
        1,                           // 最大实例数
        BUFFER_SIZE,                 // 输出缓冲区大小
        BUFFER_SIZE,                 // 输入缓冲区大小
        timeout * 1000,              // 默认超时
        NULL                         // 安全属性
    );
    
    if (pipe_handle == INVALID_HANDLE_VALUE) {
        print_error("Failed to create named pipe");
        return -1;
    }
    
    printf("Waiting for client connection...\n");
    
    // 等待客户端连接
    connected = ConnectNamedPipe(pipe_handle, NULL);
    if (!connected && GetLastError() != ERROR_PIPE_CONNECTED) {
        print_error("Failed to connect to client");
        CloseHandle(pipe_handle);
        return -1;
    }
    
    printf("Client connected successfully!\n");
    
    // 处理通信
    handle_pipe_communication(pipe_handle);
    
    // 断开连接
    DisconnectNamedPipe(pipe_handle);
    CloseHandle(pipe_handle);
    
    return 0;
}

int connect_to_named_pipe(const char* pipe_name, int timeout) {
    HANDLE pipe_handle;
    
    printf("Connecting to named pipe: %s\n", pipe_name);
    
    // 等待管道可用
    if (!WaitNamedPipeA(pipe_name, timeout * 1000)) {
        print_error("Timeout waiting for named pipe");
        return -1;
    }
    
    // 连接到命名管道
    pipe_handle = CreateFileA(
        pipe_name,                   // 管道名称
        GENERIC_READ | GENERIC_WRITE, // 读写访问
        0,                           // 不共享
        NULL,                        // 安全属性
        OPEN_EXISTING,               // 打开现有管道
        0,                           // 属性
        NULL                         // 模板文件
    );
    
    if (pipe_handle == INVALID_HANDLE_VALUE) {
        print_error("Failed to connect to named pipe");
        return -1;
    }
    
    printf("Connected to pipe successfully!\n");
    
    // 设置管道模式
    DWORD mode = PIPE_READMODE_BYTE;
    if (!SetNamedPipeHandleState(pipe_handle, &mode, NULL, NULL)) {
        print_error("Failed to set pipe mode");
        CloseHandle(pipe_handle);
        return -1;
    }
    
    // 处理通信
    handle_pipe_communication(pipe_handle);
    
    CloseHandle(pipe_handle);
    return 0;
}

void handle_pipe_communication(HANDLE pipe_handle) {
    char buffer[BUFFER_SIZE];
    char input_buffer[BUFFER_SIZE];
    DWORD bytes_read, bytes_written;
    int input_pos = 0;
    
    printf("Communication established. Type messages and press Enter (Ctrl+C to exit):\n");
    printf("================================================================\n");
    
    while (1) {
        // 检查管道是否有数据可读
        DWORD available_bytes = 0;
        if (PeekNamedPipe(pipe_handle, NULL, 0, NULL, &available_bytes, NULL)) {
            if (available_bytes > 0) {
                if (ReadFile(pipe_handle, buffer, sizeof(buffer) - 1, &bytes_read, NULL)) {
                    if (bytes_read > 0) {
                        buffer[bytes_read] = '\0';
                        printf("Received: %s", buffer);
                        fflush(stdout);
                    }
                } else {
                    DWORD error = GetLastError();
                    if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
                        printf("\nConnection closed by remote end.\n");
                        break;
                    } else {
                        print_error("Failed to read from pipe");
                        break;
                    }
                }
            }
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
                printf("\nConnection closed by remote end.\n");
                break;
            }
        }
        
        // 检查键盘输入
        if (_kbhit()) {
            int ch = _getch();
            
            if (ch == 3) { // Ctrl+C
                printf("\nExiting...\n");
                break;
            }
            
            if (ch == '\r' || ch == '\n') {
                // 发送输入的内容
                if (input_pos > 0) {
                    input_buffer[input_pos] = '\n';
                    input_buffer[input_pos + 1] = '\0';
                    
                    printf("\nSending: %s", input_buffer);
                    
                    if (!WriteFile(pipe_handle, input_buffer, input_pos + 1, &bytes_written, NULL)) {
                        DWORD error = GetLastError();
                        if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
                            printf("Connection closed by remote end.\n");
                            break;
                        } else {
                            print_error("Failed to write to pipe");
                            break;
                        }
                    }
                    FlushFileBuffers(pipe_handle);
                }
                input_pos = 0;
                printf("Input: ");
            } else if (ch == '\b' || ch == 127) { // 退格键
                if (input_pos > 0) {
                    input_pos--;
                    printf("\b \b"); // 删除字符的视觉效果
                }
            } else if (ch >= 32 && ch < 127 && input_pos < BUFFER_SIZE - 2) {
                // 可打印字符
                input_buffer[input_pos++] = ch;
                printf("%c", ch); // 回显
            }
            fflush(stdout);
        }
        
        Sleep(10); // 短暂休眠避免CPU占用过高
    }
}

int main(int argc, char* argv[]) {
    program_options_t options;
    
    // 解析命令行参数
    if (parse_arguments(argc, argv, &options) != 0) {
        if (argc > 1 && strcmp(argv[1], "-h") != 0) {
            print_usage(argv[0]);
        }
        return 1;
    }
    
    // 验证管道名称格式
    if (strncmp(options.pipe_name, "\\\\.\\pipe\\", 9) != 0) {
        fprintf(stderr, "Warning: Pipe name should start with \\\\.\\pipe\\\n");
    }
    
    if (options.verbose) {
        printf("PipeNc Configuration:\n");
        printf("  Mode: %s\n", options.listen_mode ? "Server (Listen)" : "Client (Connect)");
        printf("  Pipe: %s\n", options.pipe_name);
        printf("  Timeout: %d seconds\n", options.timeout);
        printf("\n");
    }
    
    // 根据模式执行相应操作
    int result;
    if (options.listen_mode) {
        result = create_named_pipe_server(options.pipe_name, options.timeout);
    } else {
        result = connect_to_named_pipe(options.pipe_name, options.timeout);
    }
    
    return result;
}
