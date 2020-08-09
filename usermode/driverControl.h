#pragma once
#include "Utils.h"


#define UNREGISTER_THREAD_CALLBACKS		CTL_CODE(FILE_DEVICE_UNKNOWN, 0X801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define UNREGISTER_PROCESS_CALLBACKS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0X802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CLEAR_PEB_FLAG					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X803, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FIX_HEAP_FLAG					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X804, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define SUSPEND_THREAD					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X805, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CLEAR_DEBUGPORT					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X806, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define RESUME_THREAD					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X807, METHOD_BUFFERED, FILE_ANY_ACCESS)



class driverControl;

namespace   Globals
{
    DWORD   processID;
    HANDLE	driverHandle;
    int		driverNameLength;
    char	driverName[30];
    driverControl*   driver;
}




struct COMMUNICATION_STRUCT
{
    DWORD	processID;
    char    driverName[30];
};


class driverControl
{

public:
    HANDLE  initialize(LPCSTR   registryPath)
    {
        HANDLE      newHandle;
        newHandle = CreateFileA(registryPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

        std::cout << "last error is: " << std::hex << GetLastError() << std::endl;
        std::cout << "driver handle is: " << newHandle << std::endl;

        return      newHandle;
    }




    void    clearPEBflag()
    {
        COMMUNICATION_STRUCT    input;

        DWORD   bytes;

        input.processID = Globals::processID;

        DeviceIoControl(Globals::driverHandle, CLEAR_PEB_FLAG, &input, sizeof(input), NULL, 0, &bytes, NULL);

        return;
    }




    void    clearHeapFlag()
    {
        COMMUNICATION_STRUCT    input;

        DWORD   bytes;

        input.processID = Globals::processID;

        DeviceIoControl(Globals::driverHandle, FIX_HEAP_FLAG, &input, sizeof(input), NULL, 0, &bytes, NULL);

        return;
    }



    void    DisarmCallbacks(const char* driverName)
    {
        COMMUNICATION_STRUCT    input;

        DWORD   bytes;

        strcpy(input.driverName, driverName);


        DeviceIoControl(Globals::driverHandle, UNREGISTER_PROCESS_CALLBACKS, &input, sizeof(input), NULL, 0, &bytes, NULL);

        DeviceIoControl(Globals::driverHandle, UNREGISTER_THREAD_CALLBACKS, &input, sizeof(input), NULL, 0, &bytes, NULL);

        return;
    }


    void    suspendThreadKernel(DWORD   threadID)
    {
        DWORD   bytes;

        COMMUNICATION_STRUCT    request;

        request.processID = threadID;

        DeviceIoControl(Globals::driverHandle, SUSPEND_THREAD, &request, sizeof(request), NULL, 0, &bytes, NULL);

        std::cout << "thread " << threadID << " suspended" << std::endl;
    }

    void    clearDebugPort()
    {
        DWORD   bytes;

        COMMUNICATION_STRUCT    request;

        request.processID = Globals::processID;

        DeviceIoControl(Globals::driverHandle, CLEAR_DEBUGPORT, &request, sizeof(request), NULL, 0, &bytes, NULL);
    }


    void    ResumeThreadKernel(DWORD    threadID)
    {
        DWORD   bytes;

        COMMUNICATION_STRUCT    request;

        request.processID = threadID;

        DeviceIoControl(Globals::driverHandle, RESUME_THREAD, &request, sizeof(request), NULL, 0, &bytes, NULL);

        std::cout << "thread " << threadID << " resumed" << std::endl;
    }
};













/*  suspend threads    */

BOOL    suspendThreads(DWORD    currentProcessID)
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return FALSE;


    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32))
    {
        CloseHandle(hThreadSnap);
        return(FALSE);
    }


    do
    {
        if (te32.th32OwnerProcessID == currentProcessID)
        {
            Globals::driver->suspendThreadKernel(te32.th32ThreadID);

        }
    } while (Thread32Next(hThreadSnap, &te32));


    CloseHandle(hThreadSnap);

    return TRUE;
}



/*  resume threads    */

BOOL    ResumeThreads(DWORD    currentProcessID)
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return FALSE;


    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32))
    {
        CloseHandle(hThreadSnap);
        return(FALSE);
    }


    do
    {
        if (te32.th32OwnerProcessID == currentProcessID)
        {
            Globals::driver->ResumeThreadKernel(te32.th32ThreadID);

        }
    } while (Thread32Next(hThreadSnap, &te32));


    CloseHandle(hThreadSnap);

    return TRUE;
}
