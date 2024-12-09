#pragma once
#include<iostream>
#include<vector>
#include<string>
#include <Windows.h>
#include <cstring>  // 用于 strcpy
#include <cstdint>  // 用于 uint32_t 类型
#pragma warning(disable:4996)
// 错误码枚举
enum class ErrorCode {
    SUCCESS,                // 操作成功
    FAILURE,                // 操作失败
    EXISTS,                 // 已存在
    EXCEEDED,               // 超出限制
    WAIT_REQUEST,           // 等待请求
    FILE_NOT_FOUND,         // 文件未找到
    FILE_NOT_MATCH,         // 文件不匹配
    PERMISSION_DENIED,      // 权限被拒绝
    LOCKED,                 // 被锁定
};

// 选项枚举
enum class Option {
    NONE,           // 无操作
    NEW,            // 新建
    PWD,            // 获取当前路径
    GET,            // 获取
    READ,           // 读取
    EXEC,           // 执行
    WRITE,          // 写入
    EXIT,           // 退出
    REQUEST,        // 请求
    RESPONSE,       // 响应
    CAT,            // 查看文件内容
    SWITCH,         // 切换
    PATCH,          // 补丁
    TAB             // 制表
};

// 字符串分割函数，用于解析命令
std::vector<std::string> split_command(const std::string& command);

// 路径字符串分割函数，用于解析路径
std::vector<std::string> split_path(std::string path);

// 请求结构体
// 请求结构体
struct Request {
    DWORD pid;              // 进程ID（Windows下使用 DWORD 类型）
    char data[2048];        // 数据
    uint32_t id;            // ID
    char type;              // 类型
    Option option;          // 选项
    char current_username[30]; //用户
    // 发送请求
    void send(const char _data[2048], uint32_t& _id,const char username[], Option _option = Option::NONE) {
        pid = GetCurrentProcessId();  // 获取当前进程ID
        strcpy_s(data, _data);        // 安全地复制数据
        id += 1;
        _id = id;
        option = _option;
        type = 'n';  // 默认类型设置为 'n'，根据实际需要更改
        strcpy_s(current_username , username);
    }
};

// 响应结构体
struct Response {
    char data[2000];         // 数据
    uint32_t id;             // ID
    ErrorCode code;          // 错误码
    char type;               // 类型
    Option option;           // 选项

    // 发送响应
    void send(const char _data[2000], uint32_t _id, ErrorCode _code, Option _option) {
        strcpy(data, _data);
        code = _code;
        option = _option;
        type = 'n';
        id = _id;
    }
};

// 共享内存结构体
struct SharedMemory {
    Request request;         // 请求
    Response response;       // 响应
};

#include <Windows.h>
#include <stdexcept>  // 用于抛出异常

// 信号量类
/*
class Semaphore {
public:
    // 构造函数：创建一个信号量
    Semaphore(const char* name,LONG initialCount=0 , LONG maximumCount = 1) {
        // 创建一个信号量
        hSemaphore = CreateSemaphoreA(
            nullptr,           // 默认安全性
            initialCount,      // 初始计数
            maximumCount,      // 最大计数
            name);          // 默认信号量名称

        if (hSemaphore == nullptr) {
            throw std::runtime_error("Failed to create semaphore.");
        }
    }

    // 析构函数：关闭信号量
    ~Semaphore() {
        if (hSemaphore != nullptr) {
            CloseHandle(hSemaphore);
        }
    }

    // P操作 (等待操作)：减少信号量计数
    void P() {
        DWORD dwWaitResult = WaitForSingleObject(hSemaphore, INFINITE);
        if (dwWaitResult != WAIT_OBJECT_0) {
            throw std::runtime_error("WaitForSingleObject failed.");
        }
    }

    // V操作 (释放操作)：增加信号量计数
    void V() {
        if (!ReleaseSemaphore(hSemaphore, 1, nullptr)) {
            throw std::runtime_error("ReleaseSemaphore failed.");
        }
    }

private:
    HANDLE hSemaphore;  // 信号量句柄
};
*/

class Semaphore {
public:
    // P操作
    static void P(HANDLE hSemaphore) {
        // 等待信号量，直到信号量的值大于0
        DWORD dwWaitResult = WaitForSingleObject(hSemaphore, INFINITE);
        if (dwWaitResult == WAIT_FAILED) {
            std::cerr << "P操作失败, 错误代码: " << GetLastError() << std::endl;
        }
    }

    // V操作
    static void V(HANDLE hSemaphore) {
        // 增加信号量的值，允许其他进程访问资源
        if (!ReleaseSemaphore(hSemaphore, 1, NULL)) {
            std::cerr << "V操作失败, 错误代码: " << GetLastError() << std::endl;
        }
    }
};


void enable_virtual_terminal();
COORD get_cursor_position(HANDLE hConsole);
void set_cursor_position(HANDLE hConsole, short x, short y);
void clear_line_from_cursor(HANDLE hConsole);