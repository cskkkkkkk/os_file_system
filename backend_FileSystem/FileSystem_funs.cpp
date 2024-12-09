#include "FileSystem.h"
#pragma warning(disable:4996)

using namespace std;

//����ʵ��
void Ready()	//��¼ϵͳǰ��׼������,������ʼ��+ע��+��װ
{
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


	char c;
	printf("�Ƿ��ʽ��?[y/n]");
	// ��ȡһ���ַ�������Ҫ�� Enter
	while (c = getch()) {
		//�п���������
		fflush(stdin);

		if (c == 'y') {
			printf("\n");
			printf("�ļ�ϵͳ���ڸ�ʽ������\n");
			//��ʽ��һ����������ļ�
			if (!Format()) {
				printf("�ļ�ϵͳ��ʽ��ʧ��\n");
				return;
			}
			printf("��ʽ�����\n");
			break;
		}
		else if (c == 'n') {
			printf("\n");
			break;
		}
	}

	//printf("�����ļ�ϵͳ����\n");
	if (!Install()) {
		printf("��װ�ļ�ϵͳʧ��\n");
		return;
	}
	//printf("�������\n");
}

bool Format()	//��ʽ��һ����������ļ�
{
	int i, j;

	//��ʼ��������
	superblock->s_INODE_NUM = INODE_NUM;
	superblock->s_BLOCK_NUM = BLOCK_NUM;
	superblock->s_SUPERBLOCK_SIZE = sizeof(SuperBlock);
	superblock->s_INODE_SIZE = INODE_SIZE;
	superblock->s_BLOCK_SIZE = BLOCK_SIZE;
	superblock->s_free_INODE_NUM = INODE_NUM;
	superblock->s_free_BLOCK_NUM = BLOCK_NUM;
	superblock->s_blocks_per_group = BLOCKS_PER_GROUP;
	superblock->s_free_addr = Block_StartAddr;	//���п��ջָ��Ϊ��һ��block
	superblock->s_Superblock_StartAddr = Superblock_StartAddr;
	superblock->s_BlockBitmap_StartAddr = BlockBitmap_StartAddr;
	superblock->s_InodeBitmap_StartAddr = InodeBitmap_StartAddr;
	superblock->s_Block_StartAddr = Block_StartAddr;
	superblock->s_Inode_StartAddr = Inode_StartAddr;
	//���п��ջ�ں��渳ֵ

	//��ʼ��inodeλͼ
	memset(inode_bitmap, 0, sizeof(inode_bitmap));
	//fseek(fw, InodeBitmap_StartAddr, SEEK_SET);
	fseek(file, InodeBitmap_StartAddr, SEEK_SET);
	//����������д�뵽һ���ļ�fw��
	//fwrite(inode_bitmap, sizeof(inode_bitmap), 1, fw);
	fwrite(inode_bitmap, sizeof(inode_bitmap), 1, file);
	fflush(file);
	//��ʼ��blockλͼ
	memset(block_bitmap, 0, sizeof(block_bitmap));
	//���ļ�ָ���ƶ���start_addr��Ȼ��Ϊ0�Ŀ�λͼ��д�룬SEEK_SET����˼��˵���ļ�����ʼλ��Ų����ʼ��ַ
	//fseek(fw, BlockBitmap_StartAddr, SEEK_SET);
	fseek(file, BlockBitmap_StartAddr, SEEK_SET);
	//fwrite(block_bitmap, sizeof(block_bitmap), 1, fw);
	fwrite(block_bitmap, sizeof(block_bitmap), 1, file);
	fflush(file);

	//��ʼ�����̿��������ݳ������ӷ���֯	
	for (i = BLOCK_NUM / BLOCKS_PER_GROUP - 1; i >= 0; i--) {	//һ��INODE_NUM/BLOCKS_PER_GROUP��=160�飬һ��FREESTACKNUM��128�������̿� ����һ�����̿���Ϊ����
		if (i == BLOCK_NUM / BLOCKS_PER_GROUP - 1)
			superblock->s_free[0] = -1;	//û����һ�����п���
		else
			superblock->s_free[0] = Block_StartAddr + (i + 1) * BLOCKS_PER_GROUP * BLOCK_SIZE;	//ָ����һ�����п�
		for (j = 1; j < BLOCKS_PER_GROUP; j++) {
			superblock->s_free[j] = Block_StartAddr + (i * BLOCKS_PER_GROUP + j) * BLOCK_SIZE;
		}

		//fseek(fw, Block_StartAddr + i * BLOCKS_PER_GROUP * BLOCK_SIZE, SEEK_SET);
		fseek(file, Block_StartAddr + i * BLOCKS_PER_GROUP * BLOCK_SIZE, SEEK_SET);
		//fwrite(superblock->s_free, sizeof(superblock->s_free), 1, fw);	//����������̿飬512�ֽ�
		fwrite(superblock->s_free, sizeof(superblock->s_free), 1, file);	//����������̿飬512�ֽ�
		fflush(file);
	}
	//������д�뵽��������ļ�
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	fwrite(superblock, sizeof(SuperBlock), 1, file);
	//fflush(fw);
	fflush(file);

	//��ȡinodeλͼ
	//fseek(fr, InodeBitmap_StartAddr, SEEK_SET);
	fseek(file, InodeBitmap_StartAddr, SEEK_SET);
	//fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);
	fread(inode_bitmap, sizeof(inode_bitmap), 1, file);
	//��ȡblockλͼ
	//fseek(fr, BlockBitmap_StartAddr, SEEK_SET);
	fseek(file, BlockBitmap_StartAddr, SEEK_SET);
	//fread(block_bitmap, sizeof(block_bitmap), 1, fr);
	fread(block_bitmap, sizeof(block_bitmap), 1, file);
	//fflush(fr);





	//������Ŀ¼ "/"
	Inode cur;

	//����inode
	int inoAddr = ialloc();

	//�����inode������̿�
	int blockAddr = balloc();

	//��������̿������һ����Ŀ "."
	DirItem dirlist[16] = { 0 };
	strcpy(dirlist[0].itemName, ".");
	dirlist[0].inodeAddr = inoAddr;

	//д�ش��̿�
	//fseek(fw, blockAddr, SEEK_SET);
	fseek(file, blockAddr, SEEK_SET);
	//fwrite(dirlist, sizeof(dirlist), 1, fw);
	fwrite(dirlist, sizeof(dirlist), 1, file);
	//fflush(fw);
	fflush(file);
	//���涼��Ϊ�˳�ʼ����Ŀ¼





	//��inode��ֵ
	cur.i_ino = 0;
	cur.i_atime = time(NULL);
	cur.i_ctime = time(NULL);
	cur.i_mtime = time(NULL);
	strcpy(cur.i_uname, Cur_User_Name);
	strcpy(cur.i_gname, Cur_Group_Name);
	cur.i_cnt = 1;	//һ�����ǰĿ¼,"."
	cur.i_dirBlock[0] = blockAddr;
	for (i = 1; i < 10; i++) {
		cur.i_dirBlock[i] = -1;
	}
	cur.i_size = superblock->s_BLOCK_SIZE;
	cur.i_indirBlock_1 = -1;	//ûʹ��һ����ӿ�
	cur.i_mode = MODE_DIR | DIR_DEF_PERMISSION;


	//д��inode
	//fseek(fw, inoAddr, SEEK_SET);
	fseek(file, inoAddr, SEEK_SET);
	//fwrite(&cur, sizeof(Inode), 1, fw);
	fwrite(&cur, sizeof(Inode), 1, file);
	//fflush(fw);
	fflush(file);

	//����Ŀ¼�������ļ�
	mkdir(inoAddr, "home");	//�û�Ŀ¼
	cd(Root_Dir_Addr, "home",true);

	//
	mkdir(Cur_Dir_Addr, "root");

	cd(Cur_Dir_Addr, "..",true);
	//


	mkdir(Cur_Dir_Addr, "etc");	//�����ļ�Ŀ¼
	cd(Cur_Dir_Addr, "etc",true);

	char buf[1000] = { 0 };

	sprintf(buf, "root:x:%d:%d\n", nextUID++, nextGID++);	//������Ŀ���û������������룺�û�ID���û���ID
	create(Cur_Dir_Addr, "passwd", buf);	//�����û���Ϣ�ļ�

	sprintf(buf, "root:root\n");	//������Ŀ���û���������
	create(Cur_Dir_Addr, "shadow", buf);	//�����û������ļ�
	chmod(Cur_Dir_Addr, "shadow", 0660);	//�޸�Ȩ�ޣ���ֹ�����û���ȡ���ļ�
	//ÿ�ε���springtf�����buf���и���
	sprintf(buf, "root::0:root\n");	//���ӹ���Ա�û��飬�û����������һ��Ϊ�գ�����û��ʹ�ã������ʶ�ţ������û��б��ã��ָ���
	sprintf(buf + strlen(buf), "user::1:\n");	//������ͨ�û��飬�����û��б�Ϊ��
	create(Cur_Dir_Addr, "group", buf);	//�����û�����Ϣ�ļ�

	cd(Cur_Dir_Addr, "..",true);	//�ص���Ŀ¼
	//cd(Cur_Dir_Addr, "..");
	return true;
}

bool Install()	//��װ�ļ�ϵͳ������������ļ��еĹؼ���Ϣ�糬������뵽�ڴ�
{
	//��д��������ļ�����ȡ�����飬��ȡinodeλͼ��blockλͼ����ȡ��Ŀ¼����ȡetcĿ¼����ȡ����ԱadminĿ¼����ȡ�û�xiaoĿ¼����ȡ�û�passwd�ļ���

	//��ȡ������
	//fseek(fr, Superblock_StartAddr, SEEK_SET);
	//fread(superblock, sizeof(SuperBlock), 1, fr);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fread(superblock, sizeof(SuperBlock), 1, file);

	//��ȡinodeλͼ
	//fseek(fr, InodeBitmap_StartAddr, SEEK_SET);
	//fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);
	fseek(file, InodeBitmap_StartAddr, SEEK_SET);
	fread(inode_bitmap, sizeof(inode_bitmap), 1, file);

	//��ȡblockλͼ
	//fseek(fr, BlockBitmap_StartAddr, SEEK_SET);
	//fread(block_bitmap, sizeof(block_bitmap), 1, fr);
	fseek(file, BlockBitmap_StartAddr, SEEK_SET);
	fread(block_bitmap, sizeof(block_bitmap), 1, file);
	return true;
}

void printSuperBlock()		//��ӡ��������Ϣ
{
	
	printf("\n");
	printf("����inode�� / ��inode�� ��%d / %d\n", superblock->s_free_INODE_NUM, superblock->s_INODE_NUM);
	oss << "\n����inode�� / ��inode�� ��"<< superblock->s_free_INODE_NUM << " / " << superblock->s_INODE_NUM << "\n";
	printf("����block�� / ��block�� ��%d / %d\n", superblock->s_free_BLOCK_NUM, superblock->s_BLOCK_NUM);
	oss << "����block�� / ��block�� ��" << superblock->s_free_BLOCK_NUM << " / " << superblock->s_BLOCK_NUM << "\n";
	//printf("��ϵͳ block��С��%d �ֽڣ�ÿ��inodeռ %d �ֽڣ���ʵ��С��%d �ֽڣ�\n", superblock->s_BLOCK_SIZE, superblock->s_INODE_SIZE, sizeof(Inode));
	printf("��ϵͳ block��С��%d �ֽڣ�ÿ��inodeռ %d �ֽڣ���ʵ��С��%d �ֽڣ�\n", 1024, superblock->s_INODE_SIZE, sizeof(Inode));
	oss << "��ϵͳ block��С��" << 1024 << " �ֽڣ�ÿ��inodeռ "
		<< superblock->s_INODE_SIZE << " �ֽڣ���ʵ��С��"
		<< sizeof(Inode) << " �ֽڣ�\n";
	printf("\tÿ���̿��飨���ж�ջ��������block������%d\n", superblock->s_blocks_per_group);
	oss << "��ϵͳ block��С��" << 1024 << " �ֽڣ�ÿ��inodeռ "
		<< superblock->s_INODE_SIZE << " �ֽڣ���ʵ��С��"
		<< sizeof(Inode) << " �ֽڣ�\n";
	printf("\t������ռ %d �ֽڣ���ʵ��С��%d �ֽڣ�\n", superblock->s_BLOCK_SIZE, superblock->s_SUPERBLOCK_SIZE);
	oss << "\tÿ���̿��飨���ж�ջ��������block������"
		<< superblock->s_blocks_per_group << "\n";
	printf("��������ܴ�С:100M\n");
	oss << "��������ܴ�С:100M\n";
	printf("���̷ֲ���\n");
	printf("\t�����鿪ʼλ�ã�%d B\n", superblock->s_Superblock_StartAddr);
	printf("\tinodeλͼ��ʼλ�ã�%d B\n", superblock->s_InodeBitmap_StartAddr);
	printf("\tblockλͼ��ʼλ�ã�%d B\n", superblock->s_BlockBitmap_StartAddr);
	printf("\tinode����ʼλ�ã�%d B\n", superblock->s_Inode_StartAddr);
	printf("\tblock����ʼλ�ã�%d B\n", superblock->s_Block_StartAddr);
	printf("\n");
	oss << "���̷ֲ���\n";
	oss << "\t�����鿪ʼλ�ã�" << superblock->s_Superblock_StartAddr << " B\n";
	oss << "\tinodeλͼ��ʼλ�ã�" << superblock->s_InodeBitmap_StartAddr << " B\n";
	oss << "\tblockλͼ��ʼλ�ã�" << superblock->s_BlockBitmap_StartAddr << " B\n";
	oss << "\tinode����ʼλ�ã�" << superblock->s_Inode_StartAddr << " B\n";
	oss << "\tblock����ʼλ�ã�" << superblock->s_Block_StartAddr << " B\n";
	oss << "\n"; // ����һ������
	return;
}

void printInodeBitmap()	//��ӡinodeʹ�����
{
	
	printf("\n");
	printf("inodeʹ�ñ�[uesd:%d %d/%d]\n", superblock->s_INODE_NUM - superblock->s_free_INODE_NUM, superblock->s_free_INODE_NUM, superblock->s_INODE_NUM);
	oss << "\n";
	oss << "inodeʹ�ñ�[used:"
		<< (superblock->s_INODE_NUM - superblock->s_free_INODE_NUM) << " "
		<< superblock->s_free_INODE_NUM << "/" << superblock->s_INODE_NUM << "]\n";
	int i;
	i = 0;
	printf("0 ");
	oss << "0 ";
	while (i < superblock->s_INODE_NUM) {
		if (inode_bitmap[i])
		{
			printf("*");
			oss << "*";
		}
		else
		{
			printf(".");
			oss << ".";
		}
		i++;
		if (i != 0 && i % 32 == 0) {
			printf("\n");
			oss << "\n";
			if (i != superblock->s_INODE_NUM)
			{
				printf("%d ", i / 32);
				oss << (i / 32) << " ";
			}
		}
	}
	printf("\n");
	printf("\n");
	oss << "\n\n";
	return;
}

