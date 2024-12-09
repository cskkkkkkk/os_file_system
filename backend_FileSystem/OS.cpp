#include"FileSystem.h"
using namespace std;
//ȫ�ֱ�������
const int Superblock_StartAddr = 0;																//������ ƫ�Ƶ�ַ,ռһ�����̿�
//const int InodeBitmap_StartAddr = sizeof(SuperBlock);													//inodeλͼ ƫ�Ƶ�ַ��ռ�������̿飬����� 1024 ��inode��״̬
const int InodeBitmap_StartAddr = 2*BLOCK_SIZE;
//const int BlockBitmap_StartAddr = InodeBitmap_StartAddr + 2 * BLOCK_SIZE;							//blockλͼ ƫ�Ƶ�ַ��ռ��ʮ�����̿飬����� 10240 �����̿飨5120KB����״̬
const int BlockBitmap_StartAddr = InodeBitmap_StartAddr + 14 * BLOCK_SIZE;
//const int Inode_StartAddr = BlockBitmap_StartAddr + 20 * BLOCK_SIZE;								//inode�ڵ��� ƫ�Ƶ�ַ��ռ INODE_NUM/(BLOCK_SIZE/INODE_SIZE) �����̿�
const int Inode_StartAddr = BlockBitmap_StartAddr + 104 * BLOCK_SIZE;
//const int Block_StartAddr = Inode_StartAddr + INODE_NUM / (BLOCK_SIZE / INODE_SIZE) * BLOCK_SIZE;	//block������ ƫ�Ƶ�ַ ��ռ INODE_NUM �����̿�
const int Block_StartAddr = Inode_StartAddr + 14336 * BLOCK_SIZE;
//const int Sum_Size = Block_StartAddr + BLOCK_NUM * BLOCK_SIZE;									//��������ļ���С
const int Sum_Size = 10485600;
//�����ļ�����С
const int File_Max_Size = 10 * BLOCK_SIZE +														//10��ֱ�ӿ�
BLOCK_SIZE / sizeof(int) * BLOCK_SIZE +								//һ����ӿ�
(BLOCK_SIZE / sizeof(int)) * (BLOCK_SIZE / sizeof(int)) * BLOCK_SIZE;		//������ӿ�

int Root_Dir_Addr;							//��Ŀ¼inode��ַ
int Cur_Dir_Addr;							//��ǰĿ¼
char Cur_Dir_Name[310];						//��ǰĿ¼��
char Cur_Host_Name[110];					//��ǰ������
char Cur_User_Name[110];					//��ǰ��½�û���
char Cur_Group_Name[110];					//��ǰ��½�û�����
char Cur_User_Dir_Name[310];				//��ǰ��½�û�Ŀ¼��

int nextUID;								//��һ��Ҫ������û���ʶ��
int nextGID;								//��һ��Ҫ������û����ʶ��

bool isLogin;								//�Ƿ����û���½

FILE* fw;									//��������ļ� д�ļ�ָ��
FILE* fr;									//��������ļ� ���ļ�ָ��
FILE* file;                                 //��������ļ� ��д�ļ�ָ��
SuperBlock* superblock = new SuperBlock;	//������ָ��
bool inode_bitmap[INODE_NUM];				//inodeλͼ
bool block_bitmap[BLOCK_NUM];				//���̿�λͼ

