// memory.h
#pragma once

#include <Windows.h>
#include <string>
#include <iostream>
#include "memmanager.hpp"

// debug logs
inline bool debug = false;

inline MemoryManager reader(L"RobloxPlayerBeta.exe");

// Init the memory system
inline bool initializeMemory()
{
	if (reader.Attach())
	{
		return true;
	}
	return false;
}

inline uintptr_t getModuleBaseAddress(const wchar_t* moduleName)
{
	uintptr_t base = reader.GetModuleBase(moduleName);
	if (!base)
		std::wcout << L"[-] Module not found: " << moduleName << std::endl;
	return base;
}

// Read memory from target process
template<typename T>
T read(uintptr_t address)
{
	T buffer{};
	if (!reader.Read(address, &buffer, sizeof(T)) && debug)
		std::cout << "[!] Failed to read memory at 0x" << std::hex << address << std::endl;
	return buffer;
}

template<typename T>
void write(uintptr_t address, const T& value)
{
	if (!reader.Write(address, &value, sizeof(T)) && debug)
		std::cout << "[!] Failed to write memory at 0x" << std::hex << address << std::endl;

}