void printBlockBitmap(int num)	//��ӡblockʹ�����
{

	printf("\n");
	printf("block�����̿飩ʹ�ñ�[used:%d %d/%d]\n", superblock->s_BLOCK_NUM - superblock->s_free_BLOCK_NUM, superblock->s_free_BLOCK_NUM, superblock->s_BLOCK_NUM);
	int i;
	i = 0;
	printf("0 ");

	while (i < num) {
		if (block_bitmap[i])
		{
			printf("*");

		}
		else
		{
			printf(".");
	
		}
		i++;
		if (i != 0 && i % 32 == 0) {
			printf("\n");
			if (num == superblock->s_BLOCK_NUM)
				getchar();
			if (i != superblock->s_BLOCK_NUM)
				printf("%d ", i / 32);
		}
	}
	printf("\n");
	printf("\n");
	return;
}

int balloc()	//���̿���亯��
{
	//ʹ�ó������еĿ��п��ջ
	//���㵱ǰջ��
	int top;	//ջ��ָ��
	if (superblock->s_free_BLOCK_NUM == 0) {	//ʣ����п���Ϊ0
		printf("û�п��п���Է���\n");
		oss << "û�п��п���Է���\n";
		return -1;	//û�пɷ���Ŀ��п飬����-1
	}
	else {	//����ʣ���
		top = (superblock->s_free_BLOCK_NUM - 1) % superblock->s_blocks_per_group;
	}
	//��ջ��ȡ��
	//�������ջ�ף�����ǰ��ŵ�ַ���أ���Ϊջ�׿�ţ�����ջ��ָ����¿��п��ջ����ԭ����ջ
	int retAddr;

	if (top == 0) {
		retAddr = superblock->s_free_addr;
		superblock->s_free_addr = superblock->s_free[0];	//ȡ����һ�����п��п��ջ�Ŀ��п��λ�ã����¿��п��ջָ��

		//ȡ����Ӧ���п����ݣ�����ԭ���Ŀ��п��ջ

		//ȡ����һ�����п��ջ������ԭ����,��ָ���ƶ���superblock->s_free_addr
		//fseek(fr, superblock->s_free_addr, SEEK_SET);
		fseek(file, superblock->s_free_addr, SEEK_SET);
		//��һ����п��ջ�����ݴӴ����ļ��м��ص��ڴ�(superblock->s_free)��
		//fread(superblock->s_free, sizeof(superblock->s_free), 1, fr);
		fread(superblock->s_free, sizeof(superblock->s_free), 1, file);
		//fflush(fr);

		superblock->s_free_BLOCK_NUM--;

	}
	else {	//�����Ϊջ�ף���ջ��ָ��ĵ�ַ���أ�ջ��ָ��-1.
		retAddr = superblock->s_free[top];	//���淵�ص�ַ
		superblock->s_free[top] = -1;	//��ջ��
		top--;		//ջ��ָ��-1
		superblock->s_free_BLOCK_NUM--;	//���п���-1

	}

	//���³�����
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	//fflush(fw);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, file);
	fflush(file);


	//����blockλͼ
	block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE] = 1;
	//fseek(fw, (retAddr - Block_StartAddr) / BLOCK_SIZE + BlockBitmap_StartAddr, SEEK_SET);	//(retAddr-Block_StartAddr)/BLOCK_SIZEΪ�ڼ������п�
	//fwrite(&block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE], sizeof(bool), 1, fw);
	//fflush(fw);
	fseek(file, (retAddr - Block_StartAddr) / BLOCK_SIZE + BlockBitmap_StartAddr, SEEK_SET);	//(retAddr-Block_StartAddr)/BLOCK_SIZEΪ�ڼ������п�
	fwrite(&block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE], sizeof(bool), 1, file);
	fflush(file);

	return retAddr;

}

bool bfree(int addr)	//���̿��ͷź���
{
	//�ж�
	//�õ�ַ���Ǵ��̿����ʼ��ַ
	if ((addr - Block_StartAddr) % superblock->s_BLOCK_SIZE != 0) {
		printf("��ַ����,��λ�ò���block�����̿飩��ʼλ��\n");
		oss << "��ַ����,��λ�ò���block�����̿飩��ʼλ��\n";
		return false;
	}
	unsigned int bno = (addr - Block_StartAddr) / superblock->s_BLOCK_SIZE;	//inode�ڵ��
	//�õ�ַ��δʹ�ã������ͷſռ�
	if (block_bitmap[bno] == 0) {
		printf("��block�����̿飩��δʹ�ã��޷��ͷ�\n");
		oss << "��block�����̿飩��δʹ�ã��޷��ͷ�\n";
		return false;
	}

	//�����ͷ�
	//���㵱ǰջ��
	int top;	//ջ��ָ��
	if (superblock->s_free_BLOCK_NUM == superblock->s_BLOCK_NUM) {	//û�зǿ��еĴ��̿�
		printf("û�зǿ��еĴ��̿飬�޷��ͷ�\n");
		oss << "û�зǿ��еĴ��̿飬�޷��ͷ�\n";
		return false;	//û�пɷ���Ŀ��п飬����-1
	}//��˼�ǵ�ǰ���еĴ��̿������Ѿ��������̿�����һ��˵���������ͷ���
	else {	//����
		//��һ��Ϊ���ҵ�ջ��λ�ã�����1�ҵ��ľ�����һ��ջ����ʼλ��
		top = (superblock->s_free_BLOCK_NUM - 1) % superblock->s_blocks_per_group;

		//���block����
		char tmp[BLOCK_SIZE] = { 0 };
		//fseek(fw, addr, SEEK_SET);
		//fwrite(tmp, sizeof(tmp), 1, fw);
		fseek(file, addr, SEEK_SET);
		fwrite(tmp, sizeof(tmp), 1, file);
		fflush(file);
		if (top == superblock->s_blocks_per_group - 1) {	//��ջ����

			//�ÿ��п���Ϊ�µĿ��п��ջ
			superblock->s_free[0] = superblock->s_free_addr;	//�µĿ��п��ջ��һ����ַָ��ɵĿ��п��ջָ��
			int i;
			for (i = 1; i < superblock->s_blocks_per_group; i++) {
				superblock->s_free[i] = -1;	//���ջԪ�ص�������ַ
			}

			//fseek(fw, addr, SEEK_SET);
			//fwrite(superblock->s_free, sizeof(superblock->s_free), 1, fw);	//����������̿飬512�ֽ�
			fseek(file, addr, SEEK_SET);
			fwrite(superblock->s_free, sizeof(superblock->s_free), 1, file);	//����������̿飬512�ֽ�
			fflush(file);
		}
		else {	//ջ��δ��
			top++;	//ջ��ָ��+1
			superblock->s_free[top] = addr;	//ջ���������Ҫ�ͷŵĵ�ַ����Ϊ�µĿ��п�
		}
	}


	//���³�����
	superblock->s_free_BLOCK_NUM++;	//���п���+1
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, file);

	//����blockλͼ
	block_bitmap[bno] = 0;
	//fseek(fw, bno + BlockBitmap_StartAddr, SEEK_SET);	//(addr-Block_StartAddr)/BLOCK_SIZEΪ�ڼ������п�
	//fwrite(&block_bitmap[bno], sizeof(bool), 1, fw);
	//fflush(fw);
	fseek(file, bno + BlockBitmap_StartAddr, SEEK_SET);	//(addr-Block_StartAddr)/BLOCK_SIZEΪ�ڼ������п�
	fwrite(&block_bitmap[bno], sizeof(bool), 1, file);
	fflush(file);
	return true;
}

int ialloc()	//����i�ڵ�������������inode��ַ
{
	//��inodeλͼ��˳����ҿ��е�inode���ҵ��򷵻�inode��ַ������������
	if (superblock->s_free_INODE_NUM == 0) {
		printf("û�п���inode���Է���\n");
		oss << "û�п���inode���Է���\n";
		return -1;
	}
	else {

		//˳����ҿ��е�inode
		int i;
		for (i = 0; i < superblock->s_INODE_NUM; i++) {
			if (inode_bitmap[i] == 0)	//�ҵ�����inode
				break;
		}


		//���³�����
		superblock->s_free_INODE_NUM--;	//����inode��-1
		//fseek(fw, Superblock_StartAddr, SEEK_SET);
		//fwrite(superblock, sizeof(SuperBlock), 1, fw);
		fseek(file, Superblock_StartAddr, SEEK_SET);
		fwrite(superblock, sizeof(SuperBlock), 1, file);



		//����inodeλͼ
		inode_bitmap[i] = 1;
		//fseek(fw, InodeBitmap_StartAddr + i, SEEK_SET);
		//fwrite(&inode_bitmap[i], sizeof(bool), 1, fw);
		//fflush(fw);

		fseek(file, InodeBitmap_StartAddr + i, SEEK_SET);
		fwrite(&inode_bitmap[i], sizeof(bool), 1, file);
		fflush(file);

		return Inode_StartAddr + i * superblock->s_INODE_SIZE;
	}
}

bool ifree(int addr)	//�ͷ�i���������
{
	//�ж�
	if ((addr - Inode_StartAddr) % superblock->s_INODE_SIZE != 0) {
		printf("��ַ����,��λ�ò���i�ڵ���ʼλ��\n");
		oss << "��ַ����,��λ�ò���i�ڵ���ʼλ��\n";
		return false;
	}
	unsigned short ino = (addr - Inode_StartAddr) / superblock->s_INODE_SIZE;	//inode�ڵ��
	if (inode_bitmap[ino] == 0) {
		printf("��inode��δʹ�ã��޷��ͷ�\n");
		oss << "��inode��δʹ�ã��޷��ͷ�\n";
		return false;
	}

	//���inode����
	Inode tmp = { 0 };
	//fseek(fw, addr, SEEK_SET);
	//fwrite(&tmp, sizeof(tmp), 1, fw);
	fseek(file, addr, SEEK_SET);
	fwrite(&tmp, sizeof(tmp), 1, file);

	//���³�����
	superblock->s_free_INODE_NUM++;
	//����inode��+1
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, file);


	//����inodeλͼ
	inode_bitmap[ino] = 0;
	//fseek(fw, InodeBitmap_StartAddr + ino, SEEK_SET);
	//fwrite(&inode_bitmap[ino], sizeof(bool), 1, fw);
	//fflush(fw);
	fseek(file, InodeBitmap_StartAddr + ino, SEEK_SET);
	fwrite(&inode_bitmap[ino], sizeof(bool), 1, file);
	fflush(file);

	return true;
}

//�ڲ���ǰ�����һ��constֵ
bool mkdir(int parinoAddr, const char name[])	//Ŀ¼������������������һ��Ŀ¼�ļ�inode��ַ ,Ҫ������Ŀ¼��
{
	
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("�������Ŀ¼������\n");
		oss << "�������Ŀ¼������\n";
		return false;
	}

	DirItem dirlist[16];	//��ʱĿ¼�嵥

	//�������ַȡ��inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	std::cout << cur.i_ino << " " << cur.i_mode << " " << cur.i_cnt << "��" << cur.i_dirBlock[0] << endl;//ȫ��0���·���ûд���Ŀ¼inode�ڵ���Ϣһ��
	int i = 0;
	int cnt = cur.i_cnt + 1;	//Ŀ¼����
	int posi = -1, posj = -1;
	while (i < 160) {
		//160��Ŀ¼��֮�ڣ�����ֱ����ֱ�ӿ�����
		int dno = i / 16;	//�ڵڼ���ֱ�ӿ���

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����ֱ�ӿ飬Ҫ�����Ŀ¼��Ŀ��λ��
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);


		//����ô��̿��е�����Ŀ¼��
		int j;
		for (j = 0; j < 16; j++) {

			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 1) {	//����Ŀ¼ //�����ע�ͱ�ʾ��Ŀ¼
					printf("Ŀ¼�Ѵ���\n");
					oss << "Ŀ¼�Ѵ���\n";
					return false;
				}
			}
			else if (strcmp(dirlist[j].itemName, "") == 0) {
				//�ҵ�һ�����м�¼������Ŀ¼���������λ�� 
				//��¼���λ��
				if (posi == -1) {
					//posi��ʾ�����ڵڼ���ֱ�ӿ���
					posi = dno;
					//��ʾ���Ǵ��̿���ĵڼ���Ŀ¼��
					posj = j;

				}

			}

			i++;
		}

	}
	/*  δд�� */

	if (posi != -1) {	//�ҵ��������λ��

		//ȡ�����ֱ�ӿ飬Ҫ�����Ŀ¼��Ŀ��λ��
		//fseek(fr, cur.i_dirBlock[posi], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);


		//�������Ŀ¼��
		strcpy(dirlist[posj].itemName, name);	//Ŀ¼��
		//д��������¼ "." ".."���ֱ�ָ��ǰinode�ڵ��ַ���͸�inode�ڵ�
		int chiinoAddr = ialloc();	//���䵱ǰ�ڵ��ַ 
		if (chiinoAddr == -1) {
			printf("inode����ʧ��\n");
			oss << "inode����ʧ��\n";
			return false;
		}
		dirlist[posj].inodeAddr = chiinoAddr; //������µ�Ŀ¼�����inode��ַ

		//��������Ŀ��inode
		Inode p;
		p.i_ino = (chiinoAddr - Inode_StartAddr) / superblock->s_INODE_SIZE;
		p.i_atime = time(NULL);
		p.i_ctime = time(NULL);
		p.i_mtime = time(NULL);
		strcpy(p.i_uname, Cur_User_Name);
		strcpy(p.i_gname, Cur_Group_Name);
		p.i_cnt = 2;	//�������ǰĿ¼,"."��".."

		//�������inode�Ĵ��̿飬�ڴ��̺���д��������¼ . �� ..
		int curblockAddr = balloc();
		if (curblockAddr == -1) {
			printf("block����ʧ��\n");
			oss << "block����ʧ��\n";
			return false;
		}
		DirItem dirlist2[16] = { 0 };	//��ʱĿ¼���б� - 2
		strcpy(dirlist2[0].itemName, ".");
		strcpy(dirlist2[1].itemName, "..");
		dirlist2[0].inodeAddr = chiinoAddr;	//��ǰĿ¼inode��ַ
		dirlist2[1].inodeAddr = parinoAddr;	//��Ŀ¼inode��ַ

		//д�뵽��ǰĿ¼�Ĵ��̿�
		//fseek(fw, curblockAddr, SEEK_SET);
		//fwrite(dirlist2, sizeof(dirlist2), 1, fw);
		fseek(file, curblockAddr, SEEK_SET);
		fwrite(dirlist2, sizeof(dirlist2), 1, file);
		//
		fflush(file);
		//
		p.i_dirBlock[0] = curblockAddr;
		int k;
		for (k = 1; k < 10; k++) {
			p.i_dirBlock[k] = -1;
		}
		p.i_size = superblock->s_BLOCK_SIZE;
		p.i_indirBlock_1 = -1;	//ûʹ��һ����ӿ�
		p.i_mode = MODE_DIR | DIR_DEF_PERMISSION;

		//��inodeд�뵽�����inode��ַ
		//fseek(fw, chiinoAddr, SEEK_SET);
		//fwrite(&p, sizeof(Inode), 1, fw);
		fseek(file, chiinoAddr, SEEK_SET);
		fwrite(&p, sizeof(Inode), 1, file);
		//
		fflush(file);
		//
		//����ǰĿ¼�Ĵ��̿�д��
		//fseek(fw, cur.i_dirBlock[posi], SEEK_SET);
		//fwrite(dirlist, sizeof(dirlist), 1, fw);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fwrite(dirlist, sizeof(dirlist), 1, file);
		//
		fflush(file);
		// 
		//д��inode
		cur.i_cnt++;
		//fseek(fw, parinoAddr, SEEK_SET);
		//fwrite(&cur, sizeof(Inode), 1, fw);
		//fflush(fw);
		fseek(file, parinoAddr, SEEK_SET);
		fwrite(&cur, sizeof(Inode), 1, file);
		fflush(file);
		return true;
	}
	else {
		printf("û�ҵ�����Ŀ¼��,Ŀ¼����ʧ��");
		oss << "û�ҵ�����Ŀ¼��,Ŀ¼����ʧ��";
		return false;
	}
}

