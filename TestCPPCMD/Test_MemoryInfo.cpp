// WorkingSetSize������������(�ڴ�)��������ռ�õ������ڴ�(�����������������һ����)
// WorkingSetSize�����ڴ�ר�ù����������ǳ����ռ�������ڴ�
// PagefileUsage�����ύ��С�����ǳ����ռ���ڴ�(���������ڴ����ҳ���ļ��е��ڴ�)

#include <iostream>
#include <windows.h>
#include "D:/TestPath/TestSln/DatLab/common/LabBase.h"
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

void showMemoryInfo(size_t &dMemUsedBytes,size_t &dMemAllocBytes)
{
	HANDLE handle=GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle,&pmc,sizeof(pmc));
	//cout<<"�ڴ�ʹ��:"<<pmc.WorkingSetSize/1024 <<"K/"<<pmc.PeakWorkingSetSize/1024<<"K + "<<pmc.PagefileUsage/1024 <<"K/"<<pmc.PeakPagefileUsage/1024 <<"K"<<endl;
	size_t MemUsed = pmc.WorkingSetSize>>10;
	size_t MemAlloc = pmc.PagefileUsage>>20;
	if (dMemUsedBytes!=MemUsed)
	{
		dMemUsedBytes	= MemUsed;
		dMemAllocBytes	= MemAlloc;
		std::cout<<"�ڴ�ʹ��:"<<dMemUsedBytes<<"KB"<<",�ύ��С�� "<<dMemAllocBytes<<"MB"<<std::endl;
	}
	
}

void Test_Memory_Lazy_Allocation(){
	int Num = 1024*256;
	float* pBuff = new float[Num];
	pBuff[Num-1]=1.0f;
	size_t MemUsed		= 0;
	size_t MemAlloc		= 0;
	for (int i=0;i<Num;++i){
		pBuff[i]=1.0f*i;
		showMemoryInfo(MemUsed,MemAlloc);
	}
	delete [] pBuff;
}

void Test_ModOp(){
	//ȡ���Ż�
	LogAlgoPerf logPerf;
	LOGPERF(logPerf,setLogStart)
	for (int i=1;i<600;++i){
		
			int idx = i&(320-1);	
			//int idx = i%(320-1);	
		
			//std::cout<<i<<"	"<<idx<<std::endl;
	}
	LOGPERF(logPerf,getLogEnd)
}

void Test_Memory_Copy(){
	int szImg	= 936*320;
	int Num		= 100;
	std::shared_ptr<float> pDst(new float[szImg*Num]);
	std::shared_ptr<float> pSrc(new float[szImg]);
	float*ptrSrc = pSrc.get();
	float*ptrDst = pDst.get();
	for (int i=0;i<szImg;++i){
		ptrSrc[i] = i*1.0f;
	}
	for (int i=0;i<szImg*Num;++i){
		ptrDst[i] = i*1.0f;
	}
	LogAlgoPerf logPerf;
	LOGPERF(logPerf,setLogStart)
	memset(pDst.get(),0,szImg*Num*sizeof(float));
	LOGPERF(logPerf,getLogEnd)
	
	
	
	int stepImg = szImg*sizeof(float);
	
	for (int i=0;i<Num;++i){
		LOGPERF(logPerf,setLogStart)
		memcpy(ptrDst,ptrSrc,stepImg);
		LOGPERF(logPerf,getLogEnd)
		ptrDst += szImg;
	}

}

std::string Test_Memory_Set(size_t iDataNum,int nTestNum=100){
	std::string sInfo = "";
	std::shared_ptr<float> pDst(new float[iDataNum]);
	float*ptrDst = pDst.get();
	for (int i=0;i<iDataNum;++i){
		ptrDst[i] = i*1.0f;
	}
	LogAlgoPerf logPerf;
	int szData = iDataNum*sizeof(float);
	for (int i=0;i<nTestNum;++i){
		LOGPERF(logPerf,setLogStart)
			memset(ptrDst,i,szData);
		LOGPERF(logPerf,getLogEnd)
	}
	double dTotalTime = logPerf.getTotalTime();
	size_t nTotalData = szData*nTestNum;
	double dScaleData = nTotalData/dTotalTime;
	if (dScaleData>1024.0){
		if(dScaleData>1024.0*1024.0){
			double MemBand = nTotalData/(dTotalTime*1024.0*1024.0);
			sInfo +="MemBand of memset is " + std::to_string((long double)MemBand) + " MB/s";
		}else{
			double MemBand = nTotalData/(dTotalTime*1024.0);
			sInfo +="MemBand of memset is " + std::to_string((long double)MemBand) + " KB/s";
		}
	}else{
		double MemBand = nTotalData/dTotalTime;
		sInfo +="MemBand of memset is " + std::to_string((long double)MemBand) + " B/s";
	}
	return sInfo;
}

std::string Test_Memory_Copy(size_t iDataNum,int nTestNum=100){
	//iDataNum			= 936*320;
	std::string sInfo = "";
	std::shared_ptr<float> pDst(new float[iDataNum]);
	std::shared_ptr<float> pSrc(new float[iDataNum]);
	float*ptrSrc = pSrc.get();
	float*ptrDst = pDst.get();
	for (int i=0;i<iDataNum;++i){
		ptrDst[i] = i*1.0f;
	}
	LogAlgoPerf logPerf;
	int szData = iDataNum*sizeof(float);
	for (int i=0;i<nTestNum;++i){
		for (int k=0;k<iDataNum;++k){
			ptrSrc[k] = i*k*1.0f;
		}
		LOGPERF(logPerf,setLogStart)
			memcpy(ptrDst,ptrSrc,szData);
		LOGPERF(logPerf,getLogEnd)
	}
	double dTotalTime = logPerf.getTotalTime();
	size_t nTotalData = szData*nTestNum;
	double dScaleData = nTotalData/dTotalTime;
	if (dScaleData>1024.0){
		if(dScaleData>1024.0*1024.0){
			double MemBand = nTotalData/(dTotalTime*1024.0*1024.0);
			sInfo +="MemBand of memcpy is " + std::to_string((long double)MemBand) + " MB/s";
		}else{
			double MemBand = nTotalData/(dTotalTime*1024.0);
			sInfo +="MemBand of memcpy is " + std::to_string((long double)MemBand) + " KB/s";
		}
	}else{
		double MemBand = nTotalData/dTotalTime;
		sInfo +="MemBand of memcpy is " + std::to_string((long double)MemBand) + " B/s";
	}
	return sInfo;

}

int gmain(){
	std::string sInfo = "";
	for (int i=1024;i<1024*1024*256;i=i<<1)
	//for (int i=10;i<330;i+=10)
	{
		//sInfo = Test_Memory_Set(i);
		sInfo = Test_Memory_Copy(i);
		std::cout<<" i = "<<i*sizeof(float)<<" "<<sInfo<<std::endl;
	}
	//Test_Memory_Set(1024*1024);
	//Test_Memory_Copy(1024*1024);
	//Test_Memory_Copy();
	//Test_ModOp();
	//Test_Memory_Lazy_Allocation();
}