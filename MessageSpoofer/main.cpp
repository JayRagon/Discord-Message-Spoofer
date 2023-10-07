#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <sstream>
#include <string>

#define ADDR_MAX 20

int largestram = 0;
int largestPID = 0;

HANDLE dHandle = NULL;


void ScanPatternInProcessMemory(HANDLE hProcess, const char* pattern, size_t patternSize, uintptr_t* addrarr)
{
    MEMORY_BASIC_INFORMATION memInfo;
    uintptr_t scanAddress = 0;
    int foundcounter = 0;


    while (VirtualQueryEx(hProcess, (LPVOID)scanAddress, &memInfo, sizeof(memInfo)) != 0)
    {
        if (memInfo.Protect != PAGE_NOACCESS && memInfo.State == MEM_COMMIT)
        {
            SIZE_T dummy;

            char* buffer = new char[memInfo.RegionSize + 1];

            if (ReadProcessMemory(dHandle, memInfo.BaseAddress, buffer, memInfo.RegionSize - 1, &dummy))
            {
                buffer[memInfo.RegionSize] = '\0';
                for (size_t i = 0; i < memInfo.RegionSize; i++)
                {
                    for (size_t u = 0; u < patternSize; u++)
                    {
                        if (buffer[u + i] != pattern[u])
                        {
                            break;
                        }

                        if (u == patternSize - 1)
                        {
                            //std::cout << "address: 0x" << std::hex << (uintptr_t)memInfo.BaseAddress + i << std::dec << '\n';

                            addrarr[foundcounter] = (uintptr_t)memInfo.BaseAddress + i;
                            foundcounter++;
                        }
                    }
                }

                //std::cout << "region size: 0x" << std::hex << (uintptr_t)memInfo.RegionSize << std::dec<< '\n';
            }

            buffer[memInfo.RegionSize] = '\0';

            delete[] buffer;
        }
        scanAddress += memInfo.RegionSize;
    }

    return;
}


int main()
{
    // grab the correct discord handle (the one that uses the most RAM)
    unsigned long long ramarr[6] = {}; // only 6 discord.exe s

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        int processcounter = 0;
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_stricmp(entry.szExeFile, "Discord.exe") == 0)
            {
                HANDLE currenthProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

                PROCESS_MEMORY_COUNTERS buffer;
                GetProcessMemoryInfo(currenthProcess, &buffer, sizeof(PROCESS_MEMORY_COUNTERS));

                //std::cout << "PID: "<< entry.th32ProcessID << "         ram usage = ~" << (int)(buffer.WorkingSetSize / 1000) << "K\n";


                if (buffer.WorkingSetSize > largestram)
                {
                    largestram = buffer.WorkingSetSize;
                    largestPID = entry.th32ProcessID;
                }

                CloseHandle(currenthProcess);
                processcounter++;
                if (processcounter == 6)
                {
                    break;
                }
            }
        }
    }

    std::cout << "largest ram pid: " << largestPID << '\n';
    CloseHandle(snapshot);

    dHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, largestPID);

    // // // // // //

    /*
    std::string imnotsure;
    for (;;)
    {
        std::string inputbuffer = "";
        std::cin >> inputbuffer;
        if (inputbuffer == "|")
        {
            imnotsure[imnotsure.length() - 1] = '\0';
            break;
        }
        else
        {
            imnotsure += inputbuffer;
            imnotsure += " ";
        }
    }

    std::cout << "\n\n" << imnotsure << '\n';
    */







    for (;;)
    {
        std::string messagetochange;
        std::string changeto;
        std::cout << "Enter the message that you want to change\n> ";

        // std::cin workaround 2
        
        for (;;)
        {
            std::string inputbuffer;
            std::cin >> inputbuffer;
            if (inputbuffer == "|")
            {
                messagetochange.pop_back(); // remove last char (it will be a space)
                break;
            }
            else
            {
                messagetochange += inputbuffer + ' ';
            }
        }
        
        std::cout << "And what do you want to change it to?\n> ";
        
        for (;;)
        {
            std::string inputbuffer;
            std::cin >> inputbuffer;
            if (inputbuffer == "|")
            {
                changeto.pop_back(); // remove last char (it will be a space)
                break;
            }
            else
            {
                changeto += inputbuffer + ' ';
            }
        }
        

        // std::cin workaround method 1
        
        /*
        for (size_t i = 0; i < messagetochange.length(); i++)
        {
            if (messagetochange[i] == '|')
            {
                messagetochange[i] = ' ';
            }
        }
        for (size_t i = 0; i < changeto.length(); i++)
        {
            if (changeto[i] == '|')
            {
                changeto[i] = ' ';
            }
        }
        */





        uintptr_t* addrarr = new uintptr_t[ADDR_MAX];

        for (size_t i = 0; i < ADDR_MAX; i++)
        {
            addrarr[i] = 0;
        }

        const char* pattern = messagetochange.c_str();
        size_t patternsize = messagetochange.length();
        ScanPatternInProcessMemory(dHandle, pattern, patternsize, addrarr);

        std::string writestr = changeto;
        int spacesneeded = patternsize - writestr.length();

        for (size_t i = 0; i < spacesneeded; i++)
        {
            writestr += " ";
        }

        for (size_t i = 0; i < ADDR_MAX; i++)
        {
            std::cout << "addr[" << i << "]: 0x" << std::hex << addrarr[i] << std::dec << '\n';
            if (addrarr[i] != 0)
            {
                SIZE_T dummy;
                WriteProcessMemory(dHandle, (LPVOID)addrarr[i], writestr.c_str(), writestr.length(), &dummy);
            }
        }

        delete[] addrarr;
    }
}