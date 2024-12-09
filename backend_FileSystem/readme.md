![image](https://github.com/user-attachments/assets/ef449f17-fc05-4120-a283-cabd0e9baf43)
在文件系统端实现的功能如上，在设计时先是对一些可复用性较强的功能进行封装，封装完毕后，在此基础上去设计完成相应任务的主功能函数，实现了高复用系统。在shell端和文件系统端还设计了相应的枚举类去进行命令接口上的沟通，实现了接口的统一。其次将文件系统端所有可能出现的结果将其封装成枚举类返回shell端，让其能够更好的输出错误原因及进行后续处理。  
使用的最主要算法是成组链接法：
![image](https://github.com/user-attachments/assets/3de5a634-ae87-4776-8c61-2985454c228b)
实现权限管理
2.1 权限设置
#define MODE_DIR        01000                                //目录标识
#define MODE_FILE        00000                               //文件标识
#define OWNER_R        4<<6                                  //本用户读权限
#define OWNER_W        2<<6                                  //本用户写权限
#define OWNER_X        1<<6                                  //本用户执行权限
#define GROUP_R        4<<3                                  //组用户读权限
#define GROUP_W        2<<3                                  //组用户写权限
#define GROUP_X        1<<3                                  //组用户执行权限
#define OTHERS_R        4                                    //其它用户读权限
#define OTHERS_W        2                                    //其它用户写权限
#define OTHERS_X        1                                   //其它用户执行权限
#define FILE_DEF_PERMISSION 0664                             //文件默认权限
#define DIR_DEF_PERMISSION        0755                       //目录默认权限
(tmp.i_mode >> 9) & 1) == 1)                                 //结果为1表示是目录，为0则为不是目录      //就代表了不是目录，而是文件
本用户的读权限: 000 100 000 000
本用户的写权限: 000 010 000 000
本用户的执行权: 000 001 000 000
用户组的读权限: 000 000 100 000
用户组的写权限: 000 000 010 000
用户组的执行权: 000 000 001 000
目录权限设置为755(8) = 111 101 101(2)
则可以看出本用户（拥有者）：111 → 读、写、执行（7）组用户：101 → 读、执行（5）其他用户：101 → 读、执行（5）
文件默认权限为0664（8）=110 110 100：文件拥有者有读写权限，组用户有读写权限，其它用户也有读权限。

2.2 /etc/passwd文件配置
用户信息配置方法如下：
sprintf(buf, "root:x:%d:%d\n", nextUID++, nextGID++);
sprintf 函数将一些用户信息格式化为字符串，并存储在 buf 字符数组"root:x:%d:%d\n" 是格式字符串，表示将要生成的字符串内容。"root" 是用户名。
"x" 表示密码字段，通常在 /etc/passwd 文件中，x 用来占位表示密码已被加密并存储在 /etc/shadow 文件中。
%d 用于插入整数值，后面会替换为 nextUID++ 和 nextGID++ 的值。
\n 表示换行符，表示每个用户信息在一行显示。

2.3 /etc/passwd文件配置
然后创建/etc/passwd文件存储用户信息，文件格式如下：
username:password:UID:GID:comment:home_directory:shell
username：用户名，用于登录。
password：密码占位符。实际的加密密码存储在 /etc/shadow 文件中，这里通常会显示为 x 或者 *。
UID：用户 ID（User ID），系统使用它来识别用户。
GID：用户组 ID（Group ID），指向 /etc/group 中的某个组。
comment：用户信息字段，通常用于存储用户的全名或其他信息。
home_directory：用户的主目录路径。
shell：用户的默认 shell 程序。

2.4 /etc/shadow文件配置
创建/etc/shadow文件，文件格式如下用来存储用户密码。
username:password:last_change:min:max:warn:inactive:expire
username：用户名，与 /etc/passwd 中的用户名对应。
password：加密后的密码。如果字段中有 ! 或者 *，表示该账户被禁用。
last_change：密码最后修改时间，以自 1970 年 1 月 1 日以来的天数表示。
min：两次密码修改之间的最小天数。
max：密码有效的最大天数。
warn：密码到期前的警告天数。
inactive：密码过期后禁用账户的天数。
expire：账户过期日期（自 1970 年 1 月 1 日以来的天数）

2.5 /etc/group文件配置
最后配置/etc/group文件来配置用户组信息，文件格式如下。
group_name:password:GID:user_list
group_name：组名。
password：组密码，通常为空，现代系统中通常不用。
GID：组 ID（Group ID）。
user_list：以逗号分隔的用户名列表，表示属于该组的用户。


3.实现多进程下的共享读与互斥写
                   
图14 P操作原理
主要是用来拿取一个信号量。
                  
图15 V操作原理