void rmall(int parinoAddr)	//ɾ���ýڵ��������ļ���Ŀ¼
{
	//�������ַȡ��inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;
	if (cnt <= 2) {
		bfree(cur.i_dirBlock[0]);
		ifree(parinoAddr);
		return;
	}

	//����ȡ�����̿�
	int i = 0;
	while (i < 160) {	//С��160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		//�Ӵ��̿�������ȡ��Ŀ¼��ݹ�ɾ��
		int j;
		bool f = false;
		for (j = 0; j < 16; j++) {
			//Inode tmp;

			if (!(strcmp(dirlist[j].itemName, ".") == 0 ||
				strcmp(dirlist[j].itemName, "..") == 0 ||
				strcmp(dirlist[j].itemName, "") == 0)) {
				f = true;
				rmall(dirlist[j].inodeAddr);	//�ݹ�ɾ��
			}

			cnt = cur.i_cnt;
			i++;
		}

		//����ط����ɻ�
		//�ô��̿��ѿգ�����
		if (f)
			bfree(parblockAddr);

	}
	//��inode�ѿգ�����
	ifree(parinoAddr);
	return;

}

bool rmdir(int parinoAddr, char name[])	//Ŀ¼ɾ������
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("�������Ŀ¼������\n");
		oss << "�������Ŀ¼������\n";
		return false;
	}

	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		printf("�������\n");
		oss << "�������\n";
		return 0;
	}

	//�������ַȡ��inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if ((((cur.i_mode >> filemode >> 1) & 1) == 0) && (strcmp(Cur_User_Name, "root") != 0)) {
		//û��д��Ȩ��
		printf("Ȩ�޲��㣺��д��Ȩ��\n");
		oss << "Ȩ�޲��㣺��д��Ȩ��\n";
		return false;
	}


	//����ȡ�����̿�
	int i = 0;
	while (i < 160) {	//С��160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//�ҵ�Ҫɾ����Ŀ¼
		int j;
		for (j = 0; j < 16; j++) {
			Inode tmp;
			//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
			//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
			//fread(&tmp, sizeof(Inode), 1, fr);
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, name) == 0) {
				if (((tmp.i_mode >> 9) & 1) == 1) {	//�ҵ�Ŀ¼
					//��Ŀ¼

					rmall(dirlist[j].inodeAddr);

					//ɾ����Ŀ¼��Ŀ��д�ش���
					strcpy(dirlist[j].itemName, "");
					dirlist[j].inodeAddr = -1;
					//fseek(fw, parblockAddr, SEEK_SET);
					//fwrite(&dirlist, sizeof(dirlist), 1, fw);
					fseek(file, parblockAddr, SEEK_SET);
					fwrite(&dirlist, sizeof(dirlist), 1, file);
					cur.i_cnt--;
					//fseek(fw, parinoAddr, SEEK_SET);
					//fwrite(&cur, sizeof(Inode), 1, fw);
					fseek(file, parinoAddr, SEEK_SET);
					fwrite(&cur, sizeof(Inode), 1, file);
					fflush(file);
					return true;
				}
				else {
					//����Ŀ¼������
				}
			}
			i++;
		}

	}

	printf("û���ҵ���Ŀ¼\n");
	oss << "û���ҵ���Ŀ¼\n";
	return false;
}

bool create(int parinoAddr, const char name[], char buf[])	//�����ļ��������ڸ�Ŀ¼�´����ļ����ļ����ݴ���buf
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("��������ļ�������\n");
		oss << "��������ļ�������\n";
		return false;
	}

	DirItem dirlist[16];	//��ʱĿ¼�嵥

	//�������ַȡ��inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	int i = 0;
	int posi = -1, posj = -1;	//�ҵ���Ŀ¼λ��
	int dno;
	int cnt = cur.i_cnt + 1;	//Ŀ¼����
	while (i < 160) {
		//160��Ŀ¼��֮�ڣ�����ֱ����ֱ�ӿ�����
		dno = i / 16;	//�ڵڼ���ֱ�ӿ���

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);



		//����ô��̿��е�����Ŀ¼��
		int j;
		for (j = 0; j < 16; j++) {

			if (posi == -1 && strcmp(dirlist[j].itemName, "") == 0) {
				//�ҵ�һ�����м�¼�������ļ����������λ�� 
				posi = dno;
				posj = j;
			}
			else if (strcmp(dirlist[j].itemName, name) == 0) {
				//������ȡ��inode���ж��Ƿ����ļ�
				Inode cur2;
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&cur2, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&cur2, sizeof(Inode), 1, file);
				if (((cur2.i_mode >> 9) & 1) == 0) {	//���ļ������������ܴ����ļ�
					printf("�ļ��Ѵ���\n");
					oss << "�ļ��Ѵ���\n";
					buf[0] = '\0';
					return false;
				}
			}
			i++;
		}

	}
	if (posi != -1) {	//֮ǰ�ҵ�һ��Ŀ¼����
		//ȡ��֮ǰ�Ǹ�����Ŀ¼���Ӧ�Ĵ��̿�
		//fseek(fr, cur.i_dirBlock[posi], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);


		//�������Ŀ¼��
		strcpy(dirlist[posj].itemName, name);	//�ļ���
		int chiinoAddr = ialloc();	//���䵱ǰ�ڵ��ַ 
		if (chiinoAddr == -1) {
			printf("inode����ʧ��\n");
			oss << "inode����ʧ��\n";
			return false;
		}
		dirlist[posj].inodeAddr = chiinoAddr; //������µ�Ŀ¼�����inode��ַ

		//��������Ŀ��inode
		Inode p;
		p.i_ino = (chiinoAddr - Inode_StartAddr) / superblock->s_INODE_SIZE;
		p.i_atime = time(NULL);
		p.i_ctime = time(NULL);
		p.i_mtime = time(NULL);
		strcpy(p.i_uname, Cur_User_Name);
		strcpy(p.i_gname, Cur_Group_Name);
		p.i_cnt = 1;	//ֻ��һ���ļ�ָ��


		//��buf���ݴ浽���̿� 
		int k;
		int len = strlen(buf);	//�ļ����ȣ���λΪ�ֽ�
		for (k = 0; k < len; k += superblock->s_BLOCK_SIZE) {	//���10�Σ�10�����̿죬�����5K
			//�������inode�Ĵ��̿飬�ӿ���̨��ȡ����
			int curblockAddr = balloc();
			if (curblockAddr == -1) {
				printf("block����ʧ��\n");
				oss << "block����ʧ��\n";
				return false;
			}
			p.i_dirBlock[k / superblock->s_BLOCK_SIZE] = curblockAddr;
			//д�뵽��ǰĿ¼�Ĵ��̿�
			//fseek(fw, curblockAddr, SEEK_SET);
			//fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, fw);
			fseek(file, curblockAddr, SEEK_SET);
			fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, file);
			fflush(file);
		}


		for (k = len / superblock->s_BLOCK_SIZE + 1; k < 10; k++) {
			p.i_dirBlock[k] = -1;
		}
		if (len == 0) {	//����Ϊ0�Ļ�Ҳ�ָ���һ��block
			int curblockAddr = balloc();
			if (curblockAddr == -1) {
				printf("block����ʧ��\n");
				oss << "block����ʧ��\n";
				return false;
			}
			p.i_dirBlock[k / superblock->s_BLOCK_SIZE] = curblockAddr;
			//д�뵽��ǰĿ¼�Ĵ��̿�
			//fseek(fw, curblockAddr, SEEK_SET);
			//fwrite(buf, superblock->s_BLOCK_SIZE, 1, fw);
			fseek(file, curblockAddr, SEEK_SET);
			fwrite(buf, superblock->s_BLOCK_SIZE, 1, file);
			fflush(file);
		}
		p.i_size = len;
		p.i_indirBlock_1 = -1;	//ûʹ��һ����ӿ�
		p.i_mode = 0;
		p.i_mode = MODE_FILE | FILE_DEF_PERMISSION;

		//��inodeд�뵽�����inode��ַ
		//fseek(fw, chiinoAddr, SEEK_SET);
		//fwrite(&p, sizeof(Inode), 1, fw);
		fseek(file, chiinoAddr, SEEK_SET);
		fwrite(&p, sizeof(Inode), 1, file);


		//����ǰĿ¼�Ĵ��̿�д��
		//fseek(fw, cur.i_dirBlock[posi], SEEK_SET);
		//fwrite(dirlist, sizeof(dirlist), 1, fw);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fwrite(dirlist, sizeof(dirlist), 1, file);


		//д��inode
		cur.i_cnt++;
		//fseek(fw, parinoAddr, SEEK_SET);
		//fwrite(&cur, sizeof(Inode), 1, fw);
		//fflush(fw);
		fseek(file, parinoAddr, SEEK_SET);
		fwrite(&cur, sizeof(Inode), 1, file);
		fflush(file);
		return true;
	}
	else
		return false;
}

bool del(int parinoAddr, char name[])		//ɾ���ļ��������ڵ�ǰĿ¼��ɾ���ļ�
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("�������Ŀ¼������\n");
		oss << "�������Ŀ¼������\n";
		return false;
	}

	//�������ַȡ��inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if (((cur.i_mode >> filemode >> 1) & 1) == 0) {
		//û��д��Ȩ��
		printf("Ȩ�޲��㣺��д��Ȩ��\n");
		oss << "Ȩ�޲��㣺��д��Ȩ��\n";
		return false;
	}

	//����ȡ�����̿�
	int i = 0;
	while (i < 160) {	//С��160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		//�ҵ�Ҫɾ����Ŀ¼
		int pos;
		for (pos = 0; pos < 16; pos++) {
			Inode tmp;
			//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
			//fseek(fr, dirlist[pos].inodeAddr, SEEK_SET);
			//fread(&tmp, sizeof(Inode), 1, fr);
			fseek(file, dirlist[pos].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);
			if (strcmp(dirlist[pos].itemName, name) == 0) {
				if (((tmp.i_mode >> 9) & 1) == 1) {	//�ҵ�Ŀ¼
					//��Ŀ¼������
				}
				else {
					//���ļ�

					//�ͷ�block
					int k;
					for (k = 0; k < 10; k++)
						if (tmp.i_dirBlock[k] != -1)
							bfree(tmp.i_dirBlock[k]);

					//�ͷ�inode
					ifree(dirlist[pos].inodeAddr);

					//ɾ����Ŀ¼��Ŀ��д�ش���
					strcpy(dirlist[pos].itemName, "");
					dirlist[pos].inodeAddr = -1;
					//fseek(fw, parblockAddr, SEEK_SET);
					//fwrite(&dirlist, sizeof(dirlist), 1, fw);
					fseek(file, parblockAddr, SEEK_SET);
					fwrite(&dirlist, sizeof(dirlist), 1, file);
					cur.i_cnt--;
					//fseek(fw, parinoAddr, SEEK_SET);
					//fwrite(&cur, sizeof(Inode), 1, fw);
					fseek(file, parinoAddr, SEEK_SET);
					fwrite(&cur, sizeof(Inode), 1, file);
					//fflush(fw);
					fflush(file);
					return true;
				}
			}
			i++;
		}

	}

	printf("û���ҵ����ļ�!\n");
	oss << "û���ҵ����ļ�!\n";
	return false;
}


bool ls(int parinoAddr)		//��ʾ��ǰĿ¼�µ������ļ����ļ��С���������ǰĿ¼��inode�ڵ��ַ 
{
	Inode cur;
	//ȡ�����inode
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	//fflush(fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//fflush(fr);
	//cout << cur.i_cnt << "����" << std::endl;
	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if (((cur.i_mode >> filemode >> 2) & 1) == 0) {
		//û�ж�ȡȨ��
		printf("Ȩ�޲��㣺�޶�ȡȨ��\n");
		oss << "Ȩ�޲��㣺�޶�ȡȨ��\n";
		return false;
	}

	//����ȡ�����̿�
	int i = 0;
	while (i < cnt && i < 160) {
		DirItem dirlist[16] = { 0 };
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);


		//����ô��̿��е�����Ŀ¼��
		int j;
		for (j = 0; j < 16 && i < cnt; j++) {
			Inode tmp;
			//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
			//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
			//fread(&tmp, sizeof(Inode), 1, fr);
			//fflush(fr);
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, "") == 0) {
				continue;
			}

			//�����Ϣ
			if (((tmp.i_mode >> 9) & 1) == 1) {
				printf("d");
				oss << "d";
			}
			else {
				printf("-");
				oss << "-";
			}
			//cout << tmp.i_mtime << "���Ҳ����" << std::endl;
			tm* ptr;	//�洢ʱ��
			ptr = gmtime(&tmp.i_mtime);

			//���Ȩ����Ϣ
			int t = 8;
			while (t >= 0) {
				if (((tmp.i_mode >> t) & 1) == 1) {
					if (t % 3 == 2) {
						printf("r");
						oss << "r";
					}
					if (t % 3 == 1) {
						printf("w");
						oss << "w";
					}
					if (t % 3 == 0) {
						printf("x");
						oss << "x";
					}
				}
				else {
					printf("-");
					oss << "-";
				}
				t--;
			}
			printf("\t");
			oss << "\t";

			//����
			printf("%d\t", tmp.i_cnt);	//����
			oss << tmp.i_cnt << "\t";
			printf("%s\t", tmp.i_uname);	//�ļ������û���
			oss << tmp.i_uname << "\t";
			printf("%s\t", tmp.i_gname);	//�ļ������û���
			oss << tmp.i_gname << "\t";
			printf("%d B\t", tmp.i_size);	//�ļ���С
			oss << tmp.i_size << " B\t";
			if (ptr != nullptr)
			{
				printf("%d.%d.%d %02d:%02d:%02d  ", 1900 + ptr->tm_year, ptr->tm_mon + 1, ptr->tm_mday, (8 + ptr->tm_hour) % 24, ptr->tm_min, ptr->tm_sec);	//��һ���޸ĵ�ʱ��
		
				oss << (1900 + ptr->tm_year) << "."
					<< (ptr->tm_mon + 1) << "."
					<< ptr->tm_mday << " "
					<< std::setfill('0') << std::setw(2) << (8 + ptr->tm_hour) % 24 << ":"
					<< std::setw(2) << ptr->tm_min << ":"
					<< std::setw(2) << ptr->tm_sec<<"\t";
			}
			printf("%s", dirlist[j].itemName);	//�ļ���
			oss << dirlist[j].itemName;
			printf("\n");
			oss << "\n";
			i++;
		}

	}
	/*  δд�� */
	return true;
}

