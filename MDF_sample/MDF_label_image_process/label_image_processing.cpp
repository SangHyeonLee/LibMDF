#include <iostream>
#include <windows.h> // for windows

#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys\stat.h>

#include "util.h"
#include <mdf\datatype\pagedb.h>


namespace label_imageProcessing {
	//Basic Setting
	using serial_id_t = uint64_t;
	using data_info_t = void;
	using record_offset_t = uint64_t;
	using data_size_t = uint64_t;
	using record_size_t = uint64_t;
	using data_payload_t = char[1];
	using key_payload_t = uint32_t; // key_payload store label 
	constexpr size_t PageSize = mdf::_slotted_page::default_page_size;
	bool key_value = true;

	/* define page type and its helpers (page_traits, generator_traits) */
	using page_t = mdf::slotted_page<serial_id_t, data_info_t, record_offset_t, data_size_t, record_size_t, PageSize, data_payload_t, key_payload_t>;
	using page_traits = mdf::page_traits<page_t>;
	using generator_traits = mdf::generator_traits<page_t>;

	// Define container type for storing pages (you can use any type of STL sequential container for constructing RID table, e.g., std::vector, std::list)
	using page_cont_t = std::vector<page_t>;
	// Define RID tuple type
	using rid_tuple_t = generator_traits::rid_tuple_t;
	// Define RID table type (you can use any type of STL sequential container for constructing RID table, e.g., std::vector, std::list)
	using rid_table_t = generator_traits::rid_table_t;

	void image_to_record(std::ifstream& ifs, page_traits::record_t *record, size_t data_size) {
		memset(record, 0, sizeof(page_traits::record_t) + data_size); // Init
		record->size = data_size; // insert size
		ifs.read(reinterpret_cast<char*>(record->data), data_size); // read data.
	}
	std::pair< page_traits::record_t*, page_traits::vertex_t> image_to_pair(std::string path, DIR *dir, page_traits::serial_id_t &sid) {
		page_traits::record_t* record;
		page_traits::vertex_t vertex; // serial_id , payload
		vertex.serial_id = sid++;
		struct dirent *ent; // file info
		struct stat buf; // file size
		if ((ent = readdir(dir)) == NULL) {
			record = NULL;
			return std::make_pair(record, vertex);
		}

		printf("sid = %d\n", sid);
		printf("name = %s\n", ent->d_name);
		string full_path = path + "/" + ent->d_name;
		stat(full_path.c_str(), &buf);
		const size_t temp_size = buf.st_size;
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + temp_size);

		std::ifstream ifs(full_path, ios::binary);

		image_to_record(ifs, record, temp_size);
		return std::make_pair(record, vertex);
	}

	int cifar10_to_page_db(const char* filepath, const char* pageDB_name) {
		//최종적으로 만들어야되는곳.

		

		return 0;
	}
	int generate_page_db(const char* filepath, const char* pageDB_name)
	{
		/* begin */
		puts("@ Image DataSet - PageDB Geneartion\n");
		/* init */
		struct DIR* dir;
		//string full_path;
		dir = opendir(filepath);
		if (dir == NULL) {
			puts("It is not a directory.\n");
			return EXIT_FAILURE;
		}
		else {
			readdir(dir); // . dir
			readdir(dir); // .. dir
		}
		page_traits::serial_id_t sid = 0;
		// Generate pageDB 
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(image_to_pair, std::ref(filepath), std::ref(dir), std::ref(sid)), pageDB_name);

		return 0; // SUCCESS

	}
}