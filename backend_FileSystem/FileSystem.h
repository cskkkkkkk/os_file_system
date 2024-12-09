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
#include <iomanip>     // 提供 std::setfill 和 std::setw
#pragma warning(disable:4996)
//宏定义
#define BLOCK_SIZE	512						//块号大小为512B
#define INODE_SIZE	128						//inode节点大小为128B。注：sizeof(Inode)不能超过该值
#define MAX_NAME_SIZE 28					//最大名字长度，长度要小于这个大小

//#define INODE_NUM	640						//inode节点数,最多64个文件
#define INODE_NUM	14336
//#define BLOCK_NUM	10240					//块号数，10240 * 512B = 5120KB
#define BLOCK_NUM	100593
#define BLOCKS_PER_GROUP	128				//空闲块堆栈大小，一个空闲堆栈最多能存多少个磁盘块地址，y

#define MODE_DIR	01000					//目录标识
#define MODE_FILE	00000					//文件标识
#define OWNER_R	4<<6						//本用户读权限
#define OWNER_W	2<<6						//本用户写权限
#define OWNER_X	1<<6						//本用户执行权限
#define GROUP_R	4<<3						//组用户读权限
#define GROUP_W	2<<3						//组用户写权限
#define GROUP_X	1<<3						//组用户执行权限
#define OTHERS_R	4						//其它用户读权限
#define OTHERS_W	2						//其它用户写权限
#define OTHERS_X	1						//其它用户执行权限
#define FILE_DEF_PERMISSION 0664			//文件默认权限
#define DIR_DEF_PERMISSION	0755			//目录默认权限
#define max_read_from_host  20 * 512;     
#define FILESYSNAME	"CsOS.sys"			//虚拟磁盘文件名
#define FileTotalSize  104857600                   //文件总大小为100M

//结构体声明
//超级块
struct SuperBlock {
	unsigned short s_INODE_NUM;				//inode节点数，最多 65535
	unsigned int s_BLOCK_NUM;				//磁盘块块数，最多 4294967294

	unsigned short s_free_INODE_NUM;		//空闲inode节点数
	unsigned int s_free_BLOCK_NUM;			//空闲磁盘块数
	int s_free_addr;						//空闲块堆栈指针
	int s_free[BLOCKS_PER_GROUP];			//空闲块堆栈

	unsigned short s_BLOCK_SIZE;			//磁盘块大小
	unsigned short s_INODE_SIZE;			//inode大小
	unsigned short s_SUPERBLOCK_SIZE;		//超级块大小
	unsigned short s_blocks_per_group;		//每 blockgroup 的block数量

	//磁盘分布
	int s_Superblock_StartAddr;
	int s_InodeBitmap_StartAddr;
	int s_BlockBitmap_StartAddr;
	int s_Inode_StartAddr;
	int s_Block_StartAddr;
};

//inode节点
struct Inode {
	unsigned short i_ino;					//inode标识（编号）
	unsigned short i_mode;					//存取权限。r--读取，w--写，x--执行
	unsigned short i_cnt;					//链接数。有多少文件名指向这个inode
	//unsigned short i_uid;					//文件所属用户id
	//unsigned short i_gid;					//文件所属用户组id
	char i_uname[20];						//文件所属用户
	char i_gname[20];						//文件所属用户组
	unsigned int i_size;					//文件大小，单位为字节（B）
	time_t  i_ctime;						//inode上一次变动的时间
	time_t  i_mtime;						//文件内容上一次变动的时间
	time_t  i_atime;						//文件上一次打开的时间
	int i_dirBlock[10];						//10个直接块。10*512B = 5120B = 5KB
	int i_indirBlock_1;						//一级间接块。512B/4 * 512B = 128 * 512B = 64KB
	//unsigned int i_indirBlock_2;			//二级间接块。(512B/4)*(512B/4) * 512B = 128*128*512B = 8192KB = 8MB
	//unsigned int i_indirBlock_3;			//三级间接块。(512B/4)*(512B/4)*(512B/4) * 512B = 128*128*128*512B = 1048576KB = 1024MB = 1G
											//文件系统太小，暂且省略二级、三级间接块
	std::atomic<unsigned short> read_cnt;                 //这是读线程的数量
	std::atomic<unsigned short> write_cnt;                //这是写线程的数量
};

