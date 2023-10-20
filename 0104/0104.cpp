#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

void ListProcesses() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Помилка при створенні знімку процесів" << std::endl;
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << "Помилка при отриманні інформації про процес" << std::endl;
        CloseHandle(hProcessSnap);
        return;
    }

    std::cout << "Список процесів:" << std::endl;
    do {
        std::cout << "PID: " << pe32.th32ProcessID << " Назва: " << pe32.szExeFile << std::endl;
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

void DisplayProcessInfo(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess == NULL) {
        std::cerr << "Помилка при відкритті процесу" << std::endl;
        return;
    }

    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime) == 0) {
        std::cerr << "Помилка при отриманні інформації про час процесу" << std::endl;
        CloseHandle(hProcess);
        return;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD threadCount = 0;
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);
        if (Thread32First(hThreadSnap, &te32)) {
            do {
                if (te32.th32OwnerProcessID == processID) {
                    threadCount++;
                }
            } while (Thread32Next(hThreadSnap, &te32));
        }
        CloseHandle(hThreadSnap);
    }

    std::cout << "Ідентифікатор процесу: " << processID << std::endl;
    std::cout << "Час старту: " << createTime.dwLowDateTime << std::endl;
    std::cout << "Загальна кількість процесорного часу: " << userTime.dwLowDateTime << std::endl;
    std::cout << "Кількість потоків: " << threadCount << std::endl;

    CloseHandle(hProcess);
}

bool TerminateSelectedProcess(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (hProcess == NULL) {
        std::cerr << "Помилка при відкритті процесу для завершення" << std::endl;
        return false;
    }

    if (TerminateProcess(hProcess, 0)) {
        std::cout << "Процес з ідентифікатором " << processID << " був завершений." << std::endl;
        CloseHandle(hProcess);
        return true;
    }
    else {
        std::cerr << "Помилка при завершенні процесу" << std::endl;
        CloseHandle(hProcess);
        return false;
    }
}

int main() {
    SetConsoleOutputCP(1251);

    int interval = 5000;

    while (true) {
        ListProcesses();

        DWORD processID;
        std::cout << "Введіть ідентифікатор процесу для виведення детальної інформації (0 для виходу, -1 для завершення процесу): ";
        std::cin >> processID;

        if (processID == 0) {
            break;
        }
        else if (processID == -1) {
            std::cout << "Введіть ідентифікатор процесу, який потрібно завершити: ";
            std::cin >> processID;
            if (TerminateSelectedProcess(processID)) {
                Sleep(interval);
            }
        }
        else {
            DisplayProcessInfo(processID);
            Sleep(interval);
        }
    }

    return 0;
}
