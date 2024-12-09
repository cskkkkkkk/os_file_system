#pragma once
#include<iostream>
#include<vector>
#include<string>
#include <Windows.h>
#include <cstring>  // ���� strcpy
#include <cstdint>  // ���� uint32_t ����
#pragma warning(disable:4996)
// ������ö��
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

// ѡ��ö��
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

// �ַ����ָ�������ڽ�������
std::vector<std::string> split_command(const std::string& command);

// ·���ַ����ָ�������ڽ���·��
std::vector<std::string> split_path(std::string path);

// ����ṹ��
// ����ṹ��
struct Request {
    DWORD pid;              // ����ID��Windows��ʹ�� DWORD ���ͣ�
    char data[2048];        // ����
    uint32_t id;            // ID
    char type;              // ����
    Option option;          // ѡ��
    char current_username[30]; //�û�
    // ��������
    void send(const char _data[2048], uint32_t& _id,const char username[], Option _option = Option::NONE) {
        pid = GetCurrentProcessId();  // ��ȡ��ǰ����ID
        strcpy_s(data, _data);        // ��ȫ�ظ�������
        id += 1;
        _id = id;
        option = _option;
        type = 'n';  // Ĭ����������Ϊ 'n'������ʵ����Ҫ����
        strcpy_s(current_username , username);
    }
};

// ��Ӧ�ṹ��
struct Response {
    char data[2000];         // ����
    uint32_t id;             // ID
    ErrorCode code;          // ������
    char type;               // ����
    Option option;           // ѡ��

    // ������Ӧ
    void send(const char _data[2000], uint32_t _id, ErrorCode _code, Option _option) {
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

#include <Windows.h>
#include <stdexcept>  // �����׳��쳣

// �ź�����
/*
class Semaphore {
public:
    // ���캯��������һ���ź���
    Semaphore(const char* name,LONG initialCount=0 , LONG maximumCount = 1) {
        // ����һ���ź���
        hSemaphore = CreateSemaphoreA(
            nullptr,           // Ĭ�ϰ�ȫ��
            initialCount,      // ��ʼ����
            maximumCount,      // ������
            name);          // Ĭ���ź�������

        if (hSemaphore == nullptr) {
            throw std::runtime_error("Failed to create semaphore.");
        }
    }

    // �����������ر��ź���
    ~Semaphore() {
        if (hSemaphore != nullptr) {
            CloseHandle(hSemaphore);
        }
    }

    // P���� (�ȴ�����)�������ź�������
    void P() {
        DWORD dwWaitResult = WaitForSingleObject(hSemaphore, INFINITE);
        if (dwWaitResult != WAIT_OBJECT_0) {
            throw std::runtime_error("WaitForSingleObject failed.");
        }
    }

    // V���� (�ͷŲ���)�������ź�������
    void V() {
        if (!ReleaseSemaphore(hSemaphore, 1, nullptr)) {
            throw std::runtime_error("ReleaseSemaphore failed.");
        }
    }

private:
    HANDLE hSemaphore;  // �ź������
};
*/

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


void enable_virtual_terminal();
COORD get_cursor_position(HANDLE hConsole);
void set_cursor_position(HANDLE hConsole, short x, short y);
void clear_line_from_cursor(HANDLE hConsole);