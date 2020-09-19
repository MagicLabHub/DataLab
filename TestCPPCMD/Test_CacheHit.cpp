//TODO:cache hit test should use the Windows Test Mode[reboot&F8]
//
// Tool info: Intel tool:mlc-intel Intel-PCM
#include "CPUInfo/CPUInfo.h"
using namespace WINCPUInfo;
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

double getIPC(const PerfInfo & before, const PerfInfo & after) // instructions per cycle
{
	UINT64 clocks = after.CpuClkUnhaltedThread - before.CpuClkUnhaltedThread;
	if (clocks != 0)
		return double(after.InstRetiredAny - before.InstRetiredAny) / double(clocks);
	return -1;
}

void loadMSR()
{
	CPUInfo* cpuInfo = CPUInfo::getInstance();
	char strTime[80];
	SYSTEMTIME Time = { 0 };
	char sFileName[80];
	::GetLocalTime(&Time);
	PerfInfo before,after;
	std::vector<PerfInfo>vBeforeInfo,vAfterInfo;
	vBeforeInfo	= cpuInfo->getPerfInfo();
	
	for (int i=0;i<1000;++i){

		vAfterInfo	= cpuInfo->getPerfInfo();
		std::cout << std::flush;
		::GetLocalTime(&Time);
		sprintf(strTime,"%.4u-%.2u-%.2u %.2u:%.2u:%.2u.%.3u",Time.wYear, Time.wMonth, Time.wDay,Time.wHour, Time.wMinute, Time.wSecond,Time.wMilliseconds);
		printf("%s \n",strTime);
		for (int iCore=0;iCore<vAfterInfo.size();++iCore){
			before	= vBeforeInfo[iCore];
			after	= vAfterInfo[iCore];

			double dL3HitRate	= getL3CacheHitRatio(before,after)*100;
			double dL2HitRate	= getL2CacheHitRatio(before,after)*100;
			double dIPC			= getIPC(before,after);
			UINT32 Temperature	= after.curTemp;
			printf(" %d L3Hit(%.2f%%) L2Hit(%.2f%%) IPC(%.2f) %d¡æ\n",
				iCore,dL3HitRate,dL2HitRate,dIPC,Temperature);
		}
		vBeforeInfo	= vAfterInfo;
		Sleep(1000);
	}

}


int tmain()
{

	//cache_line_size();
	//getSystemInfo();
	loadMSR();
    return 0;
}