bool cd(int parinoAddr, const char name[],bool isRelativePath)	//���뵱ǰĿ¼�µ�nameĿ¼
{
	//ȡ����ǰĿ¼��inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//����ȡ��inode��Ӧ�Ĵ��̿飬������û������Ϊname��Ŀ¼��
	int i = 0;

	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	while (i < 160) {
		DirItem dirlist[16] = { 0 };
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		int j;
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 1) {
					//�ҵ���Ŀ¼���ж��Ƿ���н���Ȩ��
					if (((tmp.i_mode >> filemode >> 0) & 1) == 0 && strcmp(Cur_User_Name, "root") != 0) {	//root�û�����Ŀ¼�����Բ鿴 
						//û��ִ��Ȩ��
						printf("Ȩ�޲��㣺��ִ��Ȩ��\n");
						oss << "Ȩ�޲��㣺��ִ��Ȩ��\n";
						return false;
					}

					//�ҵ���Ŀ¼������Ŀ¼��������ǰĿ¼

					Cur_Dir_Addr = dirlist[j].inodeAddr;
					if (isRelativePath)//����ģʽ����ʼ��ʱ�����·����Ҫ�����Ե�ȥ�ı�·������������������ʱ���������������������·���Լ���ǰĿ¼���оͲ���Ҫ�����
					{
						if (strcmp(dirlist[j].itemName, ".") == 0) {
							//��Ŀ¼������
						}
						else if (strcmp(dirlist[j].itemName, "..") == 0) {
							//��һ��Ŀ¼
							int k;
							for (k = strlen(Cur_Dir_Name); k >= 0; k--)
								if (Cur_Dir_Name[k] == '/')
									break;
							Cur_Dir_Name[k] = '\0';
							if (strlen(Cur_Dir_Name) == 0)
								Cur_Dir_Name[0] = '/', Cur_Dir_Name[1] = '\0';
						}
						else {
							if (Cur_Dir_Name[strlen(Cur_Dir_Name) - 1] != '/')
								strcat(Cur_Dir_Name, "/");
							strcat(Cur_Dir_Name, dirlist[j].itemName);
						}
					}
					return true;
				}
				else {
					//�ҵ���Ŀ¼��������Ŀ¼��������
				}

			}

			i++;
		}

	}

	//û�ҵ�
	printf("û�и�Ŀ¼\n");
	oss << "û�и�Ŀ¼\n";
	return false;

}

void gotoxy(HANDLE hOut, int x, int y)	//�ƶ���굽ָ��λ��
{
	COORD pos;
	pos.X = x;             //������
	pos.Y = y;            //������
	SetConsoleCursorPosition(hOut, pos);
}

void vi(int parinoAddr, char name[], char buf[])	//ģ��һ����vi�������ı���nameΪ�ļ���
{
	//���ж��ļ��Ƿ��Ѵ��ڡ�������ڣ�������ļ����༭
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("��������ļ�������\n");
		return;
	}

	//��ջ�����
	memset(buf, 0, sizeof(buf));
	int maxlen = 0;	//���������󳤶�

	//��������ͬ���ļ����еĻ�����༭ģʽ��û�н��봴���ļ�ģʽ
	DirItem dirlist[16];	//��ʱĿ¼�嵥

	//�������ַȡ��inode
	Inode cur, fileInode;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//�ļ���inode��ַ
	bool isExist = false;	//�ļ��Ƿ��Ѵ���
	while (i < 160) {
		//160��Ŀ¼��֮�ڣ�����ֱ����ֱ�ӿ�����
		dno = i / 16;	//�ڵڼ���ֱ�ӿ���

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//������ȡ��inode���ж��Ƿ����ļ�
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&fileInode, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//���ļ���������������ļ������༭	
					fileInodeAddr = dirlist[j].inodeAddr;
					isExist = true;
					goto label;
				}
			}
			i++;
		}
	}
label:

	//��ʼ��vi
	int cnt = 0;
	system("cls");	//����

	int winx, winy, curx, cury;

	HANDLE handle_out;                              //����һ�����  
	CONSOLE_SCREEN_BUFFER_INFO screen_info;         //���崰�ڻ�������Ϣ�ṹ��  
	COORD pos = { 0, 0 };                             //����һ������ṹ��

	if (isExist) {	//�ļ��Ѵ��ڣ�����༭ģʽ�������֮ǰ���ļ�����

		//Ȩ���жϡ��ж��ļ��Ƿ�ɶ�
		if (((fileInode.i_mode >> filemode >> 2) & 1) == 0) {
			//���ɶ�
			printf("Ȩ�޲��㣺û�ж�Ȩ��\n");
			return;
		}

		//���ļ����ݶ�ȡ��������ʾ�ڣ�������
		i = 0;
		int sumlen = fileInode.i_size;	//�ļ�����
		int getlen = 0;	//ȡ�����ĳ���
		for (i = 0; i < 10; i++) {
			char fileContent[1000] = { 0 };
			if (fileInode.i_dirBlock[i] == -1) {
				continue;
			}
			//����ȡ�����̿������
			//fseek(fr, fileInode.i_dirBlock[i], SEEK_SET);
			//fread(fileContent, superblock->s_BLOCK_SIZE, 1, fr);	//��ȡ��һ�����̿��С������
			//fflush(fr);
			fseek(file, fileInode.i_dirBlock[i], SEEK_SET);
			fread(fileContent, superblock->s_BLOCK_SIZE, 1, file);	//��ȡ��һ�����̿��С������

			//����ַ���
			int curlen = 0;	//��ǰָ��
			while (curlen < superblock->s_BLOCK_SIZE) {
				if (getlen >= sumlen)	//ȫ��������
					break;
				printf("%c", fileContent[curlen]);	//�������Ļ 
				buf[cnt++] = fileContent[curlen];	//�����buf
				curlen++;
				getlen++;
			}
			if (getlen >= sumlen)
				break;
		}
		maxlen = sumlen;
	}

	//������֮��Ĺ��λ��
	handle_out = GetStdHandle(STD_OUTPUT_HANDLE);   //��ñ�׼����豸���  
	GetConsoleScreenBufferInfo(handle_out, &screen_info);   //��ȡ������Ϣ  
	winx = screen_info.srWindow.Right - screen_info.srWindow.Left + 1;
	winy = screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1;
	curx = screen_info.dwCursorPosition.X;
	cury = screen_info.dwCursorPosition.Y;


	//����vi
	//����vi��ȡ�ļ�����

	int mode = 0;	//viģʽ��һ��ʼ������ģʽ
	unsigned char c;
	while (1) {
		if (mode == 0) {	//������ģʽ
			c = getch();

			if (c == 'i' || c == 'a') {	//����ģʽ
				if (c == 'a') {
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						/*
						if(cury>winy-2 || cury%(winy-1)==winy-2){
							//������һ�������·�ҳ
							if(cury%(winy-1)==winy-2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
							int i;
							for(i=0;i<winx-1;i++)
								printf(" ");
							gotoxy(handle_out,0,cury+1);
							printf(" - ����ģʽ - ");
							gotoxy(handle_out,0,cury);
						}
						*/
					}
				}

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//������һ�������·�ҳ
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - ����ģʽ - ");
					gotoxy(handle_out, 0, cury);
				}
				else {
					//��ʾ "����ģʽ"
					gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, winy - 1);
					printf(" - ����ģʽ - ");
					gotoxy(handle_out, curx, cury);
				}

				gotoxy(handle_out, curx, cury);
				mode = 1;


			}
			else if (c == ':') {
				//system("color 09");//�����ı�Ϊ��ɫ
				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);
				_COORD pos;
				if (cury - winy + 2 > 0)
					pos.X = 0, pos.Y = cury + 1;
				else
					pos.X = 0, pos.Y = winy - 1;
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");

				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);

				WORD att = BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY; // �ı�����
				//FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//����̨������ɫ 
				//SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//�����ı���ɫ
				printf(":");

				char pc;
				int tcnt = 1;	//������ģʽ������ַ�����
				while (c = getch()) {
					if (c == '\r') {	//�س�
						break;
					}
					else if (c == '\b') {	//�˸񣬴�������ɾ��һ���ַ� 
						//SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
						tcnt--;
						if (tcnt == 0)
							break;
						printf("\b");
						printf(" ");
						printf("\b");
						continue;
					}
					pc = c;
					printf("%c", pc);
					tcnt++;
				}
				if (pc == 'q') {
					buf[cnt] = '\0';
					//buf[maxlen] = '\0'; 
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					system("cls");
					break;	//vi >>>>>>>>>>>>>> �˳�����
				}
				else {
					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");

					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//�����ı���ɫ
					FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//����̨������ɫ
					printf(" ��������");
					//getch();
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					gotoxy(handle_out, curx, cury);
				}
			}
			else if (c == 27) {	//ESC��������ģʽ����״̬��
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				gotoxy(handle_out, curx, cury);

			}

		}
		else if (mode == 1) {	//����ģʽ

			gotoxy(handle_out, winx / 4 * 3, winy - 1);
			int i = winx / 4 * 3;
			while (i < winx - 1) {
				printf(" ");
				i++;
			}
			if (cury > winy - 2)
				gotoxy(handle_out, winx / 4 * 3, cury + 1);
			else
				gotoxy(handle_out, winx / 4 * 3, winy - 1);
			printf("[��:%d,��:%d]", curx == -1 ? 0 : curx, cury);
			gotoxy(handle_out, curx, cury);

			c = getch();
			if (c == 27) {	// ESC����������ģʽ
				mode = 0;
				//��״̬��
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				continue;
			}
			else if (c == '\b') {	//�˸�ɾ��һ���ַ�
				if (cnt == 0)	//�Ѿ��˵��ʼ
					continue;
				printf("\b");
				printf(" ");
				printf("\b");
				curx--;
				cnt--;	//ɾ���ַ�
				if (buf[cnt] == '\n') {
					//Ҫɾ��������ַ��ǻس������ص���һ��
					if (cury != 0)
						cury--;
					int k;
					curx = 0;
					for (k = cnt - 1; buf[k] != '\n' && k >= 0; k--)
						curx++;
					gotoxy(handle_out, curx, cury);
					printf(" ");
					gotoxy(handle_out, curx, cury);
					if (cury - winy + 2 >= 0) {	//��ҳʱ
						gotoxy(handle_out, curx, 0);
						gotoxy(handle_out, curx, cury + 1);
						SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
						int i;
						for (i = 0; i < winx - 1; i++)
							printf(" ");
						gotoxy(handle_out, 0, cury + 1);
						printf(" - ����ģʽ - ");

					}
					gotoxy(handle_out, curx, cury);

				}
				else
					buf[cnt] = ' ';
				continue;
			}
			else if (c == 224) {	//�ж��Ƿ��Ǽ�ͷ
				c = getch();
				if (c == 75) {	//���ͷ
					if (cnt != 0) {
						cnt--;
						curx--;
						if (buf[cnt] == '\n') {
							//��һ���ַ��ǻس�
							if (cury != 0)
								cury--;
							int k;
							curx = 0;
							for (k = cnt - 1; buf[k] != '\n' && k >= 0; k--)
								curx++;
						}
						gotoxy(handle_out, curx, cury);
					}
				}
				else if (c == 77) {	//�Ҽ�ͷ
					cnt++;
					if (cnt > maxlen)
						maxlen = cnt;
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
							//������һ�������·�ҳ
							if (cury % (winy - 1) == winy - 2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
							int i;
							for (i = 0; i < winx - 1; i++)
								printf(" ");
							gotoxy(handle_out, 0, cury + 1);
							printf(" - ����ģʽ - ");
							gotoxy(handle_out, 0, cury);
						}

					}
					gotoxy(handle_out, curx, cury);
				}
				continue;
			}
			if (c == '\r') {	//�����س�
				printf("\n");
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//������һ�������·�ҳ
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - ����ģʽ - ");
					gotoxy(handle_out, 0, cury);
				}

				buf[cnt++] = '\n';
				if (cnt > maxlen)
					maxlen = cnt;
				continue;
			}
			else {
				printf("%c", c);
			}
			//�ƶ����
			curx++;
			if (curx == winx) {
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//������һ�������·�ҳ
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // �ָ�ԭ��������
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - ����ģʽ - ");
					gotoxy(handle_out, 0, cury);
				}

				buf[cnt++] = '\n';
				if (cnt > maxlen)
					maxlen = cnt;
				if (cury == winy) {
					printf("\n");
				}
			}
			//��¼�ַ� 
			buf[cnt++] = c;
			if (cnt > maxlen)
				maxlen = cnt;
		}
		else {	//����ģʽ
		}
	}
	if (isExist) {	//����Ǳ༭ģʽ
		//��buf����д���ļ��Ĵ��̿�

		if (((fileInode.i_mode >> filemode >> 1) & 1) == 1) {	//��д
			writefile(fileInodeAddr, buf);
		}
		else {	//����д
			printf("Ȩ�޲��㣺��д��Ȩ��\n");
		}

	}
	else {	//�Ǵ����ļ�ģʽ
		if (((cur.i_mode >> filemode >> 1) & 1) == 1) {
			//��д�����Դ����ļ�
			create(parinoAddr, name, buf);	//�����ļ�
		}
		else {
			printf("Ȩ�޲��㣺��д��Ȩ��\n");
			return;
		}
	}
}

void writefile(int fileInodeAddr, char buf[])	//��buf����д���ļ��Ĵ��̿�
{	
	Inode fileInode;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&fileInode, sizeof(Inode), 1, file);
	//��buf����д�ش��̿� 
	int k;
	int len = strlen(buf);	//�ļ����ȣ���λΪ�ֽ�
	for (k = 0; k < len; k += superblock->s_BLOCK_SIZE) {	//���10�Σ�10�����̿죬�����5K
		//�������inode�Ĵ��̿飬�ӿ���̨��ȡ����
		int curblockAddr;
		if (fileInode.i_dirBlock[k / superblock->s_BLOCK_SIZE] == -1) {
			//ȱ�ٴ��̿飬����һ��
			curblockAddr = balloc();
			if (curblockAddr == -1) {
				printf("block����ʧ��\n");
				oss << "block����ʧ��\n";
				return;
			}
			fileInode.i_dirBlock[k / superblock->s_BLOCK_SIZE] = curblockAddr;
		}
		else {
			curblockAddr = fileInode.i_dirBlock[k / superblock->s_BLOCK_SIZE];
		}
		//д�뵽��ǰĿ¼�Ĵ��̿�
		//fseek(fw, curblockAddr, SEEK_SET);
		//fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, fw);
		//fflush(fw);
		fseek(file, curblockAddr, SEEK_SET);
		fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, file);
		fflush(file);
	}
	//���¸��ļ���С
	fileInode.i_size = len;
	fileInode.i_mtime = time(NULL);
	//fseek(fw, fileInodeAddr, SEEK_SET);
	//fwrite(&fileInode, sizeof(Inode), 1, fw);
	//fflush(fw);
	fseek(file, fileInodeAddr, SEEK_SET);
	fwrite(&fileInode, sizeof(Inode), 1, file);
	fflush(file);
}

void inUsername(char username[])	//�����û���
{
	printf("username:");
	scanf("%s", username);	//�û���
}

void inPasswd(char passwd[])	//��������
{
	int plen = 0;
	char c;
	fflush(stdin);	//��ջ�����
	printf("passwd:");
	while (c = getch()) {
		if (c == '\r') {	//����س�������ȷ��
			passwd[plen] = '\0';
			fflush(stdin);	//�建����
			printf("\n");
			break;
		}
		else if (c == '\b') {	//�˸�ɾ��һ���ַ�
			if (plen != 0) {	//û��ɾ��ͷ
				plen--;
			}
		}
		else {	//�����ַ�
			passwd[plen++] = c;
		}
	}
}

bool login(const char username[],const char passwd[])	//��½����
{
	std::string user_name = username;
	std::string passw_d = passwd;
	//char username[100] = { 0 };
	//char passwd[100] = { 0 };
	//inUsername(username);	//�����û���
	//inPasswd(passwd);		//�����û�����
	if (check(user_name.c_str(), passw_d.c_str())) {	//�˶��û���������
		isLogin = true;
		return true;
	}
	else {
		isLogin = false;
		return false;
	}
}

