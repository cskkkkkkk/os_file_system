#pragma once
#include<iostream>
#include <conio.h>  // ���� _getch() ����
#include<vector>
#include <fstream>
#include <string>
#include <sstream>
#include<iomanip>
#include"common.h"
// ���巽��������Խ������봦��
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

// ��ɫ���ƺ�
#define GREEN "\033[32m"
#define WHITE "\033[37m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
// ���ڴ洢��һ�ε�·����Ϣ
std::vector<std::string> last_path;
// ���ڴ洢��ǰ·����Ϣ
std::vector<std::string> current_path;
// ���ڴ洢��ǰ����
std::string current_command;
// ���ڴ洢��ʷִ������
std::vector<std::string> history;
bool cd_command = true;
bool change_command = false;
ErrorCode current_command_state = ErrorCode::SUCCESS;
// ��ǰ�û���
std::string current_username = "root";
// ��ǰ����
std::string current_password;
// ���ڴ洢����ĸ�������
std::vector<std::string> args;
// �Ѷ��������
std::vector<std::string> defined_command = {
        "ls","copy","cat","check","cd","md","rd","super","inode","block","vi","newfile",
        "del","cls","logout","useradd","userdel","chmod","help","format","exit"
};
// ��ǰ����ƥ��������������
std::vector<std::string> matches;
// ��ǰ����ƥ�������
uint32_t matches_id = 0;
// �����жϵ�ǰ���Ƿ�Ϊtab
bool is_tab = false;

int getch() {
    return _getch();  // Windows �µ� _getch() ���ڻ�ȡһ���ַ�����û�л���
}


