#include <iostream>
#include <string>
#include "utility.h";

using namespace std;
void main() {
	string path = "./testdir"; // test dir
	string db_name = "testDB"; // DB file name
	string dest = "./dest";
	puts("start~~\n");
	
	//imageProcessing::generate_page_db(path.c_str(), db_name.c_str()); // create PageDB
	//imageProcessing::print_page(db_name.c_str(),dest.c_str()); // print page (db_name, destPath)
	//imageProcessing::page_to_lmdb("path/DB_name",destPath);
	//imageProcessing::get_lmdb_data(lmdbpath); // lmdb -> jpeg files
	//imageProcessing::lmdb_to_pageDB(lmdbpath, pageDB_name); // lmdb -> pageDB
	imageProcessing::generateDB_Many_dir();
	system("pause");
}