bool check(const char username[],const char passwd[])	//�˶��û���������
{
	int passwd_Inode_Addr = -1;	//�û��ļ�inode��ַ
	int shadow_Inode_Addr = -1;	//�û������ļ�inode��ַ
	Inode passwd_Inode;		//�û��ļ���inode
	Inode shadow_Inode;		//�û������ļ���inode

	Inode cur_dir_inode;	//��ǰĿ¼��inode
	int i, j;
	DirItem dirlist[16];	//��ʱĿ¼
	cd(Root_Dir_Addr, "etc", true);
	//cd(Cur_Dir_Addr, "etc",true);	//���������ļ�Ŀ¼

	//�ҵ�passwd��shadow�ļ�inode��ַ����ȡ��
	//ȡ����ǰĿ¼��inode
	//fseek(fr, Cur_Dir_Addr, SEEK_SET);
	//fread(&cur_dir_inode, sizeof(Inode), 1, fr);
	fseek(file, Cur_Dir_Addr, SEEK_SET);
	fread(&cur_dir_inode, sizeof(Inode), 1, file);
	//����ȡ�����̿飬����passwd�ļ���inode��ַ����shadow�ļ���inode��ַ
	for (i = 0; i < 10; i++) {
		if (cur_dir_inode.i_dirBlock[i] == -1) {
			continue;
		}
		//����ȡ�����̿�
		//fseek(fr, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		for (j = 0; j < 16; j++) {	//����Ŀ¼��
			if (strcmp(dirlist[j].itemName, "passwd") == 0 ||	//�ҵ�passwd����shadow��Ŀ
				strcmp(dirlist[j].itemName, "shadow") == 0) {
				Inode tmp;	//��ʱinode
				//ȡ��inode���ж��Ƿ����ļ�
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);

				if (((tmp.i_mode >> 9) & 1) == 0) {
					//���ļ�
					//�б���passwd�ļ�����shadow�ļ�
					if (strcmp(dirlist[j].itemName, "passwd") == 0) {
						passwd_Inode_Addr = dirlist[j].inodeAddr;
						//passwd_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&passwd_Inode, sizeof(Inode), 1, file);
					}
					else if (strcmp(dirlist[j].itemName, "shadow") == 0) {
						shadow_Inode_Addr = dirlist[j].inodeAddr;
						//shadow_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&shadow_Inode, sizeof(Inode), 1, file);
					}
				}
			}
		}
		if (passwd_Inode_Addr != -1 && shadow_Inode_Addr != -1)	//���ҵ���
			break;
	}

	//����passwd�ļ������Ƿ�����û�username
	char buf[1000000];	//���1M���ݴ�passwd���ļ�����
	char buf2[600];		//�ݴ���̿�����
	j = 0;	//���̿�ָ��
	//ȡ��passwd�ļ�����
	for (i = 0; i < passwd_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������
			//���µĴ��̿�
			//fseek(fr, passwd_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, passwd_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';
	if (strstr(buf, username) == NULL) {
		//û�ҵ����û�
		printf("�û�������\n");
		oss << "�û�������\n";
		cd(Cur_Dir_Addr, "..",true);	//�ص���Ŀ¼
		return false;
	}

	//������ڣ��鿴shadow�ļ���ȡ�����룬�˶�passwd�Ƿ���ȷ
	//ȡ��shadow�ļ�����
	j = 0;
	for (i = 0; i < shadow_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������������̿�
			//���µĴ��̿�
			//fseek(fr, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	char* p;	//�ַ�ָ��
	if ((p = strstr(buf, username)) == NULL) {
		//û�ҵ����û�
		printf("shadow�ļ��в����ڸ��û�\n");
		oss << "shadow�ļ��в����ڸ��û�\n";
		cd(Cur_Dir_Addr, "..",true);	//�ص���Ŀ¼
		return false;
	}
	//�ҵ����û���ȡ������
	while ((*p) != ':') {
		p++;
	}
	p++;
	j = 0;
	while ((*p) != '\n') {
		buf2[j++] = *p;
		p++;
	}
	buf2[j] = '\0';

	//�˶�����
	if (strcmp(buf2, passwd) == 0) {	//������ȷ����½
		strcpy(Cur_User_Name, username);
		if (strcmp(username, "root") == 0)
			strcpy(Cur_Group_Name, "root");	//��ǰ��½�û�����
		else
			strcpy(Cur_Group_Name, "user");	//��ǰ��½�û�����
		cd(Cur_Dir_Addr, "..",true);
		cd(Cur_Dir_Addr, "home",true); \
			cd(Cur_Dir_Addr, username,true);	//���뵽�û�Ŀ¼
		strcpy(Cur_User_Dir_Name, Cur_Dir_Name);	//���Ƶ�ǰ��½�û�Ŀ¼��
		return true;
	}
	else {
		printf("�������\n");
		oss << "�������\n";
		cd(Cur_Dir_Addr, "..",true);	//�ص���Ŀ¼
		return false;
	}
}

void gotoRoot()	//�ص���Ŀ¼
{
	memset(Cur_User_Name, 0, sizeof(Cur_User_Name));		//��յ�ǰ�û���
	memset(Cur_User_Dir_Name, 0, sizeof(Cur_User_Dir_Name));	//��յ�ǰ�û�Ŀ¼
	Cur_Dir_Addr = Root_Dir_Addr;	//��ǰ�û�Ŀ¼��ַ��Ϊ��Ŀ¼��ַ
	strcpy(Cur_Dir_Name, "/");		//��ǰĿ¼��Ϊ"/"
}

void logout()	//�û�ע��
{
	//�ص���Ŀ¼
	gotoRoot();

	isLogin = false;
	printf("�û�ע��\n");
	oss << "�û�ע��\n";
	system("pause");
	system("cls");
}

bool useradd(char username[],char passwd[])	//�û�ע��
{
	if (strcmp(Cur_User_Name, "root") != 0) {
		printf("Ȩ�޲���\n");
		oss << "Ȩ�޲���\n";
		return false;
	}
	int passwd_Inode_Addr = -1;	//�û��ļ�inode��ַ
	int shadow_Inode_Addr = -1;	//�û������ļ�inode��ַ
	int group_Inode_Addr = -1;	//�û����ļ�inode��ַ
	Inode passwd_Inode;		//�û��ļ���inode
	Inode shadow_Inode;		//�û������ļ���inode
	Inode group_Inode;		//�û����ļ�inode
	//ԭ����Ŀ¼
	char bak_Cur_User_Name[110];
	char bak_Cur_User_Name_2[110];
	char bak_Cur_User_Dir_Name[310];
	int bak_Cur_Dir_Addr;
	char bak_Cur_Dir_Name[310];
	char bak_Cur_Group_Name[310];

	Inode cur_dir_inode;	//��ǰĿ¼��inode
	int i, j;
	DirItem dirlist[16];	//��ʱĿ¼

	//�����ֳ����ص���Ŀ¼
	strcpy(bak_Cur_User_Name, Cur_User_Name);
	strcpy(bak_Cur_User_Dir_Name, Cur_User_Dir_Name);
	bak_Cur_Dir_Addr = Cur_Dir_Addr;
	strcpy(bak_Cur_Dir_Name, Cur_Dir_Name);

	//�����û�Ŀ¼
	gotoRoot();
	cd(Cur_Dir_Addr, "home",true);
	//�����ֳ�
	strcpy(bak_Cur_User_Name_2, Cur_User_Name);
	strcpy(bak_Cur_Group_Name, Cur_Group_Name);
	//����
	strcpy(Cur_User_Name, username);
	strcpy(Cur_Group_Name, "user");
	if (!mkdir(Cur_Dir_Addr, username)) {
		strcpy(Cur_User_Name, bak_Cur_User_Name_2);
		strcpy(Cur_Group_Name, bak_Cur_Group_Name);
		//�ָ��ֳ����ص�ԭ����Ŀ¼
		strcpy(Cur_User_Name, bak_Cur_User_Name);
		strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
		Cur_Dir_Addr = bak_Cur_Dir_Addr;
		strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

		printf("�û�ע��ʧ��!\n");
		oss << "�û�ע��ʧ��!\n";
		return false;
	}
	//�ָ��ֳ�
	strcpy(Cur_User_Name, bak_Cur_User_Name_2);
	strcpy(Cur_Group_Name, bak_Cur_Group_Name);

	//�ص���Ŀ¼
	gotoRoot();

	//�����û�Ŀ¼
	cd(Cur_Dir_Addr, "etc",true);

	//�����û�����
	//char passwd[100] = { 0 };
	//inPasswd(passwd);	//��������

	//�ҵ�passwd��shadow�ļ�inode��ַ����ȡ����׼�������Ŀ

	//ȡ����ǰĿ¼��inode
	//fseek(fr, Cur_Dir_Addr, SEEK_SET);
	//fread(&cur_dir_inode, sizeof(Inode), 1, fr);
	fseek(file, Cur_Dir_Addr, SEEK_SET);
	fread(&cur_dir_inode, sizeof(Inode), 1, file);
	//����ȡ�����̿飬����passwd�ļ���inode��ַ����shadow�ļ���inode��ַ
	for (i = 0; i < 10; i++) {
		if (cur_dir_inode.i_dirBlock[i] == -1) {
			continue;
		}
		//����ȡ�����̿�
		//fseek(fr, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		for (j = 0; j < 16; j++) {	//����Ŀ¼��
			if (strcmp(dirlist[j].itemName, "passwd") == 0 ||	//�ҵ�passwd����shadow��Ŀ
				strcmp(dirlist[j].itemName, "shadow") == 0 ||
				strcmp(dirlist[j].itemName, "group") == 0) {
				Inode tmp;	//��ʱinode
				//ȡ��inode���ж��Ƿ����ļ�
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);

				if (((tmp.i_mode >> 9) & 1) == 0) {
					//���ļ�
					//�б���passwd�ļ�����shadow�ļ�
					if (strcmp(dirlist[j].itemName, "passwd") == 0) {
						passwd_Inode_Addr = dirlist[j].inodeAddr;
						//passwd_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&passwd_Inode, sizeof(Inode), 1, file);
					}
					else if (strcmp(dirlist[j].itemName, "shadow") == 0) {
						shadow_Inode_Addr = dirlist[j].inodeAddr;
						//shadow_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&shadow_Inode, sizeof(Inode), 1, file);
					}
					else if (strcmp(dirlist[j].itemName, "group") == 0) {
						group_Inode_Addr = dirlist[j].inodeAddr;
						//group_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&group_Inode, sizeof(Inode), 1, file);
					}
				}
			}
		}
		if (passwd_Inode_Addr != -1 && shadow_Inode_Addr != -1)	//���ҵ���
			break;
	}

	//����passwd�ļ������Ƿ�����û�username
	char buf[100000];	//���100K���ݴ�passwd���ļ�����
	char buf2[600];		//�ݴ���̿�����
	j = 0;	//���̿�ָ��
	//ȡ��passwd�ļ�����
	for (i = 0; i < passwd_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������
			//���µĴ��̿�
			//fseek(fr, passwd_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, passwd_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	if (strstr(buf, username) != NULL) {
		//û�ҵ����û�
		printf("�û��Ѵ���\n");
		oss << "�û��Ѵ���\n";

		//�ָ��ֳ����ص�ԭ����Ŀ¼
		strcpy(Cur_User_Name, bak_Cur_User_Name);
		strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
		Cur_Dir_Addr = bak_Cur_Dir_Addr;
		strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);
		return false;
	}

	//��������ڣ���passwd�д������û���Ŀ,�޸�group�ļ�
	sprintf(buf + strlen(buf), "%s:x:%d:%d\n", username, nextUID++, 1);	//������Ŀ���û������������룺�û�ID���û���ID���û���Ϊ��ͨ�û��飬ֵΪ1 
	passwd_Inode.i_size = strlen(buf);
	writefile(passwd_Inode_Addr, buf);	//���޸ĺ��passwdд���ļ���

	//ȡ��shadow�ļ�����
	j = 0;
	for (i = 0; i < shadow_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������������̿�
			//���µĴ��̿�
			//fseek(fr, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//����shadow��Ŀ
	sprintf(buf + strlen(buf), "%s:%s\n", username, passwd);	//������Ŀ���û���������
	shadow_Inode.i_size = strlen(buf);
	writefile( shadow_Inode_Addr, buf);	//���޸ĺ������д���ļ���


	//ȡ��group�ļ�����
	j = 0;
	for (i = 0; i < group_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������������̿�
			//���µĴ��̿�
			//fseek(fr, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//����group����ͨ�û��б�
	if (buf[strlen(buf) - 2] == ':')
		sprintf(buf + strlen(buf) - 1, "%s\n", username);	//���������û�
	else
		sprintf(buf + strlen(buf) - 1, ",%s\n", username);	//���������û�
	group_Inode.i_size = strlen(buf);
	writefile(group_Inode_Addr, buf);	//���޸ĺ������д���ļ���

	//�ָ��ֳ����ص�ԭ����Ŀ¼
	strcpy(Cur_User_Name, bak_Cur_User_Name);
	strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
	Cur_Dir_Addr = bak_Cur_Dir_Addr;
	strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

	printf("�û�ע��ɹ�\n");
	oss << "�û�ע��ɹ�\n";
	return true;
}


bool userdel(char username[])	//�û�ɾ��
{
	if (strcmp(Cur_User_Name, "root") != 0) {
		printf("Ȩ�޲���:����ҪrootȨ��\n");
		oss << "Ȩ�޲���:����ҪrootȨ��\n";
		return false;
	}
	if (strcmp(username, "root") == 0) {
		printf("�޷�ɾ��root�û�\n");
		oss << "�޷�ɾ��root�û�\n";
		return false;
	}
	int passwd_Inode_Addr = -1;	//�û��ļ�inode��ַ
	int shadow_Inode_Addr = -1;	//�û������ļ�inode��ַ
	int group_Inode_Addr = -1;	//�û����ļ�inode��ַ
	Inode passwd_Inode;		//�û��ļ���inode
	Inode shadow_Inode;		//�û������ļ���inode
	Inode group_Inode;		//�û����ļ�inode
	//ԭ����Ŀ¼
	char bak_Cur_User_Name[110];
	char bak_Cur_User_Dir_Name[310];
	int bak_Cur_Dir_Addr;
	char bak_Cur_Dir_Name[310];

	Inode cur_dir_inode;	//��ǰĿ¼��inode
	int i, j;
	DirItem dirlist[16];	//��ʱĿ¼

	//�����ֳ����ص���Ŀ¼
	strcpy(bak_Cur_User_Name, Cur_User_Name);
	strcpy(bak_Cur_User_Dir_Name, Cur_User_Dir_Name);
	bak_Cur_Dir_Addr = Cur_Dir_Addr;
	strcpy(bak_Cur_Dir_Name, Cur_Dir_Name);

	//�ص���Ŀ¼
	gotoRoot();

	//�����û�Ŀ¼
	cd(Cur_Dir_Addr, "etc",true);

	//�����û�����
	//char passwd[100] = {0};
	//inPasswd(passwd);	//��������

	//�ҵ�passwd��shadow�ļ�inode��ַ����ȡ����׼�������Ŀ

	//ȡ����ǰĿ¼��inode
	//fseek(fr, Cur_Dir_Addr, SEEK_SET);
	//fread(&cur_dir_inode, sizeof(Inode), 1, fr);
	fseek(file, Cur_Dir_Addr, SEEK_SET);
	fread(&cur_dir_inode, sizeof(Inode), 1, file);

	//����ȡ�����̿飬����passwd�ļ���inode��ַ����shadow�ļ���inode��ַ
	for (i = 0; i < 10; i++) {
		if (cur_dir_inode.i_dirBlock[i] == -1) {
			continue;
		}
		//����ȡ�����̿�
		//fseek(fr, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		for (j = 0; j < 16; j++) {	//����Ŀ¼��
			if (strcmp(dirlist[j].itemName, "passwd") == 0 ||	//�ҵ�passwd����shadow��Ŀ
				strcmp(dirlist[j].itemName, "shadow") == 0 ||
				strcmp(dirlist[j].itemName, "group") == 0) {
				Inode tmp;	//��ʱinode
				//ȡ��inode���ж��Ƿ����ļ�
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 0) {
					//���ļ�
					//�б���passwd�ļ�����shadow�ļ�
					if (strcmp(dirlist[j].itemName, "passwd") == 0) {
						passwd_Inode_Addr = dirlist[j].inodeAddr;
						//passwd_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&passwd_Inode, sizeof(Inode), 1, file);
					}
					else if (strcmp(dirlist[j].itemName, "shadow") == 0) {
						shadow_Inode_Addr = dirlist[j].inodeAddr;
						//shadow_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&shadow_Inode, sizeof(Inode), 1, file);
					}
					else if (strcmp(dirlist[j].itemName, "group") == 0) {
						group_Inode_Addr = dirlist[j].inodeAddr;
						//group_Inode = tmp;
						fseek(file, dirlist[j].inodeAddr, SEEK_SET);
						fread(&group_Inode, sizeof(Inode), 1, file);
					}
				}
			}
		}
		if (passwd_Inode_Addr != -1 && shadow_Inode_Addr != -1)	//���ҵ���
			break;
	}

	//����passwd�ļ������Ƿ�����û�username
	char buf[100000];	//���100K���ݴ�passwd���ļ�����
	char buf2[600];		//�ݴ���̿�����
	j = 0;	//���̿�ָ��
	//ȡ��passwd�ļ�����
	for (i = 0; i < passwd_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������
			//���µĴ��̿�
			//fseek(fr, passwd_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, passwd_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	if (strstr(buf, username) == NULL) {
		//û�ҵ����û�
		printf("�û�������\n");
		oss << "�û�������\n";

		//�ָ��ֳ����ص�ԭ����Ŀ¼
		strcpy(Cur_User_Name, bak_Cur_User_Name);
		strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
		Cur_Dir_Addr = bak_Cur_Dir_Addr;
		strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);
		return false;
	}

	//������ڣ���passwd��shadow��group��ɾ�����û�����Ŀ
	//ɾ��passwd��Ŀ
	char* p = strstr(buf, username);
	*p = '\0';
	while ((*p) != '\n')	//�ճ��м�Ĳ���
		p++;
	p++;
	strcat(buf, p);
	passwd_Inode.i_size = strlen(buf);	//�����ļ���С
	writefile(passwd_Inode_Addr, buf);	//���޸ĺ��passwdд���ļ���

	//ȡ��shadow�ļ�����
	j = 0;
	for (i = 0; i < shadow_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������������̿�
			//���µĴ��̿�
			//fseek(fr, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//ɾ��shadow��Ŀ
	p = strstr(buf, username);
	*p = '\0';
	while ((*p) != '\n')	//�ճ��м�Ĳ���
		p++;
	p++;
	strcat(buf, p);
	shadow_Inode.i_size = strlen(buf);	//�����ļ���С
	writefile(shadow_Inode_Addr, buf);	//���޸ĺ������д���ļ���


	//ȡ��group�ļ�����
	j = 0;
	for (i = 0; i < group_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//������������̿�
			//���µĴ��̿�
			//fseek(fr, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//����group����ͨ�û��б�
	p = strstr(buf, username);
	*p = '\0';
	while ((*p) != '\n' && (*p) != ',')	//�ճ��м�Ĳ���
		p++;
	if ((*p) == ',')
		p++;
	strcat(buf, p);
	group_Inode.i_size = strlen(buf);	//�����ļ���С
	writefile( group_Inode_Addr, buf);	//���޸ĺ������д���ļ���

	//�ָ��ֳ����ص�ԭ����Ŀ¼
	strcpy(Cur_User_Name, bak_Cur_User_Name);
	strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
	Cur_Dir_Addr = bak_Cur_Dir_Addr;
	strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

	//ɾ���û�Ŀ¼	
	Cur_Dir_Addr = Root_Dir_Addr;	//��ǰ�û�Ŀ¼��ַ��Ϊ��Ŀ¼��ַ
	strcpy(Cur_Dir_Name, "/");		//��ǰĿ¼��Ϊ"/"
	cd(Cur_Dir_Addr, "home",true);
	bool is=rmdir(Cur_Dir_Addr, username);

	//�ָ��ֳ����ص�ԭ����Ŀ¼
	strcpy(Cur_User_Name, bak_Cur_User_Name);
	strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
	Cur_Dir_Addr = bak_Cur_Dir_Addr;
	strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);
	if (true)
	{
		printf("�û���ɾ��\n");
		oss << "�û���ɾ��\n";
	}
	else
	{
		printf("�û�δɾ��\n");
		oss << "�û�δɾ��\n";
	}
	return true;

}

