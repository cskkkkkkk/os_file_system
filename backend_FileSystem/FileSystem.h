#pragma once
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <atomic>
#include <vector>
#include <sstream>
#include <cstring>
#include <fstream>
#include <iomanip>     // �ṩ std::setfill �� std::setw
#pragma warning(disable:4996)
//�궨��
#define BLOCK_SIZE	512						//��Ŵ�СΪ512B
#define INODE_SIZE	128						//inode�ڵ��СΪ128B��ע��sizeof(Inode)���ܳ�����ֵ
#define MAX_NAME_SIZE 28					//������ֳ��ȣ�����ҪС�������С

//#define INODE_NUM	640						//inode�ڵ���,���64���ļ�
#define INODE_NUM	14336
//#define BLOCK_NUM	10240					//�������10240 * 512B = 5120KB
#define BLOCK_NUM	100593
#define BLOCKS_PER_GROUP	128				//���п��ջ��С��һ�����ж�ջ����ܴ���ٸ����̿��ַ��y

#define MODE_DIR	01000					//Ŀ¼��ʶ
#define MODE_FILE	00000					//�ļ���ʶ
#define OWNER_R	4<<6						//���û���Ȩ��
#define OWNER_W	2<<6						//���û�дȨ��
#define OWNER_X	1<<6						//���û�ִ��Ȩ��
#define GROUP_R	4<<3						//���û���Ȩ��
#define GROUP_W	2<<3						//���û�дȨ��
#define GROUP_X	1<<3						//���û�ִ��Ȩ��
#define OTHERS_R	4						//�����û���Ȩ��
#define OTHERS_W	2						//�����û�дȨ��
#define OTHERS_X	1						//�����û�ִ��Ȩ��
#define FILE_DEF_PERMISSION 0664			//�ļ�Ĭ��Ȩ��
#define DIR_DEF_PERMISSION	0755			//Ŀ¼Ĭ��Ȩ��
#define max_read_from_host  20 * 512;     
#define FILESYSNAME	"CsOS.sys"			//��������ļ���
#define FileTotalSize  104857600                   //�ļ��ܴ�СΪ100M

//�ṹ������
//������
struct SuperBlock {
	unsigned short s_INODE_NUM;				//inode�ڵ�������� 65535
	unsigned int s_BLOCK_NUM;				//���̿��������� 4294967294

	unsigned short s_free_INODE_NUM;		//����inode�ڵ���
	unsigned int s_free_BLOCK_NUM;			//���д��̿���
	int s_free_addr;						//���п��ջָ��
	int s_free[BLOCKS_PER_GROUP];			//���п��ջ

	unsigned short s_BLOCK_SIZE;			//���̿��С
	unsigned short s_INODE_SIZE;			//inode��С
	unsigned short s_SUPERBLOCK_SIZE;		//�������С
	unsigned short s_blocks_per_group;		//ÿ blockgroup ��block����

	//���̷ֲ�
	int s_Superblock_StartAddr;
	int s_InodeBitmap_StartAddr;
	int s_BlockBitmap_StartAddr;
	int s_Inode_StartAddr;
	int s_Block_StartAddr;
};

//inode�ڵ�
struct Inode {
	unsigned short i_ino;					//inode��ʶ����ţ�
	unsigned short i_mode;					//��ȡȨ�ޡ�r--��ȡ��w--д��x--ִ��
	unsigned short i_cnt;					//���������ж����ļ���ָ�����inode
	//unsigned short i_uid;					//�ļ������û�id
	//unsigned short i_gid;					//�ļ������û���id
	char i_uname[20];						//�ļ������û�
	char i_gname[20];						//�ļ������û���
	unsigned int i_size;					//�ļ���С����λΪ�ֽڣ�B��
	time_t  i_ctime;						//inode��һ�α䶯��ʱ��
	time_t  i_mtime;						//�ļ�������һ�α䶯��ʱ��
	time_t  i_atime;						//�ļ���һ�δ򿪵�ʱ��
	int i_dirBlock[10];						//10��ֱ�ӿ顣10*512B = 5120B = 5KB
	int i_indirBlock_1;						//һ����ӿ顣512B/4 * 512B = 128 * 512B = 64KB
	//unsigned int i_indirBlock_2;			//������ӿ顣(512B/4)*(512B/4) * 512B = 128*128*512B = 8192KB = 8MB
	//unsigned int i_indirBlock_3;			//������ӿ顣(512B/4)*(512B/4)*(512B/4) * 512B = 128*128*128*512B = 1048576KB = 1024MB = 1G
											//�ļ�ϵͳ̫С������ʡ�Զ�����������ӿ�
	std::atomic<unsigned short> read_cnt;                 //���Ƕ��̵߳�����
	std::atomic<unsigned short> write_cnt;                //����д�̵߳�����
};

