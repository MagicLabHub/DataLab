#pragma once
#include <windows.h>
#include <iostream>
namespace WINCPUInfo{

#ifndef _THREAD_GROUP_TEMP_AFFINITY__H_
#define _THREAD_GROUP_TEMP_AFFINITY__H_

class ThreadGroupTempAffinity
{
	GROUP_AFFINITY PreviousGroupAffinity;

	ThreadGroupTempAffinity();                                              // forbidden
	ThreadGroupTempAffinity(const ThreadGroupTempAffinity &);               // forbidden
	ThreadGroupTempAffinity & operator = (const ThreadGroupTempAffinity &); // forbidden

public:
	ThreadGroupTempAffinity(UINT32 core_id, bool checkStatus = true);
	~ThreadGroupTempAffinity();
};

class TemporalThreadAffinity  
{
	TemporalThreadAffinity(); 
	ThreadGroupTempAffinity affinity;
public:
	TemporalThreadAffinity(UINT32 core, bool checkStatus = true) : affinity(core, checkStatus) {}
	bool supported() const { return true; }
};

#endif _THREAD_GROUP_TEMP_AFFINITY__H_

}