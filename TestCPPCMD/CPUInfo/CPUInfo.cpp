#include "CPUInfo.h"

CPUInfo::CPUInfo(void)
	:cpu_family(0)
	,cpu_model(0)
	,cpu_stepping(0)
	,num_cores(0)
	,perfmon_version(0){
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	HMODULE hOpenLibSys = NULL;
	HMODULE *hModule = &hOpenLibSys;
#ifdef _M_X64
	*hModule = LoadLibrary(_T("WinRing0x64.dll"));
#else
	*hModule = LoadLibrary(_T("WinRing0.dll"));
	//*hModule = LoadLibraryEx(_T("WinRing0.dll"),NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#endif
	InitializeOls =			(_InitializeOls)		GetProcAddress (*hModule, "InitializeOls");
	DeinitializeOls =		(_DeinitializeOls)		GetProcAddress (*hModule, "DeinitializeOls");
	IsCpuid =				(_IsCpuid)				GetProcAddress (*hModule, "IsCpuid");
	IsMsr =					(_IsMsr)				GetProcAddress (*hModule, "IsMsr");
	IsTsc =					(_IsTsc)				GetProcAddress (*hModule, "IsTsc");
	GetDriverVersion =		(_GetDriverVersion)		GetProcAddress (*hModule, "GetDriverVersion");
	Rdmsr =					(_Rdmsr)				GetProcAddress (*hModule, "Rdmsr");
	Wrmsr =					(_Wrmsr)				GetProcAddress (*hModule, "Wrmsr");

	InitializeOls();
	BOOL b =  IsCpuid();   
	b =  IsMsr();   
	b =  IsTsc(); 
	BYTE major, minor, revision, release;
	GetDriverVersion(&major, &minor, &revision, &release);
	wchar_t buffer[128];
	swprintf_s(buffer, 128, _T("\\\\.\\WinRing0_%d_%d_%d"),(int)major,(int)minor, (int)revision);
	
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	printf("Number of processors: %d.\n", info.dwNumberOfProcessors);
	DWORD result=SetThreadAffinityMask(GetCurrentThread(),1);

	HANDLE numInstancesSemaphore = CreateSemaphore(NULL, 0, 1 << 20, L"Global\\Number of running Processor Counter Monitor instances");
	LONG prevValue = 0;
	ReleaseSemaphore(numInstancesSemaphore, 1, &prevValue);
	UINT64 value = readMSR(IA32_CR_PERF_GLOBAL_CTRL);
	value = readMSR(IA32_PERFEVTSEL0_ADDR);
	value = readMSR(IA32_CR_FIXED_CTR_CTRL);
	char *envPath;
	_dupenv_s(&envPath, NULL, "_");
	free(envPath);
	DWORD_PTR ProcessAffinityMask = 0; 
	DWORD_PTR SystemAffinityMask = 0; 
	GetProcessAffinityMask(GetCurrentProcess(),&ProcessAffinityMask, &SystemAffinityMask);
	
	getCPUInfo();
	//getSystemInfo();
	program();
}

CPUInfo::~CPUInfo(void){
	DeinitializeOls();
}

UINT32 CPUInfo::bitCount(UINT64 n)
{
	int count = 0;
	while (n)
	{
		count += n & 0x00000001;
		n >>= 1;
	}
	return count;
}

UINT32 CPUInfo::build_bit_ui(UINT32 beg, UINT32 end)
{
	UINT32 myll = 0;
	if (end == 31)
	{
		myll = (UINT32)(-1);
	}
	else
	{
		myll = (1 << (end + 1)) - 1;
	}
	myll = myll >> beg;
	return myll;
}

UINT32 CPUInfo::extract_bits_ui(UINT32 myin, UINT32 beg, UINT32 end)
{
	UINT32 myll = 0;
	UINT32 beg1, end1;

	// Let the user reverse the order of beg & end.
	if (beg <= end)
	{
		beg1 = beg;
		end1 = end;
	}
	else
	{
		beg1 = end;
		end1 = beg;
	}
	myll = myin >> beg1;
	myll = myll & build_bit_ui(beg1, end1);
	return myll;
}

