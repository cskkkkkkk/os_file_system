#include "FileSystem.h"
#pragma warning(disable:4996)

using namespace std;

//函数实现
void Ready()	//登录系统前的准备工作,变量初始化+注册+安装
{
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


	char c;
	printf("是否格式化?[y/n]");
	// 获取一个字符，不需要按 Enter
	while (c = getch()) {
		//有可能有问题
		fflush(stdin);

		if (c == 'y') {
			printf("\n");
			printf("文件系统正在格式化……\n");
			//格式化一个虚拟磁盘文件
			if (!Format()) {
				printf("文件系统格式化失败\n");
				return;
			}
			printf("格式化完成\n");
			break;
		}
		else if (c == 'n') {
			printf("\n");
			break;
		}
	}

	//printf("载入文件系统……\n");
	if (!Install()) {
		printf("安装文件系统失败\n");
		return;
	}
	//printf("载入完成\n");
}

bool Format()	//格式化一个虚拟磁盘文件
{
	int i, j;

	//初始化超级块
	superblock->s_INODE_NUM = INODE_NUM;
	superblock->s_BLOCK_NUM = BLOCK_NUM;
	superblock->s_SUPERBLOCK_SIZE = sizeof(SuperBlock);
	superblock->s_INODE_SIZE = INODE_SIZE;
	superblock->s_BLOCK_SIZE = BLOCK_SIZE;
	superblock->s_free_INODE_NUM = INODE_NUM;
	superblock->s_free_BLOCK_NUM = BLOCK_NUM;
	superblock->s_blocks_per_group = BLOCKS_PER_GROUP;
	superblock->s_free_addr = Block_StartAddr;	//空闲块堆栈指针为第一块block
	superblock->s_Superblock_StartAddr = Superblock_StartAddr;
	superblock->s_BlockBitmap_StartAddr = BlockBitmap_StartAddr;
	superblock->s_InodeBitmap_StartAddr = InodeBitmap_StartAddr;
	superblock->s_Block_StartAddr = Block_StartAddr;
	superblock->s_Inode_StartAddr = Inode_StartAddr;
	//空闲块堆栈在后面赋值

	//初始化inode位图
	memset(inode_bitmap, 0, sizeof(inode_bitmap));
	//fseek(fw, InodeBitmap_StartAddr, SEEK_SET);
	fseek(file, InodeBitmap_StartAddr, SEEK_SET);
	//将数组内容写入到一个文件fw中
	//fwrite(inode_bitmap, sizeof(inode_bitmap), 1, fw);
	fwrite(inode_bitmap, sizeof(inode_bitmap), 1, file);
	fflush(file);
	//初始化block位图
	memset(block_bitmap, 0, sizeof(block_bitmap));
	//将文件指针移动到start_addr上然后将为0的块位图给写入，SEEK_SET的意思是说从文件的起始位置挪到起始地址
	//fseek(fw, BlockBitmap_StartAddr, SEEK_SET);
	fseek(file, BlockBitmap_StartAddr, SEEK_SET);
	//fwrite(block_bitmap, sizeof(block_bitmap), 1, fw);
	fwrite(block_bitmap, sizeof(block_bitmap), 1, file);
	fflush(file);

	//初始化磁盘块区，根据成组链接法组织	
	for (i = BLOCK_NUM / BLOCKS_PER_GROUP - 1; i >= 0; i--) {	//一共INODE_NUM/BLOCKS_PER_GROUP组=160组，一组FREESTACKNUM（128）个磁盘块 ，第一个磁盘块作为索引
		if (i == BLOCK_NUM / BLOCKS_PER_GROUP - 1)
			superblock->s_free[0] = -1;	//没有下一个空闲块了
		else
			superblock->s_free[0] = Block_StartAddr + (i + 1) * BLOCKS_PER_GROUP * BLOCK_SIZE;	//指向下一个空闲块
		for (j = 1; j < BLOCKS_PER_GROUP; j++) {
			superblock->s_free[j] = Block_StartAddr + (i * BLOCKS_PER_GROUP + j) * BLOCK_SIZE;
		}

		//fseek(fw, Block_StartAddr + i * BLOCKS_PER_GROUP * BLOCK_SIZE, SEEK_SET);
		fseek(file, Block_StartAddr + i * BLOCKS_PER_GROUP * BLOCK_SIZE, SEEK_SET);
		//fwrite(superblock->s_free, sizeof(superblock->s_free), 1, fw);	//填满这个磁盘块，512字节
		fwrite(superblock->s_free, sizeof(superblock->s_free), 1, file);	//填满这个磁盘块，512字节
		fflush(file);
	}
	//超级块写入到虚拟磁盘文件
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	fwrite(superblock, sizeof(SuperBlock), 1, file);
	//fflush(fw);
	fflush(file);

	//读取inode位图
	//fseek(fr, InodeBitmap_StartAddr, SEEK_SET);
	fseek(file, InodeBitmap_StartAddr, SEEK_SET);
	//fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);
	fread(inode_bitmap, sizeof(inode_bitmap), 1, file);
	//读取block位图
	//fseek(fr, BlockBitmap_StartAddr, SEEK_SET);
	fseek(file, BlockBitmap_StartAddr, SEEK_SET);
	//fread(block_bitmap, sizeof(block_bitmap), 1, fr);
	fread(block_bitmap, sizeof(block_bitmap), 1, file);
	//fflush(fr);





	//创建根目录 "/"
	Inode cur;

	//申请inode
	int inoAddr = ialloc();

	//给这个inode申请磁盘块
	int blockAddr = balloc();

	//在这个磁盘块里加入一个条目 "."
	DirItem dirlist[16] = { 0 };
	strcpy(dirlist[0].itemName, ".");
	dirlist[0].inodeAddr = inoAddr;

	//写回磁盘块
	//fseek(fw, blockAddr, SEEK_SET);
	fseek(file, blockAddr, SEEK_SET);
	//fwrite(dirlist, sizeof(dirlist), 1, fw);
	fwrite(dirlist, sizeof(dirlist), 1, file);
	//fflush(fw);
	fflush(file);
	//上面都是为了初始化根目录





	//给inode赋值
	cur.i_ino = 0;
	cur.i_atime = time(NULL);
	cur.i_ctime = time(NULL);
	cur.i_mtime = time(NULL);
	strcpy(cur.i_uname, Cur_User_Name);
	strcpy(cur.i_gname, Cur_Group_Name);
	cur.i_cnt = 1;	//一个项，当前目录,"."
	cur.i_dirBlock[0] = blockAddr;
	for (i = 1; i < 10; i++) {
		cur.i_dirBlock[i] = -1;
	}
	cur.i_size = superblock->s_BLOCK_SIZE;
	cur.i_indirBlock_1 = -1;	//没使用一级间接块
	cur.i_mode = MODE_DIR | DIR_DEF_PERMISSION;


	//写回inode
	//fseek(fw, inoAddr, SEEK_SET);
	fseek(file, inoAddr, SEEK_SET);
	//fwrite(&cur, sizeof(Inode), 1, fw);
	fwrite(&cur, sizeof(Inode), 1, file);
	//fflush(fw);
	fflush(file);

	//创建目录及配置文件
	mkdir(inoAddr, "home");	//用户目录
	cd(Root_Dir_Addr, "home",true);

	//
	mkdir(Cur_Dir_Addr, "root");

	cd(Cur_Dir_Addr, "..",true);
	//


	mkdir(Cur_Dir_Addr, "etc");	//配置文件目录
	cd(Cur_Dir_Addr, "etc",true);

	char buf[1000] = { 0 };

	sprintf(buf, "root:x:%d:%d\n", nextUID++, nextGID++);	//增加条目，用户名：加密密码：用户ID：用户组ID
	create(Cur_Dir_Addr, "passwd", buf);	//创建用户信息文件

	sprintf(buf, "root:root\n");	//增加条目，用户名：密码
	create(Cur_Dir_Addr, "shadow", buf);	//创建用户密码文件
	chmod(Cur_Dir_Addr, "shadow", 0660);	//修改权限，禁止其它用户读取该文件
	//每次调用springtf都会对buf进行覆盖
	sprintf(buf, "root::0:root\n");	//增加管理员用户组，用户组名：口令（一般为空，这里没有使用）：组标识号：组内用户列表（用，分隔）
	sprintf(buf + strlen(buf), "user::1:\n");	//增加普通用户组，组内用户列表为空
	create(Cur_Dir_Addr, "group", buf);	//创建用户组信息文件

	cd(Cur_Dir_Addr, "..",true);	//回到根目录
	//cd(Cur_Dir_Addr, "..");
	return true;
}

bool Install()	//安装文件系统，将虚拟磁盘文件中的关键信息如超级块读入到内存
{
	//读写虚拟磁盘文件，读取超级块，读取inode位图，block位图，读取主目录，读取etc目录，读取管理员admin目录，读取用户xiao目录，读取用户passwd文件。

	//读取超级块
	//fseek(fr, Superblock_StartAddr, SEEK_SET);
	//fread(superblock, sizeof(SuperBlock), 1, fr);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fread(superblock, sizeof(SuperBlock), 1, file);

	//读取inode位图
	//fseek(fr, InodeBitmap_StartAddr, SEEK_SET);
	//fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);
	fseek(file, InodeBitmap_StartAddr, SEEK_SET);
	fread(inode_bitmap, sizeof(inode_bitmap), 1, file);

	//读取block位图
	//fseek(fr, BlockBitmap_StartAddr, SEEK_SET);
	//fread(block_bitmap, sizeof(block_bitmap), 1, fr);
	fseek(file, BlockBitmap_StartAddr, SEEK_SET);
	fread(block_bitmap, sizeof(block_bitmap), 1, file);
	return true;
}

void printSuperBlock()		//打印超级块信息
{
	
	printf("\n");
	printf("空闲inode数 / 总inode数 ：%d / %d\n", superblock->s_free_INODE_NUM, superblock->s_INODE_NUM);
	oss << "\n空闲inode数 / 总inode数 ："<< superblock->s_free_INODE_NUM << " / " << superblock->s_INODE_NUM << "\n";
	printf("空闲block数 / 总block数 ：%d / %d\n", superblock->s_free_BLOCK_NUM, superblock->s_BLOCK_NUM);
	oss << "空闲block数 / 总block数 ：" << superblock->s_free_BLOCK_NUM << " / " << superblock->s_BLOCK_NUM << "\n";
	//printf("本系统 block大小：%d 字节，每个inode占 %d 字节（真实大小：%d 字节）\n", superblock->s_BLOCK_SIZE, superblock->s_INODE_SIZE, sizeof(Inode));
	printf("本系统 block大小：%d 字节，每个inode占 %d 字节（真实大小：%d 字节）\n", 1024, superblock->s_INODE_SIZE, sizeof(Inode));
	oss << "本系统 block大小：" << 1024 << " 字节，每个inode占 "
		<< superblock->s_INODE_SIZE << " 字节（真实大小："
		<< sizeof(Inode) << " 字节）\n";
	printf("\t每磁盘块组（空闲堆栈）包含的block数量：%d\n", superblock->s_blocks_per_group);
	oss << "本系统 block大小：" << 1024 << " 字节，每个inode占 "
		<< superblock->s_INODE_SIZE << " 字节（真实大小："
		<< sizeof(Inode) << " 字节）\n";
	printf("\t超级块占 %d 字节（真实大小：%d 字节）\n", superblock->s_BLOCK_SIZE, superblock->s_SUPERBLOCK_SIZE);
	oss << "\t每磁盘块组（空闲堆栈）包含的block数量："
		<< superblock->s_blocks_per_group << "\n";
	printf("虚拟磁盘总大小:100M\n");
	oss << "虚拟磁盘总大小:100M\n";
	printf("磁盘分布：\n");
	printf("\t超级块开始位置：%d B\n", superblock->s_Superblock_StartAddr);
	printf("\tinode位图开始位置：%d B\n", superblock->s_InodeBitmap_StartAddr);
	printf("\tblock位图开始位置：%d B\n", superblock->s_BlockBitmap_StartAddr);
	printf("\tinode区开始位置：%d B\n", superblock->s_Inode_StartAddr);
	printf("\tblock区开始位置：%d B\n", superblock->s_Block_StartAddr);
	printf("\n");
	oss << "磁盘分布：\n";
	oss << "\t超级块开始位置：" << superblock->s_Superblock_StartAddr << " B\n";
	oss << "\tinode位图开始位置：" << superblock->s_InodeBitmap_StartAddr << " B\n";
	oss << "\tblock位图开始位置：" << superblock->s_BlockBitmap_StartAddr << " B\n";
	oss << "\tinode区开始位置：" << superblock->s_Inode_StartAddr << " B\n";
	oss << "\tblock区开始位置：" << superblock->s_Block_StartAddr << " B\n";
	oss << "\n"; // 结束一个换行
	return;
}