#define cur_semaphore "cur_semaphore"
#define par_semaphore "par_semaphore"
#define SHM_NAME L"SimDiskSharedMemory"
// �����ڴ�
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
            std::cerr << "�򿪹����ڴ�ʧ��" << std::endl;
            return;
        }

        sharedMemory = (SharedMemory*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMemory));
        if (sharedMemory == NULL) {
            std::cerr << "�����ڴ�ӳ��ʧ��" << std::endl;
            CloseHandle(hMapFile);
            return;
        }
        hSemaphore1 = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, TEXT("Semaphore1"));
        hSemaphore2 = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, TEXT("Semaphore2"));
    }
    //���������رվ��
    ~Shell()
    {
        CloseHandle(hMapFile);
        UnmapViewOfFile(sharedMemory); // ��������ڴ��ӳ��
        // �ر��ź������
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
            if (path.size() == 1)//˵����ʱ�Ǹ�Ŀ¼'/'
            {
                //Ϊ�˺���ƴ����ȷ�ԣ���ֵΪ��
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
            current_path = split_path(cd_path);  //����Ǿ���·������д��current_path����
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
     * @brief �ж��ַ����Ƿ�Ϊ��һ���ַ�����ǰ׺ prefix�Ƿ���strǰ׺
     *
     * @param str Ҫ�����ַ���
     * @param prefix ǰ׺�ַ���
     * @return true ����ַ�����ǰ׺
     * @return false ����ַ�������ǰ׺
     */
    static bool is_prefix(const std::string& str, const std::string& prefix) {
        if (str.length() < prefix.length()) {
            return false;
        }
        std::string substring = str.substr(0, prefix.length());
        return substring == prefix;
    }

    /**
     * @brief ��ȡ��ǰ·���ַ���
     *
     * @return std::string ��ǰ·���ַ���
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
     * @brief ��ȡ��ʾ��·���ַ����������û������������͵�ǰ·����Ϣ
     *
     * @param path ��ǰ·���ַ���
     * @return std::string ��ʾ��·���ַ���
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
     * @brief ��������Simdisk
     *
     * @param command Ҫ���͵������ַ���
     * @param option �����ѡ�Ĭ��Ϊ Option::NONE
     * @return int ���������ͨ��Ϊ 0 ��ʾ�ɹ�
     */
    int send_request(const std::string& command, Option option = Option::NONE) {
        //Semaphore::P(semId);
        Semaphore::P(hSemaphore1);
        int time = 1000;
        while (sharedMemory->request.type == 'n') {
            Sleep(time);
        }//˵������û�д�����ͽ���æ�ȴ�
        sharedMemory->request.send(command.c_str(), request_id,current_username.c_str(), option);
        //Semaphore::V(semId);
        Semaphore::V(hSemaphore1);
        //Semaphore::V(parSemId);
        Semaphore::V(hSemaphore2);//֪ͨ�ļ�ϵͳ�˴�����Ϣ
        return 0;
    }

    /**
     * @brief ��ȡSimdisk����Ӧ
     *
     * @param response �洢Simdisk��Ӧ�Ľṹ��
     * @param state �����Ƿ�ȴ���Ӧ��Ĭ��Ϊ true
     * @return int ���������ͨ��Ϊ 0 ��ʾ�ɹ�
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
    * @brief �ն�����������
    * 
    * ʵ���˶��û������ʵʱ����֧���������ʷ�عˡ�����ƶ������벹ȫ�ȹ��ܡ�
    * 
    * @path ���ǵ�ǰ·��
    */
    std::string get_string(const std::string& path = "") {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        std::string command;  //�����ַ���
        char ch;  //ÿ�ζ�ȡ���ַ�
        int curr = 0;  //���ٵ�ǰ��ʷ��¼������������֧�֡����ϡ��͡����¡���ʷ���������
        //uint32_t pos = 0;  // ��ʾ����λ�ã����ڿ��ƹ�����������е��ƶ���
        size_t pos = 0;       // ���λ��
        while (true) {
            ch = getch();//���ն˶�ȡһ���ַ�
            //if (ch != 9) {   //9����tab��
              //  is_tab = false;
            //}
            //if (ch == 27) { //27����ESC��
            if(ch==-32){
                ch = getch();
                //if (ch == 91) {//91����[,ת�����еĿ�ʼ���ţ���ʾ���������ַ�������������Ĳ���
                    //ch = getch();
                    switch (ch) {//�������ж����ĸ������
                    case UP: {
                        if (curr + 1 <= (int)history.size()) {
                            ++curr;
                        }//����ʷ�������һ�������ƶ�
                        //printf("\r\033[K");//�����ǰ�в�������Ƶ�����
                        //std::string lastCommand = ((int)history.size() >= curr && curr > 0) ? history[history.size() - curr] : "";//���������������Ϊ��
                        //command = lastCommand;
                        //pos = lastCommand.size();
                        //printf("%s", (path + lastCommand).c_str());//��ӡ·��������
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
                           // printf("\033[1D");//�ն˽���������ƶ���
                            COORD cursorPos = get_cursor_position(hConsole);
                            set_cursor_position(hConsole, cursorPos.X - 1, cursorPos.Y);
                        }
                       
                    } break;
                    case RIGHT: {
                        if (pos < command.size()) {
                            ++pos;
                           // printf("\033[1C");//�ն˽���������ƶ�
                            COORD cursorPos = get_cursor_position(hConsole);
                            set_cursor_position(hConsole, cursorPos.X + 1, cursorPos.Y);
                        }
                        
                    } break;
                    }
                }
           
            else if (ch == '\b') {//��backspace������ɾ�����ǰ���ַ�,linux����127

                if (!command.empty() && pos > 0) {
                    command.erase(command.begin() + pos - 1);
                    --pos;
                    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                    COORD cursorPos = get_cursor_position(hConsole);
                    std::cout << "\b \b"; // �ڵ�ǰ���λ��֮ǰ����һ���ո�Ȼ�󽫹���ƻظ�λ��
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
            else if (ch == 13) {//10�ǻس���linux�£�windows����13'\r'
                printf("\n");
                if (!command.empty()) history.push_back(command);//������ʷ��������Ȼ�󷵻�command
                return command;
            }
            else if (ch == 9) {//9��tab��
                if (is_tab) {//������ȫ
                    printf("\r\033[K");//\r�ƶ������ף�\033[K��ʾ�����ǰ�е�����
                    matches_id = (matches_id + 1) % matches.size();//matches �����洢��ȫ��ѡ���¼��ǰѡ��Ĳ�ȫ��ѡ�������,ÿ�ΰ�tab��ѭ������
                    command = matches[matches_id];//����comman
                    pos = matches[matches_id].size();//���¹��λ��
                    printf("%s", (path + matches[matches_id]).c_str());//Path��ʾ��ǰ·����ϲ�ȫ�����ӡ��������
                }
                else {//������ȫ����
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
                }//��ĩβ��ֱ�Ӵ�ӡ
                else {//���м���������ʹ�ӡһ��Ȼ��
                    printf("%c", ch);
                    //printf("\033[0K");//������λ�õ���β
                    COORD cursorPos = get_cursor_position(hConsole);
                    clear_line_from_cursor(hConsole);
                    for (auto i = pos; i < command.size(); ++i) {
                        printf("%c", command[i]);
                    }//�ȴ�ӡ
                    //for (auto i = pos; i < command.size(); ++i) {
                      //  printf("\033[1D");
                    //}//�ٻ��˹��
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
    *  �û������ȡ���������������Ϣʱ���������������
    * 
    * 
    * 
    * 
    */
    static void invisible(const std::string& message, std::string& password) {
        HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);  // ��ȡ��׼������������̨�����룩
        // ��ȡ��ǰ����̨������ģʽ
        DWORD dwOriginalMode = 0;
        GetConsoleMode(hConsole, &dwOriginalMode);
        // �޸Ŀ���̨ģʽ�������������
        DWORD dwNewMode = dwOriginalMode & ~ENABLE_ECHO_INPUT;
        SetConsoleMode(hConsole, dwNewMode);
        printf("%s", message.c_str());
        // ���������ж�ȡ�û����룬��ʱ�������ݽ����ɼ�
        std::getline(std::cin, password);
        // �ָ��ն�����
        SetConsoleMode(hConsole, dwOriginalMode);
    }

    void shell() {
    begin:
        std::string path = get_display_path(get_path());//֮ǰ������ǰ׺��Ϣ
        std::cout << path;
        current_command = get_string(path);//��������
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
            if (args.size() == 2) {//�ǵ�cat�õ�ʱ��ֻ��������������������û����
                std::string path_resolute = returnResolutePath(args[1]);
                send_request("cat " + path_resolute, Option::CAT);
                Response response{};
                get_response(response);
                if (response.code == ErrorCode::SUCCESS) {
                    if (response.option == Option::PATCH) {//˵���ܴ�����Ҫ��ҳ
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
                            send_request("cat " + args[2], Option::WRITE);//�ٷ���д������Option::WRITE���������ļ���
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
            std::cout << std::right << std::setw(7) << "cat" << std::setw(60) << "չʾ�ļ�����" << std::endl;
            std::cout << std::right << std::setw(7) << "cd" << std::setw(60) << "�ı䵱ǰ����Ŀ¼" << std::endl;
            std::cout << std::right << std::setw(7) << "check" << std::setw(60) << "����ļ�ϵͳ" << std::endl;
            std::cout << std::right << std::setw(7) << "chmod" << std::setw(60) << "�ı��ļ���Ŀ¼��Ȩ�ޣ�chmod ���� Ȩ��" << std::endl;
            std::cout << std::right << std::setw(7) << "clear" << std::setw(60) << "�����Ļ" << std::endl;            
            std::cout << std::right << std::setw(7) << "copy" << std::setw(60) << "�����ļ���ָ��Ŀ¼" << std::endl;
            std::cout << std::right << std::setw(7) << "del" << std::setw(60) << "ɾ���Ѿ����ڵ��ļ�" << std::endl;
            std::cout << std::right << std::setw(7) << "dir" << std::setw(60) << "չʾ��Ŀ¼�µ��ļ���Ŀ¼" << std::endl;
            //std::cout << std::right << std::setw(7) << "echo" << std::setw(60) << "Print a message to the console" << std::endl;
            std::cout << std::right << std::setw(7) << "exit" << std::setw(60) << "�˳�shell" << std::endl;
            std::cout << std::right << std::setw(7) << "help" << std::setw(60) << "չʾ���õ���������ǵ�����" << std::endl;
            std::cout << std::right << std::setw(7) << "info" << std::setw(60) << "չʾ�ļ�ϵͳ����ϸ��Ϣ" << std::endl;
            std::cout << std::right << std::setw(7) << "ls" << std::setw(60) << "չʾ�ڴ�Ŀ¼�µ������ļ���Ŀ¼" << std::endl;
            //std::cout << std::right << std::setw(7) << "ll" << std::setw(60) << "List files and directories with detailed information" << std::endl;
            std::cout << std::right << std::setw(7) << "md" << std::setw(60) << "����һ���µ��ļ���" << std::endl;
            std::cout << std::right << std::setw(7) << "newfile" << std::setw(60) << "����һ���µ��ļ�" << std::endl;
            std::cout << std::right << std::setw(7) << "rd" << std::setw(60) << "ɾ��һ���Ѵ��ڵ�Ŀ¼" << std::endl;
            std::cout << std::right << std::setw(7) << "su" << std::setw(60) << "�ı䵽��һ���û��˻�" << std::endl;
            std::cout << std::right << std::setw(7) << "sudo" << std::setw(60) << "�ó���Ȩ��ִ������" << std::endl;
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
                printf("chmod:����С��3\n");
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
            //printf("Ŀ¼��Ϊ�գ�ȷ��ɾ����[y/n] y\n");
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
                    printf("Ŀ¼��Ϊ�գ�ȷ��ɾ����[y/n]");
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
            current_command_state = response.code;//��ǰ����ִ��״̬
            if (current_command_state == ErrorCode::SUCCESS)
            {
                change_command = true;//�����л����û�
                current_username = args[1];//��ʾ�л��û���
            }
            return;
        }//su �û��� password
        else if (args[0] == "sudo") {
            if (current_username != "root") {
                // �����ն˻���
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
        }//sudo useradd �û��� password
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
     * @brief ����Simdisk Shell
     *
     * ���� NEW ���󣬻�ȡSimdisk�ĳ�ʼ����Ӧ��Ȼ�����Shellѭ����
     * ��ѭ���У����ݵ�ǰ�����ִ��״̬�����ͣ�ִ����Ӧ�Ĳ�����
     *
     * @note ��ǰʵ���еľ�������߼���Ҫ����ʵ�ʴ�������дע�͡�
     */
    void run() {
        // ���� NEW ���󣬸�֪Simdisk�����µ�Shell
        send_request("su root root", Option::NEW);
        Response response{};
        // ��ȡSimdisk�ĳ�ʼ����Ӧ
        get_response(response);
        // �����ʼ��ʧ�ܣ���ֱ�ӷ���
        if (response.code == ErrorCode::FAILURE) return;
        while (true) {
            // ���ݵ�ǰ�����ִ��״̬������ִ����Ӧ�Ĳ���
            if (current_command_state == ErrorCode::SUCCESS) {//������͵������ȷִ��
                if (cd_command) {
                    // ����� cd �������ݵ�ǰ����ִ�� cd_path ����
                    if (current_command.empty()) cd_path("");//����current_path��vector������shell��ȥ����display_path����
                    else cd_path(args[1]);
                }
                if (change_command) cd_path("");//���л����û���ɫ���൱�ڽ���ǰ·�������
            }
            cd_command = false; change_command = false;
            // ִ��Shell�߼������������Ҫ����ʵ�ʴ�������дע��
            shell();
            // �����ǰ������ "exit"��������ѭ��
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