UINT64 CPUInfo::build_bit(UINT32 beg, UINT32 end)
{
	UINT64 myll = 0;
	if (end == 63)
	{
		myll = static_cast<UINT64>(-1);
	}
	else
	{
		myll = (1LL << (end + 1)) - 1;
	}
	myll = myll >> beg;
	return myll;
}

UINT64 CPUInfo::extract_bits(UINT64 myin, UINT32 beg, UINT32 end)
{
	UINT64 myll = 0;
	UINT32 beg1, end1;

	// Let the user reverse the order of beg & end.
	if (beg <= end)
	{
		beg1 = beg;
		end1 = end;
	}
	else
	{
		beg1 = end;
		end1 = beg;
	}
	myll = myin >> beg1;
	myll = myll & build_bit(beg1, end1);
	return myll;
}

UINT64 CPUInfo::readMSR(UINT64 msrNumber){
	SetThreadAffinityMask(GetCurrentThread(),1);
	UINT64 Value = 0;
	DWORD eax=0,edx=0;
	DWORD MSRstatus = Rdmsr((DWORD)msrNumber,  &eax, &edx);
	if (MSRstatus==0){
		return 0;
	}
	Value = edx;
	Value = Value<<32;
	Value = Value | eax;
	return Value;
}

DWORD CPUInfo::writeMSR(UINT64 msrNumber,UINT64 Value){
	SetThreadAffinityMask(GetCurrentThread(),1);
	DWORD eax = (DWORD)(Value&0x00000000ffffffff);
	DWORD edx = (DWORD)(Value&0xffffffff00000000);
	edx = edx>>16;
	DWORD MSRstatus = Wrmsr((DWORD)msrNumber,eax,edx);
	if (MSRstatus==0)
		MSRstatus=0;
	return MSRstatus;
}

DWORD CPUInfo::readMSR(UINT64 msrNumber,UINT64 &Value){
	DWORD eax=0,edx=0;
	DWORD MSRstatus = Rdmsr((DWORD)msrNumber,  &eax, &edx);
	if (MSRstatus==0){
		return MSRstatus;
	}
	Value = edx;
	Value = Value<<32;
	Value = Value | eax;
	return MSRstatus;
}

