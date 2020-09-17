#pragma once
#ifndef DATA_BASE_H
#define DATA_BASE_H
#include<fstream>
#include<memory>
#include<string>
#include<iostream>
#include <sys/stat.h>

enum READDATATYPE
{
	TINT8		= 0,
	TINT8U		= 1,
	TINT16		= 2,
	TINT16U		= 3,
	TINT32		= 4,
	TFLOAT32	= 5,
	TFLOAT64	= 6
};

class FileBase
{
public:
	FileBase(void);
	~FileBase(void);

public:
	template<typename T>
	static std::shared_ptr<T> ReadFile(CString fileName, size_t &len) {
		std::string file = CT2A(fileName.GetString());
		std::ifstream ifs(file, std::ios::in|std::ios::binary|std::ios::ate);
		if(ifs.fail()) {
			std::cout<<"failed to open file: "<<file<<std::endl;
			len = 0;
			return std::shared_ptr<T>();
		}
		size_t size = GetFileSize(file);
		size = size/sizeof(T);
		if (len>size) len = size;
		std::shared_ptr<T> intput(new T[len]);
		ifs.seekg(0, std::ios::beg);
		ifs.read((char*)intput.get(), len*sizeof(T));
		ifs.close();
		return intput;
	}
	
	template<typename T>
	static bool ReadFile(CString fileName,  std::shared_ptr<T>pData,size_t &len){
		bool bState = true;
		pData = ReadFile<T>(fileName,len);
		if (len==0)
		{
			bState = false;
		}
		return bState;
	}

	template<typename T>
	static void SaveData(T* pDataBuff,SIZE_T &nDataNum,std::string fileName)
	{
		std::ofstream ofs(fileName, std::ios::binary|std::ios::ate);
		if(ofs.fail()) {
			return;
		}
		ofs.write((char*)pDataBuff, nDataNum*sizeof(T));
		ofs.close();
	}

private:
	static size_t GetFileSize(const std::string& file_name){
		struct _stat info;
		_stat(file_name.c_str(), &info);
		size_t size = info.st_size;
		return size; //µ¥Î»ÊÇ£ºbyte
	}
};
#endif DATA_BASE_H