//Ŀ¼��
struct DirItem {								//32�ֽڣ�һ�����̿��ܴ� 512/32=16 ��Ŀ¼��
	char itemName[MAX_NAME_SIZE];			//Ŀ¼�����ļ���
	int inodeAddr;							//Ŀ¼���Ӧ��inode�ڵ��ַ
};

//ȫ�ֱ�������
extern SuperBlock* superblock;
extern const int Inode_StartAddr;
extern const int Superblock_StartAddr;                //������ ƫ�Ƶ�ַ,ռһ�����̿�
extern const int InodeBitmap_StartAddr;                //inodeλͼ ƫ�Ƶ�ַ��ռ�������̿飬����� 1024 ��inode��״̬
extern const int BlockBitmap_StartAddr;                //blockλͼ ƫ�Ƶ�ַ��ռ��ʮ�����̿飬����� 10240 �����̿飨5120KB����״̬
extern const int Inode_StartAddr;                        //inode�ڵ��� ƫ�Ƶ�ַ��ռ INODE_NUM/(BLOCK_SIZE/INODE_SIZE) �����̿�
extern const int Block_StartAddr;                        //block������ ƫ�Ƶ�ַ ��ռ INODE_NUM �����̿�
extern const int File_Max_Size;                                //�����ļ�����С
extern const int Sum_Size;                                        //��������ļ���С


//ȫ�ֱ�������
extern int Root_Dir_Addr;                                        //��Ŀ¼inode��ַ
extern int Cur_Dir_Addr;                                        //��ǰĿ¼
extern char Cur_Dir_Name[310];                                //��ǰĿ¼��
extern char Cur_Host_Name[110];                                //��ǰ������
extern char Cur_User_Name[110];                                //��ǰ��½�û���
extern char Cur_Group_Name[110];                        //��ǰ��½�û�����
extern char Cur_User_Dir_Name[310];                        //��ǰ��½�û�Ŀ¼��

extern int nextUID;                                                        //��һ��Ҫ������û���ʶ��
extern int nextGID;                                                        //��һ��Ҫ������û����ʶ��

extern bool isLogin;                                                //�Ƿ����û���½

extern FILE* fw;                                                        //��������ļ� д�ļ�ָ��
extern FILE* fr;                                                        //��������ļ� ���ļ�ָ��
extern FILE* file;                                                      //��������ļ� ��д�ļ�ָ��
extern SuperBlock* superblock;                                //������ָ��
extern bool inode_bitmap[INODE_NUM];                //inodeλͼ
extern bool block_bitmap[BLOCK_NUM];                //���̿�λͼ

extern char buffer[109000000];                                //10M������������������ļ�
extern char readFromHost[20 * 512];       //���ڴ洢��host�ļ������Ļ�����ģ���ļ��������ص���תվ

//��������
void Ready();													//��¼ϵͳǰ��׼������,ע��+��װ
bool Format();													//��ʽ��һ����������ļ�
bool Install();													//��װ�ļ�ϵͳ������������ļ��еĹؼ���Ϣ�糬������뵽�ڴ�
void printSuperBlock();											//��ӡ��������Ϣ
void printInodeBitmap();										//��ӡinodeʹ�����
void printBlockBitmap(int num = superblock->s_BLOCK_NUM);		//��ӡblockʹ�����
int	 balloc();													//���̿���亯��
bool bfree();													//���̿��ͷź���
int  ialloc();													//����i�ڵ�������
bool ifree();													//�ͷ�i���������
bool mkdir(int parinoAddr, const char name[]);							//Ŀ¼������������������һ��Ŀ¼�ļ�inode��ַ ,Ҫ������Ŀ¼��
bool rmdir(int parinoAddr, char name[]);							//Ŀ¼ɾ������
bool create(int parinoAddr, const char name[], char buf[]);				//�����ļ�����
bool del(int parinoAddr, char name[]);							//ɾ���ļ����� 
bool ls(int parinoaddr);										//��ʾ��ǰĿ¼�µ������ļ����ļ���
bool cd(int parinoaddr, const char name[],bool isRelativePath);							//���뵱ǰĿ¼�µ�nameĿ¼
void gotoxy(HANDLE hOut, int x, int y);							//�ƶ���굽ָ��λ��
void vi(int parinoaddr, char name[], char buf[]);					//ģ��һ����vi�������ı�
void writefile(int fileInodeAddr, char buf[]);	//��buf����д���ļ��Ĵ��̿�
void inUsername(char username[]);								//�����û���
void inPasswd(char passwd[]);									//��������
bool login(const char username[], const char passwd[]);													//��½����
bool check(const char username[],const char passwd[]);						//�˶��û���������
void gotoRoot();												//�ص���Ŀ¼
void logout();													//�û�ע��
bool useradd(char username[] ,char passwd[]);									//�û�ע��
bool userdel(char username[]);									//�û�ɾ��
bool chmod(int parinoAddr, const char name[], int pmode);				//�޸��ļ���Ŀ¼Ȩ��
bool touch(int parinoAddr, char name[], char buf[]);				//touch������ļ��������ַ�
void help();													//��ʾ���������嵥