void CPUInfo::getCPUInfo(){
	int i = 0;
	char buffer[1024];
	const char * UnsupportedMessage = "Error: unsupported processor. Only Intel(R) processors are supported (Atom(R) and microarchitecture codename Nehalem, Westmere and Sandy Bridge).";
	int cpuinfo[4];
	__cpuid(cpuinfo, 0);
	memset(buffer, 0, 1024);
	((int *)buffer)[0] = cpuinfo[1];
	((int *)buffer)[1] = cpuinfo[3];
	((int *)buffer)[2] = cpuinfo[2];
	if (strncmp(buffer, "GenuineIntel", 4 * 3) != 0)
	{
		std::cout << UnsupportedMessage << std::endl;
		return;
	}
	int max_cpuid = cpuinfo[0];
	UINT L3ScalingFactor = cpuinfo[1];
	__cpuid(cpuinfo, 1);
	cpu_family = (((cpuinfo[0]) >> 8) & 0xf) | ((cpuinfo[0] & 0xf00000) >> 16);
	cpu_model = (((cpuinfo[0]) & 0xf0) >> 4) | ((cpuinfo[0] & 0xf0000) >> 12);

	if (max_cpuid >= 0xa)
	{
		// get counter related info
		__cpuid(cpuinfo, 0xa);
		perfmon_version = extract_bits_ui(cpuinfo[0], 0, 7);
		core_gen_counter_num_max = extract_bits_ui(cpuinfo[0], 8, 15);
		core_gen_counter_width = extract_bits_ui(cpuinfo[0], 16, 23);
		if (perfmon_version > 1)
		{
			core_fixed_counter_num_max = extract_bits_ui(cpuinfo[3], 0, 4);
			core_fixed_counter_width = extract_bits_ui(cpuinfo[3], 5, 12);
		}
	}
	if (cpu_family != 6)
	{
		std::cout << UnsupportedMessage << " CPU Family: " << cpu_family << std::endl;
		return;
	}
	if (cpu_model != NEHALEM_EP
		&& cpu_model != NEHALEM_EX
		&& cpu_model != WESTMERE_EP
		&& cpu_model != WESTMERE_EX
		&& cpu_model != ATOM_1
		&& cpu_model != CLARKDALE
		&& cpu_model != SANDY_BRIDGE
		&& cpu_model != JAKETOWN
		&& cpu_model != IVYTOWN
		)
	{
		std::cout << UnsupportedMessage << " CPU Model: " << cpu_model << std::endl;
		return;
	}

	const UINT64 bus_freq = (
		cpu_model == SANDY_BRIDGE
		|| cpu_model == JAKETOWN
		|| cpu_model == IVYTOWN
		|| cpu_model == HASWELLX
		|| cpu_model == BDX_DE
		|| cpu_model == BDX
		|| cpu_model == IVY_BRIDGE
		|| cpu_model == HASWELL
		|| cpu_model == BROADWELL
		|| cpu_model == AVOTON
		|| cpu_model == APOLLO_LAKE
		|| cpu_model == DENVERTON
		|| cpu_model == SKL
		|| cpu_model == KBL
		|| cpu_model == KNL
		|| cpu_model == SKX
		) ? (100000000ULL) : (133333333ULL);

	UINT64 Frequency = readMSR(PLATFORM_INFO_ADDR);
	if(!Frequency){
		UINT64 before = 0, after = 0;
		before = readMSR(IA32_TIME_STAMP_COUNTER);
		Sleep(1000);
		after = readMSR(IA32_TIME_STAMP_COUNTER);
		Frequency = after-before;
	}
	Frequency = ((Frequency >> 8) & 255) * bus_freq;

	int num_sockets = 0;
	int threads_per_core = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION * slpi =  new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[1];
	DWORD len = (DWORD)sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
	BOOL res = GetLogicalProcessorInformation(slpi, &len);
	while (res == FALSE){
		delete[] slpi;
		slpi = new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)];
		res = GetLogicalProcessorInformation(slpi, &len);
	}
	UINT32 totalNum = (UINT32)(len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
	for (UINT32 i = 0; i < totalNum; ++i)
	{
		if (slpi[i].Relationship == RelationProcessorCore)
		{
			num_sockets++;
			//std::cout << "Physical core found, mask: "<<slpi[i].ProcessorMask<< std::endl;
			threads_per_core = bitCount(slpi[i].ProcessorMask);
			num_cores += threads_per_core;
		}
	}
	delete[] slpi;

	std::cout << "Num cores: " << num_cores << std::endl;
	std::cout << "Num sockets: " << num_sockets << std::endl;
	std::cout << "Threads per core: " << threads_per_core << std::endl;
	std::cout << "Core PMU (perfmon) version: " << perfmon_version << std::endl;
	std::cout << "Number of core PMU generic (programmable) counters: " << core_gen_counter_num_max << std::endl;
	std::cout << "Width of generic (programmable) counters: " << core_gen_counter_width << " bits" << std::endl;
	if (perfmon_version > 1)
	{
		std::cout << "Number of core PMU fixed counters: " << core_fixed_counter_num_max << std::endl;
		std::cout << "Width of fixed counters: " << core_fixed_counter_width << " bits" << std::endl;
	}
	std::cout << "Nominal core frequency: " << Frequency << " Hz" << std::endl;
}

size_t CPUInfo::cache_line_size() {
	size_t line_size = 0;
	DWORD buffer_size = 0;
	DWORD i = 0;
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

	GetLogicalProcessorInformation(0, &buffer_size);
	buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)malloc(buffer_size);
	GetLogicalProcessorInformation(&buffer[0], &buffer_size);

	for (i = 0; i != buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
		if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
			line_size = buffer[i].Cache.LineSize;
			break;
		}
	}

	free(buffer);
	return line_size;
}

