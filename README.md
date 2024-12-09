# os_file_system
在磁盘上模拟linux下文件系统，实现了shell,和后台文件系统
实现图如下：  
![image](https://github.com/user-attachments/assets/b142404b-9d41-40fc-82c4-aa85d2e9b83c)
系统架构图如下：
![image](https://github.com/user-attachments/assets/cb944767-1589-4ca1-a3da-7ff0fa712909)
文件盘块结构图如下：  
![image](https://github.com/user-attachments/assets/2df0636a-3032-44cc-9651-ac9221d56335)
通过对文件的合理布局去模拟真实文件系统，以实现对文件系统的基本操作。该文件系统磁盘设计结构是以100M内存空间，磁盘块大小为1k为基础去规划和实现的。这是整个文件系统的地基，包括位图，inode节点，目录项，文件都直接依赖于文件系统磁盘设计的结构，一个好的结构能更方便地实现文件系统。  
![image](https://github.com/user-attachments/assets/89ff1e91-5286-402e-a4f5-e0412bd23186)
初始化超级块的基本元信息，然后将磁盘块给逻辑意义上的分组然后在每组的第一个磁盘块专门用来存放每个磁盘块的地址。每组第一个磁盘块的第一个目录项用来存储下一组磁盘块的起始地址，其它目录项用来存储本组其它磁盘块的起始地址。然后就是初始化根目录地址，初始化/home和/home/root目录，初始化/etc目录，然后在/etc目录下初始化存放用户信息文件，用户组文件和用户密码文件，作为用户切换时验证文件。
在初始化完毕磁盘块后将，超级块，inode位图，block位图等信息读取到内存中进行文件系统信息的挂载。根据位图信息以及磁盘块成组链接法所存放的地址，就能够实现整个文件系统对inode节点，block区的分配与回收了。  
