// ����ִ��exe�ļ�.cpp : Defines the entry point for the console application.
//
// ��Ŀ�������������.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include<fstream> 
#include <string>
using namespace std;

int main(int argc,char* argv[])
{

	ifstream infile("F:\\video\\CDnet2014\\dirpath_vibe.txt");
	string exeStr = "F:\\git\\CodeRepository\\debug�ļ�\\Vibe����.exe  ";
	string input="";
	string temp="";
	int num = 1;
	while(infile>>input){
		temp = exeStr;
		temp.append("  ").append(input);
		cout<<num++<<"  "<<temp<<endl;
		system(temp.c_str());
	}

	return 0;
}