void printInodeBitmap()	//打印inode使用情况
{
	
	printf("\n");
	printf("inode使用表：[uesd:%d %d/%d]\n", superblock->s_INODE_NUM - superblock->s_free_INODE_NUM, superblock->s_free_INODE_NUM, superblock->s_INODE_NUM);
	oss << "\n";
	oss << "inode使用表：[used:"
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

void printBlockBitmap(int num)	//打印block使用情况
{

	printf("\n");
	printf("block（磁盘块）使用表：[used:%d %d/%d]\n", superblock->s_BLOCK_NUM - superblock->s_free_BLOCK_NUM, superblock->s_free_BLOCK_NUM, superblock->s_BLOCK_NUM);
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

int balloc()	//磁盘块分配函数
{
	//使用超级块中的空闲块堆栈
	//计算当前栈顶
	int top;	//栈顶指针
	if (superblock->s_free_BLOCK_NUM == 0) {	//剩余空闲块数为0
		printf("没有空闲块可以分配\n");
		oss << "没有空闲块可以分配\n";
		return -1;	//没有可分配的空闲块，返回-1
	}
	else {	//还有剩余块
		top = (superblock->s_free_BLOCK_NUM - 1) % superblock->s_blocks_per_group;
	}
	//将栈顶取出
	//如果已是栈底，将当前块号地址返回，即为栈底块号，并将栈底指向的新空闲块堆栈覆盖原来的栈
	int retAddr;

	if (top == 0) {
		retAddr = superblock->s_free_addr;
		superblock->s_free_addr = superblock->s_free[0];	//取出下一个存有空闲块堆栈的空闲块的位置，更新空闲块堆栈指针

		//取出对应空闲块内容，覆盖原来的空闲块堆栈

		//取出下一个空闲块堆栈，覆盖原来的,将指针移动到superblock->s_free_addr
		//fseek(fr, superblock->s_free_addr, SEEK_SET);
		fseek(file, superblock->s_free_addr, SEEK_SET);
		//下一组空闲块堆栈的数据从磁盘文件中加载到内存(superblock->s_free)中
		//fread(superblock->s_free, sizeof(superblock->s_free), 1, fr);
		fread(superblock->s_free, sizeof(superblock->s_free), 1, file);
		//fflush(fr);

		superblock->s_free_BLOCK_NUM--;

	}
	else {	//如果不为栈底，则将栈顶指向的地址返回，栈顶指针-1.
		retAddr = superblock->s_free[top];	//保存返回地址
		superblock->s_free[top] = -1;	//清栈顶
		top--;		//栈顶指针-1
		superblock->s_free_BLOCK_NUM--;	//空闲块数-1

	}

	//更新超级块
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	//fflush(fw);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, file);
	fflush(file);


	//更新block位图
	block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE] = 1;
	//fseek(fw, (retAddr - Block_StartAddr) / BLOCK_SIZE + BlockBitmap_StartAddr, SEEK_SET);	//(retAddr-Block_StartAddr)/BLOCK_SIZE为第几个空闲块
	//fwrite(&block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE], sizeof(bool), 1, fw);
	//fflush(fw);
	fseek(file, (retAddr - Block_StartAddr) / BLOCK_SIZE + BlockBitmap_StartAddr, SEEK_SET);	//(retAddr-Block_StartAddr)/BLOCK_SIZE为第几个空闲块
	fwrite(&block_bitmap[(retAddr - Block_StartAddr) / BLOCK_SIZE], sizeof(bool), 1, file);
	fflush(file);

	return retAddr;

}

bool bfree(int addr)	//磁盘块释放函数
{
	//判断
	//该地址不是磁盘块的起始地址
	if ((addr - Block_StartAddr) % superblock->s_BLOCK_SIZE != 0) {
		printf("地址错误,该位置不是block（磁盘块）起始位置\n");
		oss << "地址错误,该位置不是block（磁盘块）起始位置\n";
		return false;
	}
	unsigned int bno = (addr - Block_StartAddr) / superblock->s_BLOCK_SIZE;	//inode节点号
	//该地址还未使用，不能释放空间
	if (block_bitmap[bno] == 0) {
		printf("该block（磁盘块）还未使用，无法释放\n");
		oss << "该block（磁盘块）还未使用，无法释放\n";
		return false;
	}

	//可以释放
	//计算当前栈顶
	int top;	//栈顶指针
	if (superblock->s_free_BLOCK_NUM == superblock->s_BLOCK_NUM) {	//没有非空闲的磁盘块
		printf("没有非空闲的磁盘块，无法释放\n");
		oss << "没有非空闲的磁盘块，无法释放\n";
		return false;	//没有可分配的空闲块，返回-1
	}//意思是当前空闲的磁盘块数量已经和最大磁盘块数量一样说明不能再释放了
	else {	//非满
		//减一是为了找到栈顶位置，不减1找到的就是下一组栈的起始位置
		top = (superblock->s_free_BLOCK_NUM - 1) % superblock->s_blocks_per_group;

		//清空block内容
		char tmp[BLOCK_SIZE] = { 0 };
		//fseek(fw, addr, SEEK_SET);
		//fwrite(tmp, sizeof(tmp), 1, fw);
		fseek(file, addr, SEEK_SET);
		fwrite(tmp, sizeof(tmp), 1, file);
		fflush(file);
		if (top == superblock->s_blocks_per_group - 1) {	//该栈已满

			//该空闲块作为新的空闲块堆栈
			superblock->s_free[0] = superblock->s_free_addr;	//新的空闲块堆栈第一个地址指向旧的空闲块堆栈指针
			int i;
			for (i = 1; i < superblock->s_blocks_per_group; i++) {
				superblock->s_free[i] = -1;	//清空栈元素的其它地址
			}

			//fseek(fw, addr, SEEK_SET);
			//fwrite(superblock->s_free, sizeof(superblock->s_free), 1, fw);	//填满这个磁盘块，512字节
			fseek(file, addr, SEEK_SET);
			fwrite(superblock->s_free, sizeof(superblock->s_free), 1, file);	//填满这个磁盘块，512字节
			fflush(file);
		}
		else {	//栈还未满
			top++;	//栈顶指针+1
			superblock->s_free[top] = addr;	//栈顶放上这个要释放的地址，作为新的空闲块
		}
	}


	//更新超级块
	superblock->s_free_BLOCK_NUM++;	//空闲块数+1
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, file);

	//更新block位图
	block_bitmap[bno] = 0;
	//fseek(fw, bno + BlockBitmap_StartAddr, SEEK_SET);	//(addr-Block_StartAddr)/BLOCK_SIZE为第几个空闲块
	//fwrite(&block_bitmap[bno], sizeof(bool), 1, fw);
	//fflush(fw);
	fseek(file, bno + BlockBitmap_StartAddr, SEEK_SET);	//(addr-Block_StartAddr)/BLOCK_SIZE为第几个空闲块
	fwrite(&block_bitmap[bno], sizeof(bool), 1, file);
	fflush(file);
	return true;
}

int ialloc()	//分配i节点区函数，返回inode地址
{
	//在inode位图中顺序查找空闲的inode，找到则返回inode地址。函数结束。
	if (superblock->s_free_INODE_NUM == 0) {
		printf("没有空闲inode可以分配\n");
		oss << "没有空闲inode可以分配\n";
		return -1;
	}
	else {

		//顺序查找空闲的inode
		int i;
		for (i = 0; i < superblock->s_INODE_NUM; i++) {
			if (inode_bitmap[i] == 0)	//找到空闲inode
				break;
		}


		//更新超级块
		superblock->s_free_INODE_NUM--;	//空闲inode数-1
		//fseek(fw, Superblock_StartAddr, SEEK_SET);
		//fwrite(superblock, sizeof(SuperBlock), 1, fw);
		fseek(file, Superblock_StartAddr, SEEK_SET);
		fwrite(superblock, sizeof(SuperBlock), 1, file);



		//更新inode位图
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

bool ifree(int addr)	//释放i结点区函数
{
	//判断
	if ((addr - Inode_StartAddr) % superblock->s_INODE_SIZE != 0) {
		printf("地址错误,该位置不是i节点起始位置\n");
		oss << "地址错误,该位置不是i节点起始位置\n";
		return false;
	}
	unsigned short ino = (addr - Inode_StartAddr) / superblock->s_INODE_SIZE;	//inode节点号
	if (inode_bitmap[ino] == 0) {
		printf("该inode还未使用，无法释放\n");
		oss << "该inode还未使用，无法释放\n";
		return false;
	}

	//清空inode内容
	Inode tmp = { 0 };
	//fseek(fw, addr, SEEK_SET);
	//fwrite(&tmp, sizeof(tmp), 1, fw);
	fseek(file, addr, SEEK_SET);
	fwrite(&tmp, sizeof(tmp), 1, file);

	//更新超级块
	superblock->s_free_INODE_NUM++;
	//空闲inode数+1
	//fseek(fw, Superblock_StartAddr, SEEK_SET);
	//fwrite(superblock, sizeof(SuperBlock), 1, fw);
	fseek(file, Superblock_StartAddr, SEEK_SET);
	fwrite(superblock, sizeof(SuperBlock), 1, file);


	//更新inode位图
	inode_bitmap[ino] = 0;
	//fseek(fw, InodeBitmap_StartAddr + ino, SEEK_SET);
	//fwrite(&inode_bitmap[ino], sizeof(bool), 1, fw);
	//fflush(fw);
	fseek(file, InodeBitmap_StartAddr + ino, SEEK_SET);
	fwrite(&inode_bitmap[ino], sizeof(bool), 1, file);
	fflush(file);

	return true;
}

//在参数前面加了一个const值
bool mkdir(int parinoAddr, const char name[])	//目录创建函数。参数：上一层目录文件inode地址 ,要创建的目录名
{
	
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("超过最大目录名长度\n");
		oss << "超过最大目录名长度\n";
		return false;
	}

	DirItem dirlist[16];	//临时目录清单

	//从这个地址取出inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	std::cout << cur.i_ino << " " << cur.i_mode << " " << cur.i_cnt << "　" << cur.i_dirBlock[0] << endl;//全是0，仿佛我没写入根目录inode节点信息一样
	int i = 0;
	int cnt = cur.i_cnt + 1;	//目录项数
	int posi = -1, posj = -1;
	while (i < 160) {
		//160个目录项之内，可以直接在直接块里找
		int dno = i / 16;	//在第几个直接块里

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//取出这个直接块，要加入的目录条目的位置
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);


		//输出该磁盘块中的所有目录项
		int j;
		for (j = 0; j < 16; j++) {

			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 1) {	//不是目录 //后面的注释表示是目录
					printf("目录已存在\n");
					oss << "目录已存在\n";
					return false;
				}
			}
			else if (strcmp(dirlist[j].itemName, "") == 0) {
				//找到一个空闲记录，将新目录创建到这个位置 
				//记录这个位置
				if (posi == -1) {
					//posi表示的是在第几个直接块里
					posi = dno;
					//表示的是磁盘块里的第几个目录项
					posj = j;

				}

			}

			i++;
		}

	}
	/*  未写完 */

	if (posi != -1) {	//找到这个空闲位置

		//取出这个直接块，要加入的目录条目的位置
		//fseek(fr, cur.i_dirBlock[posi], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);


		//创建这个目录项
		strcpy(dirlist[posj].itemName, name);	//目录名
		//写入两条记录 "." ".."，分别指向当前inode节点地址，和父inode节点
		int chiinoAddr = ialloc();	//分配当前节点地址 
		if (chiinoAddr == -1) {
			printf("inode分配失败\n");
			oss << "inode分配失败\n";
			return false;
		}
		dirlist[posj].inodeAddr = chiinoAddr; //给这个新的目录分配的inode地址

		//设置新条目的inode
		Inode p;
		p.i_ino = (chiinoAddr - Inode_StartAddr) / superblock->s_INODE_SIZE;
		p.i_atime = time(NULL);
		p.i_ctime = time(NULL);
		p.i_mtime = time(NULL);
		strcpy(p.i_uname, Cur_User_Name);
		strcpy(p.i_gname, Cur_Group_Name);
		p.i_cnt = 2;	//两个项，当前目录,"."和".."

		//分配这个inode的磁盘块，在磁盘号中写入两条记录 . 和 ..
		int curblockAddr = balloc();
		if (curblockAddr == -1) {
			printf("block分配失败\n");
			oss << "block分配失败\n";
			return false;
		}
		DirItem dirlist2[16] = { 0 };	//临时目录项列表 - 2
		strcpy(dirlist2[0].itemName, ".");
		strcpy(dirlist2[1].itemName, "..");
		dirlist2[0].inodeAddr = chiinoAddr;	//当前目录inode地址
		dirlist2[1].inodeAddr = parinoAddr;	//父目录inode地址

		//写入到当前目录的磁盘块
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
		p.i_indirBlock_1 = -1;	//没使用一级间接块
		p.i_mode = MODE_DIR | DIR_DEF_PERMISSION;

		//将inode写入到申请的inode地址
		//fseek(fw, chiinoAddr, SEEK_SET);
		//fwrite(&p, sizeof(Inode), 1, fw);
		fseek(file, chiinoAddr, SEEK_SET);
		fwrite(&p, sizeof(Inode), 1, file);
		//
		fflush(file);
		//
		//将当前目录的磁盘块写回
		//fseek(fw, cur.i_dirBlock[posi], SEEK_SET);
		//fwrite(dirlist, sizeof(dirlist), 1, fw);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fwrite(dirlist, sizeof(dirlist), 1, file);
		//
		fflush(file);
		// 
		//写回inode
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
		printf("没找到空闲目录项,目录创建失败");
		oss << "没找到空闲目录项,目录创建失败";
		return false;
	}
}