int CPUInfo::getSystemInfo()
{
	const char *szFeatures[] ={
		"x87 FPU On Chip",
		"Virtual-8086 Mode Enhancement",
		"Debugging Extensions",
		"Page Size Extensions",
		"Time Stamp Counter",
		"RDMSR and WRMSR Support",
		"Physical Address Extensions",
		"Machine Check Exception",
		"CMPXCHG8B Instruction",
		"APIC On Chip",
		"Unknown1",
		"SYSENTER and SYSEXIT",
		"Memory Type Range Registers",
		"PTE Global Bit",
		"Machine Check Architecture",
		"Conditional Move/Compare Instruction",
		"Page Attribute Table",
		"Page Size Extension",
		"Processor Serial Number",
		"CFLUSH Extension",
		"Unknown2",
		"Debug Store",
		"Thermal Monitor and Clock Ctrl",
		"MMX Technology",
		"FXSAVE/FXRSTOR",
		"SSE Extensions",
		"SSE2 Extensions",
		"Self Snoop",
		"Hyper-threading Technology",
		"Thermal Monitor",
		"Unknown4",
		"Pend. Brk. EN."
	};

	char CPUString[0x20];
	char CPUBrandString[0x40];
	int CPUInfo[4] = {-1};
	int nSteppingID = 0;
	int nModel = 0;
	int nFamily = 0;
	int nProcessorType = 0;
	int nExtendedmodel = 0;
	int nExtendedfamily = 0;
	int nBrandIndex = 0;
	int nCLFLUSHcachelinesize = 0;
	int nAPICPhysicalID = 0;
	int nFeatureInfo = 0;
	int nCacheLineSize = 0;
	int nL2Associativity = 0;
	int nCacheSizeK = 0;
	int nRet = 0;
	unsigned    nIds, nExIds, i;
	bool    bSSE3NewInstructions = false;
	bool    bMONITOR_MWAIT = false;
	bool    bCPLQualifiedDebugStore = false;
	bool    bThermalMonitor2 = false;


	// __cpuid with an InfoType argument of 0 returns the number of
	// valid Ids in CPUInfo[0] and the CPU identification string in
	// the other three array elements. The CPU identification string is
	// not in linear order. The code below arranges the information 
	// in a human readable form.
	__cpuid(CPUInfo, 0);
	nIds = CPUInfo[0];
	memset(CPUString, 0, sizeof(CPUString));
	*((int*)CPUString) = CPUInfo[1];
	*((int*)(CPUString+4)) = CPUInfo[3];
	*((int*)(CPUString+8)) = CPUInfo[2];

	// Get the information associated with each valid Id
	for (i=0; i<=nIds; ++i)
	{
		__cpuid(CPUInfo, i);
		printf_s("\nFor InfoType %d\n", i); 
		printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
		printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
		printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
		printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

		// Interpret CPU feature information.
		if  (i == 1)
		{
			nSteppingID = CPUInfo[0] & 0xf;
			nModel = (CPUInfo[0] >> 4) & 0xf;
			nFamily = (CPUInfo[0] >> 8) & 0xf;
			nProcessorType = (CPUInfo[0] >> 12) & 0x3;
			nExtendedmodel = (CPUInfo[0] >> 16) & 0xf;
			nExtendedfamily = (CPUInfo[0] >> 20) & 0xff;
			nBrandIndex = CPUInfo[1] & 0xff;
			nCLFLUSHcachelinesize = ((CPUInfo[1] >> 8) & 0xff) * 8;
			nAPICPhysicalID = (CPUInfo[1] >> 24) & 0xff;
			bSSE3NewInstructions = (CPUInfo[2] & 0x1) || false;
			bMONITOR_MWAIT = (CPUInfo[2] & 0x8) || false;
			bCPLQualifiedDebugStore = (CPUInfo[2] & 0x10) || false;
			bThermalMonitor2 = (CPUInfo[2] & 0x100) || false;
			nFeatureInfo = CPUInfo[3];
		}
	}

	// Calling __cpuid with 0x80000000 as the InfoType argument
	// gets the number of valid extended IDs.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	// Get the information associated with each extended ID.
	for (i=0x80000000; i<=nExIds; ++i)
	{
		__cpuid(CPUInfo, i);
		printf_s("\nFor InfoType %x\n", i); 
		printf_s("CPUInfo[0] = 0x%x\n", CPUInfo[0]);
		printf_s("CPUInfo[1] = 0x%x\n", CPUInfo[1]);
		printf_s("CPUInfo[2] = 0x%x\n", CPUInfo[2]);
		printf_s("CPUInfo[3] = 0x%x\n", CPUInfo[3]);

		// Interpret CPU brand string and cache information.
		if  (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if  (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if  (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		else if  (i == 0x80000006)
		{
			nCacheLineSize = CPUInfo[2] & 0xff;
			nL2Associativity = (CPUInfo[2] >> 12) & 0xf;
			nCacheSizeK = (CPUInfo[2] >> 16) & 0xffff;
		}
	}

	// Display all the information in user-friendly format.

	printf_s("\n\nCPU String: %s\n", CPUString);

	if  (nIds >= 1)
	{
		if  (nSteppingID)
			printf_s("Stepping ID = %d\n", nSteppingID);
		if  (nModel)
			printf_s("Model = %d\n", nModel);
		if  (nFamily)
			printf_s("Family = %d\n", nFamily);
		if  (nProcessorType)
			printf_s("Processor Type = %d\n", nProcessorType);
		if  (nExtendedmodel)
			printf_s("Extended model = %d\n", nExtendedmodel);
		if  (nExtendedfamily)
			printf_s("Extended family = %d\n", nExtendedfamily);
		if  (nBrandIndex)
			printf_s("Brand Index = %d\n", nBrandIndex);
		if  (nCLFLUSHcachelinesize)
			printf_s("CLFLUSH cache line size = %d\n",
			nCLFLUSHcachelinesize);
		if  (nAPICPhysicalID)
			printf_s("APIC Physical ID = %d\n", nAPICPhysicalID);

		if  (nFeatureInfo || bSSE3NewInstructions ||
			bMONITOR_MWAIT || bCPLQualifiedDebugStore ||
			bThermalMonitor2)
		{
			printf_s("\nThe following features are supported:\n");

			if  (bSSE3NewInstructions)
				printf_s("\tSSE3 New Instructions\n");
			if  (bMONITOR_MWAIT)
				printf_s("\tMONITOR/MWAIT\n");
			if  (bCPLQualifiedDebugStore)
				printf_s("\tCPL Qualified Debug Store\n");
			if  (bThermalMonitor2)
				printf_s("\tThermal Monitor 2\n");

			i = 0;
			nIds = 1;
			while (i < (sizeof(szFeatures)/sizeof(char*)))
			{
				if  (nFeatureInfo & nIds)
				{
					printf_s("\t");
					printf_s(szFeatures[i]);
					printf_s("\n");
				}

				nIds <<= 1;
				++i;
			}
		}
	}

	if  (nExIds >= 0x80000004)
		printf_s("\nCPU Brand String: %s\n", CPUBrandString);

	if  (nExIds >= 0x80000006)
	{
		printf_s("Cache Line Size = %d\n", nCacheLineSize);
		printf_s("L2 Associativity = %d\n", nL2Associativity);
		printf_s("Cache Size = %dK\n", nCacheSizeK);
	}
	return  nRet;
}

void CPUInfo::getMemInfo()
{
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;
	// Print the process identifier.
	hProcess = GetCurrentProcess();
	if (NULL == hProcess)
		return;

	if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
	{
		printf( "\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount );
		printf( "\tPeakWorkingSetSize: 0x%08X\n", 
			pmc.PeakWorkingSetSize );
		printf( "\tWorkingSetSize: 0x%08X\n", pmc.WorkingSetSize );
		printf( "\tQuotaPeakPagedPoolUsage: 0x%08X\n", 
			pmc.QuotaPeakPagedPoolUsage );
		printf( "\tQuotaPagedPoolUsage: 0x%08X\n", 
			pmc.QuotaPagedPoolUsage );
		printf( "\tQuotaPeakNonPagedPoolUsage: 0x%08X\n", 
			pmc.QuotaPeakNonPagedPoolUsage );
		printf( "\tQuotaNonPagedPoolUsage: 0x%08X\n", 
			pmc.QuotaNonPagedPoolUsage );
		printf( "\tPagefileUsage: 0x%08X\n", pmc.PagefileUsage ); 
		printf( "\tPeakPagefileUsage: 0x%08X\n", 
			pmc.PeakPagefileUsage );
	}

	CloseHandle( hProcess );

}

PerfInfo CPUInfo::readAndAggregate()
{
	UINT64 cInstRetiredAny			=  readMSR(INST_RETIRED_ANY_ADDR);
	UINT64 cCpuClkUnhaltedThread	=  readMSR(CPU_CLK_UNHALTED_THREAD_ADDR);
	UINT64 cCpuClkUnhaltedRef		=  readMSR(CPU_CLK_UNHALTED_REF_ADDR);
	UINT64 cL3Miss					=  readMSR(IA32_PMC0);
	UINT64 cL3UnsharedHit			=  readMSR(IA32_PMC1);
	UINT64 cL2HitM 					=  readMSR(IA32_PMC2);
	UINT64 cL2Hit					=  readMSR(IA32_PMC3);
	UINT64 cInvariantTSC			=  readMSR(IA32_TIME_STAMP_COUNTER);


	m_PerfInfo.InstRetiredAny		+= cInstRetiredAny;
	m_PerfInfo.CpuClkUnhaltedThread += cCpuClkUnhaltedThread;
	m_PerfInfo.CpuClkUnhaltedRef	+= cCpuClkUnhaltedRef;
	m_PerfInfo.L3Miss				+= cL3Miss;
	m_PerfInfo.L3UnsharedHit		+= cL3UnsharedHit;
	m_PerfInfo.L2HitM				+= cL2HitM;
	m_PerfInfo.L2Hit				+= cL2Hit;
	m_PerfInfo.InvariantTSC			+= cInvariantTSC;

	return m_PerfInfo;
}

void CPUInfo::program(){

	struct CustomCoreEventDescription
	{
		int event_number, umask_value;
	};

	struct EventSelectRegister
	{
		union
		{
			struct
			{
				UINT64 event_select : 8;
				UINT64 umask : 8;
				UINT64 usr : 1;
				UINT64 os : 1;
				UINT64 edge : 1;
				UINT64 pin_control : 1;
				UINT64 apic_int : 1;
				UINT64 any_thread : 1;
				UINT64 enable : 1;
				UINT64 invert : 1;
				UINT64 cmask : 8;
				UINT64 in_tx : 1;
				UINT64 in_txcp : 1;
				UINT64 reservedX : 30;
			} fields;
			UINT64 value;
		};

		EventSelectRegister() : value(0) {}
	};

	struct FixedEventControlRegister
	{
		union
		{
			struct
			{
				// CTR0
				UINT64 os0 : 1;
				UINT64 usr0 : 1;
				UINT64 any_thread0 : 1;
				UINT64 enable_pmi0 : 1;
				// CTR1
				UINT64 os1 : 1;
				UINT64 usr1 : 1;
				UINT64 any_thread1 : 1;
				UINT64 enable_pmi1 : 1;
				// CTR2
				UINT64 os2 : 1;
				UINT64 usr2 : 1;
				UINT64 any_thread2 : 1;
				UINT64 enable_pmi2 : 1;
				
				UINT64 reserved1 : 52;
			} fields;
			UINT64 value;
		};
	};

	CustomCoreEventDescription coreEventDesc[4];
	UINT32 core_gen_counter_num_used;
	UINT32 core_fixed_counter_num_used = 3;

	coreEventDesc[0].event_number = ARCH_LLC_MISS_EVTNR;
	coreEventDesc[0].umask_value = ARCH_LLC_MISS_UMASK;
	coreEventDesc[1].event_number = MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_NONE_EVTNR;
	coreEventDesc[1].umask_value = MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_NONE_UMASK;
	coreEventDesc[2].event_number = MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_EVTNR;
	coreEventDesc[2].umask_value = MEM_LOAD_UOPS_LLC_HIT_RETIRED_XSNP_UMASK;
	coreEventDesc[3].event_number = MEM_LOAD_UOPS_RETIRED_L2_HIT_EVTNR;
	coreEventDesc[3].umask_value = MEM_LOAD_UOPS_RETIRED_L2_HIT_UMASK;

	core_gen_counter_num_used = 4;

	UINT64 val64 = 0;
	val64 = readMSR(0x39C);
	val64 |= 1ULL;
	writeMSR(0x39C, val64);

		// disable counters while programming
		writeMSR(IA32_CR_PERF_GLOBAL_CTRL, 0);
		FixedEventControlRegister ctrl_reg;
		ctrl_reg.value = readMSR(IA32_CR_FIXED_CTR_CTRL);

		ctrl_reg.fields.os0 = 1;
		ctrl_reg.fields.usr0 = 1;
		ctrl_reg.fields.any_thread0 = 0;
		ctrl_reg.fields.enable_pmi0 = 0;

		ctrl_reg.fields.os1 = 1;
		ctrl_reg.fields.usr1 = 1;
		ctrl_reg.fields.any_thread1 = (perfmon_version >= 3) ? 1 : 0;         // sum the nuber of cycles from both logical cores on one physical core
		ctrl_reg.fields.enable_pmi1 = 0;

		ctrl_reg.fields.os2 = 1;
		ctrl_reg.fields.usr2 = 1;
		ctrl_reg.fields.any_thread2 = (perfmon_version >= 3) ? 1 : 0;         // sum the nuber of cycles from both logical cores on one physical core
		ctrl_reg.fields.enable_pmi2 = 0;
		writeMSR(IA32_CR_FIXED_CTR_CTRL, ctrl_reg.value);
	
		EventSelectRegister event_select_reg;

		for (UINT32 j = 0; j < 4; ++j)
		{
			event_select_reg.value = readMSR(IA32_PERFEVTSEL0_ADDR + j);
			event_select_reg.fields.event_select = coreEventDesc[j].event_number;
			event_select_reg.fields.umask = coreEventDesc[j].umask_value;
			event_select_reg.fields.usr = 1;
			event_select_reg.fields.os = 1;
			event_select_reg.fields.edge = 0;
			event_select_reg.fields.pin_control = 0;
			event_select_reg.fields.apic_int = 0;
			event_select_reg.fields.any_thread = 0;
			event_select_reg.fields.enable = 1;
			event_select_reg.fields.invert = 0;
			event_select_reg.fields.cmask = 0;
			writeMSR(IA32_PERFEVTSEL0_ADDR + j, event_select_reg.value);
		}

		// start counting, enable all (4 programmable + 3 fixed) counters
		UINT64 value = (1ULL << 0) + (1ULL << 1) + (1ULL << 2) + (1ULL << 3) + (1ULL << 32) + (1ULL << 33) + (1ULL << 34);
		writeMSR(IA32_CR_PERF_GLOBAL_CTRL, value);
}

CPUInfo * CPUInfo::instance = NULL;

CPUInfo* CPUInfo::getInstance(){
	 if (instance) return instance;
	 return instance = new CPUInfo();
}

PerfInfo CPUInfo::getPerfInfo(){
	CPUInfo * inst = CPUInfo::getInstance();
	PerfInfo result = inst->readAndAggregate();
	return result;
}
