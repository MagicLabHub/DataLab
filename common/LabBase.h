#pragma once
#ifndef LAB_BASE
#define LAB_BASE 
#include <stdio.h> 
#include <fstream>
#include <windows.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <math.h>
#include <string>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

class LabBase
{
public:
	LabBase(void);
	~LabBase(void);
};


class LogAlgoPerf
{
public:
	LogAlgoPerf(void)
	{
		QueryPerformanceFrequency(&m_nFreq);
		reset();
	}
	~LogAlgoPerf(void)
	{
		log2File();
	}
public:
	void setLogStart() {
		QueryPerformanceCounter(&m_nBegTime);
		m_vBegTime.push_back(m_nBegTime);
	}
	void getLogEnd() {
		QueryPerformanceCounter(&m_nEndTime);
		m_vEndTime.push_back(m_nEndTime);
		getMemoryInfo();
	}

public:
	void reset(){
		m_vBegTime.clear();
		m_vEndTime.clear();
		m_vMemUsed.clear();
		m_vBegTime.reserve(100000);
		m_vBegTime.reserve(100000);
		m_vMemUsed.reserve(100000);
	}

	void getMemoryInfo(){
		HANDLE handle=GetCurrentProcess();
		PROCESS_MEMORY_COUNTERS pmc;
		GetProcessMemoryInfo(handle,&pmc,sizeof(pmc));
		m_iMemUsed = pmc.WorkingSetSize>>10;		//KB
		m_vMemUsed.push_back(m_iMemUsed);
	}

	double getTotalTime() {
		int nTimeNum = (int)m_vEndTime.size();
		double dProcTime	= 0.0;
		double dTotalTime	= 0.0;
		for (int i = 0;i<nTimeNum;++i){
			dProcTime = double(m_vEndTime[i].QuadPart - m_vBegTime[i].QuadPart)/(double)m_nFreq.QuadPart;
			dTotalTime += dProcTime;
		}
		return dTotalTime;
	}
private:
	void log2File(){
		int nTimeNum = (int)m_vEndTime.size();
		if (nTimeNum){
			DWORD ThreadID = GetCurrentThreadId();
			std::string sTxtPath = "D:\\LogAlgoPerf\\logfile_"+std::to_string((_Longlong)ThreadID)+"_"+std::to_string((_Longlong)m_nBegTime.QuadPart)+ ".txt";
			std::ofstream outfile00( sTxtPath , std::ios::binary|std::ios::app);
			outfile00<<"m_nFreq.QuadPart: "<<std::to_string((_Longlong)m_nFreq.QuadPart)<<",ProcTimeNum: "<<std::to_string((_Longlong)nTimeNum)<<'\r'<<'\n';
			double dProcTime = 0.0;
			//double dWaitTime = 0.0;
			for (int i = 0;i<nTimeNum;++i){
				dProcTime = double(m_vEndTime[i].QuadPart - m_vBegTime[i].QuadPart)/(double)m_nFreq.QuadPart;
				//if (i>0){
				//	dWaitTime = double(m_vBegTime[i].QuadPart - m_vEndTime[i-1].QuadPart)/(double)m_nFreq.QuadPart;
				//}
				//outfile00<<std::to_string((_Longlong)i)<<"	"<<std::to_string((long double)dProcTime)<<"	"<<std::to_string((long double)dWaitTime)<<'\r'<<'\n';
				outfile00<<std::to_string((_Longlong)i)<<"	"<<std::to_string((long double)dProcTime)<<"	"<<std::to_string((LONGLONG)m_vMemUsed[i])<<'\r'<<'\n';
				//std::cout<<i<<"	"<<std::setprecision(8) <<dProcTime<<"	"<<std::to_string((LONGLONG)m_vMemUsed[i])<<std::endl;
			}
			outfile00.close();
			m_vBegTime.clear();
			m_vEndTime.clear();
		}

	}
private:
	LARGE_INTEGER m_nBegTime;
	LARGE_INTEGER m_nEndTime;
	LARGE_INTEGER m_nFreq;
	size_t			m_iMemUsed;
	std::vector<LARGE_INTEGER>	m_vBegTime;
	std::vector<LARGE_INTEGER>	m_vEndTime;
	std::vector<size_t>			m_vMemUsed;
};
//#define LOGPERF(_ins, _m, ...)
#define LOGPERF(_ins, _m, ...) {_ins.##_m(##__VA_ARGS__);}

#endif LAB_BASE