char buffer[109000000] = { 0 };				//10M������������������ļ�
char readFromHost[20 * 512] = { 0 };       //���ڴ洢��host�ļ������Ļ�����ģ���ļ��������ص���תվ
ErrorCode code;
std::string response;
SharedMemory* sharedMemory;
HANDLE hMapFile;
HANDLE hSemaphore1, hSemaphore2;
std::ostringstream oss;
Option response_op = Option::NONE;//Ĭ�ϻظ�ѡ��
Option receive_op=Option::NONE;
/*
int main()
{
	//����������ļ� 

	if ((file = fopen(FILESYSNAME, "rb+")) == NULL) {	//ֻ������������ļ����������ļ���
		//��������ļ������ڣ�����һ��
		//fw = fopen(FILESYSNAME, "wb");	//ֻд����������ļ����������ļ���
		file = fopen(FILESYSNAME, "wb+");
		if (file == NULL) {
			//if (fw == NULL) {
			printf("��������ļ���ʧ��\n");
			return 0;	//���ļ�ʧ��
		}
		//fr = fopen(FILESYSNAME, "rb");	//���ڿ��Դ���

		//��ʼ������
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//��ȡ������
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);

		//��Ŀ¼inode��ַ ����ǰĿ¼��ַ������
		Root_Dir_Addr = Inode_StartAddr;	//��һ��inode��ַ
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		printf("�ļ�ϵͳ���ڸ�ʽ������\n");
		if (!Format()) {
			printf("�ļ�ϵͳ��ʽ��ʧ��\n");
			return 0;
		}
		printf("��ʽ�����\n");
		printf("����������е�һ�ε�½\n");
		system("pause");
		system("cls");


		if (!Install()) {
			printf("��װ�ļ�ϵͳʧ��\n");
			return 0;
		}
	}
	else {	//��������ļ��Ѵ���
		//fread(buffer, Sum_Size, 1, fr);
		fread(buffer, Sum_Size, 1, file);
		//ȡ���ļ������ݴ浽�����У���д��ʽ���ļ�֮����д���ļ���д��ʽ�򿪻�����ļ���
		file = fopen(FILESYSNAME, "wb+");
		//fw = fopen(FILESYSNAME, "wb");	//ֻд����������ļ����������ļ���
		if (file == NULL) {
			printf("��������ļ���ʧ��\n");
			return false;	//���ļ�ʧ��
		}
		//fwrite(buffer, Sum_Size, 1, fw);
		fwrite(buffer, Sum_Size, 1, file);
		//* ��ʾ�Ƿ�Ҫ��ʽ��
		 //* ��Ϊ���ǵ�һ�ε�½������ȥ��һ��
		 //* ������Ҫ�ֶ����ñ���
		//Ready();
		//system("pause");
		//system("cls");
		

		//��ʼ������
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//��ȡ������
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);

		//��Ŀ¼inode��ַ ����ǰĿ¼��ַ������
		Root_Dir_Addr = Inode_StartAddr;	//��һ��inode��ַ
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		if (!Install()) {
			printf("��װ�ļ�ϵͳʧ��\n");
			return 0;
		}

	}


	//testPrint();

	//��¼
	while (1) {
		if (isLogin) {	//��½�ɹ������ܽ���shell
			char str[100];
			char* p;
			if ((p = strstr(Cur_Dir_Name, Cur_User_Dir_Name)) == NULL)	//û�ҵ�
				printf("[%s@%s:%s]# ", Cur_Host_Name, Cur_User_Name, Cur_Dir_Name);
			else
				//printf("[%s@%s ~%s]# ", Cur_Host_Name, Cur_User_Name, Cur_Dir_Name + strlen(Cur_User_Dir_Name));
				printf("[%s@%s:~%s]# ", Cur_Host_Name, Cur_User_Name, Cur_Dir_Name );
			//gets(str); ���Ƴ���
			fgets(str, sizeof(str), stdin);
			cmd(str);
		}
		else {
			printf("��ӭ����MingOS�����ȵ�¼\n");
			while (!login());	//��½
			printf("��½�ɹ���\n");
			//system("pause");
			system("cls");
		}
	}

	//fclose(fw);		//�ͷ��ļ�ָ��
	//fclose(fr);		//�ͷ��ļ�ָ��
	fclose(file);       //�ͷŶ�д�ļ�ָ��
	return 0;
}
*/
int main()
{
	//����������ļ� 

	if ((file = fopen(FILESYSNAME, "rb+")) == NULL) {	//ֻ������������ļ����������ļ���
		//��������ļ������ڣ�����һ��
		//fw = fopen(FILESYSNAME, "wb");	//ֻд����������ļ����������ļ���
		file = fopen(FILESYSNAME, "wb+");
		if (file == NULL) {
			//if (fw == NULL) {
			printf("��������ļ���ʧ��\n");
			return 0;	//���ļ�ʧ��
		}
		//fr = fopen(FILESYSNAME, "rb");	//���ڿ��Դ���
		 // �����ļ���С
		if (fseek(file, FileTotalSize - 1, SEEK_SET) != 0) {
			printf("�ļ���չʧ��\n");
			fclose(file);
			return 0; // fseek ����
		}

		// д��һ���ֽڣ�ʹ�ļ���չ��Ŀ���С
		if (fwrite("\0", 1, 1, file) != 1) {
			printf("�ļ���չʧ��\n");
			fclose(file);
			return 0; // д��ʧ��
		}
		//��ʼ������
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//��ȡ������
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);
		
		//��Ŀ¼inode��ַ ����ǰĿ¼��ַ������
		Root_Dir_Addr = Inode_StartAddr;	//��һ��inode��ַ
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		printf("�ļ�ϵͳ���ڸ�ʽ������\n");
		if (!Format()) {
			printf("�ļ�ϵͳ��ʽ��ʧ��\n");
			return 0;
		}
		printf("��ʽ�����\n");
		printf("����������е�һ�ε�½\n");
		system("pause");
		system("cls");


		if (!Install()) {
			printf("��װ�ļ�ϵͳʧ��\n");
			return 0;
		}
	}
	else {	//��������ļ��Ѵ���
		//fread(buffer, Sum_Size, 1, fr);
		fread(buffer, Sum_Size, 1, file);
		//ȡ���ļ������ݴ浽�����У���д��ʽ���ļ�֮����д���ļ���д��ʽ�򿪻�����ļ���
		file = fopen(FILESYSNAME, "wb+");
		//fw = fopen(FILESYSNAME, "wb");	//ֻд����������ļ����������ļ���
		if (file == NULL) {
			printf("��������ļ���ʧ��\n");
			return false;	//���ļ�ʧ��
		}
		//fwrite(buffer, Sum_Size, 1, fw);
		fwrite(buffer, Sum_Size, 1, file);

		//��ʼ������
		nextUID = 0;
		nextGID = 0;
		isLogin = false;
		strcpy(Cur_User_Name, "root");
		strcpy(Cur_Group_Name, "root");

		//��ȡ������
		memset(Cur_Host_Name, 0, sizeof(Cur_Host_Name));
		DWORD k = 100;
		GetComputerNameA(Cur_Host_Name, &k);

		//��Ŀ¼inode��ַ ����ǰĿ¼��ַ������
		Root_Dir_Addr = Inode_StartAddr;	//��һ��inode��ַ
		Cur_Dir_Addr = Root_Dir_Addr;
		strcpy(Cur_Dir_Name, "/");

		if (!Install()) {
			printf("��װ�ļ�ϵͳʧ��\n");
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
    sharedMemory->request.type = 'y';//ֻ�����������Ϊy,shell�˲�������д������
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
    //std::string response;//�洢ִ�н��
    //response = Filesystem::response.str();
    //����fileSystem����洢��
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
		sharedMemory->request.type = 'y';//ֻ�����������Ϊy,shell�˲�������д������
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
		//����fileSystem����洢��
		//ErrorCode code;
		
		sharedMemory->response.send(response.c_str(), id, code, response_op);
	}

	fclose(file);       //�ͷŶ�д�ļ�ָ��
	return 0;
}