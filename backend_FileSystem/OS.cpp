#include"FileSystem.h"
using namespace std;
//全局变量定义
const int Superblock_StartAddr = 0;																//超级块 偏移地址,占一个磁盘块
//const int InodeBitmap_StartAddr = sizeof(SuperBlock);													//inode位图 偏移地址，占两个磁盘块，最多监控 1024 个inode的状态
const int InodeBitmap_StartAddr = 2*BLOCK_SIZE;
//const int BlockBitmap_StartAddr = InodeBitmap_StartAddr + 2 * BLOCK_SIZE;							//block位图 偏移地址，占二十个磁盘块，最多监控 10240 个磁盘块（5120KB）的状态
const int BlockBitmap_StartAddr = InodeBitmap_StartAddr + 14 * BLOCK_SIZE;
//const int Inode_StartAddr = BlockBitmap_StartAddr + 20 * BLOCK_SIZE;								//inode节点区 偏移地址，占 INODE_NUM/(BLOCK_SIZE/INODE_SIZE) 个磁盘块
const int Inode_StartAddr = BlockBitmap_StartAddr + 104 * BLOCK_SIZE;
//const int Block_StartAddr = Inode_StartAddr + INODE_NUM / (BLOCK_SIZE / INODE_SIZE) * BLOCK_SIZE;	//block数据区 偏移地址 ，占 INODE_NUM 个磁盘块
const int Block_StartAddr = Inode_StartAddr + 14336 * BLOCK_SIZE;
//const int Sum_Size = Block_StartAddr + BLOCK_NUM * BLOCK_SIZE;									//虚拟磁盘文件大小
const int Sum_Size = 10485600;
//单个文件最大大小
const int File_Max_Size = 10 * BLOCK_SIZE +														//10个直接块
BLOCK_SIZE / sizeof(int) * BLOCK_SIZE +								//一级间接块
(BLOCK_SIZE / sizeof(int)) * (BLOCK_SIZE / sizeof(int)) * BLOCK_SIZE;		//二级间接块

int Root_Dir_Addr;							//根目录inode地址
int Cur_Dir_Addr;							//当前目录
char Cur_Dir_Name[310];						//当前目录名
char Cur_Host_Name[110];					//当前主机名
char Cur_User_Name[110];					//当前登陆用户名
char Cur_Group_Name[110];					//当前登陆用户组名
char Cur_User_Dir_Name[310];				//当前登陆用户目录名

int nextUID;								//下一个要分配的用户标识号
int nextGID;								//下一个要分配的用户组标识号

bool isLogin;								//是否有用户登陆

FILE* fw;									//虚拟磁盘文件 写文件指针
FILE* fr;									//虚拟磁盘文件 读文件指针
FILE* file;                                 //虚拟磁盘文件 读写文件指针
SuperBlock* superblock = new SuperBlock;	//超级块指针
bool inode_bitmap[INODE_NUM];				//inode位图
bool block_bitmap[BLOCK_NUM];				//磁盘块位图

