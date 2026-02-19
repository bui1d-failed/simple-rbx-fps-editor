// memmanager.hpp

#pragma once
#include <Windows.h>
#include <string>

extern "C" NTSTATUS NtReadVirtualMemory(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    ULONG NumberOfBytesToRead,
    PULONG NumberOfBytesReaded
);

extern "C" NTSTATUS NtWriteVirtualMemory(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    ULONG NumberOfBytesToWrite,
    PULONG NumberOfBytesWritten
);

class MemoryManager {
public:
    explicit MemoryManager(const std::wstring& processName);
    bool Attach();
    uintptr_t GetModuleBase(const std::wstring& moduleName);
    bool Read(uintptr_t address, void* buffer, SIZE_T size);
    bool Write(uintptr_t address, const void* buffer, SIZE_T size);

    HANDLE GetHandle() const {
        return hProcess;
    }

private:
    DWORD FindPID();
    uintptr_t GetPEBModuleBase(const std::wstring& moduleName);

    std::wstring targetProcessName;
    HANDLE hProcess = nullptr;
    DWORD pid = 0;
};
