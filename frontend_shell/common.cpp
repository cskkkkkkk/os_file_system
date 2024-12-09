#include"common.h"
/**
 * @brief 将输入的命令字符串拆分成单词的向量
 *
 * @param command 输入的命令字符串
 * @return std::vector<std::string> 包含拆分后单词的向量
 */
std::vector<std::string> split_command(const std::string& command) {
    std::vector<std::string> res;       // 存储拆分后的单词向量
    std::string command_token;          // 存储当前正在构建的单词
    for (char c : command) {
        if (c != ' ') {
            command_token.push_back(c);  // 如果字符不为空格，则将其添加到当前单词中
        }
        else {
            if (!command_token.empty()) {
                res.push_back(command_token);  // 如果遇到空格且当前单词非空，则将其添加到向量中并清空当前单词
                command_token.clear();
            }
        }
    }
    if (!command_token.empty()) res.push_back(command_token);  // 处理最后一个单词
    return res;
}

/**
 * @brief 将输入的路径字符串拆分成目录/文件名的向量
 *
 * @param path 输入的路径字符串
 * @return std::vector<std::string> 包含拆分后目录/文件名的向量
 */
std::vector<std::string> split_path(std::string path) {
    std::vector<std::string> res;       // 存储拆分后的目录/文件名向量
    std::string curr_path;              // 存储当前正在构建的目录/文件名
    if (path.back() == '/') {
        path.push_back('.');             // 如果路径以'/'结尾，添加'.'以便处理最后一个目录/文件名
    }
    for (char c : path) {
        if (c != '/') {
            curr_path.push_back(c);      // 如果字符不为'/'，将其添加到当前目录/文件名中
        }
        else {
            if (!curr_path.empty()) {
                res.push_back(curr_path);  // 如果遇到'/'且当前目录/文件名非空，则将其添加到向量中并清空当前目录/文件名
                curr_path.clear();
            }
        }
    }
    if (!curr_path.empty()) res.push_back(curr_path);  // 处理最后一个目录/文件名
    return std::move(res);
}

void enable_virtual_terminal() {
    DWORD mode;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取标准输出句柄
    if (GetConsoleMode(hOut, &mode)) {  // 获取当前控制台模式
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;  // 启用虚拟终端处理
        SetConsoleMode(hOut, mode);  // 设置新的控制台模式
    }
    else {
        std::cerr << "无法获取控制台模式。" << std::endl;
    }
}

// 获取光标位置
COORD get_cursor_position(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return csbi.dwCursorPosition;
}

// 设置光标位置
void set_cursor_position(HANDLE hConsole, short x, short y) {
    COORD position = { x, y };
    SetConsoleCursorPosition(hConsole, position);
}

void clear_line_from_cursor(HANDLE hConsole) {
    // 获取当前光标位置
    COORD cursorPos = get_cursor_position(hConsole);
    DWORD written;

    // 获取控制台窗口的大小
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    DWORD consoleWidth = consoleInfo.dwSize.X;

    // 计算需要清除的字符数
    DWORD charsToClear = consoleWidth - cursorPos.X;

    // 填充空格字符来清除行尾内容
    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', charsToClear, cursorPos, &written);

    // 填充空格后，需要将这些字符的属性设置为当前属性
    FillConsoleOutputAttribute(hConsole, consoleInfo.wAttributes, charsToClear, cursorPos, &written);
}