void rmall(int parinoAddr)	//删除该节点下所有文件或目录
{
	//从这个地址取出inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//取出目录项数
	int cnt = cur.i_cnt;
	if (cnt <= 2) {
		bfree(cur.i_dirBlock[0]);
		ifree(parinoAddr);
		return;
	}

	//依次取出磁盘块
	int i = 0;
	while (i < 160) {	//小于160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		//从磁盘块中依次取出目录项，递归删除
		int j;
		bool f = false;
		for (j = 0; j < 16; j++) {
			//Inode tmp;

			if (!(strcmp(dirlist[j].itemName, ".") == 0 ||
				strcmp(dirlist[j].itemName, "..") == 0 ||
				strcmp(dirlist[j].itemName, "") == 0)) {
				f = true;
				rmall(dirlist[j].inodeAddr);	//递归删除
			}

			cnt = cur.i_cnt;
			i++;
		}

		//这个地方有疑惑
		//该磁盘块已空，回收
		if (f)
			bfree(parblockAddr);

	}
	//该inode已空，回收
	ifree(parinoAddr);
	return;

}

bool rmdir(int parinoAddr, char name[])	//目录删除函数
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("超过最大目录名长度\n");
		oss << "超过最大目录名长度\n";
		return false;
	}

	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		printf("错误操作\n");
		oss << "错误操作\n";
		return 0;
	}

	//从这个地址取出inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//取出目录项数
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if ((((cur.i_mode >> filemode >> 1) & 1) == 0) && (strcmp(Cur_User_Name, "root") != 0)) {
		//没有写入权限
		printf("权限不足：无写入权限\n");
		oss << "权限不足：无写入权限\n";
		return false;
	}


	//依次取出磁盘块
	int i = 0;
	while (i < 160) {	//小于160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//找到要删除的目录
		int j;
		for (j = 0; j < 16; j++) {
			Inode tmp;
			//取出该目录项的inode，判断该目录项是目录还是文件
			//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
			//fread(&tmp, sizeof(Inode), 1, fr);
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, name) == 0) {
				if (((tmp.i_mode >> 9) & 1) == 1) {	//找到目录
					//是目录

					rmall(dirlist[j].inodeAddr);

					//删除该目录条目，写回磁盘
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
					//不是目录，不管
				}
			}
			i++;
		}

	}

	printf("没有找到该目录\n");
	oss << "没有找到该目录\n";
	return false;
}

bool create(int parinoAddr, const char name[], char buf[])	//创建文件函数，在该目录下创建文件，文件内容存在buf
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("超过最大文件名长度\n");
		oss << "超过最大文件名长度\n";
		return false;
	}

	DirItem dirlist[16];	//临时目录清单

	//从这个地址取出inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	int i = 0;
	int posi = -1, posj = -1;	//找到的目录位置
	int dno;
	int cnt = cur.i_cnt + 1;	//目录项数
	while (i < 160) {
		//160个目录项之内，可以直接在直接块里找
		dno = i / 16;	//在第几个直接块里

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);



		//输出该磁盘块中的所有目录项
		int j;
		for (j = 0; j < 16; j++) {

			if (posi == -1 && strcmp(dirlist[j].itemName, "") == 0) {
				//找到一个空闲记录，将新文件创建到这个位置 
				posi = dno;
				posj = j;
			}
			else if (strcmp(dirlist[j].itemName, name) == 0) {
				//重名，取出inode，判断是否是文件
				Inode cur2;
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&cur2, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&cur2, sizeof(Inode), 1, file);
				if (((cur2.i_mode >> 9) & 1) == 0) {	//是文件且重名，不能创建文件
					printf("文件已存在\n");
					oss << "文件已存在\n";
					buf[0] = '\0';
					return false;
				}
			}
			i++;
		}

	}
	if (posi != -1) {	//之前找到一个目录项了
		//取出之前那个空闲目录项对应的磁盘块
		//fseek(fr, cur.i_dirBlock[posi], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);


		//创建这个目录项
		strcpy(dirlist[posj].itemName, name);	//文件名
		int chiinoAddr = ialloc();	//分配当前节点地址 
		if (chiinoAddr == -1) {
			printf("inode分配失败\n");
			oss << "inode分配失败\n";
			return false;
		}
		dirlist[posj].inodeAddr = chiinoAddr; //给这个新的目录分配的inode地址

		//设置新条目的inode
		Inode p;
		p.i_ino = (chiinoAddr - Inode_StartAddr) / superblock->s_INODE_SIZE;
		p.i_atime = time(NULL);
		p.i_ctime = time(NULL);
		p.i_mtime = time(NULL);
		strcpy(p.i_uname, Cur_User_Name);
		strcpy(p.i_gname, Cur_Group_Name);
		p.i_cnt = 1;	//只有一个文件指向


		//将buf内容存到磁盘块 
		int k;
		int len = strlen(buf);	//文件长度，单位为字节
		for (k = 0; k < len; k += superblock->s_BLOCK_SIZE) {	//最多10次，10个磁盘快，即最多5K
			//分配这个inode的磁盘块，从控制台读取内容
			int curblockAddr = balloc();
			if (curblockAddr == -1) {
				printf("block分配失败\n");
				oss << "block分配失败\n";
				return false;
			}
			p.i_dirBlock[k / superblock->s_BLOCK_SIZE] = curblockAddr;
			//写入到当前目录的磁盘块
			//fseek(fw, curblockAddr, SEEK_SET);
			//fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, fw);
			fseek(file, curblockAddr, SEEK_SET);
			fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, file);
			fflush(file);
		}


		for (k = len / superblock->s_BLOCK_SIZE + 1; k < 10; k++) {
			p.i_dirBlock[k] = -1;
		}
		if (len == 0) {	//长度为0的话也分给它一个block
			int curblockAddr = balloc();
			if (curblockAddr == -1) {
				printf("block分配失败\n");
				oss << "block分配失败\n";
				return false;
			}
			p.i_dirBlock[k / superblock->s_BLOCK_SIZE] = curblockAddr;
			//写入到当前目录的磁盘块
			//fseek(fw, curblockAddr, SEEK_SET);
			//fwrite(buf, superblock->s_BLOCK_SIZE, 1, fw);
			fseek(file, curblockAddr, SEEK_SET);
			fwrite(buf, superblock->s_BLOCK_SIZE, 1, file);
			fflush(file);
		}
		p.i_size = len;
		p.i_indirBlock_1 = -1;	//没使用一级间接块
		p.i_mode = 0;
		p.i_mode = MODE_FILE | FILE_DEF_PERMISSION;

		//将inode写入到申请的inode地址
		//fseek(fw, chiinoAddr, SEEK_SET);
		//fwrite(&p, sizeof(Inode), 1, fw);
		fseek(file, chiinoAddr, SEEK_SET);
		fwrite(&p, sizeof(Inode), 1, file);


		//将当前目录的磁盘块写回
		//fseek(fw, cur.i_dirBlock[posi], SEEK_SET);
		//fwrite(dirlist, sizeof(dirlist), 1, fw);
		fseek(file, cur.i_dirBlock[posi], SEEK_SET);
		fwrite(dirlist, sizeof(dirlist), 1, file);


		//写回inode
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

bool del(int parinoAddr, char name[])		//删除文件函数。在当前目录下删除文件
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("超过最大目录名长度\n");
		oss << "超过最大目录名长度\n";
		return false;
	}

	//从这个地址取出inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//取出目录项数
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if (((cur.i_mode >> filemode >> 1) & 1) == 0) {
		//没有写入权限
		printf("权限不足：无写入权限\n");
		oss << "权限不足：无写入权限\n";
		return false;
	}

	//依次取出磁盘块
	int i = 0;
	while (i < 160) {	//小于160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		//找到要删除的目录
		int pos;
		for (pos = 0; pos < 16; pos++) {
			Inode tmp;
			//取出该目录项的inode，判断该目录项是目录还是文件
			//fseek(fr, dirlist[pos].inodeAddr, SEEK_SET);
			//fread(&tmp, sizeof(Inode), 1, fr);
			fseek(file, dirlist[pos].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);
			if (strcmp(dirlist[pos].itemName, name) == 0) {
				if (((tmp.i_mode >> 9) & 1) == 1) {	//找到目录
					//是目录，不管
				}
				else {
					//是文件

					//释放block
					int k;
					for (k = 0; k < 10; k++)
						if (tmp.i_dirBlock[k] != -1)
							bfree(tmp.i_dirBlock[k]);

					//释放inode
					ifree(dirlist[pos].inodeAddr);

					//删除该目录条目，写回磁盘
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

	printf("没有找到该文件!\n");
	oss << "没有找到该文件!\n";
	return false;
}


bool ls(int parinoAddr)		//显示当前目录下的所有文件和文件夹。参数：当前目录的inode节点地址 
{
	Inode cur;
	//取出这个inode
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	//fflush(fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//fflush(fr);
	//cout << cur.i_cnt << "可以" << std::endl;
	//取出目录项数
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if (((cur.i_mode >> filemode >> 2) & 1) == 0) {
		//没有读取权限
		printf("权限不足：无读取权限\n");
		oss << "权限不足：无读取权限\n";
		return false;
	}

	//依次取出磁盘块
	int i = 0;
	while (i < cnt && i < 160) {
		DirItem dirlist[16] = { 0 };
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);


		//输出该磁盘块中的所有目录项
		int j;
		for (j = 0; j < 16 && i < cnt; j++) {
			Inode tmp;
			//取出该目录项的inode，判断该目录项是目录还是文件
			//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
			//fread(&tmp, sizeof(Inode), 1, fr);
			//fflush(fr);
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, "") == 0) {
				continue;
			}

			//输出信息
			if (((tmp.i_mode >> 9) & 1) == 1) {
				printf("d");
				oss << "d";
			}
			else {
				printf("-");
				oss << "-";
			}
			//cout << tmp.i_mtime << "这个也可以" << std::endl;
			tm* ptr;	//存储时间
			ptr = gmtime(&tmp.i_mtime);

			//输出权限信息
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

			//其它
			printf("%d\t", tmp.i_cnt);	//链接
			oss << tmp.i_cnt << "\t";
			printf("%s\t", tmp.i_uname);	//文件所属用户名
			oss << tmp.i_uname << "\t";
			printf("%s\t", tmp.i_gname);	//文件所属用户名
			oss << tmp.i_gname << "\t";
			printf("%d B\t", tmp.i_size);	//文件大小
			oss << tmp.i_size << " B\t";
			if (ptr != nullptr)
			{
				printf("%d.%d.%d %02d:%02d:%02d  ", 1900 + ptr->tm_year, ptr->tm_mon + 1, ptr->tm_mday, (8 + ptr->tm_hour) % 24, ptr->tm_min, ptr->tm_sec);	//上一次修改的时间
		
				oss << (1900 + ptr->tm_year) << "."
					<< (ptr->tm_mon + 1) << "."
					<< ptr->tm_mday << " "
					<< std::setfill('0') << std::setw(2) << (8 + ptr->tm_hour) % 24 << ":"
					<< std::setw(2) << ptr->tm_min << ":"
					<< std::setw(2) << ptr->tm_sec<<"\t";
			}
			printf("%s", dirlist[j].itemName);	//文件名
			oss << dirlist[j].itemName;
			printf("\n");
			oss << "\n";
			i++;
		}

	}
	/*  未写完 */
	return true;
}

