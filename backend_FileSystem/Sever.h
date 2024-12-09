/*
#include"FileSystem.h"
#pragma warning(disable:4996)

enum class ErrorCode {
    SUCCESS,                // �����ɹ�
    FAILURE,                // ����ʧ��
    EXISTS,                 // �Ѵ���
    EXCEEDED,               // ��������
    WAIT_REQUEST,           // �ȴ�����
    FILE_NOT_FOUND,         // �ļ�δ�ҵ�
    FILE_NOT_MATCH,         // �ļ���ƥ��
    PERMISSION_DENIED,      // Ȩ�ޱ��ܾ�
    LOCKED,                 // ������
};
enum class Option {
    NONE,           // �޲���
    NEW,            // �½�
    PWD,            // ��ȡ��ǰ·��
    GET,            // ��ȡ
    READ,           // ��ȡ
    EXEC,           // ִ��
    WRITE,          // д��
    EXIT,           // �˳�
    REQUEST,        // ����
    RESPONSE,       // ��Ӧ
    CAT,            // �鿴�ļ�����
    SWITCH,         // �л�
    PATCH,          // ����
    TAB             // �Ʊ�
};
// ����ṹ��
struct Request {
    DWORD pid;              // ����ID��Windows��ʹ�� DWORD ���ͣ�
    char data[2048];        // ����
    uint32_t id;            // ID
    char type;              // ����
    Option option;          // ѡ��

    // ��������                     
    void send(const char _data[2048], uint32_t& _id, Option _option = Option::NONE) {
        pid = GetCurrentProcessId();  // ��ȡ��ǰ����ID
        strcpy_s(data, _data);        // ��ȫ�ظ�������
        id += 1;
        _id = id;
        option = _option;
        type = 'n';  // Ĭ����������Ϊ 'n'������ʵ����Ҫ����
    }
};

// ��Ӧ�ṹ��
struct Response {
    char data[20000];         // ����
    uint32_t id;             // ID
    ErrorCode code;          // ������
    char type;               // ����
    Option option;           // ѡ��

    // ������Ӧ
    void send(const char _data[20000], uint32_t _id, ErrorCode _code, Option _option) {
        strcpy(data, _data);
        code = _code;
        option = _option;
        type = 'n';
        id = _id;
    }
};

// �����ڴ�ṹ��
struct SharedMemory {
    Request request;         // ����
    Response response;       // ��Ӧ
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
    // P����
    static void P(HANDLE hSemaphore) {
        // �ȴ��ź�����ֱ���ź�����ֵ����0
        DWORD dwWaitResult = WaitForSingleObject(hSemaphore, INFINITE);
        if (dwWaitResult == WAIT_FAILED) {
            std::cerr << "P����ʧ��, �������: " << GetLastError() << std::endl;
        }
    }

    // V����
    static void V(HANDLE hSemaphore) {
        // �����ź�����ֵ�������������̷�����Դ
        if (!ReleaseSemaphore(hSemaphore, 1, NULL)) {
            std::cerr << "V����ʧ��, �������: " << GetLastError() << std::endl;
        }
    }
};















SharedMemory* sharedMemory;
HANDLE hMapFile;
HANDLE hSemaphore1, hSemaphore2;
#define SHM_NAME L"SimDiskSharedMemory"
#define cur_semaphore "cur_semaphore"
#define par_semaphore "par_semaphore"

// �ر��ź������
//CloseHandle(hSemaphore1);
//CloseHandle(hSemaphore2);
void init()
{
    hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMemory), SHM_NAME);
    if (hMapFile == NULL) {
        std::cerr << "���������ڴ�ʧ��" << std::endl;
        return;
    }
    sharedMemory = (SharedMemory*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory));
    if (sharedMemory == NULL) {
        std::cerr << "�����ڴ�ӳ��ʧ��" << std::endl;
        CloseHandle(hMapFile);
        return;
    }

    //��ʼ��һЩ����
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

    //��ʼ�����ź�����hSemaphore1�ǻ���ģ�hSemaphore2��ͬ����
    LPCTSTR szName1 = TEXT("Semaphore1");
    LPCTSTR szName2 = TEXT("Semaphore2");

    // �����ź��� 1 (��ʼֵΪ1)
    hSemaphore1 = CreateSemaphore(NULL, 1, 1, szName1);
    if (hSemaphore1 == NULL) {
        std::cerr << "�޷������ź��� 1, �������: " << GetLastError() << std::endl;
        return ;
    }

    // �����ź��� 2 (��ʼֵΪ0)
    hSemaphore2 = CreateSemaphore(NULL, 0, 1, szName2);
    if (hSemaphore2 == NULL) {
        std::cerr << "�޷������ź��� 2, �������: " << GetLastError() << std::endl;
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
    sharedMemory->request.type = 'y';//ֻ�����������Ϊy,shell�˲�������д������

    int time = 1000;
    int cnt = 0;
    while (sharedMemory->response.type == 'n') {
        if (cnt >= 10) {
            break;
        }
        Sleep(time);
        ++cnt;
    }
    std::string response;//�洢ִ�н��
    //response = Filesystem::response.str();
    //����fileSystem����洢��
    //ErrorCode code;
    //sharedMemory->response.send(response.c_str(), id, code, option);
    return;
}

*/