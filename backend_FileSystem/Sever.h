/*
#include"FileSystem.h"
#pragma warning(disable:4996)

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
// 请求结构体
struct Request {
    DWORD pid;              // 进程ID（Windows下使用 DWORD 类型）
    char data[2048];        // 数据
    uint32_t id;            // ID
    char type;              // 类型
    Option option;          // 选项

    // 发送请求                     
    void send(const char _data[2048], uint32_t& _id, Option _option = Option::NONE) {
        pid = GetCurrentProcessId();  // 获取当前进程ID
        strcpy_s(data, _data);        // 安全地复制数据
        id += 1;
        _id = id;
        option = _option;
        type = 'n';  // 默认类型设置为 'n'，根据实际需要更改
    }
};

// 响应结构体
struct Response {
    char data[20000];         // 数据
    uint32_t id;             // ID
    ErrorCode code;          // 错误码
    char type;               // 类型
    Option option;           // 选项

    // 发送响应
    void send(const char _data[20000], uint32_t _id, ErrorCode _code, Option _option) {
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

static std::string to_string(Option option) {
    switch (option) {
    case Option::NONE: return "NONE";
    case Option::NEW: return "NEW";
    case Option::PWD: return "PWD";
    case Option::GET: return "GET";
    case Option::READ: return "READ";
    case Option::EXEC: return "EXEC";
    case Option::WRITE: return "WRITE";
    case Option::EXIT: return "EXIT";
    case Option::REQUEST: return "REQUEST";
    case Option::RESPONSE: return "RESPONSE";
    case Option::CAT: return "CAT";
    case Option::SWITCH: return "SWITCH";
    case Option::PATCH: return "PATCH";
    case Option::TAB: return "TAB";
    default: return "Unknown Option";
    }
};
bool is_prefix(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    std::string substring = str.substr(0, prefix.length());
    return substring == prefix;
}
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















SharedMemory* sharedMemory;
HANDLE hMapFile;
HANDLE hSemaphore1, hSemaphore2;
#define SHM_NAME L"SimDiskSharedMemory"
#define cur_semaphore "cur_semaphore"
#define par_semaphore "par_semaphore"

// 关闭信号量句柄
//CloseHandle(hSemaphore1);
//CloseHandle(hSemaphore2);
void init()
{
    hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMemory), SHM_NAME);
    if (hMapFile == NULL) {
        std::cerr << "创建共享内存失败" << std::endl;
        return;
    }
    sharedMemory = (SharedMemory*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory));
    if (sharedMemory == NULL) {
        std::cerr << "共享内存映射失败" << std::endl;
        CloseHandle(hMapFile);
        return;
    }

    //初始化一些变量
    sharedMemory->request.pid = 0;
    memset(sharedMemory->request.data, 0, sizeof(char) * 2048);
    sharedMemory->request.id = 0;
    sharedMemory->request.type = 'y';
    sharedMemory->request.option = Option::NONE;
    memset(sharedMemory->response.data, 0, sizeof(char) * 20000);
    sharedMemory->response.id = 0;
    sharedMemory->response.code = ErrorCode::SUCCESS;
    sharedMemory->response.type = 'y';
    sharedMemory->response.option = Option::NONE;

    //开始创建信号量，hSemaphore1是互斥的，hSemaphore2是同步的
    LPCTSTR szName1 = TEXT("Semaphore1");
    LPCTSTR szName2 = TEXT("Semaphore2");

    // 创建信号量 1 (初始值为1)
    hSemaphore1 = CreateSemaphore(NULL, 1, 1, szName1);
    if (hSemaphore1 == NULL) {
        std::cerr << "无法创建信号量 1, 错误代码: " << GetLastError() << std::endl;
        return ;
    }

    // 创建信号量 2 (初始值为0)
    hSemaphore2 = CreateSemaphore(NULL, 0, 1, szName2);
    if (hSemaphore2 == NULL) {
        std::cerr << "无法创建信号量 2, 错误代码: " << GetLastError() << std::endl;
        CloseHandle(hSemaphore1);
        return;
    }
    std::ofstream output("C:\\Users\\86183\\Desktop\\ids.txt");
    output <<hSemaphore1 << ' ' << hSemaphore2;
    output.close();
}

void testLogic()
{
    Semaphore::P(hSemaphore2);
    uint32_t id = sharedMemory->request.id;
    std::string request(sharedMemory->request.data);
    Option option = sharedMemory->request.option;
    DWORD pid = sharedMemory->request.pid;
    sharedMemory->request.type = 'y';//只有这个被设置为y,shell端才能重新写入请求

    int time = 1000;
    int cnt = 0;
    while (sharedMemory->response.type == 'n') {
        if (cnt >= 10) {
            break;
        }
        Sleep(time);
        ++cnt;
    }
    std::string response;//存储执行结果
    //response = Filesystem::response.str();
    //调用fileSystem结果存储在
    //ErrorCode code;
    //sharedMemory->response.send(response.c_str(), id, code, option);
    return;
}

*/