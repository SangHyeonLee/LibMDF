#include <iostream>
#include "proto_util.h"
/*
* ������ ��ó : 
* Google Protocol Buffer
*
*
*
*/
//2018/1/5
using namespace std;
using namespace protobuf;
void main() {
	const char* filename = "test.bin";
	const char* DB_name = "testPageDB";
	const char* pageDBtoProtobin = "address.bin";
	const size_t size = 10;
	

	//createProtoBin(filename, size);
	//readProtoBin(filename);

	//generateProtoPageDB(filename, DB_name);
	readProtoPageDB(DB_name);
	//protoPageDBtoProtoBin(DB_name, pageDBtoProtobin);
	system("pause");
}