bool cd(int parinoAddr, const char name[],bool isRelativePath)	//进入当前目录下的name目录
{
	//取出当前目录的inode
	Inode cur;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//依次取出inode对应的磁盘块，查找有没有名字为name的目录项
	int i = 0;

	//取出目录项数
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
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
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		int j;
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//取出该目录项的inode，判断该目录项是目录还是文件
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 1) {
					//找到该目录，判断是否具有进入权限
					if (((tmp.i_mode >> filemode >> 0) & 1) == 0 && strcmp(Cur_User_Name, "root") != 0) {	//root用户所有目录都可以查看 
						//没有执行权限
						printf("权限不足：无执行权限\n");
						oss << "权限不足：无执行权限\n";
						return false;
					}

					//找到该目录项，如果是目录，更换当前目录

					Cur_Dir_Addr = dirlist[j].inodeAddr;
					if (isRelativePath)//两个模式，初始化时用相对路径需要这个相对地去改变路径，但是命令行输入时会在这个函数外面计算出了路径以及当前目录所有就不需要这个了
					{
						if (strcmp(dirlist[j].itemName, ".") == 0) {
							//本目录，不动
						}
						else if (strcmp(dirlist[j].itemName, "..") == 0) {
							//上一次目录
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
					//找到该目录项，如果不是目录，继续找
				}

			}

			i++;
		}

	}

	//没找到
	printf("没有该目录\n");
	oss << "没有该目录\n";
	return false;

}

void gotoxy(HANDLE hOut, int x, int y)	//移动光标到指定位置
{
	COORD pos;
	pos.X = x;             //横坐标
	pos.Y = y;            //纵坐标
	SetConsoleCursorPosition(hOut, pos);
}

void vi(int parinoAddr, char name[], char buf[])	//模拟一个简单vi，输入文本，name为文件名
{
	//先判断文件是否已存在。如果存在，打开这个文件并编辑
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("超过最大文件名长度\n");
		return;
	}

	//清空缓冲区
	memset(buf, 0, sizeof(buf));
	int maxlen = 0;	//到达过的最大长度

	//查找有无同名文件，有的话进入编辑模式，没有进入创建文件模式
	DirItem dirlist[16];	//临时目录清单

	//从这个地址取出inode
	Inode cur, fileInode;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//文件的inode地址
	bool isExist = false;	//文件是否已存在
	while (i < 160) {
		//160个目录项之内，可以直接在直接块里找
		dno = i / 16;	//在第几个直接块里

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//重名，取出inode，判断是否是文件
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&fileInode, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//是文件且重名，打开这个文件，并编辑	
					fileInodeAddr = dirlist[j].inodeAddr;
					isExist = true;
					goto label;
				}
			}
			i++;
		}
	}
label:

	//初始化vi
	int cnt = 0;
	system("cls");	//清屏

	int winx, winy, curx, cury;

	HANDLE handle_out;                              //定义一个句柄  
	CONSOLE_SCREEN_BUFFER_INFO screen_info;         //定义窗口缓冲区信息结构体  
	COORD pos = { 0, 0 };                             //定义一个坐标结构体

	if (isExist) {	//文件已存在，进入编辑模式，先输出之前的文件内容

		//权限判断。判断文件是否可读
		if (((fileInode.i_mode >> filemode >> 2) & 1) == 0) {
			//不可读
			printf("权限不足：没有读权限\n");
			return;
		}

		//将文件内容读取出来，显示在，窗口上
		i = 0;
		int sumlen = fileInode.i_size;	//文件长度
		int getlen = 0;	//取出来的长度
		for (i = 0; i < 10; i++) {
			char fileContent[1000] = { 0 };
			if (fileInode.i_dirBlock[i] == -1) {
				continue;
			}
			//依次取出磁盘块的内容
			//fseek(fr, fileInode.i_dirBlock[i], SEEK_SET);
			//fread(fileContent, superblock->s_BLOCK_SIZE, 1, fr);	//读取出一个磁盘块大小的内容
			//fflush(fr);
			fseek(file, fileInode.i_dirBlock[i], SEEK_SET);
			fread(fileContent, superblock->s_BLOCK_SIZE, 1, file);	//读取出一个磁盘块大小的内容

			//输出字符串
			int curlen = 0;	//当前指针
			while (curlen < superblock->s_BLOCK_SIZE) {
				if (getlen >= sumlen)	//全部输出完毕
					break;
				printf("%c", fileContent[curlen]);	//输出到屏幕 
				buf[cnt++] = fileContent[curlen];	//输出到buf
				curlen++;
				getlen++;
			}
			if (getlen >= sumlen)
				break;
		}
		maxlen = sumlen;
	}

	//获得输出之后的光标位置
	handle_out = GetStdHandle(STD_OUTPUT_HANDLE);   //获得标准输出设备句柄  
	GetConsoleScreenBufferInfo(handle_out, &screen_info);   //获取窗口信息  
	winx = screen_info.srWindow.Right - screen_info.srWindow.Left + 1;
	winy = screen_info.srWindow.Bottom - screen_info.srWindow.Top + 1;
	curx = screen_info.dwCursorPosition.X;
	cury = screen_info.dwCursorPosition.Y;


	//进入vi
	//先用vi读取文件内容

	int mode = 0;	//vi模式，一开始是命令模式
	unsigned char c;
	while (1) {
		if (mode == 0) {	//命令行模式
			c = getch();

			if (c == 'i' || c == 'a') {	//插入模式
				if (c == 'a') {
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						/*
						if(cury>winy-2 || cury%(winy-1)==winy-2){
							//超过这一屏，向下翻页
							if(cury%(winy-1)==winy-2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
							int i;
							for(i=0;i<winx-1;i++)
								printf(" ");
							gotoxy(handle_out,0,cury+1);
							printf(" - 插入模式 - ");
							gotoxy(handle_out,0,cury);
						}
						*/
					}
				}

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//超过这一屏，向下翻页
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - 插入模式 - ");
					gotoxy(handle_out, 0, cury);
				}
				else {
					//显示 "插入模式"
					gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, winy - 1);
					printf(" - 插入模式 - ");
					gotoxy(handle_out, curx, cury);
				}

				gotoxy(handle_out, curx, cury);
				mode = 1;


			}
			else if (c == ':') {
				//system("color 09");//设置文本为蓝色
				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);
				_COORD pos;
				if (cury - winy + 2 > 0)
					pos.X = 0, pos.Y = cury + 1;
				else
					pos.X = 0, pos.Y = winy - 1;
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");

				if (cury - winy + 2 > 0)
					gotoxy(handle_out, 0, cury + 1);
				else
					gotoxy(handle_out, 0, winy - 1);

				WORD att = BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_INTENSITY; // 文本属性
				//FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//控制台部分着色 
				//SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//设置文本颜色
				printf(":");

				char pc;
				int tcnt = 1;	//命令行模式输入的字符计数
				while (c = getch()) {
					if (c == '\r') {	//回车
						break;
					}
					else if (c == '\b') {	//退格，从命令条删除一个字符 
						//SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
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
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					system("cls");
					break;	//vi >>>>>>>>>>>>>> 退出出口
				}
				else {
					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");

					if (cury - winy + 2 > 0)
						gotoxy(handle_out, 0, cury + 1);
					else
						gotoxy(handle_out, 0, winy - 1);
					SetConsoleTextAttribute(handle_out, FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_GREEN);	//设置文本颜色
					FillConsoleOutputAttribute(handle_out, att, winx, pos, NULL);	//控制台部分着色
					printf(" 错误命令");
					//getch();
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					gotoxy(handle_out, curx, cury);
				}
			}
			else if (c == 27) {	//ESC，命令行模式，清状态条
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				gotoxy(handle_out, curx, cury);

			}

		}
		else if (mode == 1) {	//插入模式

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
			printf("[行:%d,列:%d]", curx == -1 ? 0 : curx, cury);
			gotoxy(handle_out, curx, cury);

			c = getch();
			if (c == 27) {	// ESC，进入命令模式
				mode = 0;
				//清状态条
				gotoxy(handle_out, 0, winy - 1);
				SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
				int i;
				for (i = 0; i < winx - 1; i++)
					printf(" ");
				continue;
			}
			else if (c == '\b') {	//退格，删除一个字符
				if (cnt == 0)	//已经退到最开始
					continue;
				printf("\b");
				printf(" ");
				printf("\b");
				curx--;
				cnt--;	//删除字符
				if (buf[cnt] == '\n') {
					//要删除的这个字符是回车，光标回到上一行
					if (cury != 0)
						cury--;
					int k;
					curx = 0;
					for (k = cnt - 1; buf[k] != '\n' && k >= 0; k--)
						curx++;
					gotoxy(handle_out, curx, cury);
					printf(" ");
					gotoxy(handle_out, curx, cury);
					if (cury - winy + 2 >= 0) {	//翻页时
						gotoxy(handle_out, curx, 0);
						gotoxy(handle_out, curx, cury + 1);
						SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
						int i;
						for (i = 0; i < winx - 1; i++)
							printf(" ");
						gotoxy(handle_out, 0, cury + 1);
						printf(" - 插入模式 - ");

					}
					gotoxy(handle_out, curx, cury);

				}
				else
					buf[cnt] = ' ';
				continue;
			}
			else if (c == 224) {	//判断是否是箭头
				c = getch();
				if (c == 75) {	//左箭头
					if (cnt != 0) {
						cnt--;
						curx--;
						if (buf[cnt] == '\n') {
							//上一个字符是回车
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
				else if (c == 77) {	//右箭头
					cnt++;
					if (cnt > maxlen)
						maxlen = cnt;
					curx++;
					if (curx == winx) {
						curx = 0;
						cury++;

						if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
							//超过这一屏，向下翻页
							if (cury % (winy - 1) == winy - 2)
								printf("\n");
							SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
							int i;
							for (i = 0; i < winx - 1; i++)
								printf(" ");
							gotoxy(handle_out, 0, cury + 1);
							printf(" - 插入模式 - ");
							gotoxy(handle_out, 0, cury);
						}

					}
					gotoxy(handle_out, curx, cury);
				}
				continue;
			}
			if (c == '\r') {	//遇到回车
				printf("\n");
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//超过这一屏，向下翻页
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - 插入模式 - ");
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
			//移动光标
			curx++;
			if (curx == winx) {
				curx = 0;
				cury++;

				if (cury > winy - 2 || cury % (winy - 1) == winy - 2) {
					//超过这一屏，向下翻页
					if (cury % (winy - 1) == winy - 2)
						printf("\n");
					SetConsoleTextAttribute(handle_out, screen_info.wAttributes); // 恢复原来的属性
					int i;
					for (i = 0; i < winx - 1; i++)
						printf(" ");
					gotoxy(handle_out, 0, cury + 1);
					printf(" - 插入模式 - ");
					gotoxy(handle_out, 0, cury);
				}

				buf[cnt++] = '\n';
				if (cnt > maxlen)
					maxlen = cnt;
				if (cury == winy) {
					printf("\n");
				}
			}
			//记录字符 
			buf[cnt++] = c;
			if (cnt > maxlen)
				maxlen = cnt;
		}
		else {	//其他模式
		}
	}
	if (isExist) {	//如果是编辑模式
		//将buf内容写回文件的磁盘块

		if (((fileInode.i_mode >> filemode >> 1) & 1) == 1) {	//可写
			writefile(fileInodeAddr, buf);
		}
		else {	//不可写
			printf("权限不足：无写入权限\n");
		}

	}
	else {	//是创建文件模式
		if (((cur.i_mode >> filemode >> 1) & 1) == 1) {
			//可写。可以创建文件
			create(parinoAddr, name, buf);	//创建文件
		}
		else {
			printf("权限不足：无写入权限\n");
			return;
		}
	}
}

void writefile(int fileInodeAddr, char buf[])	//将buf内容写回文件的磁盘块
{	
	Inode fileInode;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&fileInode, sizeof(Inode), 1, file);
	//将buf内容写回磁盘块 
	int k;
	int len = strlen(buf);	//文件长度，单位为字节
	for (k = 0; k < len; k += superblock->s_BLOCK_SIZE) {	//最多10次，10个磁盘快，即最多5K
		//分配这个inode的磁盘块，从控制台读取内容
		int curblockAddr;
		if (fileInode.i_dirBlock[k / superblock->s_BLOCK_SIZE] == -1) {
			//缺少磁盘块，申请一个
			curblockAddr = balloc();
			if (curblockAddr == -1) {
				printf("block分配失败\n");
				oss << "block分配失败\n";
				return;
			}
			fileInode.i_dirBlock[k / superblock->s_BLOCK_SIZE] = curblockAddr;
		}
		else {
			curblockAddr = fileInode.i_dirBlock[k / superblock->s_BLOCK_SIZE];
		}
		//写入到当前目录的磁盘块
		//fseek(fw, curblockAddr, SEEK_SET);
		//fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, fw);
		//fflush(fw);
		fseek(file, curblockAddr, SEEK_SET);
		fwrite(buf + k, superblock->s_BLOCK_SIZE, 1, file);
		fflush(file);
	}
	//更新该文件大小
	fileInode.i_size = len;
	fileInode.i_mtime = time(NULL);
	//fseek(fw, fileInodeAddr, SEEK_SET);
	//fwrite(&fileInode, sizeof(Inode), 1, fw);
	//fflush(fw);
	fseek(file, fileInodeAddr, SEEK_SET);
	fwrite(&fileInode, sizeof(Inode), 1, file);
	fflush(file);
}