char buffer[109000000] = { 0 };				//10M，缓存整个虚拟磁盘文件
char readFromHost[20 * 512] = { 0 };       //用于存储从host文件读来的或者是模拟文件读到本地的中转站
ErrorCode code;
std::string response;
SharedMemory* sharedMemory;
HANDLE hMapFile;
HANDLE hSemaphore1, hSemaphore2;
std::ostringstream oss;
Option response_op = Option::NONE;//默认回复选项
Option receive_op=Option::NONE;
/*
int main()
{
	//打开虚拟磁盘文件 

	if ((file = fopen(FILESYSNAME, "rb+")) == NULL) {	//只读打开虚拟磁盘文件（二进制文件）
		//虚拟磁盘文件不存在，创建一个
		//fw = fopen(FILESYSNAME, "wb");	//只写打开虚拟磁盘文件（二进制文件）
		file = fopen(FILESYSNAME, "wb+");
		if (file == NULL) {
			//if (fw == NULL) {
			printf("虚拟磁盘文件打开失败\n");
			return 0;	//打开文件失败
		}
		//fr = fopen(FILESYSNAME, "rb");	//现在可以打开了

		//初始化变量
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//获取主机名
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);

		//根目录inode地址 ，当前目录地址和名字
		Root_Dir_Addr = Inode_StartAddr;	//第一个inode地址
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		printf("文件系统正在格式化……\n");
		if (!Format()) {
			printf("文件系统格式化失败\n");
			return 0;
		}
		printf("格式化完成\n");
		printf("按任意键进行第一次登陆\n");
		system("pause");
		system("cls");


		if (!Install()) {
			printf("安装文件系统失败\n");
			return 0;
		}
	}
	else {	//虚拟磁盘文件已存在
		//fread(buffer, Sum_Size, 1, fr);
		fread(buffer, Sum_Size, 1, file);
		//取出文件内容暂存到内容中，以写方式打开文件之后再写回文件（写方式打开会清空文件）
		file = fopen(FILESYSNAME, "wb+");
		//fw = fopen(FILESYSNAME, "wb");	//只写打开虚拟磁盘文件（二进制文件）
		if (file == NULL) {
			printf("虚拟磁盘文件打开失败\n");
			return false;	//打开文件失败
		}
		//fwrite(buffer, Sum_Size, 1, fw);
		fwrite(buffer, Sum_Size, 1, file);
		//* 提示是否要格式化
		 //* 因为不是第一次登陆，先略去这一步
		 //* 下面需要手动设置变量
		//Ready();
		//system("pause");
		//system("cls");
		

		//初始化变量
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//获取主机名
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);

		//根目录inode地址 ，当前目录地址和名字
		Root_Dir_Addr = Inode_StartAddr;	//第一个inode地址
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		if (!Install()) {
			printf("安装文件系统失败\n");
			return 0;
		}

	}


	//testPrint();

	//登录
	while (1) {
		if (isLogin) {	//登陆成功，才能进入shell
			char str[100];
			char* p;
			if ((p = strstr(Cur_Dir_Name, Cur_User_Dir_Name)) == NULL)	//没找到
				printf("[%s@%s:%s]# ", Cur_Host_Name, Cur_User_Name, Cur_Dir_Name);
			else
				//printf("[%s@%s ~%s]# ", Cur_Host_Name, Cur_User_Name, Cur_Dir_Name + strlen(Cur_User_Dir_Name));
				printf("[%s@%s:~%s]# ", Cur_Host_Name, Cur_User_Name, Cur_Dir_Name );
			//gets(str); 被移除了
			fgets(str, sizeof(str), stdin);
			cmd(str);
		}
		else {
			printf("欢迎来到MingOS，请先登录\n");
			while (!login());	//登陆
			printf("登陆成功！\n");
			//system("pause");
			system("cls");
		}
	}

	//fclose(fw);		//释放文件指针
	//fclose(fr);		//释放文件指针
	fclose(file);       //释放读写文件指针
	return 0;
}
*/
int main()
{
	//打开虚拟磁盘文件 

	if ((file = fopen(FILESYSNAME, "rb+")) == NULL) {	//只读打开虚拟磁盘文件（二进制文件）
		//虚拟磁盘文件不存在，创建一个
		//fw = fopen(FILESYSNAME, "wb");	//只写打开虚拟磁盘文件（二进制文件）
		file = fopen(FILESYSNAME, "wb+");
		if (file == NULL) {
			//if (fw == NULL) {
			printf("虚拟磁盘文件打开失败\n");
			return 0;	//打开文件失败
		}
		//fr = fopen(FILESYSNAME, "rb");	//现在可以打开了
		 // 设置文件大小
		if (fseek(file, FileTotalSize - 1, SEEK_SET) != 0) {
			printf("文件扩展失败\n");
			fclose(file);
			return 0; // fseek 错误
		}

		// 写入一个字节，使文件扩展到目标大小
		if (fwrite("\0", 1, 1, file) != 1) {
			printf("文件扩展失败\n");
			fclose(file);
			return 0; // 写入失败
		}
		//初始化变量
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//获取主机名
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);
		
		//根目录inode地址 ，当前目录地址和名字
		Root_Dir_Addr = Inode_StartAddr;	//第一个inode地址
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		printf("文件系统正在格式化……\n");
		if (!Format()) {
			printf("文件系统格式化失败\n");
			return 0;
		}
		printf("格式化完成\n");
		printf("按任意键进行第一次登陆\n");
		system("pause");
		system("cls");


		if (!Install()) {
			printf("安装文件系统失败\n");
			return 0;
		}
	}
	else {	//虚拟磁盘文件已存在
		//fread(buffer, Sum_Size, 1, fr);
		fread(buffer, Sum_Size, 1, file);
		//取出文件内容暂存到内容中，以写方式打开文件之后再写回文件（写方式打开会清空文件）
		file = fopen(FILESYSNAME, "wb+");
		//fw = fopen(FILESYSNAME, "wb");	//只写打开虚拟磁盘文件（二进制文件）
		if (file == NULL) {
			printf("虚拟磁盘文件打开失败\n");
			return false;	//打开文件失败
		}
		//fwrite(buffer, Sum_Size, 1, fw);
		fwrite(buffer, Sum_Size, 1, file);

		//初始化变量
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//获取主机名
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);

		//根目录inode地址 ，当前目录地址和名字
		Root_Dir_Addr = Inode_StartAddr;	//第一个inode地址
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		if (!Install()) {
			printf("安装文件系统失败\n");
			return 0;
		}

	}
	init();
	Semaphore::P(hSemaphore2);
    uint32_t id = sharedMemory->request.id;
    std::string request(sharedMemory->request.data);
    Option option = sharedMemory->request.option;
    DWORD pid = sharedMemory->request.pid;
	strcpy(Cur_User_Name, sharedMemory->request.current_username);
    sharedMemory->request.type = 'y';//只有这个被设置为y,shell端才能重新写入请求
	cmd(request.c_str());
    int time = 1000;
    int cnt = 0;
    while (sharedMemory->response.type == 'n') {
        if (cnt >= 10) {
            break;
        }
        Sleep(time);
        ++cnt;
    }
	sharedMemory->response.send(response.c_str(), id, code, Option::NONE);
    //std::string response;//存储执行结果
    //response = Filesystem::response.str();
    //调用fileSystem结果存储在
    //ErrorCode code;
    //sharedMemory->response.send(response.c_str(), id, code, option);
	while (true)
	{
		Semaphore::P(hSemaphore2);
		uint32_t id = sharedMemory->request.id;
		std::string request(sharedMemory->request.data);
		receive_op = sharedMemory->request.option;
		DWORD pid = sharedMemory->request.pid;
		strcpy(Cur_User_Name,sharedMemory->request.current_username);
		sharedMemory->request.type = 'y';//只有这个被设置为y,shell端才能重新写入请求
		cmd(request.c_str());

		int time = 1000;
		int cnt = 0;
		while (sharedMemory->response.type == 'n') {
			if (cnt >= 10) {
				break;
			}
			Sleep(time);
			++cnt;
		}
		//response = Filesystem::response.str();
		//调用fileSystem结果存储在
		//ErrorCode code;
		
		sharedMemory->response.send(response.c_str(), id, code, response_op);
	}

	fclose(file);       //释放读写文件指针
	return 0;
}