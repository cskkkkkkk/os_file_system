#pragma once
#include<iostream>
#include <conio.h>  // 包含 _getch() 函数
#include<vector>
#include <fstream>
#include <string>
#include <sstream>
#include<iomanip>
#include"common.h"
// 定义方向键代码以进行输入处理
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

// 颜色控制宏
#define GREEN "\033[32m"
#define WHITE "\033[37m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
// 用于存储上一次的路径信息
std::vector<std::string> last_path;
// 用于存储当前路径信息
std::vector<std::string> current_path;
// 用于存储当前命令
std::string current_command;
// 用于存储历史执行命令
std::vector<std::string> history;
bool cd_command = true;
bool change_command = false;
ErrorCode current_command_state = ErrorCode::SUCCESS;
// 当前用户名
std::string current_username = "root";
// 当前密码
std::string current_password;
// 用于存储命令的各个参数
std::vector<std::string> args;
// 已定义的命令
std::vector<std::string> defined_command = {
        "ls","copy","cat","check","cd","md","rd","super","inode","block","vi","newfile",
        "del","cls","logout","useradd","userdel","chmod","help","format","exit"
};
// 当前命令匹配的所有相关命令
std::vector<std::string> matches;
// 当前命令匹配的索引
uint32_t matches_id = 0;
// 用于判断当前键是否为tab
bool is_tab = false;

int getch() {
    return _getch();  // Windows 下的 _getch() 用于获取一个字符，且没有回显
}


#define cur_semaphore "cur_semaphore"
#define par_semaphore "par_semaphore"
#define SHM_NAME L"SimDiskSharedMemory"
// 共享内存
SharedMemory* sharedMemory;
class Shell {
public:
    //Semaphore*semaphore;
    //Semaphore* parsemaphore;
    HANDLE hMapFile;
    HANDLE hSemaphore1;
    HANDLE hSemaphore2;
    Shell()
    {
        //semaphore = new Semaphore(cur_semaphore,1, 1);
        //parsemaphore = new Semaphore(par_semaphore, 0, 1);
        hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_NAME);
        if (hMapFile == NULL) {
            std::cerr << "打开共享内存失败" << std::endl;
            return;
        }

