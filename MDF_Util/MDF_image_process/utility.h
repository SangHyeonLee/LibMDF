#ifndef _LibMDF_SAMPLES_PAGEDB_GENERATOR_UTILITY_H_
#define _LibMDF_SAMPLES_PAGEDB_GENERATOR_UTILITY_H_
#include <mdf\datatype\universal_slotted_page.h>


using namespace std;

namespace utility {




}

namespace imageProcessing {
	int generateDB_Many_dir();
	int generate_page_db(const char* filepath, const char* pageDB_name);
	void print_page(const char* DB_name, const char* destPath);
	void page_to_lmdb(const char* filename, const char* path);
	void page_to_lmdb_basic(const char* filename, const char* path);
	void print_lmdb_data(const char* path);
	void get_lmdb_data(const char* path);
	void lmdb_to_pageDB(const char* path, const char* filename);

	//void LMDBdummy(const char* path);
	//void testLMDB(const char* filename, const char* path);
	//void page_to_lmdb2(const char* filename, const char* path);
	//void page_to_lmdb3(const char* filename, const char* path);
}



#endif //!_LibMDF_SAMPLES_PAGEDB_GENERATOR_UTILITY_H_