bool chmod(int parinoAddr, const char name[], int pmode)	//�޸��ļ���Ŀ¼Ȩ��
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("�������Ŀ¼������\n");
		oss << "�������Ŀ¼������\n";
		return false;
	}
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		printf("�������\n");
		oss << "�������\n";
		return false;
	}
	//ȡ�����ļ���Ŀ¼inode
	Inode cur, fileInode;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//����ȡ�����̿�
	int i = 0, j;
	DirItem dirlist[16] = { 0 };
	while (i < 160) {
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {	//�ҵ���Ŀ¼�����ļ�
				//ȡ����Ӧ��inode
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&fileInode, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				goto label;
			}
		}
		i++;
	}
label:
	if (i >= 160) {
		printf("�ļ�������\n");
		oss << "�ļ�������\n";
		return false;
	}

	//�ж��Ƿ��Ǳ��û�
	if (strcmp(Cur_User_Name, fileInode.i_uname) != 0 && strcmp(Cur_User_Name, "root") != 0) {
		printf("Ȩ�޲���\n");
		oss << "Ȩ�޲���\n";
		return false;
	}

	//��inode��mode���Խ����޸�
	fileInode.i_mode = (fileInode.i_mode >> 9 << 9) | pmode;	//�޸�Ȩ��

	//��inodeд�ش���
	//fseek(fw, dirlist[j].inodeAddr, SEEK_SET);
	//fwrite(&fileInode, sizeof(Inode), 1, fw);
	//fflush(fw);
	fseek(file, dirlist[j].inodeAddr, SEEK_SET);
	fwrite(&fileInode, sizeof(Inode), 1, file);
	fflush(file);
}

bool touch(int parinoAddr, char name[], char buf[])	//touch������ļ��������ַ�
{
	//���ж��ļ��Ƿ��Ѵ��ڡ�������ڣ�������ļ����༭
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("��������ļ�������\n");
		oss << "��������ļ�������\n";
		return false;
	}
	//��������ͬ���ļ����еĻ���ʾ�����˳�����û�еĻ�������һ�����ļ�
	DirItem dirlist[16];	//��ʱĿ¼�嵥

	//�������ַȡ��inode
	Inode cur, fileInode;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//�ļ���inode��ַ
	while (i < 160) {
		//160��Ŀ¼��֮�ڣ�����ֱ����ֱ�ӿ�����
		dno = i / 16;	//�ڵڼ���ֱ�ӿ���

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//������ȡ��inode���ж��Ƿ����ļ�
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&fileInode, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//���ļ�����������ʾ�����˳�����
					printf("�ļ��Ѵ���\n");
					oss << "�ļ��Ѵ���\n";
					return false;
				}
			}
			i++;
		}
	}

	//�ļ������ڣ�����һ�����ļ�
	if (((cur.i_mode >> filemode >> 1) & 1) == 1) {
		//��д�����Դ����ļ�
		buf[0] = '\0';
		bool is=create(parinoAddr, name, buf);	//�����ļ�
		return is;
	}
	else {
		printf("Ȩ�޲��㣺��д��Ȩ��\n");
		oss << "Ȩ�޲��㣺��д��Ȩ��\n";
		return false;
	}
	return true;
}

void help()	//��ʾ���������嵥 
{
	printf("help - ��ʾ�����嵥\n");
	printf("\n");
	printf("----Ŀ¼���----\n");
	printf("ls - ��ʾ��ǰĿ¼�嵥\n");
	printf("cd - ����Ŀ¼\n");
	printf("md - ����Ŀ¼\n");
	printf("rd - ɾ��Ŀ¼\n");
	printf("dir - ��ʾĿ¼:��ʾָ��Ŀ¼�µ���Ϣ ��/s������dir�����ʾ������Ŀ¼\n");
	printf("\n");
	printf("----�ļ����----\n");
	printf("newfile - ����һ�����ļ�\n");
	printf("cat - ���ļ���ʾ�ļ�����\n");
	printf("copy - �����ļ�\n");
	printf("del - ɾ���ļ�\n");
	printf("chmod - �޸��ļ���Ŀ¼Ȩ��,chmod �ļ��� Ȩ��\n");
	printf("vi - vi�༭��\n");
	printf("\n");
	printf("----�û����----\n");
	printf("logout - �û�ע��\n");
	printf("useradd - ����û�\n");
	printf("userdel - ɾ���û�\n");
	printf("\n");
	printf("----ϵͳ���----\n");
	printf("info -��ʾ����ϵͳ��Ϣ\n");
	printf("super - �鿴������\n");
	printf("inode - �鿴inodeλͼ\n");
	printf("block - �鿴blockλͼ\n");
	printf("cls - ����\n");
	printf("exit - �˳�ϵͳ\n");

	return;
}

