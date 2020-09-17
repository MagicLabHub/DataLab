#include"LabBase.h"


int main( int argc, const char* argv[] ) { 
	AlgoLogPerf logfile;
	int nTime = 1000;
	for(int i=0;i<nTime;++i){
		LOGPERF(logfile,setLogStart)
		printf( "\nHello World\n\n" ); 
		LOGPERF(logfile,getLogEnd)
	}
}
