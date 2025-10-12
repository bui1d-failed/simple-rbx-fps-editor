#include <iostream>
#include <Windows.h>
#include "memory.h"

using namespace std;

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#include <regex>
#include <unordered_map>


// offsets

namespace offsets { // update these offsets by yourself, if you don't want a automatic-update
    uintptr_t TaskSchedulerPointer = 0x7434168;
    uintptr_t TaskSchedulerMaxFPS = 0x1B0;
}

std::string getOffsetHpp() {
    const char* url = "https://github.com/NtReadVirtualMemory/Roblox-Offsets-Website/raw/refs/heads/main/offsets.hpp"; // not my offsets , im lazy
    HINTERNET hInternet = InternetOpenA("OffsetFetcher", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    HINTERNET hFile = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);

    std::string content;
    char buffer[4096];
    DWORD bytesRead;

    while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
        content.append(buffer, bytesRead);
    }

    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return content;
}

bool fetchOffsets() {
    std::string content = getOffsetHpp();
    std::regex pattern(R"(inline constexpr uintptr_t (\w+) = (0x[0-9A-Fa-f]+);)");
    std::smatch match;
    std::unordered_map<std::string, uintptr_t> offsetMap;

    auto begin = content.cbegin();
    auto end = content.cend();

    while (std::regex_search(begin, end, match, pattern)) {
        std::string name = match[1];
        std::string hex = match[2];
        offsetMap[name] = std::stoul(hex, nullptr, 16);
        begin = match.suffix().first;
    }

    offsets::TaskSchedulerPointer = offsetMap["TaskSchedulerPointer"];
    offsets::TaskSchedulerMaxFPS = offsetMap["TaskSchedulerMaxFPS"];
    return true;
}


// Get Base Address & Init Memory
uintptr_t baseModule = 0;

bool initGame(bool verbose) {
    initializeMemory();
    baseModule = getModuleBaseAddress(L"RobloxPlayerBeta.exe");
    if (!baseModule) return false;
    return true;
}


int main() {
    SetConsoleTitleA("roblox-max-fps-editor");
    if (!initGame(true)) return EXIT_FAILURE;

    if (!fetchOffsets()) {
        MessageBoxA(NULL, "Failed to fetch offsets!", "Error", MB_ICONERROR);
        return EXIT_FAILURE;
    }

    cout << "(set max fps, or press Enter to uncap fps)" << endl;

    while (true) {
        cout << endl << "[o(*£þ¨Œ£þ*)¥Ö] Set Max FPS: ";
        string input;
        getline(cin, input);

        double targetFPS = 9e9; // default is 9000000000, to uncap max fps
        try {
            targetFPS = input.empty() ? 9e9 : stod(input); // empty input detection
        }
        catch (...) {
            cout << "[!!!] ERROR: new FPS must be a number / invalid FPS Value" << endl; continue; // validate FPS value check
        }

        double frameTime = 1.0 / targetFPS;
        uintptr_t TaskScheduler = read<uintptr_t>(baseModule + offsets::TaskSchedulerPointer);
        write<double>(TaskScheduler + offsets::TaskSchedulerMaxFPS, frameTime);
        cout << "[+] FPS set to: " << targetFPS << endl;
    }
    return EXIT_SUCCESS;
}