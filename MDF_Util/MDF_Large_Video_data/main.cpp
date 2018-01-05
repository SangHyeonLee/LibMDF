#include <iostream>

#include "video_util.h"
using namespace std;
using namespace video_util;

void main() {
	const char* filename = "Sample.mkv";
	const char* DB_name = "testPageDB";
	const char* output_name = "output.mkv";
	const char* dir = "./test";
	const char* dir_DB_name = "PageDB_dir";
	const char* dir_out = "out_sample";
	//generatePageDB(filename, DB_name);
	//PageDBtoData(DB_name, output_name);
	//generatePageDB_dir(dir, dir_DB_name);
	//PageDBtoMkv(dir_DB_name, dir_out);
	
	getDataSize(dir_DB_name, 2);
	//printRIDtable(dir_DB_name);
	system("pause");
}