//目录项
struct DirItem {								//32字节，一个磁盘块能存 512/32=16 个目录项
	char itemName[MAX_NAME_SIZE];			//目录或者文件名
	int inodeAddr;							//目录项对应的inode节点地址
};

//全局变量声明
extern SuperBlock* superblock;
extern const int Inode_StartAddr;
extern const int Superblock_StartAddr;                //超级块 偏移地址,占一个磁盘块
extern const int InodeBitmap_StartAddr;                //inode位图 偏移地址，占两个磁盘块，最多监控 1024 个inode的状态
extern const int BlockBitmap_StartAddr;                //block位图 偏移地址，占二十个磁盘块，最多监控 10240 个磁盘块（5120KB）的状态
extern const int Inode_StartAddr;                        //inode节点区 偏移地址，占 INODE_NUM/(BLOCK_SIZE/INODE_SIZE) 个磁盘块
extern const int Block_StartAddr;                        //block数据区 偏移地址 ，占 INODE_NUM 个磁盘块
extern const int File_Max_Size;                                //单个文件最大大小
extern const int Sum_Size;                                        //虚拟磁盘文件大小


//全局变量声明
extern int Root_Dir_Addr;                                        //根目录inode地址
extern int Cur_Dir_Addr;                                        //当前目录
extern char Cur_Dir_Name[310];                                //当前目录名
extern char Cur_Host_Name[110];                                //当前主机名
extern char Cur_User_Name[110];                                //当前登陆用户名
extern char Cur_Group_Name[110];                        //当前登陆用户组名
extern char Cur_User_Dir_Name[310];                        //当前登陆用户目录名

extern int nextUID;                                                        //下一个要分配的用户标识号
extern int nextGID;                                                        //下一个要分配的用户组标识号

extern bool isLogin;                                                //是否有用户登陆

extern FILE* fw;                                                        //虚拟磁盘文件 写文件指针
extern FILE* fr;                                                        //虚拟磁盘文件 读文件指针
extern FILE* file;                                                      //虚拟磁盘文件 读写文件指针
extern SuperBlock* superblock;                                //超级块指针
extern bool inode_bitmap[INODE_NUM];                //inode位图
extern bool block_bitmap[BLOCK_NUM];                //磁盘块位图

extern char buffer[109000000];                                //10M，缓存整个虚拟磁盘文件
extern char readFromHost[20 * 512];       //用于存储从host文件读来的或者是模拟文件读到本地的中转站

//函数声明
void Ready();													//登录系统前的准备工作,注册+安装
bool Format();													//格式化一个虚拟磁盘文件
bool Install();													//安装文件系统，将虚拟磁盘文件中的关键信息如超级块读入到内存
void printSuperBlock();											//打印超级块信息
void printInodeBitmap();										//打印inode使用情况
void printBlockBitmap(int num = superblock->s_BLOCK_NUM);		//打印block使用情况
int	 balloc();													//磁盘块分配函数
bool bfree();													//磁盘块释放函数
int  ialloc();													//分配i节点区函数
bool ifree();													//释放i结点区函数
bool mkdir(int parinoAddr, const char name[]);							//目录创建函数。参数：上一层目录文件inode地址 ,要创建的目录名
bool rmdir(int parinoAddr, char name[]);							//目录删除函数
bool create(int parinoAddr, const char name[], char buf[]);				//创建文件函数
bool del(int parinoAddr, char name[]);							//删除文件函数 
bool ls(int parinoaddr);										//显示当前目录下的所有文件和文件夹
bool cd(int parinoaddr, const char name[],bool isRelativePath);							//进入当前目录下的name目录
void gotoxy(HANDLE hOut, int x, int y);							//移动光标到指定位置
void vi(int parinoaddr, char name[], char buf[]);					//模拟一个简单vi，输入文本
void writefile(int fileInodeAddr, char buf[]);	//将buf内容写回文件的磁盘块
void inUsername(char username[]);								//输入用户名
void inPasswd(char passwd[]);									//输入密码
bool login(const char username[], const char passwd[]);													//登陆界面
bool check(const char username[],const char passwd[]);						//核对用户名，密码
void gotoRoot();												//回到根目录
void logout();													//用户注销
bool useradd(char username[] ,char passwd[]);									//用户注册
bool userdel(char username[]);									//用户删除
bool chmod(int parinoAddr, const char name[], int pmode);				//修改文件或目录权限
bool touch(int parinoAddr, char name[], char buf[]);				//touch命令创建文件，读入字符
void help();													//显示所有命令清单

