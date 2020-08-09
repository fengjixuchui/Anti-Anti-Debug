#include "Operations.h"



/*
isdebuggerpresent
heap flags
obregistercallbacks
tls callbacks
breakpoint scanning
ntqueryinformationprocess
*/



UNICODE_STRING		deviceName;
UNICODE_STRING		symLinkName;





NTSTATUS	DriverUnload(_In_ struct _DRIVER_OBJECT* DriverObject)
{
	DbgPrint("driver unload\n");

	Globals::callbackManager.resetAllCallbacks();

	IoDeleteSymbolicLink(&symLinkName);
	IoDeleteDevice(DriverObject->DeviceObject);

	return	STATUS_SUCCESS;
}




NTSTATUS DeviceControlHandler(DEVICE_OBJECT* DeviceObject, PIRP	 Irp)
{
	PIO_STACK_LOCATION		currentStackLocation = IoGetCurrentIrpStackLocation(Irp);

	COMMUNICATION_STRUCT*	systemBuffer = (COMMUNICATION_STRUCT*)Irp->AssociatedIrp.SystemBuffer;




	switch (currentStackLocation->Parameters.DeviceIoControl.IoControlCode)
	{
	case	UNREGISTER_THREAD_CALLBACKS:

		Globals::processID = systemBuffer->processID;
		strcpy(Globals::driverName, systemBuffer->driverName);
		unprotectThread();
		Irp->IoStatus.Status = STATUS_SUCCESS;

		break;

	case	UNREGISTER_PROCESS_CALLBACKS:

		Globals::processID = systemBuffer->processID;
		strcpy(Globals::driverName, systemBuffer->driverName);
		unprotectProcess();
		Irp->IoStatus.Status = STATUS_SUCCESS;

		break;

	case	FIX_HEAP_FLAG:

		Globals::processID = systemBuffer->processID;
		fixHeapFlag(systemBuffer->processID);
		Irp->IoStatus.Status = STATUS_SUCCESS;

		break;

	case	CLEAR_PEB_FLAG:
		
		Globals::processID = systemBuffer->processID;
		clearPEBFlag(systemBuffer->processID);
		Irp->IoStatus.Status = STATUS_SUCCESS;

		break;
	
	case	CLEAR_DEBUGPORT:

		Globals::processID = systemBuffer->processID;
		clearDebugPort();

		break;

	case	SUSPEND_THREAD:

		suspendThread(systemBuffer->processID);
		Irp->IoStatus.Status = STATUS_SUCCESS;

		break;

	case	RESUME_THREAD:

		resumeThread(systemBuffer->processID);
		Irp->IoStatus.Status = STATUS_SUCCESS;

		break;

	default:

		DbgPrint("unsupported control code\n");
		Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		break;
	}



	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;

}

NTSTATUS CreateHandler(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	DbgPrint("create request\n");
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS CloseHandler(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}




NTSTATUS	DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	DbgPrint("driver load \n");

	NTSTATUS			status;
	PDEVICE_OBJECT		myDeviceObject;

	CALLBACK_MANAGER	callbackManager;

	Globals::callbackManager = callbackManager;

	Globals::callbackManager.currentIndex = 0;

	RtlInitUnicodeString(&deviceName, L"\\Device\\antiantiDebugDevice");
	RtlInitUnicodeString(&symLinkName, L"\\DosDevices\\AntiantiDebugDevice");



	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &myDeviceObject);


	status = IoCreateSymbolicLink(&symLinkName, &deviceName);


	DriverObject->DriverUnload = (PDRIVER_UNLOAD)DriverUnload;



	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlHandler;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateHandler;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseHandler;

	ClearFlag(myDeviceObject->Flags, DO_DEVICE_INITIALIZING);

	return STATUS_SUCCESS;
}