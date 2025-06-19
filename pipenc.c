#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <getopt.h>
#include <conio.h>

#define BUFFER_SIZE 4096
#define PIPE_TIMEOUT 5000

// 程序选项结构
typedef struct {
    char* pipe_name;
    int listen_mode;
    int verbose;
    int timeout;
    int hexdump;
} program_options_t;

// 函数声明
void print_usage(const char* program_name);
int create_named_pipe_server(const char* pipe_name, int timeout, program_options_t* options);
int connect_to_named_pipe(const char* pipe_name, int timeout, program_options_t* options);
void handle_pipe_communication(HANDLE pipe_handle, program_options_t* options);
void print_error(const char* message);
void print_hexdump(const char* label, const unsigned char* data, size_t length);

void print_usage(const char* program_name) {
    printf("PipeNc - Windows Named Pipe Netcat-like Tool\n");
    printf("Usage: %s [OPTIONS] PIPE_NAME\n\n", program_name);
    printf("Options:\n");
    printf("  -l, --listen        Listen mode (create named pipe server)\n");
    printf("  -t, --timeout SEC   Connection timeout in seconds (default: 5)\n");
    printf("  -v, --verbose       Verbose output\n");
    printf("  -x, --hexdump       Display data in hexadecimal format\n");
    printf("  -h, --help          Show this help message\n\n");
    printf("Examples:\n");
    printf("  Server mode: %s -l \\\\.\\pipe\\test\n", program_name);
    printf("  Client mode: %s \\\\.\\pipe\\test\n", program_name);
    printf("  Hexdump mode: %s -x \\\\.\\pipe\\test\n", program_name);
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

int create_named_pipe_server(const char* pipe_name, int timeout, program_options_t* options) {
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
    handle_pipe_communication(pipe_handle, options);
    
    // 断开连接
    DisconnectNamedPipe(pipe_handle);
    CloseHandle(pipe_handle);
    
    return 0;
}

int connect_to_named_pipe(const char* pipe_name, int timeout, program_options_t* options) {
    HANDLE pipe_handle;
    DWORD wait_result;
    
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
    handle_pipe_communication(pipe_handle, options);
    
    CloseHandle(pipe_handle);
    return 0;
}

void handle_pipe_communication(HANDLE pipe_handle, program_options_t* options) {
    char buffer[BUFFER_SIZE];
    DWORD bytes_read, bytes_written;
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    
    printf("Communication established. Type messages (Ctrl+C to exit):\n");
    printf("=================================================\n");
    
    // 设置控制台模式为非阻塞
    DWORD console_mode;
    GetConsoleMode(stdin_handle, &console_mode);
    SetConsoleMode(stdin_handle, console_mode & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT);
    
    while (1) {
        // 检查管道是否有数据可读
        DWORD available_bytes = 0;
        if (PeekNamedPipe(pipe_handle, NULL, 0, NULL, &available_bytes, NULL)) {
            if (available_bytes > 0) {
                if (ReadFile(pipe_handle, buffer, sizeof(buffer) - 1, &bytes_read, NULL)) {
                    if (bytes_read > 0) {
                        if (options->hexdump) {
                            print_hexdump("Received", (unsigned char*)buffer, bytes_read);
                        } else {
                            buffer[bytes_read] = '\0';
                            printf("Received: %s", buffer);
                        }
                        fflush(stdout);
                    }
                } else {
                    print_error("Failed to read from pipe");
                    break;
                }
            }
        }
        
        // 检查标准输入是否有数据
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 3) { // Ctrl+C
                printf("\nExiting...\n");
                break;
            }
            
            // 读取一行输入
            int pos = 0;
            buffer[pos++] = ch;
            
            if (ch != '\r' && ch != '\n') {
                printf("%c", ch); // 回显字符
            }
            
            if (ch == '\r' || ch == '\n') {
                buffer[pos++] = '\n';
                buffer[pos] = '\0';
                printf("\n");
                
                // 发送数据到管道
                if (!WriteFile(pipe_handle, buffer, pos, &bytes_written, NULL)) {
                    print_error("Failed to write to pipe");
                    break;
                }
                
                // 如果启用了hexdump，显示发送的数据
                if (options->hexdump) {
                    print_hexdump("Sent", (unsigned char*)buffer, pos);
                }
                
                FlushFileBuffers(pipe_handle);
            }
        }
        
        Sleep(10); // 短暂休眠避免CPU占用过高
    }
    
    // 恢复控制台模式
    SetConsoleMode(stdin_handle, console_mode);
}

void print_hexdump(const char* label, const unsigned char* data, size_t length) {
    printf("%s (%zu bytes):\n", label, length);
    
    for (size_t i = 0; i < length; i += 16) {
        // 打印偏移地址
        printf("%08zx  ", i);
        
        // 打印十六进制数据
        for (size_t j = 0; j < 16; j++) {
            if (i + j < length) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   "); // 空白填充
            }
            
            // 在第8个字节后添加额外空格
            if (j == 7) {
                printf(" ");
            }
        }
        
        printf(" |");
        
        // 打印ASCII表示
        for (size_t j = 0; j < 16 && i + j < length; j++) {
            unsigned char c = data[i + j];
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        
        printf("|\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    program_options_t options = {0};
    options.timeout = 5; // 默认超时5秒
    
    // 长选项定义
    static struct option long_options[] = {
        {"listen", no_argument, 0, 'l'},
        {"timeout", required_argument, 0, 't'},
        {"verbose", no_argument, 0, 'v'},
        {"hexdump", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // 解析命令行参数
    while ((c = getopt_long(argc, argv, "lt:vxh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'l':
                options.listen_mode = 1;
                break;
            case 't':
                options.timeout = atoi(optarg);
                if (options.timeout <= 0) {
                    fprintf(stderr, "Error: Invalid timeout value\n");
                    return 1;
                }
                break;
            case 'v':
                options.verbose = 1;
                break;
            case 'x':
                options.hexdump = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                break;
        }
    }
    
    // 检查是否提供了管道名称
    if (optind >= argc) {
        fprintf(stderr, "Error: Pipe name is required\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    options.pipe_name = argv[optind];
    
    // 验证管道名称格式
    if (strncmp(options.pipe_name, "\\\\.\\pipe\\", 9) != 0) {
        fprintf(stderr, "Warning: Pipe name should start with \\\\.\\pipe\\\n");
    }
    
    if (options.verbose) {
        printf("PipeNc Configuration:\n");
        printf("  Mode: %s\n", options.listen_mode ? "Server (Listen)" : "Client (Connect)");
        printf("  Pipe: %s\n", options.pipe_name);
        printf("  Timeout: %d seconds\n", options.timeout);
        printf("  Hexdump: %s\n", options.hexdump ? "Enabled" : "Disabled");
        printf("\n");
    }
    
    // 根据模式执行相应操作
    int result;
    if (options.listen_mode) {
        result = create_named_pipe_server(options.pipe_name, options.timeout, &options);
    } else {
        result = connect_to_named_pipe(options.pipe_name, options.timeout, &options);
    }
    
    return result;
}
