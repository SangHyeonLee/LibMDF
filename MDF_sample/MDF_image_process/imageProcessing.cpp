#include <iostream>
#include <windows.h> // for windows

#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys\stat.h>

#include "utility.h"
#include <mdf\datatype\pagedb.h>
//#include "mdf\datatype\data_info.h"
#include <mdf\mdf_lmdb.h> // need to download lmdb.h file

using namespace std;
/*
������ �����ߵ� ..

*/
namespace imageProcessing {
	//Basic Setting
	using serial_id_t = uint64_t;
	using data_info_t = void;
	using record_offset_t = uint64_t;
	using data_size_t = uint64_t;
	using record_size_t = uint64_t;
	using data_payload_t = char[1];
	using key_payload_t = void;
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

	std::vector<page_traits::record_t> record_list;
	std::string path;
	
	

	void image_to_record(std::ifstream& ifs, page_traits::record_t *record, size_t data_size) {
		memset(record, 0, sizeof(page_traits::record_t) + data_size); // Init
		record->size = data_size; // insert size
		ifs.read(reinterpret_cast<char*>(record->data), data_size); // read data.
	}



	std::pair< page_traits::record_t*, bool> image_to_pair(std::string path, DIR *dir) {
		page_traits::record_t* record;
		struct dirent *ent; // file info
		struct stat buf; // file size
		if ((ent = readdir(dir)) == NULL) {
			record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t));
			return std::make_pair(record, false);
		}
		printf("name = %s\n", ent->d_name);
		string full_path = path + "/" + ent->d_name;
		stat(full_path.c_str(), &buf);
		const size_t temp_size = buf.st_size;
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + temp_size);
		serial_id_t sid;
		std::ifstream ifs(full_path, ios::binary);


		image_to_record(ifs, record, temp_size);
		return std::make_pair(record, true);
	}

	// Function of genetating PageDB
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
		
		// Generate pageDB 
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(image_to_pair, std::ref(filepath), std::ref(dir)), pageDB_name);

		return 0; // SUCCESS

	}

	//print pageDB. 
	void print_page(const char* DB_name,const char* destPath) {
		//Setting
		char rid_name[256];
		char page_name[256];
		sprintf_s(rid_name, "%s.rid_table", DB_name);
		sprintf_s(page_name, "%s.pages", DB_name);
		/* Read RID Table */
		rid_table_t rtable = mdf::read_rid_table<rid_tuple_t, std::vector>(rid_name);
		/* Read pageDB */
		page_cont_t pages = mdf::read_pages<page_t, std::vector>(page_name);
		puts("Start PageDB Print \n\n");
		size_t count = 0; // total slot size
		size_t page_size = pages.size(); // total pages size
		std::ofstream ofs;
		printf("Page Size = %d  \n ", page_size);
		
		
		char name[256]; //  image name buffer
		_mkdir(destPath); // create dir
		/* iterate page size */
		for (size_t j = 0; j < page_size; j++) { 
			/* get slot size*/
			size_t num = pages[j].number_of_slots();
			printf("Page ID = %d and Slot Size = %d \n",j, num);
			/* iterate slot size */
			for (size_t i = 0; i < num; i++) {
				sprintf_s(name, "%s/p%d.jpeg", destPath, count++); // image name
				
				ofs.open(name, ios::trunc | ios::binary);
				/* get record and write image */
				auto record = pages[j].record_start(pages[j].slot(i));
				ofs.write(reinterpret_cast<const char*>(record->payload), record->data_size);
				ofs.close();
			}
			printf("page[%d] print end \n", j);
		}
		printf("total slot is %d \n\n", count);
		puts("Strart RID info \n");
		for (size_t a = 0; a < rtable.size(); a++) {
			printf("rid_table %d %d \n", rtable[a].start_sid, rtable[a].auxiliary);
		}
	}

	pair<size_t, char*> get_record(rid_table_t &table, page_cont_t &pages,size_t &page_id ,size_t &count) {

		size_t slot_size = pages[page_id].number_of_slots();
		size_t last_slot_num = pages[page_id].slot(slot_size).serial_id;
		size_t t_size = table.size(); // ���̺� ������.
		char *data = "EOF";
		
		if (count > slot_size - 1) { // ī���Ͱ� �Ѿ������.
			if (t_size == page_id + 1) return std::make_pair(0, data); // ������ ���̸� ����
			puts("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
			printf("page id++ = %d", page_id);
			page_id++; // ������ ��ȣ ����
			count = 0; // �������� ù�κ�.
			slot_size = pages[page_id].number_of_slots(); //���� ������ �� ����.
		}
		auto record = pages[page_id].record_start(pages[page_id].slot(count));
		data = (char *)malloc(record->data_size);
		memmove(data, record->payload, record->data_size);
		
		count++;
		return make_pair(record->data_size, data);
	}
	void page_to_lmdb_basic(const char* filename,const char* path) {
		puts("Start page to lmdb \n");
		//Setting
		char rid_name[256];
		char page_name[256];
		sprintf_s(rid_name, "%s.rid_table", filename);
		sprintf_s(page_name, "%s.pages", filename);
		rid_table_t rtable = mdf::read_rid_table<rid_tuple_t, std::vector>("imageTest.rid_table");
		page_cont_t pages = mdf::read_pages<page_t, std::vector>("imageTest.pages");
		size_t count = 0,page_id = 0;
		size_t pages_size = pages.size();
		size_t page_size = PageSize;

		puts("Setting ends.. start func..\n");
		mdf::lmdb::generate_lmdb(std::bind(get_record, std::ref(rtable), std::ref(pages),std::ref(page_id), std::ref(count)),page_size * pages_size ,path);
	}
	void page_to_lmdb(const char* filename, const char* path) {
		puts("Start page to lmdb \n");
		//Setting
		char rid_name[256];
		char page_name[256];
		sprintf_s(rid_name, "%s.rid_table", filename);
		printf("%s  imageTest.pages \n", rid_name);
		sprintf_s(page_name, "%s.pages", filename);
		printf("%s  imageTest.rid_table\n", page_name);
		rid_table_t table = mdf::read_rid_table<rid_tuple_t, std::vector>(rid_name);
		page_cont_t pages = mdf::read_pages<page_t, std::vector>(page_name);
		puts("load complete \n");
		size_t count = 0, page_id = 0;
		//size_t pages_size = pages.size();
		size_t page_size = PageSize;

		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB());
		lmdb->Open(path, mdf::lmdb::Mode::NEW);
		mdf::lmdb::Transaction *txn(lmdb->NewTransaction());
	
		//���� �����ŭ ������.
		for(int i = 0; i < pages.size(); i++) {// ������ �� ��ŭ ���ͷ���Ʈ
			size_t page_slot_size = pages[i].number_of_slots();

			for (int j = 0; j < page_slot_size; j++) { // ������ ���� ���� ���� ��ŭ ���ͷ���Ʈ
				//get record
				auto record = pages[i].record_start(pages[i].slot(j));

				size_t sid = pages[i].slot(j).serial_id;
				
				std::string key(mdf::format_size_t(sid)); // lmdb key
				std::string value(record->payload,record->data_size); // lmdb value
				
				txn->Put(key, value);

			}
		}
		txn->Commit();
		lmdb->Close();
		puts("Commit Transaction \n");
	}

	void print_lmdb_data(const char* path) {
		puts("set lmdb\n");
		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB()); 
		puts("lmdb open \n");
		lmdb->Open(path, mdf::lmdb::Mode::READ);
		puts("cursor setting \n");
		mdf::lmdb::Cursor *cursor(lmdb->NewCursor());
		puts("Enter the loop\n");
		while (cursor->valid()) {
			printf("key = %d value size = %d \n", cursor->key_size_t(), cursor->value_size());
			cursor->Next();
		}
		puts("end loop\n");

	}

	void get_lmdb_data(const char* path) {
		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB());
		lmdb->Open(path, mdf::lmdb::Mode::READ);
		mdf::lmdb::Cursor *cursor(lmdb->NewCursor());
		char name[256];
		size_t count = 0;
		puts("Enter the loop\n");
		while (cursor->valid()) {
			
			sprintf_s(name, "%s/p%d.jpeg", path, mdf::format_str(cursor->key_str_t())); // image name
			//cout << "key is " << cursor->key_str_t() << endl;
			
			std::ofstream ofs(name, ios::trunc | ios::binary);
			/* get record */
			//auto record = pages[j].record_start(pages[j].slot(i));
			//printf("slot number = %d  ", pages[j].slot(i).serial_id);
			//printf("record size %d\n", record->data_size);
			//printf("create image... %s \n", name);
			const string image = cursor->value_str_t();
			ofs.write(image.c_str(),image.size());
			ofs.close();
			count++;
			//printf("%s  size = %d\n", name , cursor->value_size());

			cursor->Next();
		}
		printf("count %d \n", count);
		
	}
	/*
	std::pair< page_traits::record_t*, bool> image_to_pair(std::string path, struct _finddata_t* fd, intptr_t* handle)
	{
		
		page_traits::record_t* record;
		
		if (_findnext(*handle, fd) != 0) { 
			record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t));
			return std::make_pair(record, false);
		}
		const size_t temp_size = fd->size;
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + temp_size);
		serial_id_t sid;
		string filepath = path + fd->name;
		bool flag;

		std::ifstream ifs(filepath, ios::binary);

		///ifs.read(reinterpret_cast<char*>(RecRec->data), temp_size); // ������ �κп� data �б�


		image_to_record(ifs, record, temp_size);
		return std::make_pair(record, true);

	}
	*/
	// need to lmdb to pageDB for creating record pair
	std::pair< page_traits::record_t*, bool> lmdb_to_pair(mdf::lmdb::Cursor *cursor){
		page_traits::record_t* record;
		if(cursor->valid()) {
			const string value = cursor->value_str_t();
			record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + value.size());
			record->size = value.size();
			//record->data = reinterpret_cast<char *>(value.c_str());
			memmove(record->data, value.c_str(), record->size);

			cursor->Next();
			return std::make_pair(record, true);
		}
		else {
			record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t));
			return std::make_pair(record, false);
		}
	}
	void lmdb_to_pageDB(const char* path,const char* filename) {
		puts("Start lmdb to pageDB \n");
		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB());
		lmdb->Open(path, mdf::lmdb::Mode::READ);
		mdf::lmdb::Cursor *cursor(lmdb->NewCursor());
		char buf[256];
		sprintf_s(buf, "%s/%s", path, filename);
		puts("Setting complete \n");
		cout << buf << endl;
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(lmdb_to_pair, std::ref(cursor)), buf);
		puts("pageDB generated...\n");

	}




}