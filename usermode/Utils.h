#pragma once


#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <sstream>
#include <string>




using namespace std;


wstring widen(const string& str)
{
	wostringstream wstm;
	const ctype<wchar_t>& ctfacet = use_facet<ctype<wchar_t>>(wstm.getloc());
	for (size_t i = 0; i < str.size(); ++i)
		wstm << ctfacet.widen(str[i]);
	return wstm.str();
}


DWORD	GetProcessId(const wchar_t* p_ProcessName)
{
	DWORD l_ProcessId = 0;

	HANDLE l_Snap = (CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (l_Snap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 l_ProcEntry;
		l_ProcEntry.dwSize = sizeof(l_ProcEntry);
		if (Process32First(l_Snap, &l_ProcEntry))
		{
			do
			{
				if (!_wcsicmp(widen(l_ProcEntry.szExeFile).c_str(), p_ProcessName))
				{
					l_ProcessId = l_ProcEntry.th32ProcessID;
					break;
				}

			} while (Process32Next(l_Snap, &l_ProcEntry));
		}
	}
	CloseHandle(l_Snap);

	return l_ProcessId;
}


