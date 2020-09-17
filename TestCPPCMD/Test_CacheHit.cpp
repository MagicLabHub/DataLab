//TODO:cache hit test should use the Windows Test Mode[reboot&F8]
//
// Tool info: Intel tool:mlc-intel Intel-PCM
#include "CPUInfo/CPUInfo.h"
//#include "types.h"
//
//void loadSMR_Old()
//{
//	struct MSR_Request{
//		int core_id;
//		ULONG64 msr_address;
//		ULONG64 write_value;     /* value to write if write requet
//                                 ignored if read request */
//	};
//#define MSR_DEV_TYPE 50000
//
//#define IO_CTL_MSR_READ     CTL_CODE(MSR_DEV_TYPE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
//#define IO_CTL_MSR_WRITE    CTL_CODE(MSR_DEV_TYPE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
//
//	TCHAR dir[MAX_PATH];   
//	GetModuleFileName(NULL, dir, MAX_PATH);  
//
//	//int pos = str.find("\\", 1);
//	//HANDLE hDriver = CreateFile(L"D:\\Software\\mlc_v3.9\\Windows\\mlcdrv.sys",0,FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_EXISTING, 0, NULL);
//	//hDriver = CreateFile(L"C:\\Windows\\System32\\cmd.exe",0,FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,OPEN_EXISTING, 0, NULL);
//	//HANDLE hDriver = CreateFile(L"D:\\Software\\mlc_v3.9\\Windows\\mlcdrv.sys", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
//	HANDLE hDriver = CreateFile(L"D:\\Software\\TestTool\\pcm-all\\WinRing0.sys", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
//	BOOL status =FALSE;
//	double * value;
//	if (hDriver != INVALID_HANDLE_VALUE)
//	{
//		
//		MSR_Request req;
//		// ULONG64 result;
//		DWORD reslength = 0;
//		req.core_id = 1;
//		req.msr_address = 0x19c;
//		BOOL status = DeviceIoControl(hDriver, IO_CTL_MSR_READ, &req, sizeof(MSR_Request), value, sizeof(double), &reslength, NULL);
//	}
//	if (hDriver != INVALID_HANDLE_VALUE)
//	{
//		MSR_Request req;
//		ULONG64 result;
//		DWORD reslength = 0;
//		req.core_id = 0;
//		req.msr_address = 0;
//		req.write_value = 1;
//		status = DeviceIoControl(hDriver, IO_CTL_MSR_WRITE, &req, sizeof(MSR_Request), &result, sizeof(double), &reslength, NULL);
//	}
//	//ZwSystemDebugControl
//	//if (hDriver != INVALID_HANDLE_VALUE) CloseHandle(hDriver);
//}

double getL3CacheHitRatio(const PerfInfo & before, const PerfInfo & after) // 0.0 - 1.0
{
	UINT64 L3Miss = after.L3Miss - before.L3Miss;
	UINT64 L3UnsharedHit = after.L3UnsharedHit - before.L3UnsharedHit;
	UINT64 L2HitM = after.L2HitM - before.L2HitM;
	UINT64 hits = L3UnsharedHit + L2HitM;
	UINT64 all = L2HitM + L3UnsharedHit + L3Miss;
	if (all) return double(hits) / double(all);

	return 1;
}

double getL2CacheHitRatio(const PerfInfo & before, const PerfInfo & after){
	UINT64 L3Miss = after.L3Miss - before.L3Miss;
	UINT64 L3UnsharedHit = after.L3UnsharedHit - before.L3UnsharedHit;
	UINT64 L2HitM = after.L2HitM - before.L2HitM;
	UINT64 L2Hit = after.L2Hit - before.L2Hit;
	UINT64 hits = L2Hit;
	UINT64 all = L2Hit + L2HitM + L3UnsharedHit + L3Miss;
	return double(hits) / double(all);
}

UINT64 getL2CacheMisses(const PerfInfo & before, const PerfInfo & after){

	UINT64 L3Miss = after.L3Miss - before.L3Miss;
	UINT64 L3UnsharedHit = after.L3UnsharedHit - before.L3UnsharedHit;
	UINT64 L2HitM = after.L2HitM - before.L2HitM;
	return L2HitM + L3UnsharedHit + L3Miss;
}

UINT64 getL2CacheHits(const PerfInfo & before, const PerfInfo & after){
    return after.L2Hit - before.L2Hit;
}

void loadMSR()
{
	CPUInfo* cpuInfo = CPUInfo::getInstance();

	PerfInfo before,after;
	for (int i=0;i<1000;++i){
		std::cout << std::flush;
		before = cpuInfo->getPerfInfo();
		int curTemp = (cpuInfo->readMSR(MSR_IA32_THERM_STATUS)& 0x7f0000) >> 16;
		//std::cout << "CPU Temperature  :" << std::dec <<curTemp<< "℃\n";
		UINT64 cInstRetiredAny = cpuInfo->readMSR(INST_RETIRED_ANY_ADDR);
		UINT64 cCpuClkUnhaltedThread = cpuInfo->readMSR(CPU_CLK_UNHALTED_THREAD_ADDR);
		UINT64 cCpuClkUnhaltedRef = cpuInfo->readMSR(CPU_CLK_UNHALTED_REF_ADDR);
		

		UINT64 cCustomEvents[PERF_MAX_CUSTOM_COUNTERS] = {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
		for (int i = 0; i < 4; ++i)
		{
			cCustomEvents[i] = cpuInfo->readMSR(IA32_PMC0 + i);
		}
		UINT64 cSMICount = cpuInfo->readMSR(MSR_SMI_COUNT);//System Management Interrupts (SMIs)系统管理中断
		

		Sleep(1000);
		after = cpuInfo->getPerfInfo();
		std::cout << "L3HitRatio:	" <<getL3CacheHitRatio(before,after)<<" L2HitRatio:	" <<getL2CacheHitRatio(before,after)<< " CPU Temperature  :" << curTemp<<"\n";
	}

}


int main()
{
	//cache_line_size();
	//getSystemInfo();
	loadMSR();
    return 0;
}