#pragma once
#include "undocumented.h"


struct CALLBACK_MANAGER
{
private:
	PVOID	oldPostOperation[12];
	PVOID	oldPreOperation[12];
	PCALLBACK_ENTRY_ITEM	callbacks[12];
public:

	int currentIndex = 0;

	void	push(PCALLBACK_ENTRY_ITEM	entryItem)
	{
		oldPostOperation[currentIndex] = entryItem->PostOperation;	
		oldPreOperation[currentIndex] = entryItem->PreOperation;
		callbacks[currentIndex] = entryItem;

		currentIndex += 1;
	}
	void	resetAllCallbacks()
	{
		for(currentIndex; currentIndex >= 0; --currentIndex)
		{
			callbacks[currentIndex]->PostOperation = (POB_POST_OPERATION_CALLBACK)oldPostOperation[currentIndex];
			callbacks[currentIndex]->PreOperation = (POB_PRE_OPERATION_CALLBACK)oldPreOperation[currentIndex];
		}
	}
};
