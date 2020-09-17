#pragma once
#ifndef __CPU_INFO__
#define __CPU_INFO__
#include "../types.h"
#include <iostream>
#include <array>
#include <intrin.h>
#include <vector>
#include <cstring>
#include <tchar.h>
#include <windows.h>
#include <winternl.h>
#include <Winsvc.h>   
#include <WinIoCtl.h>  
#include <psapi.h>
#pragma comment(lib, "psapi.lib") 

typedef NTSTATUS (WINAPI *PFUN_NtQuerySystemInformation)(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
	);

enum SupportedCPUModels
{
	NEHALEM_EP = 26,
	NEHALEM = 30,
	ATOM_1 = 28,
	ATOM_2 = 53,
	CENTERTON = 54,
	BAYTRAIL = 55,
	AVOTON = 77,
	CHERRYTRAIL = 76,
	APOLLO_LAKE = 92,
	DENVERTON = 95,
	CLARKDALE = 37,
	WESTMERE_EP = 44,
	NEHALEM_EX = 46,
	WESTMERE_EX = 47,
	SANDY_BRIDGE = 42,
	JAKETOWN = 45,
	IVY_BRIDGE = 58,
	HASWELL = 60,
	HASWELL_ULT = 69,
	HASWELL_2 = 70,
	IVYTOWN = 62,
	HASWELLX = 63,
	BROADWELL = 61,
	BROADWELL_XEON_E3 = 71,
	BDX_DE = 86,
	SKL_UY = 78,
	KBL = 158,
	KBL_1 = 142,
	BDX = 79,
	KNL = 87,
	SKL = 94,
	SKX = 85,
	END_OF_MODEL_LIST = 0x0ffff
};


struct PerfInfo
{
	UINT64 InstRetiredAny;
	UINT64 CpuClkUnhaltedThread;
	UINT64 CpuClkUnhaltedRef;
	union {
		UINT64 L3Miss;
		UINT64 Event0;
		UINT64 ArchLLCRef;
	};
	union {
		UINT64 L3UnsharedHit;
		UINT64 Event1;
		UINT64 ArchLLCMiss;
	};
	union {
		UINT64 L2HitM;
		UINT64 Event2;
	};
	union {
		UINT64 L2Hit;
		UINT64 Event3;
	};
	UINT64 InvariantTSC; // invariant time stamp counter

	PerfInfo()
		: InstRetiredAny(0)
		, CpuClkUnhaltedThread(0)
		, CpuClkUnhaltedRef(0)
		, L3Miss(0)
		, L3UnsharedHit(0)
		, L2HitM(0)
		, L2Hit(0)
		, InvariantTSC(0){
	}
};

class CPUInfo
{
public:
	
	~CPUInfo(void);
	UINT64 readMSR(UINT64 msrNumber);
	DWORD readMSR(UINT64 msrNumber,UINT64 &Value);	
	DWORD writeMSR(UINT64 msrNumber,UINT64 Value);	
	void getCPUInfo();
	PerfInfo readAndAggregate();
	PerfInfo getPerfInfo();
	static CPUInfo * getInstance();        // the only way to get access
private:
	CPUInfo(void);
	UINT32 bitCount(UINT64 n);
	UINT32 build_bit_ui(UINT32 beg, UINT32 end);
	UINT32 extract_bits_ui(UINT32 myin, UINT32 beg, UINT32 end);
	UINT64 build_bit(UINT32 beg, UINT32 end);
	UINT64 extract_bits(UINT64 myin, UINT32 beg, UINT32 end);
	void getMemInfo();
	int getSystemInfo();
	size_t cache_line_size();
	void program();
private:
	typedef BOOL (WINAPI *_InitializeOls) ();
	typedef VOID (WINAPI *_DeinitializeOls) ();
	typedef BOOL (WINAPI *_IsCpuid) (); 
	typedef BOOL (WINAPI *_IsMsr) (); 
	typedef BOOL (WINAPI *_IsTsc) ();
	typedef DWORD (WINAPI *_Rdmsr) (DWORD index, PDWORD eax, PDWORD edx);
	typedef DWORD (WINAPI *_Wrmsr) (DWORD index, DWORD eax, DWORD edx);
	typedef DWORD (WINAPI *_GetDriverVersion) (PBYTE major, PBYTE minor, PBYTE revision, PBYTE release);

	_InitializeOls InitializeOls;
	_DeinitializeOls DeinitializeOls;
	_GetDriverVersion GetDriverVersion;
	_Rdmsr Rdmsr;
	_Wrmsr Wrmsr;
	_IsCpuid IsCpuid; 
	_IsMsr IsMsr; 
	_IsTsc IsTsc;
private:
	UINT32 cpu_family;
	UINT32 cpu_model;
	UINT32 cpu_stepping;
	UINT32 num_cores;
	UINT32 perfmon_version;
	UINT32 core_gen_counter_num_max;
	UINT32 core_gen_counter_width;
	UINT32 core_fixed_counter_num_max;
	UINT32 core_fixed_counter_width;
	PerfInfo m_PerfInfo;
	static CPUInfo * instance;
};

#endif __CPU_INFO__

