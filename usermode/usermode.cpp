#include "driverControl.h"







int main()
{


	Globals::driverHandle = Globals::driver->initialize("\\\\.\\AntiantiDebugDevice");

    Globals::processID = GetProcessId(L"targetApp.exe");
    
    std::cout << "driver handle is: " << std::hex << Globals::driverHandle << std::endl;

    Globals::driver->DisarmCallbacks("Antidebug.sys");

    suspendThreads(Globals::processID);



    std::cout << "suspended all threads  and unregistered callbacks for targetApp. now would be a good time to attach the debugger \n";
    std::cout << "enter 3 to continue and clear debugger traces\n";

    int a;
    while (1)
    {
        std::cin >> a;
        if (a == 3)
        {
            break;
        }
    }



    
    Globals::driver->clearPEBflag();

    Globals::driver->clearHeapFlag();

    Globals::driver->clearDebugPort();

    ResumeThreads(Globals::processID);


    CloseHandle(Globals::driverHandle);


    while (1)
    {
        std::cin.get();
    }
}

