#include <iostream>
#include <string>
#include "utility.h";

using namespace std;
void main() {
	string path = "./testdir"; // test dir
	string db_name = "testDB"; // DB file name

	imageProcessing::generate_page_db(path.c_str(), db_name.c_str()); // create PageDB
	//imageProcessing::print_page(test.c_str(),test.c_str()); // print page (db_name, destPath)
	//imageProcessing::page_to_lmdb("path/DB_name",destPath);
	//imageProcessing::get_lmdb_data(lmdbpath); // lmdb -> jpeg files
	//imageProcessing::lmdb_to_pageDB(lmdbpath, pageDB_name); // lmdb -> pageDB
	
	system("pause");
}