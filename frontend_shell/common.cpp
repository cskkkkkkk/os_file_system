#include"common.h"
/**
 * @brief ������������ַ�����ֳɵ��ʵ�����
 *
 * @param command ����������ַ���
 * @return std::vector<std::string> ������ֺ󵥴ʵ�����
 */
std::vector<std::string> split_command(const std::string& command) {
    std::vector<std::string> res;       // �洢��ֺ�ĵ�������
    std::string command_token;          // �洢��ǰ���ڹ����ĵ���
    for (char c : command) {
        if (c != ' ') {
            command_token.push_back(c);  // ����ַ���Ϊ�ո�������ӵ���ǰ������
        }
        else {
            if (!command_token.empty()) {
                res.push_back(command_token);  // ��������ո��ҵ�ǰ���ʷǿգ�������ӵ������в���յ�ǰ����
                command_token.clear();
            }
        }
    }
    if (!command_token.empty()) res.push_back(command_token);  // �������һ������
    return res;
}

/**
 * @brief �������·���ַ�����ֳ�Ŀ¼/�ļ���������
 *
 * @param path �����·���ַ���
 * @return std::vector<std::string> ������ֺ�Ŀ¼/�ļ���������
 */
std::vector<std::string> split_path(std::string path) {
    std::vector<std::string> res;       // �洢��ֺ��Ŀ¼/�ļ�������
    std::string curr_path;              // �洢��ǰ���ڹ�����Ŀ¼/�ļ���
    if (path.back() == '/') {
        path.push_back('.');             // ���·����'/'��β�����'.'�Ա㴦�����һ��Ŀ¼/�ļ���
    }
    for (char c : path) {
        if (c != '/') {
            curr_path.push_back(c);      // ����ַ���Ϊ'/'��������ӵ���ǰĿ¼/�ļ�����
        }
        else {
            if (!curr_path.empty()) {
                res.push_back(curr_path);  // �������'/'�ҵ�ǰĿ¼/�ļ����ǿգ�������ӵ������в���յ�ǰĿ¼/�ļ���
                curr_path.clear();
            }
        }
    }
    if (!curr_path.empty()) res.push_back(curr_path);  // �������һ��Ŀ¼/�ļ���
    return std::move(res);
}

void enable_virtual_terminal() {
    DWORD mode;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);  // ��ȡ��׼������
    if (GetConsoleMode(hOut, &mode)) {  // ��ȡ��ǰ����̨ģʽ
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;  // ���������ն˴���
        SetConsoleMode(hOut, mode);  // �����µĿ���̨ģʽ
    }
    else {
        std::cerr << "�޷���ȡ����̨ģʽ��" << std::endl;
    }
}

// ��ȡ���λ��
COORD get_cursor_position(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return csbi.dwCursorPosition;
}

// ���ù��λ��
void set_cursor_position(HANDLE hConsole, short x, short y) {
    COORD position = { x, y };
    SetConsoleCursorPosition(hConsole, position);
}

void clear_line_from_cursor(HANDLE hConsole) {
    // ��ȡ��ǰ���λ��
    COORD cursorPos = get_cursor_position(hConsole);
    DWORD written;

    // ��ȡ����̨���ڵĴ�С
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    DWORD consoleWidth = consoleInfo.dwSize.X;

    // ������Ҫ������ַ���
    DWORD charsToClear = consoleWidth - cursorPos.X;

    // ���ո��ַ��������β����
    FillConsoleOutputCharacter(hConsole, (TCHAR)' ', charsToClear, cursorPos, &written);

    // ���ո����Ҫ����Щ�ַ�����������Ϊ��ǰ����
    FillConsoleOutputAttribute(hConsole, consoleInfo.wAttributes, charsToClear, cursorPos, &written);
}