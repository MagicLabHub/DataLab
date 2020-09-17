#include <string>  
#include <map>  
#include <iostream> 
#include <iomanip>
#include <windows.h>  
#include <TlHelp32.h> 

std::string WCHAR2String(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
	if (nLen <= 0)
		return std::string("");

	char* pszDst = new char[nLen];
	if (NULL == pszDst)
		return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;

	std::string strTmp(pszDst);
	delete[] pszDst;

	return strTmp;
}

bool traverseProcesses()   
{  
	PROCESSENTRY32 pe32;  
	pe32.dwSize = sizeof(pe32); 

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//??????  
	if(hProcessSnap == INVALID_HANDLE_VALUE) {  
		std::cout<<"CreateToolhelp32Snapshot Error!" << std::endl;;  
		return false;  
	}  

	BOOL bResult =Process32First(hProcessSnap, &pe32);  

	int num(0);  

	while(bResult)   
	{  
		int id = pe32.th32ProcessID;  
		int ParentiD = pe32.th32ParentProcessID;
		std::string strPIDName = WCHAR2String(pe32.szExeFile);
		std::cout <<std::left<<std::setw(8)<<"["+ std::to_string((_Longlong)++num) +"] : "<<std::setw(28)<<strPIDName<<std::setw(8)<< id<<std::setw(8)<< ParentiD<< std::endl;  
		bResult = Process32Next(hProcessSnap,&pe32);  
	}  

	CloseHandle(hProcessSnap);  

	return true;  
}  



//int main()  
//{  
//	if (!traverseProcesses()) {  
//		std::cout << "Start Process Error!" << std::endl;  
//	}  
//	int a = 0;
//	std::cin>>a;
//	return 0;  
//}
