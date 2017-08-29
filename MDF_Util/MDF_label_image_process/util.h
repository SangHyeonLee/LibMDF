#ifndef _LibMDF_LABEL_IMAGE_PROCESS_H_
#define _LibMDF_LABEL_IMAGE_PROCESS_H_

namespace label_imageProcessing {

	int cifar10_to_page_db();
	void print_label_page(const char* DB_name, const char* destPath);
	void print_page(const char* DB_name, const char* destPath);
	void print_page_slot(const char* DB_name);

	void page_to_lmdb(const char* filename, const char* path);
	void testLMDB(const char* filename, const char* path);
}

#endif // !_LibMDF_LABEL_IMAGE_PROCESS_H_
