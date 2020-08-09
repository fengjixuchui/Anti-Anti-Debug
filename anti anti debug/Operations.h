#pragma once
#include "KernelUtils.h"



#define UNREGISTER_THREAD_CALLBACKS		CTL_CODE(FILE_DEVICE_UNKNOWN, 0X801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define UNREGISTER_PROCESS_CALLBACKS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0X802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CLEAR_PEB_FLAG					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X803, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FIX_HEAP_FLAG					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X804, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define SUSPEND_THREAD					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X805, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CLEAR_DEBUGPORT					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X806, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define RESUME_THREAD					CTL_CODE(FILE_DEVICE_UNKNOWN, 0X807, METHOD_BUFFERED, FILE_ANY_ACCESS)


struct COMMUNICATION_STRUCT
{
	DWORD	processID;
	char    driverName[30];
};





/*	dummy callbacks	*/

void	dummyPostCallback(
	PVOID RegistrationContext,
	POB_POST_OPERATION_INFORMATION OperationInformation
)
{
	UNREFERENCED_PARAMETER(RegistrationContext);
	UNREFERENCED_PARAMETER(OperationInformation);

	return;
}


OB_PREOP_CALLBACK_STATUS	dummyPreCallback(PVOID  registrationContext,
	POB_PRE_OPERATION_INFORMATION OperationInformation)
{
	UNREFERENCED_PARAMETER(registrationContext);
	UNREFERENCED_PARAMETER(OperationInformation);

	return OB_PREOP_SUCCESS;
}


typedef NTSTATUS(NTAPI* RESUME_THREAD_FUNCTION)(__in PETHREAD Thread);

void	resumeThread(DWORD		threadID)
{
	PETHREAD	thread;

	PsLookupThreadByThreadId((HANDLE)threadID, &thread);

	ULONG	size;

	PVOID	ntoskrnlBase = getKernelBase(&size);

	RESUME_THREAD_FUNCTION		PsResumeThread = (RESUME_THREAD_FUNCTION)((DWORD64)ntoskrnlBase + Offsets::PsResumeThreadOffset);

	PsResumeThread(thread);

	DbgPrint("thread %i resumed!! \n", PsGetThreadId(thread));

	return;
}


typedef NTSTATUS(*SUSPEND_THREAD_FUNCTION) (__in PETHREAD 	Thread, __out_opt PULONG PreviousCount);

void	suspendThread(DWORD		threadID)
{
	PETHREAD	thread;
	PsLookupThreadByThreadId((HANDLE)threadID, &thread);

	ULONG	size;

	PVOID	ntoskrnlBase = getKernelBase(&size);


	/*	address of PsSuspendThread	(needs to be changed)	*/

	SUSPEND_THREAD_FUNCTION		PsSuspendThread = (SUSPEND_THREAD_FUNCTION)((DWORD64)ntoskrnlBase + Offsets::psSuspendThreadOffset);


	PsSuspendThread(thread, NULL);

	DbgPrint("thread %i suspended!! \n", PsGetThreadId(thread));

	return;
}




void	fixHeapFlag(DWORD		processID)
{
	PEPROCESS	process;

	process = getProcessByProcessId(processID);

	KAPC_STATE	apcState;

	KeStackAttachProcess(process, &apcState);

	PPEB	peb = PsGetProcessPeb(process);

	_HEAP* processHeap = (_HEAP*)peb->ProcessHeap;

	processHeap->ForceFlags = 0;

	processHeap->Flags |= HEAP_GROWABLE;

	KeUnstackDetachProcess(&apcState);

	ObDereferenceObject(process);

	return;
}



void	clearPEBFlag(DWORD		processID)
{
	PEPROCESS	process;

	process = getProcessByProcessId(processID);

	KAPC_STATE	apcState;

	KeStackAttachProcess(process, &apcState);

	PPEB	peb = PsGetProcessPeb(process);

	peb->BeingDebugged = 0;

	KeUnstackDetachProcess(&apcState);

	ObDereferenceObject(process);
}



void	 unprotectThread()
{
	PLIST_ENTRY		callbackList = (PLIST_ENTRY)((uintptr_t)*PsThreadType + 0xC8);

	PLIST_ENTRY		ListEntry = callbackList->Flink;




	while (ListEntry != NULL && ListEntry != callbackList)
	{
		PCALLBACK_ENTRY_ITEM		callbackEntry = (PCALLBACK_ENTRY_ITEM)ListEntry;


		if (isFromAntiDebugRange(callbackEntry->PostOperation) || isFromAntiDebugRange(callbackEntry->PreOperation))
		{

			DbgPrint("found anti debug thread callback! setting operation to dummy callback\n");

			Globals::callbackManager.push(callbackEntry);

			callbackEntry->PostOperation = dummyPostCallback;
			callbackEntry->PreOperation = dummyPreCallback;
		}
		DbgPrint("searching for callback... \n");

		ListEntry = ListEntry->Flink;
	}
}


void	unprotectProcess()
{

	/*	global OBJECT_TYPE structures are opaque	
		the callback list offset is at 0xC8
	*/
	PLIST_ENTRY		callbackList = (PLIST_ENTRY)((uintptr_t)*PsProcessType + 0xC8);

	PLIST_ENTRY		ListEntry = callbackList->Flink;



	while (ListEntry != NULL && ListEntry != callbackList)
	{
		PCALLBACK_ENTRY_ITEM		callbackEntry = (PCALLBACK_ENTRY_ITEM)ListEntry;


		if (isFromAntiDebugRange(callbackEntry->PostOperation) || isFromAntiDebugRange(callbackEntry->PreOperation))
		{

			Globals::callbackManager.push(callbackEntry);

			DbgPrint("found anti debug  process callback! setting operation to dummy callback\n");

			callbackEntry->PostOperation = dummyPostCallback;
			callbackEntry->PreOperation = dummyPreCallback;
		}
		DbgPrint("searching for callback... \n");

		ListEntry = ListEntry->Flink;
	}
}



void	clearDebugPort()
{
	PEPROCESS	targetProcess;

	targetProcess = getProcessByProcessId(Globals::processID);

	/*PVOID DebugPort;     0x578	*/

	*(PVOID*)(((DWORD64)targetProcess) + Offsets::processDebugPort) = 0;

	DbgPrint("debug Port cleared!!\n");

	return;
}


