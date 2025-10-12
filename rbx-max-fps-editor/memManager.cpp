// memManager.cpp

#include "memManager.hpp"
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>

extern "C" NTSTATUS NtReadVirtualMemory(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    ULONG NumberOfBytesToRead,
    PULONG NumberOfBytesReaded
);

MemoryManager::MemoryManager(const std::wstring& name) : targetProcessName(name) {}

bool MemoryManager::Attach() {
    pid = FindPID();
    //hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid); // Read ONLY
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid); // Read + Write

    return hProcess != nullptr;
}

DWORD MemoryManager::FindPID() {
    DWORD outPID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = { sizeof(entry) };

    for (BOOL ok = Process32FirstW(hSnap, &entry); ok; ok = Process32NextW(hSnap, &entry)) {
        if (targetProcessName == entry.szExeFile) {
            outPID = entry.th32ProcessID;
            break;
        }
    }

    CloseHandle(hSnap);
    return outPID;
}

uintptr_t MemoryManager::GetModuleBase(const std::wstring& moduleName) {
    return GetPEBModuleBase(moduleName);
}

uintptr_t MemoryManager::GetPEBModuleBase(const std::wstring& moduleName) {
    HMODULE mods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, mods, sizeof(mods), &cbNeeded)) {
        for (size_t i = 0; i < cbNeeded / sizeof(HMODULE); ++i) {
            wchar_t modName[MAX_PATH];
            if (GetModuleBaseNameW(hProcess, mods[i], modName, MAX_PATH)) {
                if (moduleName == modName)
                    return (uintptr_t)mods[i];
            }
        }
    }
    return 0;
}

bool MemoryManager::Read(uintptr_t address, void* buffer, SIZE_T size) {
    static auto NtRead = reinterpret_cast<decltype(&NtReadVirtualMemory)>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtReadVirtualMemory")
        );

    if (!NtRead) {
        std::cout << "[!] Failed to locate NtReadVirtualMemory." << std::endl;
        return false;
    }

    ULONG bytesRead = 0;
    NTSTATUS status = NtRead(hProcess, reinterpret_cast<PVOID>(address), buffer, static_cast<ULONG>(size), &bytesRead);
    return status == 0;
}

bool MemoryManager::Write(uintptr_t address, const void* buffer, SIZE_T size)
{
    static auto NtWrite = reinterpret_cast<decltype(&NtWriteVirtualMemory)>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtWriteVirtualMemory")
        );

    if (!NtWrite) {
        std::cout << "[!] Failed to locate NtWriteVirtualMemory." << std::endl;
        return false;
    }

    ULONG written = 0;
    NTSTATUS status = NtWrite(hProcess, reinterpret_cast<PVOID>(address), const_cast<void*>(buffer), static_cast<ULONG>(size), &written);
    return status == 0;
}