void inUsername(char username[])	//输入用户名
{
	printf("username:");
	scanf("%s", username);	//用户名
}

void inPasswd(char passwd[])	//输入密码
{
	int plen = 0;
	char c;
	fflush(stdin);	//清空缓冲区
	printf("passwd:");
	while (c = getch()) {
		if (c == '\r') {	//输入回车，密码确定
			passwd[plen] = '\0';
			fflush(stdin);	//清缓冲区
			printf("\n");
			break;
		}
		else if (c == '\b') {	//退格，删除一个字符
			if (plen != 0) {	//没有删到头
				plen--;
			}
		}
		else {	//密码字符
			passwd[plen++] = c;
		}
	}
}

bool login(const char username[],const char passwd[])	//登陆界面
{
	std::string user_name = username;
	std::string passw_d = passwd;
	//char username[100] = { 0 };
	//char passwd[100] = { 0 };
	//inUsername(username);	//输入用户名
	//inPasswd(passwd);		//输入用户密码
	if (check(user_name.c_str(), passw_d.c_str())) {	//核对用户名和密码
		isLogin = true;
		return true;
	}
	else {
		isLogin = false;
		return false;
	}
}

bool check(const char username[],const char passwd[])	//核对用户名，密码
{
	int passwd_Inode_Addr = -1;	//用户文件inode地址
	int shadow_Inode_Addr = -1;	//用户密码文件inode地址
	Inode passwd_Inode;		//用户文件的inode
	Inode shadow_Inode;		//用户密码文件的inode

	Inode cur_dir_inode;	//当前目录的inode
	int i, j;
	DirItem dirlist[16];	//临时目录
	cd(Root_Dir_Addr, "etc", true);
	//cd(Cur_Dir_Addr, "etc",true);	//进入配置文件目录

	//找到passwd和shadow文件inode地址，并取出
	//取出当前目录的inode
	//fseek(fr, Cur_Dir_Addr, SEEK_SET);
	//fread(&cur_dir_inode, sizeof(Inode), 1, fr);
	fseek(file, Cur_Dir_Addr, SEEK_SET);
	fread(&cur_dir_inode, sizeof(Inode), 1, file);
	//依次取出磁盘块，查找passwd文件的inode地址，和shadow文件的inode地址
	for (i = 0; i < 10; i++) {
		if (cur_dir_inode.i_dirBlock[i] == -1) {
			continue;
		}
		//依次取出磁盘块
		//fseek(fr, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		for (j = 0; j < 16; j++) {	//遍历目录项
			if (strcmp(dirlist[j].itemName, "passwd") == 0 ||	//找到passwd或者shadow条目
				strcmp(dirlist[j].itemName, "shadow") == 0) {
				Inode tmp;	//临时inode
				//取出inode，判断是否是文件
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);

				if (((tmp.i_mode >> 9) & 1) == 0) {
					//是文件
					//判别是passwd文件还是shadow文件
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
		if (passwd_Inode_Addr != -1 && shadow_Inode_Addr != -1)	//都找到了
			break;
	}

	//查找passwd文件，看是否存在用户username
	char buf[1000000];	//最大1M，暂存passwd的文件内容
	char buf2[600];		//暂存磁盘块内容
	j = 0;	//磁盘块指针
	//取出passwd文件内容
	for (i = 0; i < passwd_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了
			//换新的磁盘块
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
		//没找到该用户
		printf("用户不存在\n");
		oss << "用户不存在\n";
		cd(Cur_Dir_Addr, "..",true);	//回到根目录
		return false;
	}

	//如果存在，查看shadow文件，取出密码，核对passwd是否正确
	//取出shadow文件内容
	j = 0;
	for (i = 0; i < shadow_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了这个磁盘块
			//换新的磁盘块
			//fseek(fr, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	char* p;	//字符指针
	if ((p = strstr(buf, username)) == NULL) {
		//没找到该用户
		printf("shadow文件中不存在该用户\n");
		oss << "shadow文件中不存在该用户\n";
		cd(Cur_Dir_Addr, "..",true);	//回到根目录
		return false;
	}
	//找到该用户，取出密码
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

	//核对密码
	if (strcmp(buf2, passwd) == 0) {	//密码正确，登陆
		strcpy(Cur_User_Name, username);
		if (strcmp(username, "root") == 0)
			strcpy(Cur_Group_Name, "root");	//当前登陆用户组名
		else
			strcpy(Cur_Group_Name, "user");	//当前登陆用户组名
		cd(Cur_Dir_Addr, "..",true);
		cd(Cur_Dir_Addr, "home",true); \
			cd(Cur_Dir_Addr, username,true);	//进入到用户目录
		strcpy(Cur_User_Dir_Name, Cur_Dir_Name);	//复制当前登陆用户目录名
		return true;
	}
	else {
		printf("密码错误\n");
		oss << "密码错误\n";
		cd(Cur_Dir_Addr, "..",true);	//回到根目录
		return false;
	}
}

void gotoRoot()	//回到根目录
{
	memset(Cur_User_Name, 0, sizeof(Cur_User_Name));		//清空当前用户名
	memset(Cur_User_Dir_Name, 0, sizeof(Cur_User_Dir_Name));	//清空当前用户目录
	Cur_Dir_Addr = Root_Dir_Addr;	//当前用户目录地址设为根目录地址
	strcpy(Cur_Dir_Name, "/");		//当前目录设为"/"
}

void logout()	//用户注销
{
	//回到根目录
	gotoRoot();

	isLogin = false;
	printf("用户注销\n");
	oss << "用户注销\n";
	system("pause");
	system("cls");
}

bool useradd(char username[],char passwd[])	//用户注册
{
	if (strcmp(Cur_User_Name, "root") != 0) {
		printf("权限不足\n");
		oss << "权限不足\n";
		return false;
	}
	int passwd_Inode_Addr = -1;	//用户文件inode地址
	int shadow_Inode_Addr = -1;	//用户密码文件inode地址
	int group_Inode_Addr = -1;	//用户组文件inode地址
	Inode passwd_Inode;		//用户文件的inode
	Inode shadow_Inode;		//用户密码文件的inode
	Inode group_Inode;		//用户组文件inode
	//原来的目录
	char bak_Cur_User_Name[110];
	char bak_Cur_User_Name_2[110];
	char bak_Cur_User_Dir_Name[310];
	int bak_Cur_Dir_Addr;
	char bak_Cur_Dir_Name[310];
	char bak_Cur_Group_Name[310];

	Inode cur_dir_inode;	//当前目录的inode
	int i, j;
	DirItem dirlist[16];	//临时目录

	//保存现场，回到根目录
	strcpy(bak_Cur_User_Name, Cur_User_Name);
	strcpy(bak_Cur_User_Dir_Name, Cur_User_Dir_Name);
	bak_Cur_Dir_Addr = Cur_Dir_Addr;
	strcpy(bak_Cur_Dir_Name, Cur_Dir_Name);

	//创建用户目录
	gotoRoot();
	cd(Cur_Dir_Addr, "home",true);
	//保存现场
	strcpy(bak_Cur_User_Name_2, Cur_User_Name);
	strcpy(bak_Cur_Group_Name, Cur_Group_Name);
	//更改
	strcpy(Cur_User_Name, username);
	strcpy(Cur_Group_Name, "user");
	if (!mkdir(Cur_Dir_Addr, username)) {
		strcpy(Cur_User_Name, bak_Cur_User_Name_2);
		strcpy(Cur_Group_Name, bak_Cur_Group_Name);
		//恢复现场，回到原来的目录
		strcpy(Cur_User_Name, bak_Cur_User_Name);
		strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
		Cur_Dir_Addr = bak_Cur_Dir_Addr;
		strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

		printf("用户注册失败!\n");
		oss << "用户注册失败!\n";
		return false;
	}
	//恢复现场
	strcpy(Cur_User_Name, bak_Cur_User_Name_2);
	strcpy(Cur_Group_Name, bak_Cur_Group_Name);

	//回到根目录
	gotoRoot();

	//进入用户目录
	cd(Cur_Dir_Addr, "etc",true);

	//输入用户密码
	//char passwd[100] = { 0 };
	//inPasswd(passwd);	//输入密码

	//找到passwd和shadow文件inode地址，并取出，准备添加条目

	//取出当前目录的inode
	//fseek(fr, Cur_Dir_Addr, SEEK_SET);
	//fread(&cur_dir_inode, sizeof(Inode), 1, fr);
	fseek(file, Cur_Dir_Addr, SEEK_SET);
	fread(&cur_dir_inode, sizeof(Inode), 1, file);
	//依次取出磁盘块，查找passwd文件的inode地址，和shadow文件的inode地址
	for (i = 0; i < 10; i++) {
		if (cur_dir_inode.i_dirBlock[i] == -1) {
			continue;
		}
		//依次取出磁盘块
		//fseek(fr, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		for (j = 0; j < 16; j++) {	//遍历目录项
			if (strcmp(dirlist[j].itemName, "passwd") == 0 ||	//找到passwd或者shadow条目
				strcmp(dirlist[j].itemName, "shadow") == 0 ||
				strcmp(dirlist[j].itemName, "group") == 0) {
				Inode tmp;	//临时inode
				//取出inode，判断是否是文件
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);

				if (((tmp.i_mode >> 9) & 1) == 0) {
					//是文件
					//判别是passwd文件还是shadow文件
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
		if (passwd_Inode_Addr != -1 && shadow_Inode_Addr != -1)	//都找到了
			break;
	}

	//查找passwd文件，看是否存在用户username
	char buf[100000];	//最大100K，暂存passwd的文件内容
	char buf2[600];		//暂存磁盘块内容
	j = 0;	//磁盘块指针
	//取出passwd文件内容
	for (i = 0; i < passwd_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了
			//换新的磁盘块
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
		//没找到该用户
		printf("用户已存在\n");
		oss << "用户已存在\n";

		//恢复现场，回到原来的目录
		strcpy(Cur_User_Name, bak_Cur_User_Name);
		strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
		Cur_Dir_Addr = bak_Cur_Dir_Addr;
		strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);
		return false;
	}

	//如果不存在，在passwd中创建新用户条目,修改group文件
	sprintf(buf + strlen(buf), "%s:x:%d:%d\n", username, nextUID++, 1);	//增加条目，用户名：加密密码：用户ID：用户组ID。用户组为普通用户组，值为1 
	passwd_Inode.i_size = strlen(buf);
	writefile(passwd_Inode_Addr, buf);	//将修改后的passwd写回文件中

	//取出shadow文件内容
	j = 0;
	for (i = 0; i < shadow_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了这个磁盘块
			//换新的磁盘块
			//fseek(fr, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//增加shadow条目
	sprintf(buf + strlen(buf), "%s:%s\n", username, passwd);	//增加条目，用户名：密码
	shadow_Inode.i_size = strlen(buf);
	writefile( shadow_Inode_Addr, buf);	//将修改后的内容写回文件中


	//取出group文件内容
	j = 0;
	for (i = 0; i < group_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了这个磁盘块
			//换新的磁盘块
			//fseek(fr, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//增加group中普通用户列表
	if (buf[strlen(buf) - 2] == ':')
		sprintf(buf + strlen(buf) - 1, "%s\n", username);	//增加组内用户
	else
		sprintf(buf + strlen(buf) - 1, ",%s\n", username);	//增加组内用户
	group_Inode.i_size = strlen(buf);
	writefile(group_Inode_Addr, buf);	//将修改后的内容写回文件中

	//恢复现场，回到原来的目录
	strcpy(Cur_User_Name, bak_Cur_User_Name);
	strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
	Cur_Dir_Addr = bak_Cur_Dir_Addr;
	strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

	printf("用户注册成功\n");
	oss << "用户注册成功\n";
	return true;
}


bool userdel(char username[])	//用户删除
{
	if (strcmp(Cur_User_Name, "root") != 0) {
		printf("权限不足:您需要root权限\n");
		oss << "权限不足:您需要root权限\n";
		return false;
	}
	if (strcmp(username, "root") == 0) {
		printf("无法删除root用户\n");
		oss << "无法删除root用户\n";
		return false;
	}
	int passwd_Inode_Addr = -1;	//用户文件inode地址
	int shadow_Inode_Addr = -1;	//用户密码文件inode地址
	int group_Inode_Addr = -1;	//用户组文件inode地址
	Inode passwd_Inode;		//用户文件的inode
	Inode shadow_Inode;		//用户密码文件的inode
	Inode group_Inode;		//用户组文件inode
	//原来的目录
	char bak_Cur_User_Name[110];
	char bak_Cur_User_Dir_Name[310];
	int bak_Cur_Dir_Addr;
	char bak_Cur_Dir_Name[310];

	Inode cur_dir_inode;	//当前目录的inode
	int i, j;
	DirItem dirlist[16];	//临时目录

	//保存现场，回到根目录
	strcpy(bak_Cur_User_Name, Cur_User_Name);
	strcpy(bak_Cur_User_Dir_Name, Cur_User_Dir_Name);
	bak_Cur_Dir_Addr = Cur_Dir_Addr;
	strcpy(bak_Cur_Dir_Name, Cur_Dir_Name);

	//回到根目录
	gotoRoot();

	//进入用户目录
	cd(Cur_Dir_Addr, "etc",true);

	//输入用户密码
	//char passwd[100] = {0};
	//inPasswd(passwd);	//输入密码

	//找到passwd和shadow文件inode地址，并取出，准备添加条目

	//取出当前目录的inode
	//fseek(fr, Cur_Dir_Addr, SEEK_SET);
	//fread(&cur_dir_inode, sizeof(Inode), 1, fr);
	fseek(file, Cur_Dir_Addr, SEEK_SET);
	fread(&cur_dir_inode, sizeof(Inode), 1, file);

	//依次取出磁盘块，查找passwd文件的inode地址，和shadow文件的inode地址
	for (i = 0; i < 10; i++) {
		if (cur_dir_inode.i_dirBlock[i] == -1) {
			continue;
		}
		//依次取出磁盘块
		//fseek(fr, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		fseek(file, cur_dir_inode.i_dirBlock[i], SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);
		for (j = 0; j < 16; j++) {	//遍历目录项
			if (strcmp(dirlist[j].itemName, "passwd") == 0 ||	//找到passwd或者shadow条目
				strcmp(dirlist[j].itemName, "shadow") == 0 ||
				strcmp(dirlist[j].itemName, "group") == 0) {
				Inode tmp;	//临时inode
				//取出inode，判断是否是文件
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 0) {
					//是文件
					//判别是passwd文件还是shadow文件
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
		if (passwd_Inode_Addr != -1 && shadow_Inode_Addr != -1)	//都找到了
			break;
	}

	//查找passwd文件，看是否存在用户username
	char buf[100000];	//最大100K，暂存passwd的文件内容
	char buf2[600];		//暂存磁盘块内容
	j = 0;	//磁盘块指针
	//取出passwd文件内容
	for (i = 0; i < passwd_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了
			//换新的磁盘块
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
		//没找到该用户
		printf("用户不存在\n");
		oss << "用户不存在\n";

		//恢复现场，回到原来的目录
		strcpy(Cur_User_Name, bak_Cur_User_Name);
		strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
		Cur_Dir_Addr = bak_Cur_Dir_Addr;
		strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);
		return false;
	}

	//如果存在，在passwd、shadow、group中删除该用户的条目
	//删除passwd条目
	char* p = strstr(buf, username);
	*p = '\0';
	while ((*p) != '\n')	//空出中间的部分
		p++;
	p++;
	strcat(buf, p);
	passwd_Inode.i_size = strlen(buf);	//更新文件大小
	writefile(passwd_Inode_Addr, buf);	//将修改后的passwd写回文件中

	//取出shadow文件内容
	j = 0;
	for (i = 0; i < shadow_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了这个磁盘块
			//换新的磁盘块
			//fseek(fr, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, shadow_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//删除shadow条目
	p = strstr(buf, username);
	*p = '\0';
	while ((*p) != '\n')	//空出中间的部分
		p++;
	p++;
	strcat(buf, p);
	shadow_Inode.i_size = strlen(buf);	//更新文件大小
	writefile(shadow_Inode_Addr, buf);	//将修改后的内容写回文件中


	//取出group文件内容
	j = 0;
	for (i = 0; i < group_Inode.i_size; i++) {
		if (i % superblock->s_BLOCK_SIZE == 0) {	//超出了这个磁盘块
			//换新的磁盘块
			//fseek(fr, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			//fread(&buf2, superblock->s_BLOCK_SIZE, 1, fr);
			fseek(file, group_Inode.i_dirBlock[i / superblock->s_BLOCK_SIZE], SEEK_SET);
			fread(&buf2, superblock->s_BLOCK_SIZE, 1, file);
			j = 0;
		}
		buf[i] = buf2[j++];
	}
	buf[i] = '\0';

	//增加group中普通用户列表
	p = strstr(buf, username);
	*p = '\0';
	while ((*p) != '\n' && (*p) != ',')	//空出中间的部分
		p++;
	if ((*p) == ',')
		p++;
	strcat(buf, p);
	group_Inode.i_size = strlen(buf);	//更新文件大小
	writefile( group_Inode_Addr, buf);	//将修改后的内容写回文件中

	//恢复现场，回到原来的目录
	strcpy(Cur_User_Name, bak_Cur_User_Name);
	strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
	Cur_Dir_Addr = bak_Cur_Dir_Addr;
	strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

	//删除用户目录	
	Cur_Dir_Addr = Root_Dir_Addr;	//当前用户目录地址设为根目录地址
	strcpy(Cur_Dir_Name, "/");		//当前目录设为"/"
	cd(Cur_Dir_Addr, "home",true);
	bool is=rmdir(Cur_Dir_Addr, username);

	//恢复现场，回到原来的目录
	strcpy(Cur_User_Name, bak_Cur_User_Name);
	strcpy(Cur_User_Dir_Name, bak_Cur_User_Dir_Name);
	Cur_Dir_Addr = bak_Cur_Dir_Addr;
	strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);
	if (true)
	{
		printf("用户已删除\n");
		oss << "用户已删除\n";
	}
	else
	{
		printf("用户未删除\n");
		oss << "用户未删除\n";
	}
	return true;

}

bool chmod(int parinoAddr, const char name[], int pmode)	//修改文件或目录权限
{
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("超过最大目录名长度\n");
		oss << "超过最大目录名长度\n";
		return false;
	}
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		printf("错误操作\n");
		oss << "错误操作\n";
		return false;
	}
	//取出该文件或目录inode
	Inode cur, fileInode;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//依次取出磁盘块
	int i = 0, j;
	DirItem dirlist[16] = { 0 };
	while (i < 160) {
		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		//fseek(fr, parblockAddr, SEEK_SET);
		//fread(&dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {	//找到该目录或者文件
				//取出对应的inode
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
		printf("文件不存在\n");
		oss << "文件不存在\n";
		return false;
	}

	//判断是否是本用户
	if (strcmp(Cur_User_Name, fileInode.i_uname) != 0 && strcmp(Cur_User_Name, "root") != 0) {
		printf("权限不足\n");
		oss << "权限不足\n";
		return false;
	}

	//对inode的mode属性进行修改
	fileInode.i_mode = (fileInode.i_mode >> 9 << 9) | pmode;	//修改权限

	//将inode写回磁盘
	//fseek(fw, dirlist[j].inodeAddr, SEEK_SET);
	//fwrite(&fileInode, sizeof(Inode), 1, fw);
	//fflush(fw);
	fseek(file, dirlist[j].inodeAddr, SEEK_SET);
	fwrite(&fileInode, sizeof(Inode), 1, file);
	fflush(file);
}

bool touch(int parinoAddr, char name[], char buf[])	//touch命令创建文件，读入字符
{
	//先判断文件是否已存在。如果存在，打开这个文件并编辑
	if (strlen(name) >= MAX_NAME_SIZE) {
		printf("超过最大文件名长度\n");
		oss << "超过最大文件名长度\n";
		return false;
	}
	//查找有无同名文件，有的话提示错误，退出程序。没有的话，创建一个空文件
	DirItem dirlist[16];	//临时目录清单

	//从这个地址取出inode
	Inode cur, fileInode;
	//fseek(fr, parinoAddr, SEEK_SET);
	//fread(&cur, sizeof(Inode), 1, fr);
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//文件的inode地址
	while (i < 160) {
		//160个目录项之内，可以直接在直接块里找
		dno = i / 16;	//在第几个直接块里

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//fseek(fr, cur.i_dirBlock[dno], SEEK_SET);
		//fread(dirlist, sizeof(dirlist), 1, fr);
		//fflush(fr);
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//重名，取出inode，判断是否是文件
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&fileInode, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//是文件且重名，提示错误，退出程序
					printf("文件已存在\n");
					oss << "文件已存在\n";
					return false;
				}
			}
			i++;
		}
	}

	//文件不存在，创建一个空文件
	if (((cur.i_mode >> filemode >> 1) & 1) == 1) {
		//可写。可以创建文件
		buf[0] = '\0';
		bool is=create(parinoAddr, name, buf);	//创建文件
		return is;
	}
	else {
		printf("权限不足：无写入权限\n");
		oss << "权限不足：无写入权限\n";
		return false;
	}
	return true;
}