void cmd(const char str[]);
bool cat_file(int parinoAddr, const char name[]);                //直接在窗口打印文件内容，也算是读取的过程

//用于实现对文件的共享读，互斥写功能的函数
void reader_p(int fileInodeAddr);                                               //读者的p操作
void reader_v(int fileInodeAddr);                                               //读者的v操作
void writer_p(int fileInodeAddr);												//读者的v操作
void writer_v(int fileInodeAddr);                                               //写者的v操作
const int MAX_RETRY = 30;                                      // 最大重试次数

struct PathResult        //检查路径时函数返回的结果
{
	bool is_a_path; //检查是否正确这条路径在规则上（绝对路径 or 相对路径）
	int addr;        //最后一个组件的父目录inode地址
	char nameLast[MAX_NAME_SIZE];  //最后一个组件，可能是目录也可能是文件。
};

PathResult ParseFromRoot(const char path[]);                      //实现绝对路径操作文件系统，可以输入从/的绝对路径，然后解析找到最后目标文件或者目录的父目录的InodeAddr
int FindPathInCurdir(int parinoAddr, const char* name);                  //每一个路径的东西去找

char* parsePath(const char* path, char* Cur_Dir_Name, char* Root_Dir_Name);  //先解析输入的绝对路径合法性

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
void readFileFromHost(const char* hostFilePath);  //从本地读取文件到内存中
void writeFileToHost(const char* hostFilePath, int bufferSize);    //从内存中buffer写入本地文件（覆盖写）
int FindFileInCurdir(int parinoAddr, const char* name);              //在目录中找到文件然后返回inode地址
void readFromFile(int parinoAddr, const char name[]);               //读取文件内容到缓冲区
extern ErrorCode code;                                                    //存储每次处理后的状态码，写入response中进行返回
extern std::string response;                                              //存储每次处理后返回的结果
extern std::ostringstream oss;                                              //存储缓冲区，存储每次执行信息的返回

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
    char current_username[30]; //用户
    // 发送请求                     
    void send(const char _data[2048], uint32_t& _id, char username[], Option _option = Option::NONE) {
        pid = GetCurrentProcessId();  // 获取当前进程ID
        strcpy_s(data, _data);        // 安全地复制数据
        id += 1;
        _id = id;
        option = _option;
        type = 'n';  // 默认类型设置为 'n'，根据实际需要更改
        strcpy(current_username, username);
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















extern SharedMemory* sharedMemory;
extern HANDLE hMapFile;
extern HANDLE hSemaphore1, hSemaphore2;
#define SHM_NAME L"SimDiskSharedMemory"
#define cur_semaphore "cur_semaphore"
#define par_semaphore "par_semaphore"
extern Option response_op;//默认回复选项
extern Option receive_op;
// 关闭信号量句柄
//CloseHandle(hSemaphore1);
//CloseHandle(hSemaphore2);
void init();

void testLogic();

bool recursivelyLs(int addr, const char* name);           //这个是为了实现dir -s功能而写出来的函数
bool find_file_is_empty(int parinoAddr, const char name[]);