#include "ThreadGroupTempAffinity.h"

using namespace WINCPUInfo;

ThreadGroupTempAffinity::ThreadGroupTempAffinity(UINT32 core_id, bool checkStatus)
{
	GROUP_AFFINITY NewGroupAffinity;
	memset(&NewGroupAffinity, 0, sizeof(GROUP_AFFINITY));
	memset(&PreviousGroupAffinity, 0, sizeof(GROUP_AFFINITY));
	DWORD currentGroupSize = 0;

	while ((DWORD)core_id >= (currentGroupSize = GetActiveProcessorCount(NewGroupAffinity.Group)))
	{
		core_id -= (UINT32)currentGroupSize;
		++NewGroupAffinity.Group;
	}
	NewGroupAffinity.Mask = 1ULL << core_id;
	const auto res = SetThreadGroupAffinity(GetCurrentThread(), &NewGroupAffinity, &PreviousGroupAffinity);
	if (res == FALSE && checkStatus)
	{
		std::cerr << "ERROR: SetThreadGroupAffinity for core " << core_id << " failed with error " << GetLastError() << "\n";
		throw std::exception();
	}
}

ThreadGroupTempAffinity::~ThreadGroupTempAffinity()
{
	SetThreadGroupAffinity(GetCurrentThread(), &PreviousGroupAffinity, NULL);
}
