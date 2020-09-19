#include <limits>
#include <iostream>
#include <string>
#include <sstream>

int main()
{
	int iPositionX = -252;
	auto Exp = abs(iPositionX);
	auto LargeConst = (512/2.0F-80/2.0F);
	bool bSuc = true;
	bool bParasSuc = true;
	std::string errStr = "";
	bSuc = Exp < LargeConst;              
	bParasSuc = bSuc && bParasSuc; 
	if(!bSuc) 
	{    
		std::stringstream sstream,sstream1;                            
		sstream<<(Exp); 
		sstream1<<(LargeConst); 
		//errStr += (#Exp);
		errStr += " (Value:" + sstream.str(); 
		errStr += ") should be small than " + sstream1.str(); 
	}
}