        sharedMemory = (SharedMemory*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory));
        if (sharedMemory == NULL) {
            std::cerr << "共享内存映射失败" << std::endl;
            CloseHandle(hMapFile);
            return;
        }
        hSemaphore1 = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, TEXT("Semaphore1"));
        hSemaphore2 = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, TEXT("Semaphore2"));
    }
    //析构函数关闭句柄
    ~Shell()
    {
        CloseHandle(hMapFile);
        UnmapViewOfFile(sharedMemory); // 解除共享内存的映射
        // 关闭信号量句柄
        CloseHandle(hSemaphore1);
        CloseHandle(hSemaphore2);
    }
    static std::string returnResolutePath(std::string cd_path)
    {
        std::string path = get_path();
        if (cd_path[0] == '/')
        {
            path = cd_path;
        }
        else
        {
            if (path.size() == 1)//说明此时是根目录'/'
            {
                //为了后续拼接正确性，赋值为空
                path = "";
            }
            auto append_path = split_path(cd_path);
            for (const std::string& path_item : append_path) {
                path += '/';
                path += path_item;
            }
        }
        return path;

    }
    static void cd_path(std::string cd_path) {
        if (cd_path.empty()) {
            current_path.clear();
            return;
        }
        if (cd_path == "-") {
            auto temp = last_path;
            last_path = current_path;
            current_path = temp;
            return;
        }
        //        if (cd_path[0] == '~') cd_path = "/home" + cd_path.substr(1);
        if (cd_path[0] == '/') {
            current_path = split_path(cd_path);  //如果是绝对路径覆盖写到current_path里面
        }
        else {
            auto append_path = split_path(cd_path);
            for (const std::string& path : append_path) {
                current_path.emplace_back(path);
            }
        }
        auto temp_path = current_path;
        current_path.clear();
        for (const std::string& path : temp_path) {
            if (path == ".") continue;
            if (path == "..") {
                if (!current_path.empty()) {
                    current_path.pop_back();
                }
                continue;
            }
            current_path.push_back(path);
        }
    }
    /**
     * @brief 判断字符串是否为另一个字符串的前缀 prefix是否是str前缀
     *
     * @param str 要检查的字符串
     * @param prefix 前缀字符串
     * @return true 如果字符串是前缀
     * @return false 如果字符串不是前缀
     */
    static bool is_prefix(const std::string& str, const std::string& prefix) {
        if (str.length() < prefix.length()) {
            return false;
        }
        std::string substring = str.substr(0, prefix.length());
        return substring == prefix;
    }

    /**
     * @brief 获取当前路径字符串
     *
     * @return std::string 当前路径字符串
     */
    static std::string get_path() {
        std::string paths;
        if (current_path.empty()) {
            return "/";
        }
        //        if (current_path[0] == "home") {
        //            for (uint32_t i = 1; i < current_path.size(); ++i) {
        //                paths += "/" + current_path[i];
        //            }
        //            return "~" + paths;
        //        } else {
        for (const auto& path : current_path) {
            paths += "/" + path;
        }
        return paths;
        //        }
    }

    /**
     * @brief 获取显示的路径字符串，包含用户名、主机名和当前路径信息
     *
     * @param path 当前路径字符串
     * @return std::string 显示的路径字符串
     */
    static std::string get_display_path(const std::string& path) {
        std::ostringstream oss;
        oss << GREEN << current_username << '@' << "SHELL-VM-SIMDISK" << WHITE << ":";
        oss << BLUE << get_path();
        if (current_username == "root") {
            oss << WHITE << "# ";
        }
        else {
            oss << WHITE << "$ ";
        }
        return oss.str();
    }
    uint32_t request_id;
    /**
     * @brief 发送请求到Simdisk
     *
     * @param command 要发送的命令字符串
     * @param option 请求的选项，默认为 Option::NONE
     * @return int 操作结果，通常为 0 表示成功
     */
    int send_request(const std::string& command, Option option = Option::NONE) {
        //Semaphore::P(semId);
        Semaphore::P(hSemaphore1);
        int time = 1000;
        while (sharedMemory->request.type == 'n') {
            Sleep(time);
        }//说明请求还没有处理完就进行忙等待
        sharedMemory->request.send(command.c_str(), request_id,current_username.c_str(), option);
        //Semaphore::V(semId);
        Semaphore::V(hSemaphore1);
        //Semaphore::V(parSemId);
        Semaphore::V(hSemaphore2);//通知文件系统端处理消息
        return 0;
    }

    /**
     * @brief 获取Simdisk的响应
     *
     * @param response 存储Simdisk响应的结构体
     * @param state 控制是否等待响应，默认为 true
     * @return int 操作结果，通常为 0 表示成功
     */
    int get_response(Response& response, bool state = true) const {
        int time = 1000;
        while (true) {
            if (sharedMemory->response.id == request_id) {
                strcpy(response.data, sharedMemory->response.data);
                response.id = sharedMemory->response.id;
                response.code = sharedMemory->response.code;
                response.option = sharedMemory->response.option;
                sharedMemory->response.type = 'y';
                break;
            }
            if (state) Sleep(time);
        }
        return 0;
    }
    /**
    * @brief 终端命令行输入
    * 
    * 实现了对用户输入的实时处理，支持命令的历史回顾、光标移动、输入补全等功能。
    * 
    * @path 就是当前路径
    */
    std::string get_string(const std::string& path = "") {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        std::string command;  //命令字符串
        char ch;  //每次读取的字符
        int curr = 0;  //跟踪当前历史记录的索引，用于支持“向上”和“向下”历史命令浏览。
        //uint32_t pos = 0;  // 表示光标的位置，用于控制光标在命令行中的移动。
        size_t pos = 0;       // 光标位置
        while (true) {
            ch = getch();//从终端读取一个字符
            //if (ch != 9) {   //9代表tab键
              //  is_tab = false;
            //}
            //if (ch == 27) { //27代表ESC键
            if(ch==-32){
                ch = getch();
                //if (ch == 91) {//91代表[,转义序列的开始符号，表示接下来的字符会描述方向键的操作
                    //ch = getch();
                    switch (ch) {//接下来判断是哪个方向键
                    case UP: {
                        if (curr + 1 <= (int)history.size()) {
                            ++curr;
                        }//向历史命令的上一条命令移动
                        //printf("\r\033[K");//清除当前行并将光标移到行首
                        //std::string lastCommand = ((int)history.size() >= curr && curr > 0) ? history[history.size() - curr] : "";//如果超过条件，就为空
                        //command = lastCommand;
                        //pos = lastCommand.size();
                        //printf("%s", (path + lastCommand).c_str());//打印路径和命令
                        std::string lastCommand = (curr > 0 && curr <= (int)history.size())
                            ? history[history.size() - curr]
                            : "";
                        command = lastCommand;
                        pos = lastCommand.size();
                        COORD cursorPos = get_cursor_position(hConsole);
                        set_cursor_position(hConsole, 0, cursorPos.Y);
                        clear_line_from_cursor(hConsole);
                        //std::cout << path << lastCommand << std::string(50, ' ') << "\r" << path << lastCommand;
                        //clear_line(hConsole, path);
                        std::cout << path << lastCommand;

                    } break;
                    case DOWN: {
                        if (curr - 1 >= 0) {
                            --curr;
                        }
                        //printf("\r\033[K");
                        //std::string lastCommand = ((int)history.size() >= curr && curr > 0) ? history[history.size() - curr] : "";
                        //command = lastCommand;
                        //pos = lastCommand.size();
                        //printf("%s", (path + lastCommand).c_str());
                        std::string lastCommand = (curr > 0 && curr <= (int)history.size())
                            ? history[history.size() - curr]
                            : "";
                        command = lastCommand;
                        pos = lastCommand.size();
                        COORD cursorPos = get_cursor_position(hConsole);
                        set_cursor_position(hConsole, 0, cursorPos.Y);
                        clear_line_from_cursor(hConsole);
                        //std::cout << path << lastCommand << std::string(50, ' ') << "\r" << path << lastCommand;
                        std::cout << path << lastCommand;
                    } break;
                    case LEFT: {
                        if (pos > 0) {
                            --pos;
                           // printf("\033[1D");//终端将光标向左移动。
                            COORD cursorPos = get_cursor_position(hConsole);
                            set_cursor_position(hConsole, cursorPos.X - 1, cursorPos.Y);
                        }
                       
                    } break;
                    case RIGHT: {
                        if (pos < command.size()) {
                            ++pos;
                           // printf("\033[1C");//终端将光标向右移动
                            COORD cursorPos = get_cursor_position(hConsole);
                            set_cursor_position(hConsole, cursorPos.X + 1, cursorPos.Y);
                        }
                        
                    } break;
                    }
                }
           
            else if (ch == '\b') {//是backspace键用于删除光标前的字符,linux下是127

                if (!command.empty() && pos > 0) {
                    command.erase(command.begin() + pos - 1);
                    --pos;
                    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                    COORD cursorPos = get_cursor_position(hConsole);
                    std::cout << "\b \b"; // 在当前光标位置之前插入一个空格，然后将光标移回该位置
                    if (pos < command.size()) {
                        //COORD cursorPos = get_cursor_position(hConsole);
                        //set_cursor_position(hConsole, cursorPos.X + 1, cursorPos.Y);
                        clear_line_from_cursor(hConsole);
                        std::cout << &command[pos];


                        //SetConsoleCursorPosition(hConsole, { cursorPos.X - 1, cursorPos.Y });
                        set_cursor_position(hConsole, cursorPos.X - 1, cursorPos.Y);
                    }
                    else {
                        //SetConsoleCursorPosition(hConsole, { cursorPos.X - 1, cursorPos.Y });
                        set_cursor_position(hConsole, cursorPos.X - 1, cursorPos.Y);
                    }
                }
            }
            else if (ch == 13) {//10是回车键linux下，windows下是13'\r'
                printf("\n");
                if (!command.empty()) history.push_back(command);//放入历史命令里面然后返回command
                return command;
            }
            else if (ch == 9) {//9是tab键
                if (is_tab) {//启动补全
                    printf("\r\033[K");//\r移动到行首，\033[K表示清除当前行的内容
                    matches_id = (matches_id + 1) % matches.size();//matches 向量存储补全候选项，记录当前选择的补全候选项的索引,每次按tab就循环遍历
                    command = matches[matches_id];//更新comman
                    pos = matches[matches_id].size();//更新光标位置
                    printf("%s", (path + matches[matches_id]).c_str());//Path表示当前路径结合补全结果打印完整命令
                }
                else {//启动补全建议
                    std::vector<std::string> commands = split_command(command);
                    if (commands.size() == 1) {
                        matches.clear();
                        for (const auto& defined : defined_command) {
                            if (is_prefix(defined, commands[0])) {
                                matches.push_back(defined);
                            }
                        }
                        if (!matches.empty()) {
                            printf("\r\033[K");
                            is_tab = true;
                            matches_id = 0;
                            command = matches[matches_id];
                            pos = matches[matches_id].size();
                            printf("%s", (path + matches[matches_id]).c_str());
                        }
                    }
                    if (commands.size() == 2) {
                        if (commands[0] == "sudo") {
                            matches.clear();
                            for (const auto& defined : defined_command) {
                                if (is_prefix(defined, commands[1])) {
                                    matches.push_back(defined);
                                }
                            }
                            if (!matches.empty()) {
                                is_tab = true;
                                matches_id = 0;
                                printf("\r\033[K");
                                command = matches[matches_id];
                                pos = matches[matches_id].size();
                                printf("%s", (path + matches[matches_id]).c_str());
                            }
                        }
                        else {
                            send_request(command, Option::TAB);
                            Response response{};
                            get_response(response);
                            std::string match = response.data;
                            std::istringstream iss(match);
                            std::vector<std::string> results;
                            std::string result;
                            matches.clear();
                            while (iss >> result) {
                                results.push_back(result);
                            }
                            if (!results.empty()) {
                                is_tab = true;
                                matches_id = 0;
                                printf("\r\033[K");
                                while (!command.empty()) {
                                    if (command.back() == '/' || command.back() == ' ') {
                                        for (const auto& res : results) {
                                            matches.push_back(command + res);
                                        }
                                        break;
                                    }
                                    else {
                                        command.pop_back();
                                    }
                                }
                                command = matches[0];
                                pos = command.size();
                                printf("%s", (path + command).c_str());
                            }
                        }
                    }
                    if (commands.size() > 2) {
                        send_request(command, Option::TAB);
                        Response response{};
                        get_response(response);
                        std::string match = response.data;
                        std::istringstream iss(match);
                        std::vector<std::string> results;
                        std::string result;
                        matches.clear();
                        while (iss >> result) {
                            results.push_back(result);
                        }
                        if (!results.empty()) {
                            is_tab = true;
                            matches_id = 0;
                            printf("\r\033[K");
                            while (!command.empty()) {
                                if (command.back() == '/' || command.back() == ' ') {
                                    for (const auto& res : results) {
                                        matches.push_back(command + res);
                                    }
                                    break;
                                }
                                else {
                                    command.pop_back();
                                }
                            }
                            command = matches[0];
                            pos = command.size();
                            printf("%s", (path + command).c_str());
                        }
                    }
                }
            }
            else {
                command.insert(pos, 1, ch);
                ++pos;
                if (command.size() == pos) {
                    printf("%c", ch);
                }//在末尾就直接打印
                else {//在中间或者其他就打印一个然后
                    printf("%c", ch);
                    //printf("\033[0K");//清除光标位置到行尾
                    COORD cursorPos = get_cursor_position(hConsole);
                    clear_line_from_cursor(hConsole);
                    for (auto i = pos; i < command.size(); ++i) {
                        printf("%c", command[i]);
                    }//先打印
                    //for (auto i = pos; i < command.size(); ++i) {
                      //  printf("\033[1D");
                    //}//再回退光标
                    set_cursor_position(hConsole, cursorPos.X, cursorPos.Y);
                }
                //COORD cursorPos = get_cursor_position(hConsole);
                //set_cursor_position(hConsole, 0, cursorPos.Y);
                //std::cout << path << command;
                //set_cursor_position(hConsole, path.size() + pos, cursorPos.Y);
            }
        }
    }
    /**
    *  用户输入获取密码或其他敏感信息时，隐藏输入的内容
    * 
    * 
    * 
    * 
    */
    static void invisible(const std::string& message, std::string& password) {
        HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);  // 获取标准输入句柄（控制台的输入）
        // 获取当前控制台的输入模式
        DWORD dwOriginalMode = 0;
        GetConsoleMode(hConsole, &dwOriginalMode);
        // 修改控制台模式，禁用输入回显
        DWORD dwNewMode = dwOriginalMode & ~ENABLE_ECHO_INPUT;
        SetConsoleMode(hConsole, dwNewMode);
        printf("%s", message.c_str());
        // 从输入流中读取用户输入，此时输入内容将不可见
        std::getline(std::cin, password);
        // 恢复终端设置
        SetConsoleMode(hConsole, dwOriginalMode);
    }

    void shell() {
    begin:
        std::string path = get_display_path(get_path());//之前看到的前缀信息
        std::cout << path;
        current_command = get_string(path);//输入命令
        args = split_command(current_command);
        current_command.clear();
        for (const auto& arg : args) {
            current_command += arg + ' ';
        }
        if (!current_command.empty()) current_command.pop_back();
        if (args.empty()) goto begin;
        if (args.empty()) {
            goto begin;
        }
        else if (args[0] == "cat") {
            if (args.size() == 1) {
                printf("cat: missing operand\n");
                goto begin;
            }
            if (args.size() > 3) {
                printf("cat: too many arguments\n");
                goto begin;
            }
            if (args.size() == 2) {//记得cat用的时候只能有两个参数，其他的没处理
                std::string path_resolute = returnResolutePath(args[1]);
                send_request("cat " + path_resolute, Option::CAT);
                Response response{};
                get_response(response);
                if (response.code == ErrorCode::SUCCESS) {
                    if (response.option == Option::PATCH) {//说明很大所以要分页
                        uint32_t size = (std::stoul(std::string(response.data)) + 1023) / 1024;
                        std::string result;
                        for (uint32_t i = 0; i < size; ++i) {
                            send_request("cat " + std::to_string(i), Option::PATCH);
                            get_response(response, false);
                            printf("%s", response.data);
                        }
                    }
                    else {
                        printf("%s", response.data);
                    }
                  
                }
                else {
                    printf("%s", response.data);
                }
                return;
            }
            else {
                send_request("cat " + args[2], Option::NONE);
                Response response{};
                get_response(response);
                if (response.code == ErrorCode::FAILURE) {
                    printf("%s", response.data);
                    return;
                }
                if (args[1] == "-w") {
                    send_request("cat " + args[2], Option::GET);
                    get_response(response);
                    if (response.code == ErrorCode::FAILURE) {
                        printf("%s", response.data);
                        return;
                    }

                    //
                    /*pid_t pid = fork();
                    std::string name = response.data;
                    if (pid == -1) {
                        printf("\n");
                        return;
                    }
                    else if (pid == 0) {
                        name = response.data;
                        system(("nano " + name).c_str());
                        exit(0);
                    }
                    else {
                        int status;
                        waitpid(pid, &status, 0);
                        if (WIFEXITED(status)) {
                            send_request("cat " + args[2], Option::WRITE);//再发送写操作（Option::WRITE）来保存文件。
                            get_response(response);
                        }
                    }
                    */
                    //


                }
                else if (args[1] == "-r") {
                    send_request("cat " + args[2], Option::READ);
                    get_response(response);
                    if (response.code == ErrorCode::FAILURE) {
                        printf("%s", response.data);
                        return;
                    }

                    //
                    /*pid_t pid = fork();
                    std::string name = response.data;
                    if (pid == -1) {
                        printf("\n");
                        return;
                    }
                    else if (pid == 0) {
                        name = response.data;
                        system(("less -N " + name).c_str());
                        exit(0);
                    }
                    else {
                        int status;
                        waitpid(pid, &status, 0);
                        if (WIFEXITED(status)) {
                            send_request("cat " + args[2], Option::EXIT);
                            get_response(response);
                        }
                    }
                    */
                    //




                }
                else {
                    goto begin;
                }
            }
            return;
        }
        else if (args[0] == "cd") {
            if (args.size() == 1)
            {
                printf("cd: one more argument needed\n");
                goto begin;
            }
            if (args.size() > 2) {
                printf("cd: too many arguments\n");
                goto begin;
            }
            cd_command = true;
            
            std::string command = args[0];
            std::string args1 = returnResolutePath(args[1]);
            command += ' ';
            command += args1;
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "check") {
            if (args.size() != 1)
            {
                printf("check: count of arguments is not correct\n");
                goto begin;
            }
        }
        else if (args[0] == "copy") {
            if (args.size() < 3) {
                printf("copy: missing operand\n");
                goto begin;
            }
            if (args.size() > 3) {
                printf("copy: too many arguments\n");
                goto begin;
            }
            std::string command=args[0];
            std::string args1;
            std::string args2;
            if (std::strstr(args[1].c_str(), "<host>"))
            {
                args1=args[1];
            }
            else
            {
                args1 = returnResolutePath(args[1]);
            }
            if (std::strstr(args[2].c_str(), "<host>"))
            {
                args2 = args[2];
            }
            else
            {
                args2 = returnResolutePath(args[2]);
            }
            command += " ";
            command += args1;
            command += " ";
            command += args2;
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "del") {
            if (args.size() != 2)
            {
                printf("del: count of arguments is not correct\n");
                goto begin;
            }
            std::string command = args[0];
            std::string path_resolute = returnResolutePath(args[1]);
            command += " ";
            command += path_resolute;
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "dir") {
            if (args.size()==1||args.size()>3)
            {
                printf("dir: count of arguments is not correct\n");
                goto begin;
            }
            std::string command = args[0];
            if (args[1] == "-s")
            {
                std::string path_resolute = returnResolutePath(args[2]);
                command += " ";
                command += "-s";
                command += " ";
                command += path_resolute;
            }
            else
            {
                std::string path_resolute = returnResolutePath(args[1]);
                command += " ";
                command += path_resolute;
            }
            
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "echo") {

        }
        else if (args[0] == "exec") {

        }
        else if (args[0] == "help") {
            std::cout << "These shell commands are defined internally.  Type 'help' to see this list." << std::endl;
           
            std::cout << "-------------------------------------------------------------------" << std::endl;
            std::cout << std::right << std::setw(7) << "Command" << std::setw(60) << "Information" << std::endl;
            std::cout << "-------------------------------------------------------------------" << std::endl;
            std::cout << std::right << std::setw(7) << "cat" << std::setw(60) << "展示文件内容" << std::endl;
            std::cout << std::right << std::setw(7) << "cd" << std::setw(60) << "改变当前工作目录" << std::endl;
            std::cout << std::right << std::setw(7) << "check" << std::setw(60) << "检查文件系统" << std::endl;
            std::cout << std::right << std::setw(7) << "chmod" << std::setw(60) << "改变文件和目录的权限：chmod 名字 权限" << std::endl;
            std::cout << std::right << std::setw(7) << "clear" << std::setw(60) << "清空屏幕" << std::endl;            
            std::cout << std::right << std::setw(7) << "copy" << std::setw(60) << "复制文件到指定目录" << std::endl;
            std::cout << std::right << std::setw(7) << "del" << std::setw(60) << "删除已经存在的文件" << std::endl;
            std::cout << std::right << std::setw(7) << "dir" << std::setw(60) << "展示此目录下的文件和目录" << std::endl;
            //std::cout << std::right << std::setw(7) << "echo" << std::setw(60) << "Print a message to the console" << std::endl;
            std::cout << std::right << std::setw(7) << "exit" << std::setw(60) << "退出shell" << std::endl;
            std::cout << std::right << std::setw(7) << "help" << std::setw(60) << "展示可用的命令和它们的描述" << std::endl;
            std::cout << std::right << std::setw(7) << "info" << std::setw(60) << "展示文件系统的详细信息" << std::endl;
            std::cout << std::right << std::setw(7) << "ls" << std::setw(60) << "展示在此目录下的所有文件和目录" << std::endl;
            //std::cout << std::right << std::setw(7) << "ll" << std::setw(60) << "List files and directories with detailed information" << std::endl;
            std::cout << std::right << std::setw(7) << "md" << std::setw(60) << "创建一个新的文件夹" << std::endl;
            std::cout << std::right << std::setw(7) << "newfile" << std::setw(60) << "创建一个新的文件" << std::endl;
            std::cout << std::right << std::setw(7) << "rd" << std::setw(60) << "删除一个已存在的目录" << std::endl;
            std::cout << std::right << std::setw(7) << "su" << std::setw(60) << "改变到另一个用户账户" << std::endl;
            std::cout << std::right << std::setw(7) << "sudo" << std::setw(60) << "用超级权限执行命令" << std::endl;
            std::cout << "-------------------------------------------------------------------" << std::endl;
            std::cout << std::left;
        }
        else if (args[0] == "info") {
            // TODO:
            if (args.size() != 1)
            {
                printf("check: count of arguments is not correct\n");
                goto begin;
            }
        }
        else if (args[0] == "cls")
        {
            if (args.size() != 1)
            {
                printf("check: count of arguments is not correct\n");
                goto begin;
            }
            system("cls");
        }
        else if (args[0] == "chmod")
        {   
            if (args.size() < 3)
            {
                printf("chmod:参数小于3\n");
                goto begin;
            }
            if (args.size() > 3)
            {
                printf("chmod:count of arguments exceed three\n");
                goto begin;
            }
            std::string command = args[0];
            std::string path_resolute = returnResolutePath(args[1]);
            command += " ";
            command += path_resolute;
            command += " ";
            command += args[2];
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "ls") {
            if (args.size() != 1)
            {
                printf("ls: count of arguments is not correct\n");
                goto begin;
            }
            std::string command = args[0];
            std::string path_resolute = get_path();
            command += " ";
            command += path_resolute;
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "ll") {

        }
        else if (args[0] == "md") {
            if (args.size() == 1) {
                printf("md: missing operand\n");
                goto begin;
            }
            if (args.size() != 2)
            {
                printf("md: count of arguments is not correct\n");
                goto begin;
            }
            std::string command = args[0];
            std::string path_resolute = returnResolutePath(args[1]);
            command += " ";
            command += path_resolute;
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "newfile") {
            if (args.size() == 1) {
                printf("newfile: missing operand\n");
                goto begin;
            }
            if (args.size() != 2)
            {
                printf("newfile: count of arguments is not correct\n");
                goto begin;
            }
            std::string command = args[0];
            std::string path_resolute = returnResolutePath(args[1]);
            command += " ";
            command += path_resolute;
            send_request(command);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;
            return;
        }
        else if (args[0] == "rd") {
            if (args.size() == 1) {
                printf("rd: missing operand\n");
                goto begin;
            }
            //printf("目录不为空，确认删除吗？[y/n] y\n");
            for (int i = 1; i < args.size(); ++i) {
                std::string path_resolute = returnResolutePath(args[i]);
                std::string sub_command = "rd " + path_resolute;
                send_request(sub_command);
                Response response{};
                get_response(response);
                current_command_state = response.code;
                if (response.code == ErrorCode::SUCCESS && strlen(response.data) > 0 && args.size() > 2) {
                    printf("rd %s: %s ", args[i].c_str(), response.data);
                }
                else {
                    if (strlen(response.data) > 0) printf("%s ", response.data);

                }
                if (response.option == Option::REQUEST) {
                    std::string option;
                    printf("目录不为空，确认删除吗？[y/n]");
                    std::getline(std::cin, option);
                    if (option == "Y" || option == "y") {
                        send_request(sub_command, Option::RESPONSE);
                        get_response(response);
                        current_command_state = response.code;
                        printf("%s", response.data);
                    }
                }
            }
            return;
        }
        else if (args[0] == "scp") {

        }
        else if (args[0] == "save") {

        }
        else if (args[0] == "su") {
            if (args.size() == 1) {
                printf("su: missing operand\n");
                goto begin;
            }
            if (args.size() > 2) {
                printf("su: too many arguments\n");
                goto begin;
            }
            invisible("su: password for " + args[1] + ": ", current_password);
            //printf("\n");
            send_request(current_command + ' ' + current_password, Option::SWITCH);
            Response response{};
            get_response(response);
            printf("%s", response.data);
            current_command_state = response.code;//当前命令执行状态
            if (current_command_state == ErrorCode::SUCCESS)
            {
                change_command = true;//代表切换了用户
                current_username = args[1];//表示切换用户了
            }
            return;
        }//su 用户名 password
        else if (args[0] == "sudo") {
            if (current_username != "root") {
                // 禁用终端回显
                invisible("[sudo] password for root: ", current_password);
                printf("\n");
                send_request("su root " + current_password, Option::NONE);
                Response response{};
                get_response(response);
                if (response.code == ErrorCode::FAILURE) {
                    printf("[sudo]: Authentication failure\n");
                    goto begin;
                }
            }
            if (args.size() == 3) {
                if (args[1] == "useradd") {
                    std::string password1;
                    invisible("[sudo] setting password for " + args[2] + ": ", password1);
                    //printf("\n");
                    std::string password2;
                    invisible("[sudo] setting password for " + args[2] + " again: ", password2);
                    //printf("\n");
                    if (password1 == password2) {
                        current_password = password1;
                        current_command += ' ' + current_password;
                    }
                    else {
                        printf("[sudo] Sorry, passwords do not match.\n");
                        goto begin;
                    }
                }
                else if (args[1] == "userdel")
                {
                    
                }
            }
        }//sudo useradd 用户名 password
        else if (args[0] == "clear") {
            system("cls");
            goto begin;
        }
        else if (args[0] == "exit") {
            std::cout << "Do you want to exit the file system? [Y/n] ";
            std::string option;
            std::getline(std::cin, option);
            if (option == "y" || option == "Y") {
                send_request(current_command);
                return;
            }
            else {
                goto begin;
            }
        }
        else if (args[0] == "super")
        {

        }
        else {
            printf("%s: command not found\n", args[0].c_str());
            goto begin;
        }

        send_request(current_command);
        Response response{};
        get_response(response);
        printf("%s", response.data);
        current_command_state = response.code;
    }
    /**
     * @brief 运行Simdisk Shell
     *
     * 发送 NEW 请求，获取Simdisk的初始化响应，然后进入Shell循环。
     * 在循环中，根据当前命令的执行状态和类型，执行相应的操作。
     *
     * @note 当前实现中的具体操作逻辑需要根据实际代码来填写注释。
     */
    void run() {
        // 发送 NEW 请求，告知Simdisk创建新的Shell
        send_request("su root root", Option::NEW);
        Response response{};
        // 获取Simdisk的初始化响应
        get_response(response);
        // 如果初始化失败，则直接返回
        if (response.code == ErrorCode::FAILURE) return;
        while (true) {
            // 根据当前命令的执行状态和类型执行相应的操作
            if (current_command_state == ErrorCode::SUCCESS) {//如果发送的命令被正确执行
                if (cd_command) {
                    // 如果是 cd 命令，则根据当前命令执行 cd_path 操作
                    if (current_command.empty()) cd_path("");//更新current_path的vector，进入shell再去调用display_path生成
                    else cd_path(args[1]);
                }
                if (change_command) cd_path("");//当切换了用户角色，相当于将当前路径给清空
            }
            cd_command = false; change_command = false;
            // 执行Shell逻辑，具体操作需要根据实际代码来填写注释
            shell();
            // 如果当前命令是 "exit"，则跳出循环
            if (current_command == "exit") break;
        }

    }

};







int main()
{
    //enable_virtual_terminal();
    Shell shell{};
    shell.run();
    return 0;
}