void cmd(const char str[])	//�������������
{	//ÿ�δ���ǰ�������һ�εĽ��
	response.clear();
	// ��� oss
	oss.str("");
	oss.clear();
	//��ʼ��ѡ��
	response_op = Option::NONE;
	char p1[100] = { 0 };
	char p2[100] = { 0 };
	char p3[100] = { 0 };
	char p4[100] = { 0 };
	char buf[1000] = { 0 };	//���100K
	int tmp = 0;
	int i;
	sscanf(str, "%s", p1);
	if (strcmp(p1, "ls") == 0) {
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		bool is;
		if (pr.is_a_path)
		{
			int addr_final=FindPathInCurdir(pr.addr, pr.nameLast);
			is=ls(addr_final);
		}
		if (is)
		{
			code = ErrorCode::SUCCESS;
		}
		else
		{
			code = ErrorCode::FAILURE;
		}
		//ls(Cur_Dir_Addr);	//��ʾ��ǰĿ¼
	}
	else if (strcmp(p1, "copy")==0)
	{
		sscanf(str, "%s%s%s", p1, p2, p3);
		bool p2_is_host=false;//p2�Ƿ��Ǵ�<host>
		bool p3_is_host=false;//p3�Ƿ��<host>
		//���p2,p3������<host>,�����ñ�־λ
		//���p2,p3������host
			// ��� p2 �Ƿ���� <host>
		if (strstr(p2, "<host>") != nullptr) {
			p2_is_host = true;
			memmove(p2, p2 + 6, strlen(p2) - 5);//-5��Ϊ�˱���'\0',dest,src,long
		}

		// ��� p3 �Ƿ���� <host>
		if (strstr(p3, "<host>") != nullptr) {
			p3_is_host = true;
			memmove(p3, p3 + 6, strlen(p3) - 5);
		}
		// ����Ǹ��Ʊ����ļ��������ļ�ϵͳ�ڲ�����
		if (p2_is_host || p3_is_host) {
			// ��� p2 �� p3 ���� <host>��˵���漰�������ļ�ϵͳ��ģ���ļ�ϵͳ֮����ļ�����
			

			// ��������Լ��������ļ������߼���������� Windows API �������ļ�
			if (p2_is_host) {
				// ����Դ·��Ϊ����·�������
				readFileFromHost(p2);
				//�ҵ�Ҫд���ļ���inode�ڵ��ַ
				PathResult pr = ParseFromRoot(p3);
				if (pr.is_a_path)
				{
					int code=FindFileInCurdir(pr.addr, pr.nameLast);
					if (code == -1)
					{
						printf("·����û�и��ļ�\n");
						return;
					}
					writefile(code, readFromHost);
					//��ջ�����
					memset(readFromHost, 0, sizeof(readFromHost));
				}

			}
			else {
				// ����Ŀ��·��Ϊ����·�������
				//�ҵ�Ҫд���ļ���inode�ڵ��ַ
				PathResult pr = ParseFromRoot(p2);
				if (pr.is_a_path)
				{
					readFromFile(pr.addr, pr.nameLast);
				}
				int length = strlen(readFromHost);
				writeFileToHost(p3, length);
				memset(readFromHost, 0, sizeof(readFromHost));
				
			}
		}
		else {
			// �������ļ�ϵͳ�ڲ��ĸ��ƣ�ģ���ļ�ϵͳ�ڲ�·��֮��ĸ���
			PathResult pr = ParseFromRoot(p2);
			if (pr.is_a_path)
			{
				readFromFile(pr.addr, pr.nameLast);
			}
			PathResult pr1 = ParseFromRoot(p3);
			if (pr1.is_a_path)
			{
				int code = FindFileInCurdir(pr1.addr, pr1.nameLast);
				if (code == -1)
				{
					printf("·����û�и��ļ�\n");
					return;
				}
				writefile(code, readFromHost);
				//��ջ�����
				memset(readFromHost, 0, sizeof(readFromHost));
			}
		}
		code = ErrorCode::SUCCESS;

	}
	else if (strcmp(p1, "cat") == 0)
	{
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		bool is;
		if (pr.is_a_path)
		{
			is=cat_file(pr.addr, pr.nameLast);
		}
		if (is)
		{
			code = ErrorCode::SUCCESS;
		}
		else
		{
			code = ErrorCode::FAILURE;
		}
	}
	else if (strcmp(p1, "check") == 0)
	{
		printf("��Ⲣ�ָ��ļ�ϵͳ�ɹ�\n");
		oss << "��Ⲣ�ָ��ļ�ϵͳ�ɹ�\n";
	}
	else if (strcmp(p1, "cd") == 0) {
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		if (pr.is_a_path)
		{
			char rootName[] = "/";
			char* tmp1 = new char[strlen(rootName) + 1];  // +1 ��Ϊ�˴洢��ֹ�� '\0'
			strcpy(tmp1, rootName);//tmp1=='/'
			char*tmp= parsePath(p2, Cur_Dir_Name, tmp1);
			char bak_Cur_Dir_Name[310];
			strcpy(bak_Cur_Dir_Name, Cur_Dir_Name);
			strcpy(Cur_Dir_Name, tmp);//���ƹ�ȥ
			bool is=cd(pr.addr, pr.nameLast,false);
			code = ErrorCode::SUCCESS;
			if (!is)//����false��ǰĿ¼��ַҪ��ԭ
			{
				strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

				code = ErrorCode::FAILURE;
			}

		}
		else
		{
			code = ErrorCode::FAILURE;
		}
		//����Ҫ�Ѿ���·���ָ�һ�·ָ��ֻ��û�����һ�������·����������
		//cd(Cur_Dir_Addr, p2);
	}
	else if (strcmp(p1, "md") == 0) {
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		bool is;
		if (pr.is_a_path)
		{
		
			is=mkdir(pr.addr, pr.nameLast);
		}
		if (is)
		{
			code = ErrorCode::SUCCESS;
		}
		else
		{
			code = ErrorCode::FAILURE;
		}
		//mkdir(Cur_Dir_Addr, p2);
	}
	else if (strcmp(p1, "rd") == 0) {
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		bool is;
		if (pr.is_a_path)
		{
			//�Ȳ�ѯ����ļ����Ƿ�Ϊ��
			bool is_empty = false;
			is_empty = find_file_is_empty(pr.addr, pr.nameLast);
			if (is_empty|| Option::RESPONSE==receive_op)//���Ϊ�ջ����ǽӵ�ֱ��ɾ���������ֱ��ɾ
			{
				is = rmdir(pr.addr, pr.nameLast);
			}
			else
			{
				//��Ϊ�վͷ�����ѯ
				response_op = Option::REQUEST;
				is = true;
			}
		}
		if (is)
		{
			code = ErrorCode::SUCCESS;
		}
		else
		{
			code = ErrorCode::FAILURE;
		}
		//rmdir(Cur_Dir_Addr, p2);
	}
	else if (strcmp(p1, "super") == 0) {
		printSuperBlock();
	}
	else if (strcmp(p1, "inode") == 0) {
		printInodeBitmap();
	}
	else if (strcmp(p1, "block") == 0) {
		sscanf(str, "%s%s", p1, p2);
		tmp = 0;
		if ('0' <= p2[0] && p2[0] <= '9') {
			for (i = 0; p2[i]; i++)
				tmp = tmp * 10 + p2[i] - '0';
			printBlockBitmap(tmp);
		}
		else
			printBlockBitmap();
	}
	else if (strcmp(p1, "vi") == 0) {	//����һ���ļ�
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		if (pr.is_a_path)
		{
			vi(pr.addr, pr.nameLast, buf);
		}
		//vi(Cur_Dir_Addr, p2, buf);	//��ȡ���ݵ�buf
	}
	else if (strcmp(p1, "newfile") == 0) {
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		bool is = false;
		if (pr.is_a_path)
		{
		is=	touch(pr.addr, pr.nameLast,buf);
		}
		if (is)
		{
			code = ErrorCode::SUCCESS;

		}
		else
		{
			code = ErrorCode::FAILURE;
		}
		//touch(Cur_Dir_Addr, p2, buf);	//��ȡ���ݵ�buf
	}
	else if (strcmp(p1, "del") == 0) {	//ɾ��һ���ļ�
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		bool is = false;
		if (pr.is_a_path)
		{
			is=del(pr.addr, pr.nameLast);
		}
		if (is)
		{
			code = ErrorCode::SUCCESS;
		}
		else
		{
			code = ErrorCode::FAILURE;
		}
		//del(Cur_Dir_Addr, p2);
	}
	else if (strcmp(p1, "cls") == 0) {
		system("cls");
	}
	else if (strcmp(p1, "logout") == 0) {
		logout();
	}
	else if (strcmp(p1, "userdel") == 0) {
		p2[0] = '\0';
		sscanf(str, "%s%s", p1, p2);
		if (strlen(p2) == 0) {
			printf("��������\n");
		}
		else {
			userdel(p2);
		}
	}
	else if (strcmp(p1, "chmod") == 0) {
		p2[0] = '\0';
		p3[0] = '\0';
		bool is=false;
		sscanf(str, "%s%s%s", p1, p2, p3);
		if (strlen(p2) == 0 || strlen(p3) == 0) {
			printf("��������\n");
			code = ErrorCode::FAILURE;
		}
		else {
			
			tmp = 0;
			for (i = 0; p3[i]; i++)
				tmp = tmp * 8 + p3[i] - '0';
			
			PathResult pr = ParseFromRoot(p2);
			if (pr.is_a_path)
			{
				is=chmod(pr.addr,pr.nameLast,tmp);
			}
			//chmod(Cur_Dir_Addr, p2, tmp);
			if (is)
			{
				code = ErrorCode::SUCCESS;
			}
			else
			{
				code = ErrorCode::FAILURE;
			}
		}
	}
	else if (strcmp(p1, "help") == 0) {
		help();
	}
	else if (strcmp(p1, "format") == 0) {
		if (strcmp(Cur_User_Name, "root") != 0) {
			printf("Ȩ�޲��㣺����ҪrootȨ��\n");
			return;
		}
		Ready();
		logout();
	}
	else if (strcmp(p1, "exit") == 0) {
		printf("�˳�MingOS\n");
		exit(0);
	}
	else if (strcmp(p1, "su")==0)
	{
		sscanf(str, "%s%s%s", p1, p2,p3);
		bool is=login(p2, p3);
		if (is)
		{
			code = ErrorCode::SUCCESS;
		}
		else
		{
			code = ErrorCode::FAILURE;
		}
	}
	else if (strcmp(p1, "sudo")==0)
	{
		bool is;
		sscanf(str, "%s%s", p1, p2);
		if (strcmp(p2, "useradd") == 0) {
		p2[0] = '\0';
		sscanf(str, "%s%s%s%s", p1, p2,p3,p4);
		if (strlen(p2) == 0) {
			printf("��������\n");
			oss << "��������\n";
		}
		else {
			 is=useradd(p3,p4 );
		}

		}
		else if (strcmp(p2, "userdel") == 0)
		{
			p2[0] = '\0';
			sscanf(str, "%s%s%s", p1, p2,p3);
			if (strlen(p2) == 0) {
				printf("��������\n");
				oss << "��������\n";
			}
			else {
				is=userdel(p3);
			}
		}

		if (is)
		{
			code = ErrorCode::SUCCESS;
		}
		else
		{
			code = ErrorCode::FAILURE;
		}
	}
	else if (strcmp(p1, "info") == 0)
	{
		printSuperBlock();
	}
	else if (strcmp(p1, "dir") == 0)
	{
		sscanf(str, "%s%s", p1, p2);
		if (strcmp(p2 ,"-s")==0)
		{
			sscanf(str, "%s%s%s", p1, p2, p3);
			PathResult pr = ParseFromRoot(p3);
			bool is;
			int addr=-1;
			if (pr.is_a_path)
			{	
				addr = FindPathInCurdir(pr.addr, pr.nameLast);
				if (addr == -1)
				{
					code = ErrorCode::FAILURE;
				}
				else
				{
					is = recursivelyLs(addr, p3);
				}
			}
			if (is)
			{
				code = ErrorCode::SUCCESS;
			}
			else
			{
				code = ErrorCode::FAILURE;
			}
		}
		else
		{
			PathResult pr = ParseFromRoot(p2);
			int addr = -1;
			if (pr.is_a_path)
			{
				addr=FindPathInCurdir(pr.addr, pr.nameLast);

			}
			if (addr!=-1)
			{	
				bool is;
				is = ls(addr);
				if (is)
				{
					code = ErrorCode::SUCCESS;
				}
				else
				{
					code = ErrorCode::FAILURE;
				}
			}
			else
			{
				code = ErrorCode::FAILURE;
			}
		}
	}
	else {
		if (strlen(str) != 0)
		{
			printf("��Ǹ��û�и�����\n");
			oss << "��Ǹ��û�и�����\n";
		}
	}
	response = oss.str();
	return;
}

//�ź���(P��V����)��ʵ��
void reader_p(int fileInodeAddr)
{
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	while (cur.write_cnt.load() > 0)
	{
		//�����д�߳�����д�����߳���Ҫ�ȴ�
		Sleep(1000);
	}
	cur.read_cnt.fetch_add(1);//���Ӷ�������
}

void reader_v(int fileInodeAddr)
{	//��ȡ������
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//��ɶ�����
	cur.read_cnt.fetch_sub(1);//���ٶ�������
}

void writer_p(int fileInodeAddr)
{
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	// �ȴ�����������ж��̻߳�д�߳�����д��д�߳���Ҫ�ȴ�
	while (cur.read_cnt.load() > 0 || cur.write_cnt.load() > 0) {
		Sleep(1000);  // ����ж��̻߳�д�̣߳�д�̵߳ȴ�
	}
	cur.write_cnt.fetch_add(1);  // ����д������
}

void writer_v(int fileInodeAddr)
{
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//����д������
	cur.write_cnt.fetch_sub(1);
}

//����ļ�����
bool cat_file(int parinoAddr, const char name[])
{
	//�ȰѸ�Ŀ¼��Inode�ڵ��ȡ����
	Inode cur;
	Inode fileInode;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	DirItem dirlist[16];	//��ʱĿ¼�嵥
	
	//����ȡ��inode��Ӧ�Ĵ��̿飬������û������Ϊname��Ŀ¼��



	//�ж�Ȩ��
	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//�ļ���inode��ַ
	bool isExist = false;	//�ļ��Ƿ��Ѵ���
	while (i < 160) {
		//160��Ŀ¼��֮�ڣ�����ֱ����ֱ�ӿ�����
		dno = i / 16;	//�ڵڼ���ֱ�ӿ���

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//ȡ���ô��̿��Ŀ¼�嵥
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//����filenode�д洢
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//���ļ���������������ļ������༭	
					fileInodeAddr = dirlist[j].inodeAddr;
					isExist = true;
					
				}
				
			}
			i++;
		}
	}

	//�ж��Ƿ��������ļ�
	if (!isExist)
	{
		printf("�ļ�������\n");
		oss << "�ļ�������\n";
		return false;
	}
	//�ж���û�ж�ȡȨ��
	if (((fileInode.i_mode >> filemode >> 2)& 1) == 0)
	{
		printf("û�ж�ȡȨ��\n");
		oss << "û�ж�ȡȨ��\n";
		return false;
	}


	//���ļ����ݶ�ȡ��������ʾ�ڣ�������
	i = 0;
	int sumlen = fileInode.i_size;	//�ļ�����
	int getlen = 0;	//ȡ�����ĳ���
	char buf[10000] = { 0 };
	for (i = 0; i < 10; i++) {
		char fileContent[1000] = { 0 };
		if (fileInode.i_dirBlock[i] == -1) {
			continue;
		}
		//ȡ����ǰ�ļ���ֱ�ӿ�
		fseek(file, fileInode.i_dirBlock[i], SEEK_SET);
		fread(fileContent, superblock->s_BLOCK_SIZE, 1, file);	//��ȡ��һ�����̿��С������

		//����ַ���
		int curlen = 0;	//��ǰָ��
		while (curlen < superblock->s_BLOCK_SIZE) {
			if (getlen >= sumlen)	//ȫ��������
				break;
			printf("%c", fileContent[curlen]);	//�������Ļ 
			oss << fileContent[curlen];
			buf[getlen++] = fileContent[curlen];	//�����buf
			curlen++;
			
		}
		if (getlen >= sumlen)
			break;
	}
	//printf("\n");
	return true;

}


//��������·��
PathResult ParseFromRoot(const char path[])
{
	//���и�·���ַ���
	//  ����������ͷ  '/'  '.' '..'
	std::vector<char*> pathComponents;
	if (strcmp(path, "/") == 0)
	{
		char temp[] = ".";
		pathComponents.push_back(temp);  // ���⴦����Ӹ�Ŀ¼
	}
	else{
	  char* pathCopy = strdup(path);  // ����·���������޸�ԭʼ·��
	  char* token = strtok(pathCopy, "/");
	  //�и�·������ÿ�����ֶ�����·���������
	  while (token != nullptr)
	  {
		pathComponents.push_back(token);
		token = strtok(nullptr, "/");//����Nullptr��ʾ��������һ��ֹͣ��λ�ü����ָ�
	  } 
    }

	bool is_a_path = true;
	int targetAddr = -1;//������������洢·�������һ��Componenet������Ŀ¼��inode��ַ
	int size = pathComponents.size();
	char* name = pathComponents[size-1]; //��strcpy(.... ,name)��char*ת��Ϊchar[]
	int vir_addr = path[0] == '/'?  Root_Dir_Addr :Cur_Dir_Addr;//��ֻ��һ���ֲ�����ȥ��֤�Ƿ�·����ȷ��������ı䵱ǰĿ¼��ַ�������֣���ÿ��for�лᱻ����
	//���·����һ����ʼ��/˵���Ӹ�Ŀ¼ȥ��ʼ�Ҷ��������·��
	
	//��ʼ����·�����
	for (int i = 0; i < size - 1;i++) {
		const char* component = pathComponents[i];
		if (strcmp(component, ".") == 0)
		{
			continue;
		}
		else
		{
			int returnAdrr = FindPathInCurdir(vir_addr, component);
			if (returnAdrr == -1)//����û���ҵ����Ŀ¼��˵��·������,����
			{
				is_a_path = false;//����·������
				printf("·������!\n");
				oss << "·������!\n";
				break;
			}
			//�������-1�Ǿ͸�ֵ,Ȼ���������
			vir_addr = returnAdrr;
		}
	}
	//��Ҫ��Cur_Dir_addr,cur_dir_name,Cur_user_name���ö���Cur_User_Dir_NameҲ���ù��˷���û�õ�
	//Cur_Dir_addr,cur_dir_name����������ά���þ���
	//cd ��Ҫ��cur_dir_name�ģ�

	//���ؽ��
	PathResult pr;
	pr.addr = vir_addr;
	pr.is_a_path = is_a_path;
	strcpy(pr.nameLast ,name);

	return pr;
}

int FindPathInCurdir(int parinoAddr,const char*name) 
{
	char findName[28];
	strcpy(findName, name);//�ȿ���

	//ȡ����ǰĿ¼��inode
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//����ȡ��inode��Ӧ�Ĵ��̿飬������û������Ϊname��Ŀ¼��
	int i = 0;

	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	while (i < 160) {
		DirItem dirlist[16] = { 0 };
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		int j;
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 1) {
					//�ҵ���Ŀ¼���ж��Ƿ���н���Ȩ��
					if (((tmp.i_mode >> filemode >> 0) & 1) == 0 && strcmp(Cur_User_Name, "root") != 0) {	//root�û�����Ŀ¼�����Բ鿴 
						//û��ִ��Ȩ��
						printf("Ȩ�޲��㣺��ִ��Ȩ��\n");
						oss << "Ȩ�޲��㣺��ִ��Ȩ��\n";
						return -1;
					}

					//�ҵ���Ŀ¼������Ŀ¼��
					//returnһ��inode��ַ
					return dirlist[j].inodeAddr;
				}
				else {
					//�ҵ���Ŀ¼��������Ŀ¼��������
				}

			}

			i++;
		}

	}

	//���û�ҵ���ЩĿ¼
	return -1;
}