void help()	//显示所有命令清单 
{
	printf("help - 显示命令清单\n");
	printf("\n");
	printf("----目录相关----\n");
	printf("ls - 显示当前目录清单\n");
	printf("cd - 进入目录\n");
	printf("md - 创建目录\n");
	printf("rd - 删除目录\n");
	printf("dir - 显示目录:显示指定目录下的信息 带/s参数的dir命令，显示所有子目录\n");
	printf("\n");
	printf("----文件相关----\n");
	printf("newfile - 创建一个空文件\n");
	printf("cat - 打开文件显示文件内容\n");
	printf("copy - 拷贝文件\n");
	printf("del - 删除文件\n");
	printf("chmod - 修改文件或目录权限,chmod 文件名 权限\n");
	printf("vi - vi编辑器\n");
	printf("\n");
	printf("----用户相关----\n");
	printf("logout - 用户注销\n");
	printf("useradd - 添加用户\n");
	printf("userdel - 删除用户\n");
	printf("\n");
	printf("----系统相关----\n");
	printf("info -显示整个系统信息\n");
	printf("super - 查看超级块\n");
	printf("inode - 查看inode位图\n");
	printf("block - 查看block位图\n");
	printf("cls - 清屏\n");
	printf("exit - 退出系统\n");

	return;
}