void cmd(const char str[]);
bool cat_file(int parinoAddr, const char name[]);                //ֱ���ڴ��ڴ�ӡ�ļ����ݣ�Ҳ���Ƕ�ȡ�Ĺ���

//����ʵ�ֶ��ļ��Ĺ����������д���ܵĺ���
void reader_p(int fileInodeAddr);                                               //���ߵ�p����
void reader_v(int fileInodeAddr);                                               //���ߵ�v����
void writer_p(int fileInodeAddr);												//���ߵ�v����
void writer_v(int fileInodeAddr);                                               //д�ߵ�v����
const int MAX_RETRY = 30;                                      // ������Դ���

struct PathResult        //���·��ʱ�������صĽ��
{
	bool is_a_path; //����Ƿ���ȷ����·���ڹ����ϣ�����·�� or ���·����
	int addr;        //���һ������ĸ�Ŀ¼inode��ַ
	char nameLast[MAX_NAME_SIZE];  //���һ�������������Ŀ¼Ҳ�������ļ���
};

PathResult ParseFromRoot(const char path[]);                      //ʵ�־���·�������ļ�ϵͳ�����������/�ľ���·����Ȼ������ҵ����Ŀ���ļ�����Ŀ¼�ĸ�Ŀ¼��InodeAddr
int FindPathInCurdir(int parinoAddr, const char* name);                  //ÿһ��·���Ķ���ȥ��

char* parsePath(const char* path, char* Cur_Dir_Name, char* Root_Dir_Name);  //�Ƚ�������ľ���·���Ϸ���

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
void readFileFromHost(const char* hostFilePath);  //�ӱ��ض�ȡ�ļ����ڴ���
void writeFileToHost(const char* hostFilePath, int bufferSize);    //���ڴ���bufferд�뱾���ļ�������д��
int FindFileInCurdir(int parinoAddr, const char* name);              //��Ŀ¼���ҵ��ļ�Ȼ�󷵻�inode��ַ
void readFromFile(int parinoAddr, const char name[]);               //��ȡ�ļ����ݵ�������
extern ErrorCode code;                                                    //�洢ÿ�δ�����״̬�룬д��response�н��з���
extern std::string response;                                              //�洢ÿ�δ���󷵻صĽ��
extern std::ostringstream oss;                                              //�洢���������洢ÿ��ִ����Ϣ�ķ���

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
    char current_username[30]; //�û�
    // ��������                     
    void send(const char _data[2048], uint32_t& _id, char username[], Option _option = Option::NONE) {
        pid = GetCurrentProcessId();  // ��ȡ��ǰ����ID
        strcpy_s(data, _data);        // ��ȫ�ظ�������
        id += 1;
        _id = id;
        option = _option;
        type = 'n';  // Ĭ����������Ϊ 'n'������ʵ����Ҫ����
        strcpy(current_username, username);
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
bool is_prefix(const std::string& str, const std::string& prefix);
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















extern SharedMemory* sharedMemory;
extern HANDLE hMapFile;
extern HANDLE hSemaphore1, hSemaphore2;
#define SHM_NAME L"SimDiskSharedMemory"
#define cur_semaphore "cur_semaphore"
#define par_semaphore "par_semaphore"
extern Option response_op;//Ĭ�ϻظ�ѡ��
extern Option receive_op;
// �ر��ź������
//CloseHandle(hSemaphore1);
//CloseHandle(hSemaphore2);
void init();

void testLogic();

bool recursivelyLs(int addr, const char* name);           //�����Ϊ��ʵ��dir -s���ܶ�д�����ĺ���
bool find_file_is_empty(int parinoAddr, const char name[]);