int FindFileInCurdir(int parinoAddr, const char* name) //��parinoAddr���ҵ���Ϊname���ļ���Ȼ�󷵻�����inode��ַ
{
	char findName[28];
	strcpy(findName, name);//�ȿ���

	//ȡ����ǰĿ¼��inode
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//����ȡ��inode��Ӧ�Ĵ��̿飬������û������Ϊname��Ŀ¼��
	int i = 0;

	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	while (i < 160) {
		DirItem dirlist[16] = { 0 };
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		int j;
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 0) {//˵�����ļ�
					//�ҵ���Ŀ¼���ж��Ƿ���н���Ȩ��
					if (((tmp.i_mode >> filemode >> 1) & 1) == 0 && strcmp(Cur_User_Name, "root") != 0) {	//root�û�����Ŀ¼������д����ִ�е���copy
						//û��ִ��Ȩ��
						printf("Ȩ�޲��㣺��д��Ȩ��,�޷�copy\n");
						oss << "Ȩ�޲��㣺��д��Ȩ��,�޷�copy\n";
						return -1;
					}

					//�ҵ���Ŀ¼�������ļ���
					//returnһ��inode��ַ
					return dirlist[j].inodeAddr;
				}
				else {
					//�ҵ���Ŀ¼��������Ŀ¼��������
				}

			}

			i++;
		}

	}

	//���û�ҵ�����ļ�
	return -1;
}
char* parsePath(const char* path, char* Cur_Dir_Name, char* Root_Dir_Name) {
	char* pathCopy = strdup(path);  // ����·���������޸�ԭʼ·��
	char* token = strtok(pathCopy, "/");  // ʹ�� '/' ��Ϊ�ָ����и�·��
	char* baseDir = nullptr;

	// �ж�·���Ǿ���·���������·��
	if (path[0] == '/') {
		// ����·�����Ӹ�Ŀ¼��ʼ
		baseDir = new char[strlen(Root_Dir_Name) + 1];  // Ϊ baseDir �����㹻���ڴ�
		strcpy(baseDir, Root_Dir_Name);  // ��������
	}
	else {
		// ���·�����Ե�ǰĿ¼Ϊ����
		baseDir = new char[strlen(Cur_Dir_Name) + 1];  // Ϊ baseDir �����㹻���ڴ�
		strcpy(baseDir, Cur_Dir_Name);  // ��������
	}

	// ����·�����
	while (token != nullptr) {
		if (strcmp(token, ".") == 0) {
			// ��ǰĿ¼��ʲô������
		}
		else if (strcmp(token, "..") == 0) {
			// �ϼ�Ŀ¼��ɾ�����һ�����
			int len = strlen(baseDir);
			// �����ǰ·�����Ǹ�Ŀ¼�����Ƴ����һ�����
			if (len > 1) {
				for (int i = len - 1; i >= 0; --i) {
					if (baseDir[i] == '/' && i == 0)
					{
						baseDir[0] = '/';
						baseDir[1] = '\0';
					}
					if (baseDir[i] == '/'&&i!=0) {
						baseDir[i] = '\0';
						break;
					}

				}
			}
			else if (len == 1) {
				// �����ǰĿ¼���Ǹ�Ŀ¼����������
				//std::cout << "Already at root directory." << std::endl;
			}
		}
		else {
			// ����·���������ӵ���ǰ·����
			if (baseDir[strlen(baseDir) - 1] != '/') {
				strcat(baseDir, "/");  // ��� '/' �ָ���
			}
			strcat(baseDir, token);  // ����ǰ�����ӵ�·����
		}

		token = strtok(nullptr, "/");  // �����и���һ�����
	}

	return baseDir;
}


void readFileFromHost(const char* hostFilePath) {
	// ��Դ�ļ� (�����ϵ��ļ�)
	//��һ�����е��ļ����豸��������һ�����,���ļ����ж�д
	HANDLE hFile = CreateFileA(hostFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		//std::cerr << "�򿪱����ļ�ʧ��" << std::endl;
		printf("�򿪱����ļ�ʧ��\n");
		oss << "�򿪱����ļ�ʧ��\n";
		return;
	}

	// ��ȡ�ļ���С
	DWORD fileSize = GetFileSize(hFile, NULL);

	// ��ȡ�ļ�����
	//char* buffer = new char[fileSize];
	DWORD bytesRead;//����ʵ�ʴ洢��ȡ���ֽ���
	if (!ReadFile(hFile, readFromHost, fileSize, &bytesRead, NULL)) {
		//std::cerr << "��ȡ�����ļ�ʧ��" << std::endl;
		printf("��ȡ�����ļ�ʧ��\n");
		oss << "��ȡ�����ļ�ʧ��\n";
		CloseHandle(hFile);
		//delete[] buffer;
		memset(readFromHost, 0, sizeof(readFromHost));
		return;
	}
	CloseHandle(hFile);
	if (bytesRead > BLOCK_SIZE * 10)//������ֻ����10��ֱ�ӿ�
	{
		printf("���������ļ����ڴ��޷�д��simulink��\n");
		oss << "���������ļ����ڴ��޷�д��simulink��\n";
		//delete[] buffer;
		memset(readFromHost, 0, sizeof(readFromHost));
		return;
	}
	// ���ļ�����д��ģ��� Linux �ļ�ϵͳ 

}

void writeFileToHost(const char* hostFilePath,int bufferSize)
{
	// ��Ŀ���ļ�������ļ��������򴴽������ļ��Ѵ����򸲸�
	HANDLE hFile = CreateFileA(
		hostFilePath,          // �ļ�·��
		GENERIC_WRITE,         // дȨ��
		0,                     // ������
		NULL,                  // Ĭ�ϰ�ȫ����
		CREATE_ALWAYS,         // ����ļ��Ѵ����򸲸�
		FILE_ATTRIBUTE_NORMAL, // ��ͨ�ļ�����
		NULL                   // ����Ҫģ���ļ�
	);

	// ����ļ��Ƿ�ɹ���
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("���ļ�ʧ��\n" );
		oss << "���ļ�ʧ��\n";
		return;
	}

	// д���ļ�
	DWORD bytesWritten;
	BOOL writeResult = WriteFile(
		hFile,         // �ļ����
		readFileFromHost,        // Ҫд��Ļ�����
		bufferSize,    // д����ֽ���
		&bytesWritten, // ʵ��д����ֽ���
		NULL           // ��ʹ���ص�I/O
	);

	// ���д���Ƿ�ɹ�
	if (!writeResult) {
		printf("д���ļ�ʧ��\n");
		oss << "д���ļ�ʧ��\n";
	}
	else {
		std::cout << "�ɹ�д�� " << bytesWritten << " �ֽڵ��ļ���" << std::endl;
		oss << "�ɹ�д�� " << bytesWritten << "�ֽڵ��ļ�\n";
	}

	// �ر��ļ����
	CloseHandle(hFile);
}

void readFromFile(int parinoAddr, const char name[])
{
	//�ȰѸ�Ŀ¼��Inode�ڵ��ȡ����
	Inode cur;
	Inode fileInode;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	DirItem dirlist[16];	//��ʱĿ¼�嵥

	//����ȡ��inode��Ӧ�Ĵ��̿飬������û������Ϊname��Ŀ¼��



	//�ж�Ȩ��
	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//�ļ���inode��ַ
	bool isExist = false;	//�ļ��Ƿ��Ѵ���
	while (i < 160) {
		//160��Ŀ¼��֮�ڣ�����ֱ����ֱ�ӿ�����
		dno = i / 16;	//�ڵڼ���ֱ�ӿ���

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//ȡ���ô��̿��Ŀ¼�嵥
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//����filenode�д洢
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//���ļ���������������ļ������༭	
					fileInodeAddr = dirlist[j].inodeAddr;
					isExist = true;

				}

			}
			i++;
		}
	}

	//�ж��Ƿ��������ļ�
	if (!isExist)
	{
		printf("�ļ�������\n");
		oss << "�ļ�������\n";
		return;
	}
	//�ж���û�ж�ȡȨ��
	if (((fileInode.i_mode >> filemode >> 2) & 1) == 0)
	{
		printf("û�ж�ȡȨ��\n");
		oss << "û�ж�ȡȨ��\n";
		return;
	}


	//���ļ����ݶ�ȡ��������ʾ�ڣ�������
	i = 0;
	int sumlen = fileInode.i_size;	//�ļ�����
	int getlen = 0;	//ȡ�����ĳ���
	//char buf[10000] = { 0 };
	for (i = 0; i < 10; i++) {
		char fileContent[1000] = { 0 };
		if (fileInode.i_dirBlock[i] == -1) {
			continue;
		}
		//ȡ����ǰ�ļ���ֱ�ӿ�
		fseek(file, fileInode.i_dirBlock[i], SEEK_SET);
		fread(fileContent, superblock->s_BLOCK_SIZE, 1, file);	//��ȡ��һ�����̿��С������

		//����ַ���
		int curlen = 0;	//��ǰָ��
		while (curlen < superblock->s_BLOCK_SIZE) {
			if (getlen >= sumlen)	//ȫ��������
				break;
			//printf("%c", fileContent[curlen]);	//�������Ļ 
			readFromHost[getlen++] = fileContent[curlen];	//�����buf
			curlen++;

		}
		if (getlen >= sumlen)
			break;
	}
	//printf("\n");
	//oss << "\n";
	return;
}

bool is_prefix(const std::string& str, const std::string& prefix) {
	if (str.length() < prefix.length()) {
		return false;
	}
	std::string substring = str.substr(0, prefix.length());
	return substring == prefix;
}

void init()
{
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMemory), SHM_NAME);
	if (hMapFile == NULL) {
		std::cerr << "���������ڴ�ʧ��" << std::endl;
		oss << "���������ڴ�ʧ��\n";
		return;
	}
	sharedMemory = (SharedMemory*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory));
	if (sharedMemory == NULL) {
		std::cerr << "�����ڴ�ӳ��ʧ��" << std::endl;
		oss << "�����ڴ�ӳ��ʧ��\n";
		CloseHandle(hMapFile);
		return;
	}

	//��ʼ��һЩ����
	sharedMemory->request.pid = 0;
	memset(sharedMemory->request.data, 0, sizeof(char) * 2048);
	sharedMemory->request.id = 0;
	sharedMemory->request.type = 'y';
	sharedMemory->request.option = Option::NONE;
	memset(sharedMemory->response.data, 0, sizeof(char) * 2000);
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
		oss << "�޷������ź���1\n";
		return;
	}

	// �����ź��� 2 (��ʼֵΪ0)
	hSemaphore2 = CreateSemaphore(NULL, 0, 1, szName2);
	if (hSemaphore2 == NULL) {
		std::cerr << "�޷������ź��� 2, �������: " << GetLastError() << std::endl;
		oss << "�޷������ź��� 2\n";
		CloseHandle(hSemaphore1);
		return;
	}
	std::ofstream output("C:\\Users\\86183\\Desktop\\ids.txt");
	output << hSemaphore1 << ' ' << hSemaphore2;
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

bool recursivelyLs(int parinoAddr, const char* name)
{
	std::vector<int>dir_in_it;
	std::string base = std::string(name);
	std::vector<std::string>nameofdir;
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if (((cur.i_mode >> filemode >> 2) & 1) == 0) {
		//û�ж�ȡȨ��
		printf("Ȩ�޲��㣺�޶�ȡȨ��\n");
		oss << "Ȩ�޲��㣺�޶�ȡȨ��\n";
		return false;
	}

	//����ȡ�����̿�
	int i = 0;
	//��ӡ·��
	oss << name;
	printf(name);
	oss << ":";
	printf(":");
	printf("\n");
	oss << "\n";
	while (i < cnt && i < 160) {
		DirItem dirlist[16] = { 0 };
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//����ô��̿��е�����Ŀ¼��
		int j;
		
		for (j = 0; j < 16 && i < cnt; j++) {
			Inode tmp;
			//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, "") == 0) {
				continue;
			}

			//�����Ϣ
			if (((tmp.i_mode >> 9) & 1) == 1) {
				printf("d");
				oss << "d";
				if ((strcmp(dirlist[j].itemName, ".") != 0) && (strcmp(dirlist[j].itemName, "..") != 0))
				{
					dir_in_it.push_back(dirlist[j].inodeAddr);
					nameofdir.push_back(base + '/' + dirlist[j].itemName);
				}
			}
			else {
				printf("-");
				oss << "-";
			}
			//cout << tmp.i_mtime << "���Ҳ����" << std::endl;
			tm* ptr;	//�洢ʱ��
			ptr = gmtime(&tmp.i_mtime);

			//���Ȩ����Ϣ
			int t = 8;
			while (t >= 0) {
				if (((tmp.i_mode >> t) & 1) == 1) {
					if (t % 3 == 2) {
						printf("r");
						oss << "r";
					}
					if (t % 3 == 1) {
						printf("w");
						oss << "w";
					}
					if (t % 3 == 0) {
						printf("x");
						oss << "x";
					}
				}
				else {
					printf("-");
					oss << "-";
				}
				t--;
			}
			printf("\t");
			oss << "\t";

			//����
			printf("%d\t", tmp.i_cnt);	//����
			oss << tmp.i_cnt << "\t";
			printf("%s\t", tmp.i_uname);	//�ļ������û���
			oss << tmp.i_uname << "\t";
			printf("%s\t", tmp.i_gname);	//�ļ������û���
			oss << tmp.i_gname << "\t";
			printf("%d B\t", tmp.i_size);	//�ļ���С
			oss << tmp.i_size << " B\t";
			if (ptr != nullptr)
			{
				printf("%d.%d.%d %02d:%02d:%02d  ", 1900 + ptr->tm_year, ptr->tm_mon + 1, ptr->tm_mday, (8 + ptr->tm_hour) % 24, ptr->tm_min, ptr->tm_sec);	//��һ���޸ĵ�ʱ��

				oss << (1900 + ptr->tm_year) << "."
					<< (ptr->tm_mon + 1) << "."
					<< ptr->tm_mday << " "
					<< std::setfill('0') << std::setw(2) << (8 + ptr->tm_hour) % 24 << ":"
					<< std::setw(2) << ptr->tm_min << ":"
					<< std::setw(2) << ptr->tm_sec << "\t";
			}
			printf("%s", dirlist[j].itemName);	//�ļ���
			oss << dirlist[j].itemName;
			printf("\n");
			oss << "\n";
			i++;
		}

	}
	//���еݹ�
	bool is;
	for (int i=0;i<dir_in_it.size();i++)
	{
		is=recursivelyLs(dir_in_it[i], nameofdir[i].c_str());
		if (!is)
		{
			return false;
		}
	}
	/*  δд�� */
	return true;
}

bool find_file_is_empty(int parinoAddr, const char name[])
{
	//�������ַȡ��inode
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//ȡ��Ŀ¼����
	int cnt = cur.i_cnt;

	//�ж��ļ�ģʽ��6Ϊowner��3Ϊgroup��0Ϊother
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if ((((cur.i_mode >> filemode >> 2) & 1) == 0) && (strcmp(Cur_User_Name, "root") != 0)) {
		//û��д��Ȩ��
		printf("Ȩ�޲��㣺�޶�ȡȨ��\n");
		oss << "Ȩ�޲��㣺�޶�ȡȨ��\n";
		return false;
	}


	//����ȡ�����̿�
	int i = 0;
	while (i < 160) {	//С��160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//ȡ�����̿�
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//�ҵ�Ҫɾ����Ŀ¼
		int j;
		for (j = 0; j < 16; j++) {
			Inode tmp;
			//ȡ����Ŀ¼���inode���жϸ�Ŀ¼����Ŀ¼�����ļ�
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, name) == 0) {
				if (((tmp.i_mode >> 9) & 1) == 1) {	//�ҵ�Ŀ¼
					//��Ŀ¼
					if (tmp.i_cnt <= 2)
					{
						return true;//��ʾ����ļ���Ϊ�գ�����ֱ��ɾ��
					}
				}
				else {
					//����Ŀ¼������
				}
			}
			i++;
		}

	}
	//printf("û���ҵ���Ŀ¼\n");
	//oss << "û���ҵ���Ŀ¼\n";
	return false;
}