void cmd(const char str[])	//处理输入的命令
{	//每次处理前先清空上一次的结果
	response.clear();
	// 清空 oss
	oss.str("");
	oss.clear();
	//初始化选项
	response_op = Option::NONE;
	char p1[100] = { 0 };
	char p2[100] = { 0 };
	char p3[100] = { 0 };
	char p4[100] = { 0 };
	char buf[1000] = { 0 };	//最大100K
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
		//ls(Cur_Dir_Addr);	//显示当前目录
	}
	else if (strcmp(p1, "copy")==0)
	{
		sscanf(str, "%s%s%s", p1, p2, p3);
		bool p2_is_host=false;//p2是否是带<host>
		bool p3_is_host=false;//p3是否带<host>
		//检查p2,p3含不含<host>,并设置标志位
		//检测p2,p3含不含host
			// 检查 p2 是否包含 <host>
		if (strstr(p2, "<host>") != nullptr) {
			p2_is_host = true;
			memmove(p2, p2 + 6, strlen(p2) - 5);//-5是为了保留'\0',dest,src,long
		}

		// 检查 p3 是否包含 <host>
		if (strstr(p3, "<host>") != nullptr) {
			p3_is_host = true;
			memmove(p3, p3 + 6, strlen(p3) - 5);
		}
		// 检查是复制本地文件还是在文件系统内部复制
		if (p2_is_host || p3_is_host) {
			// 如果 p2 或 p3 包含 <host>，说明涉及到本地文件系统与模拟文件系统之间的文件复制
			

			// 在这里可以加入具体的文件复制逻辑，比如调用 Windows API 来拷贝文件
			if (p2_is_host) {
				// 处理源路径为主机路径的情况
				readFileFromHost(p2);
				//找到要写入文件的inode节点地址
				PathResult pr = ParseFromRoot(p3);
				if (pr.is_a_path)
				{
					int code=FindFileInCurdir(pr.addr, pr.nameLast);
					if (code == -1)
					{
						printf("路径下没有该文件\n");
						return;
					}
					writefile(code, readFromHost);
					//清空缓冲区
					memset(readFromHost, 0, sizeof(readFromHost));
				}

			}
			else {
				// 处理目标路径为主机路径的情况
				//找到要写入文件的inode节点地址
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
			// 否则是文件系统内部的复制，模拟文件系统内部路径之间的复制
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
					printf("路径下没有该文件\n");
					return;
				}
				writefile(code, readFromHost);
				//清空缓冲区
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
		printf("检测并恢复文件系统成功\n");
		oss << "检测并恢复文件系统成功\n";
	}
	else if (strcmp(p1, "cd") == 0) {
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		if (pr.is_a_path)
		{
			char rootName[] = "/";
			char* tmp1 = new char[strlen(rootName) + 1];  // +1 是为了存储终止符 '\0'
			strcpy(tmp1, rootName);//tmp1=='/'
			char*tmp= parsePath(p2, Cur_Dir_Name, tmp1);
			char bak_Cur_Dir_Name[310];
			strcpy(bak_Cur_Dir_Name, Cur_Dir_Name);
			strcpy(Cur_Dir_Name, tmp);//复制过去
			bool is=cd(pr.addr, pr.nameLast,false);
			code = ErrorCode::SUCCESS;
			if (!is)//返回false则当前目录地址要复原
			{
				strcpy(Cur_Dir_Name, bak_Cur_Dir_Name);

				code = ErrorCode::FAILURE;
			}

		}
		else
		{
			code = ErrorCode::FAILURE;
		}
		//这里要把绝对路径分割一下分割成只有没有最后一个组件的路径传给函数
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
			//先查询这个文件夹是否为空
			bool is_empty = false;
			is_empty = find_file_is_empty(pr.addr, pr.nameLast);
			if (is_empty|| Option::RESPONSE==receive_op)//如果为空或者是接到直接删除的命令就直接删
			{
				is = rmdir(pr.addr, pr.nameLast);
			}
			else
			{
				//不为空就发出问询
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
	else if (strcmp(p1, "vi") == 0) {	//创建一个文件
		sscanf(str, "%s%s", p1, p2);
		PathResult pr = ParseFromRoot(p2);
		if (pr.is_a_path)
		{
			vi(pr.addr, pr.nameLast, buf);
		}
		//vi(Cur_Dir_Addr, p2, buf);	//读取内容到buf
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
		//touch(Cur_Dir_Addr, p2, buf);	//读取内容到buf
	}
	else if (strcmp(p1, "del") == 0) {	//删除一个文件
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
			printf("参数错误\n");
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
			printf("参数错误\n");
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
			printf("权限不足：您需要root权限\n");
			return;
		}
		Ready();
		logout();
	}
	else if (strcmp(p1, "exit") == 0) {
		printf("退出MingOS\n");
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
			printf("参数错误\n");
			oss << "参数错误\n";
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
				printf("参数错误\n");
				oss << "参数错误\n";
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
			printf("抱歉，没有该命令\n");
			oss << "抱歉，没有该命令\n";
		}
	}
	response = oss.str();
	return;
}

//信号量(P和V操作)的实现
void reader_p(int fileInodeAddr)
{
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	while (cur.write_cnt.load() > 0)
	{
		//如果有写线程正在写，读线程需要等待
		Sleep(1000);
	}
	cur.read_cnt.fetch_add(1);//增加读计数器
}

void reader_v(int fileInodeAddr)
{	//先取出数据
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//完成读操作
	cur.read_cnt.fetch_sub(1);//减少读计数器
}

void writer_p(int fileInodeAddr)
{
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	// 等待操作：如果有读线程或写线程正在写，写线程需要等待
	while (cur.read_cnt.load() > 0 || cur.write_cnt.load() > 0) {
		Sleep(1000);  // 如果有读线程或写线程，写线程等待
	}
	cur.write_cnt.fetch_add(1);  // 增加写计数器
}

void writer_v(int fileInodeAddr)
{
	Inode cur;
	fseek(file, fileInodeAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//减少写计数器
	cur.write_cnt.fetch_sub(1);
}

//输出文件内容
bool cat_file(int parinoAddr, const char name[])
{
	//先把父目录的Inode节点给取出来
	Inode cur;
	Inode fileInode;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	DirItem dirlist[16];	//临时目录清单
	
	//依次取出inode对应的磁盘块，查找有没有名字为name的目录项



	//判断权限
	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//文件的inode地址
	bool isExist = false;	//文件是否已存在
	while (i < 160) {
		//160个目录项之内，可以直接在直接块里找
		dno = i / 16;	//在第几个直接块里

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//取出该磁盘块的目录清单
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//读到filenode中存储
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//是文件且重名，打开这个文件，并编辑	
					fileInodeAddr = dirlist[j].inodeAddr;
					isExist = true;
					
				}
				
			}
			i++;
		}
	}

	//判断是否存在这个文件
	if (!isExist)
	{
		printf("文件不存在\n");
		oss << "文件不存在\n";
		return false;
	}
	//判断有没有读取权限
	if (((fileInode.i_mode >> filemode >> 2)& 1) == 0)
	{
		printf("没有读取权限\n");
		oss << "没有读取权限\n";
		return false;
	}


	//将文件内容读取出来，显示在，窗口上
	i = 0;
	int sumlen = fileInode.i_size;	//文件长度
	int getlen = 0;	//取出来的长度
	char buf[10000] = { 0 };
	for (i = 0; i < 10; i++) {
		char fileContent[1000] = { 0 };
		if (fileInode.i_dirBlock[i] == -1) {
			continue;
		}
		//取出当前文件的直接块
		fseek(file, fileInode.i_dirBlock[i], SEEK_SET);
		fread(fileContent, superblock->s_BLOCK_SIZE, 1, file);	//读取出一个磁盘块大小的内容

		//输出字符串
		int curlen = 0;	//当前指针
		while (curlen < superblock->s_BLOCK_SIZE) {
			if (getlen >= sumlen)	//全部输出完毕
				break;
			printf("%c", fileContent[curlen]);	//输出到屏幕 
			oss << fileContent[curlen];
			buf[getlen++] = fileContent[curlen];	//输出到buf
			curlen++;
			
		}
		if (getlen >= sumlen)
			break;
	}
	//printf("\n");
	return true;

}


//解析绝对路径
PathResult ParseFromRoot(const char path[])
{
	//先切割路径字符串
	//  以这三个开头  '/'  '.' '..'
	std::vector<char*> pathComponents;
	if (strcmp(path, "/") == 0)
	{
		char temp[] = ".";
		pathComponents.push_back(temp);  // 特殊处理，添加根目录
	}
	else{
	  char* pathCopy = strdup(path);  // 拷贝路径，避免修改原始路径
	  char* token = strtok(pathCopy, "/");
	  //切割路径并将每个部分都存入路径组件数组
	  while (token != nullptr)
	  {
		pathComponents.push_back(token);
		token = strtok(nullptr, "/");//传入Nullptr表示继续从上一次停止的位置继续分割
	  } 
    }

	bool is_a_path = true;
	int targetAddr = -1;//这个变量用来存储路径中最后一个Componenet的所在目录的inode地址
	int size = pathComponents.size();
	char* name = pathComponents[size-1]; //用strcpy(.... ,name)将char*转化为char[]
	int vir_addr = path[0] == '/'?  Root_Dir_Addr :Cur_Dir_Addr;//这只是一个局部变量去验证是否路径正确，并不会改变当前目录地址或者名字，在每个for中会被更新
	//如果路径第一个起始是/说明从根目录去开始找都在是相对路径
	
	//开始遍历路径组件
	for (int i = 0; i < size - 1;i++) {
		const char* component = pathComponents[i];
		if (strcmp(component, ".") == 0)
		{
			continue;
		}
		else
		{
			int returnAdrr = FindPathInCurdir(vir_addr, component);
			if (returnAdrr == -1)//表明没有找到这个目录，说明路径错误,报错
			{
				is_a_path = false;//表面路径错误
				printf("路径错误!\n");
				oss << "路径错误!\n";
				break;
			}
			//如果不是-1那就赋值,然后继续迭代
			vir_addr = returnAdrr;
		}
	}
	//主要是Cur_Dir_addr,cur_dir_name,Cur_user_name不用动，Cur_User_Dir_Name也不用管了反正没用到
	//Cur_Dir_addr,cur_dir_name这两个东西维护好就行
	//cd 是要改cur_dir_name的，

	//返回结果
	PathResult pr;
	pr.addr = vir_addr;
	pr.is_a_path = is_a_path;
	strcpy(pr.nameLast ,name);

	return pr;
}

int FindPathInCurdir(int parinoAddr,const char*name) 
{
	char findName[28];
	strcpy(findName, name);//先拷贝

	//取出当前目录的inode
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//依次取出inode对应的磁盘块，查找有没有名字为name的目录项
	int i = 0;

	//取出目录项数
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
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
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		int j;
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//取出该目录项的inode，判断该目录项是目录还是文件
				//fseek(fr, dirlist[j].inodeAddr, SEEK_SET);
				//fread(&tmp, sizeof(Inode), 1, fr);
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 1) {
					//找到该目录，判断是否具有进入权限
					if (((tmp.i_mode >> filemode >> 0) & 1) == 0 && strcmp(Cur_User_Name, "root") != 0) {	//root用户所有目录都可以查看 
						//没有执行权限
						printf("权限不足：无执行权限\n");
						oss << "权限不足：无执行权限\n";
						return -1;
					}

					//找到该目录项，如果是目录，
					//return一个inode地址
					return dirlist[j].inodeAddr;
				}
				else {
					//找到该目录项，如果不是目录，继续找
				}

			}

			i++;
		}

	}

	//如果没找到这些目录
	return -1;
}

int FindFileInCurdir(int parinoAddr, const char* name) //在parinoAddr中找到名为name的文件，然后返回它的inode地址
{
	char findName[28];
	strcpy(findName, name);//先拷贝

	//取出当前目录的inode
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	//依次取出inode对应的磁盘块，查找有没有名字为name的目录项
	int i = 0;

	//取出目录项数
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
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
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		int j;
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				Inode tmp;
				//取出该目录项的inode，判断该目录项是目录还是文件
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&tmp, sizeof(Inode), 1, file);
				if (((tmp.i_mode >> 9) & 1) == 0) {//说明是文件
					//找到该目录，判断是否具有进入权限
					if (((tmp.i_mode >> filemode >> 1) & 1) == 0 && strcmp(Cur_User_Name, "root") != 0) {	//root用户所有目录都可以写，这执行的是copy
						//没有执行权限
						printf("权限不足：无写入权限,无法copy\n");
						oss << "权限不足：无写入权限,无法copy\n";
						return -1;
					}

					//找到该目录项，如果是文件，
					//return一个inode地址
					return dirlist[j].inodeAddr;
				}
				else {
					//找到该目录项，如果不是目录，继续找
				}

			}

			i++;
		}

	}

	//如果没找到这个文件
	return -1;
}
char* parsePath(const char* path, char* Cur_Dir_Name, char* Root_Dir_Name) {
	char* pathCopy = strdup(path);  // 拷贝路径，避免修改原始路径
	char* token = strtok(pathCopy, "/");  // 使用 '/' 作为分隔符切割路径
	char* baseDir = nullptr;

	// 判断路径是绝对路径还是相对路径
	if (path[0] == '/') {
		// 绝对路径，从根目录开始
		baseDir = new char[strlen(Root_Dir_Name) + 1];  // 为 baseDir 分配足够的内存
		strcpy(baseDir, Root_Dir_Name);  // 复制内容
	}
	else {
		// 相对路径，以当前目录为基础
		baseDir = new char[strlen(Cur_Dir_Name) + 1];  // 为 baseDir 分配足够的内存
		strcpy(baseDir, Cur_Dir_Name);  // 复制内容
	}

	// 处理路径组件
	while (token != nullptr) {
		if (strcmp(token, ".") == 0) {
			// 当前目录，什么都不做
		}
		else if (strcmp(token, "..") == 0) {
			// 上级目录，删除最后一个组件
			int len = strlen(baseDir);
			// 如果当前路径不是根目录，就移除最后一个组件
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
				// 如果当前目录就是根目录，则不能上移
				//std::cout << "Already at root directory." << std::endl;
			}
		}
		else {
			// 其他路径组件，添加到当前路径中
			if (baseDir[strlen(baseDir) - 1] != '/') {
				strcat(baseDir, "/");  // 添加 '/' 分隔符
			}
			strcat(baseDir, token);  // 将当前组件添加到路径中
		}

		token = strtok(nullptr, "/");  // 继续切割下一个组件
	}

	return baseDir;
}


void readFileFromHost(const char* hostFilePath) {
	// 打开源文件 (主机上的文件)
	//打开一个现有的文件或设备，并返回一个句柄,对文件进行读写
	HANDLE hFile = CreateFileA(hostFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		//std::cerr << "打开本地文件失败" << std::endl;
		printf("打开本地文件失败\n");
		oss << "打开本地文件失败\n";
		return;
	}

	// 获取文件大小
	DWORD fileSize = GetFileSize(hFile, NULL);

	// 读取文件内容
	//char* buffer = new char[fileSize];
	DWORD bytesRead;//用于实际存储读取的字节数
	if (!ReadFile(hFile, readFromHost, fileSize, &bytesRead, NULL)) {
		//std::cerr << "读取本地文件失败" << std::endl;
		printf("读取本地文件失败\n");
		oss << "读取本地文件失败\n";
		CloseHandle(hFile);
		//delete[] buffer;
		memset(readFromHost, 0, sizeof(readFromHost));
		return;
	}
	CloseHandle(hFile);
	if (bytesRead > BLOCK_SIZE * 10)//我这里只用了10个直接块
	{
		printf("所拷贝的文件过于大，无法写入simulink中\n");
		oss << "所拷贝的文件过于大，无法写入simulink中\n";
		//delete[] buffer;
		memset(readFromHost, 0, sizeof(readFromHost));
		return;
	}
	// 将文件内容写入模拟的 Linux 文件系统 

}

void writeFileToHost(const char* hostFilePath,int bufferSize)
{
	// 打开目标文件，如果文件不存在则创建，若文件已存在则覆盖
	HANDLE hFile = CreateFileA(
		hostFilePath,          // 文件路径
		GENERIC_WRITE,         // 写权限
		0,                     // 不共享
		NULL,                  // 默认安全属性
		CREATE_ALWAYS,         // 如果文件已存在则覆盖
		FILE_ATTRIBUTE_NORMAL, // 普通文件属性
		NULL                   // 不需要模板文件
	);

	// 检查文件是否成功打开
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("打开文件失败\n" );
		oss << "打开文件失败\n";
		return;
	}

	// 写入文件
	DWORD bytesWritten;
	BOOL writeResult = WriteFile(
		hFile,         // 文件句柄
		readFileFromHost,        // 要写入的缓冲区
		bufferSize,    // 写入的字节数
		&bytesWritten, // 实际写入的字节数
		NULL           // 不使用重叠I/O
	);

	// 检查写入是否成功
	if (!writeResult) {
		printf("写入文件失败\n");
		oss << "写入文件失败\n";
	}
	else {
		std::cout << "成功写入 " << bytesWritten << " 字节到文件。" << std::endl;
		oss << "成功写入 " << bytesWritten << "字节到文件\n";
	}

	// 关闭文件句柄
	CloseHandle(hFile);
}

void readFromFile(int parinoAddr, const char name[])
{
	//先把父目录的Inode节点给取出来
	Inode cur;
	Inode fileInode;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	DirItem dirlist[16];	//临时目录清单

	//依次取出inode对应的磁盘块，查找有没有名字为name的目录项



	//判断权限
	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	int i = 0, j;
	int dno;
	int fileInodeAddr = -1;	//文件的inode地址
	bool isExist = false;	//文件是否已存在
	while (i < 160) {
		//160个目录项之内，可以直接在直接块里找
		dno = i / 16;	//在第几个直接块里

		if (cur.i_dirBlock[dno] == -1) {
			i += 16;
			continue;
		}
		//取出该磁盘块的目录清单
		fseek(file, cur.i_dirBlock[dno], SEEK_SET);
		fread(dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		for (j = 0; j < 16; j++) {
			if (strcmp(dirlist[j].itemName, name) == 0) {
				//读到filenode中存储
				fseek(file, dirlist[j].inodeAddr, SEEK_SET);
				fread(&fileInode, sizeof(Inode), 1, file);
				if (((fileInode.i_mode >> 9) & 1) == 0) {	//是文件且重名，打开这个文件，并编辑	
					fileInodeAddr = dirlist[j].inodeAddr;
					isExist = true;

				}

			}
			i++;
		}
	}

	//判断是否存在这个文件
	if (!isExist)
	{
		printf("文件不存在\n");
		oss << "文件不存在\n";
		return;
	}
	//判断有没有读取权限
	if (((fileInode.i_mode >> filemode >> 2) & 1) == 0)
	{
		printf("没有读取权限\n");
		oss << "没有读取权限\n";
		return;
	}


	//将文件内容读取出来，显示在，窗口上
	i = 0;
	int sumlen = fileInode.i_size;	//文件长度
	int getlen = 0;	//取出来的长度
	//char buf[10000] = { 0 };
	for (i = 0; i < 10; i++) {
		char fileContent[1000] = { 0 };
		if (fileInode.i_dirBlock[i] == -1) {
			continue;
		}
		//取出当前文件的直接块
		fseek(file, fileInode.i_dirBlock[i], SEEK_SET);
		fread(fileContent, superblock->s_BLOCK_SIZE, 1, file);	//读取出一个磁盘块大小的内容

		//输出字符串
		int curlen = 0;	//当前指针
		while (curlen < superblock->s_BLOCK_SIZE) {
			if (getlen >= sumlen)	//全部输出完毕
				break;
			//printf("%c", fileContent[curlen]);	//输出到屏幕 
			readFromHost[getlen++] = fileContent[curlen];	//输出到buf
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
		std::cerr << "创建共享内存失败" << std::endl;
		oss << "创建共享内存失败\n";
		return;
	}
	sharedMemory = (SharedMemory*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory));
	if (sharedMemory == NULL) {
		std::cerr << "共享内存映射失败" << std::endl;
		oss << "共享内存映射失败\n";
		CloseHandle(hMapFile);
		return;
	}

	//初始化一些变量
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

	//开始创建信号量，hSemaphore1是互斥的，hSemaphore2是同步的
	LPCTSTR szName1 = TEXT("Semaphore1");
	LPCTSTR szName2 = TEXT("Semaphore2");

	// 创建信号量 1 (初始值为1)
	hSemaphore1 = CreateSemaphore(NULL, 1, 1, szName1);
	if (hSemaphore1 == NULL) {
		std::cerr << "无法创建信号量 1, 错误代码: " << GetLastError() << std::endl;
		oss << "无法创建信号量1\n";
		return;
	}

	// 创建信号量 2 (初始值为0)
	hSemaphore2 = CreateSemaphore(NULL, 0, 1, szName2);
	if (hSemaphore2 == NULL) {
		std::cerr << "无法创建信号量 2, 错误代码: " << GetLastError() << std::endl;
		oss << "无法创建信号量 2\n";
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

bool recursivelyLs(int parinoAddr, const char* name)
{
	std::vector<int>dir_in_it;
	std::string base = std::string(name);
	std::vector<std::string>nameofdir;
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if (((cur.i_mode >> filemode >> 2) & 1) == 0) {
		//没有读取权限
		printf("权限不足：无读取权限\n");
		oss << "权限不足：无读取权限\n";
		return false;
	}

	//依次取出磁盘块
	int i = 0;
	//打印路径
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
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//输出该磁盘块中的所有目录项
		int j;
		
		for (j = 0; j < 16 && i < cnt; j++) {
			Inode tmp;
			//取出该目录项的inode，判断该目录项是目录还是文件
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, "") == 0) {
				continue;
			}

			//输出信息
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
			//cout << tmp.i_mtime << "这个也可以" << std::endl;
			tm* ptr;	//存储时间
			ptr = gmtime(&tmp.i_mtime);

			//输出权限信息
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

			//其它
			printf("%d\t", tmp.i_cnt);	//链接
			oss << tmp.i_cnt << "\t";
			printf("%s\t", tmp.i_uname);	//文件所属用户名
			oss << tmp.i_uname << "\t";
			printf("%s\t", tmp.i_gname);	//文件所属用户名
			oss << tmp.i_gname << "\t";
			printf("%d B\t", tmp.i_size);	//文件大小
			oss << tmp.i_size << " B\t";
			if (ptr != nullptr)
			{
				printf("%d.%d.%d %02d:%02d:%02d  ", 1900 + ptr->tm_year, ptr->tm_mon + 1, ptr->tm_mday, (8 + ptr->tm_hour) % 24, ptr->tm_min, ptr->tm_sec);	//上一次修改的时间

				oss << (1900 + ptr->tm_year) << "."
					<< (ptr->tm_mon + 1) << "."
					<< ptr->tm_mday << " "
					<< std::setfill('0') << std::setw(2) << (8 + ptr->tm_hour) % 24 << ":"
					<< std::setw(2) << ptr->tm_min << ":"
					<< std::setw(2) << ptr->tm_sec << "\t";
			}
			printf("%s", dirlist[j].itemName);	//文件名
			oss << dirlist[j].itemName;
			printf("\n");
			oss << "\n";
			i++;
		}

	}
	//进行递归
	bool is;
	for (int i=0;i<dir_in_it.size();i++)
	{
		is=recursivelyLs(dir_in_it[i], nameofdir[i].c_str());
		if (!is)
		{
			return false;
		}
	}
	/*  未写完 */
	return true;
}

bool find_file_is_empty(int parinoAddr, const char name[])
{
	//从这个地址取出inode
	Inode cur;
	fseek(file, parinoAddr, SEEK_SET);
	fread(&cur, sizeof(Inode), 1, file);

	//取出目录项数
	int cnt = cur.i_cnt;

	//判断文件模式。6为owner，3为group，0为other
	int filemode;
	if (strcmp(Cur_User_Name, cur.i_uname) == 0)
		filemode = 6;
	else if (strcmp(Cur_User_Name, cur.i_gname) == 0)
		filemode = 3;
	else
		filemode = 0;

	if ((((cur.i_mode >> filemode >> 2) & 1) == 0) && (strcmp(Cur_User_Name, "root") != 0)) {
		//没有写入权限
		printf("权限不足：无读取权限\n");
		oss << "权限不足：无读取权限\n";
		return false;
	}


	//依次取出磁盘块
	int i = 0;
	while (i < 160) {	//小于160
		DirItem dirlist[16] = { 0 };

		if (cur.i_dirBlock[i / 16] == -1) {
			i += 16;
			continue;
		}
		//取出磁盘块
		int parblockAddr = cur.i_dirBlock[i / 16];
		fseek(file, parblockAddr, SEEK_SET);
		fread(&dirlist, sizeof(dirlist), 1, file);

		//找到要删除的目录
		int j;
		for (j = 0; j < 16; j++) {
			Inode tmp;
			//取出该目录项的inode，判断该目录项是目录还是文件
			fseek(file, dirlist[j].inodeAddr, SEEK_SET);
			fread(&tmp, sizeof(Inode), 1, file);

			if (strcmp(dirlist[j].itemName, name) == 0) {
				if (((tmp.i_mode >> 9) & 1) == 1) {	//找到目录
					//是目录
					if (tmp.i_cnt <= 2)
					{
						return true;//表示这个文件夹为空，可以直接删除
					}
				}
				else {
					//不是目录，不管
				}
			}
			i++;
		}

	}
	//printf("没有找到该目录\n");
	//oss << "没有找到该目录\n";
	return false;
}