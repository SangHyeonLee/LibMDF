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

#include <mdf\mdf_lmdb.h> // need to download lmdb.h file

using namespace std;
/* Util의 Dataset은 ImageNet 입니다.

*/

namespace imageProcessing {
	//Basic Setting
	constexpr size_t SIZE_MB = 1024U * 1024U;
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


	///std::string path;
	
	
	/** @brief Image를 Record로 변환 해주는 함수.
	*
	*/
	void image_to_record(std::ifstream& ifs, page_traits::record_t *record, size_t data_size) {
		memset(record, 0, sizeof(page_traits::record_t) + data_size); // Init
		record->size = data_size; // insert size
		ifs.read(reinterpret_cast<char*>(record->data), data_size); // read data.
	}


	/** @brief Image를 Record and Slot의 pair로 변환 해주는 함수.
	*
	*/
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
	
	
	/** @brief Function of genetating PageDB.
	*
	*/
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
		pagedb_generator.generateDB(std::bind(image_to_pair, std::ref(filepath), std::ref(dir),std::ref(sid)), pageDB_name);
		pagedb_generator.commit();
		return 0; // SUCCESS

	}
	/** @brief Test function
	*
	*/
	int generateDB_Many_dir() {
		//example of many_dir...
		puts("start test many dir \n");
		string source1 = "./source1"; // need to jpeg files in dir
		string source2 = "./source2"; // need to jpeg files in dir
		string pageDB_name = "testDB"; // pageDB name
		const char* dd = source1.c_str();
		const char* dd2 = source2.c_str();
		/* begin */
		puts("@ Image DataSet - PageDB Geneartion\n");
		/* init */
		struct DIR* dir;
		//string full_path;
		dir = opendir(source1.c_str());
		if (dir == NULL) {
			puts("It is not a directory.\n");
			return EXIT_FAILURE;
		}
		else {
			readdir(dir); // . dir
			readdir(dir); // .. dir
		}
		struct DIR* dir2;
		dir2 = opendir(dd2);
		if (dir == NULL) {
			puts("It is not a directory.\n");
			return EXIT_FAILURE;
		}
		else {
			readdir(dir2); // . dir
			readdir(dir2); // .. dir
		}
		page_traits::serial_id_t sid = 0;
		// Generate pageDB 
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(image_to_pair, std::ref(dd), std::ref(dir), std::ref(sid)), pageDB_name.c_str());
		pagedb_generator.updateDB(std::bind(image_to_pair, std::ref(dd2), std::ref(dir2), std::ref(sid)));
		pagedb_generator.commit();
		puts("end function\n");
		return 0; // SUCCESS
	}

	
	/** @brief  print information and translate pageDB to jpeg files
	*
	*/
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
				//printf("slot sid = %d\n", pages[j].slot(i).serial_id);
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

	/** @brief 
	*
	*/
	pair<size_t, char*> get_record(rid_table_t &table, page_cont_t &pages,size_t &page_id ,size_t &count) {

		size_t slot_size = pages[page_id].number_of_slots();
		size_t last_slot_num = pages[page_id].slot(slot_size).serial_id;
		size_t t_size = table.size(); // 테이블 사이즈.
		char *data = "EOF";
		
		if (count > slot_size - 1) { // 카운터가 넘어가버리면.
			if (t_size == page_id + 1) return std::make_pair(0, data); // 페이지 끝이면 종료
			//puts("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
			printf("page id++ = %d", page_id);
			page_id++; // 페이지 번호 증가
			count = 0; // 페이지의 첫부분.
			slot_size = pages[page_id].number_of_slots(); //슬랏 사이즈 재 조사.
		}
		auto record = pages[page_id].record_start(pages[page_id].slot(count));
		data = (char *)malloc(record->data_size);
		memmove(data, record->payload, record->data_size);
		
		count++;
		return make_pair(record->data_size, data);
	}

	/** @brief
	*
	*/
	void page_to_lmdb_basic(const char* filename,const char* path) {
		puts("Start page to lmdb \n");
		//Setting
		char rid_name[256];
		char page_name[256];
		sprintf_s(rid_name, "%s.rid_table", filename);
		sprintf_s(page_name, "%s.pages", filename);
		rid_table_t rtable = mdf::read_rid_table<rid_tuple_t, std::vector>(rid_name);
		page_cont_t pages = mdf::read_pages<page_t, std::vector>(page_name);
		size_t count = 0,page_id = 0;
		size_t pages_size = pages.size();
		size_t page_size = PageSize;

		puts("Setting ends.. start func..\n");
		mdf::lmdb::generate_lmdb(std::bind(get_record, std::ref(rtable), std::ref(pages),std::ref(page_id), std::ref(count)),page_size * pages_size ,path);
	}

	/** @brief
	*
	*/
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
		txn->ChangeAddMapSize(PageSize);
		//슬롯 사이즈만큼 돌린다.
		for(int i = 0; i < pages.size(); i++) {// 페이지 수 만큼 이터레이트
			size_t page_slot_size = pages[i].number_of_slots();

			for (int j = 0; j < page_slot_size; j++) { // 페이지 안의 슬롯 갯수 만큼 이터레이트
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


	/** @brief
	*
	*/
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

	/** @brief
	*
	*/
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
#if 0 // temp..
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

		///ifs.read(reinterpret_cast<char*>(RecRec->data), temp_size); // 데이터 부분에 data 읽기


		image_to_record(ifs, record, temp_size);
		return std::make_pair(record, true);

	}
#endif 
	 

	/** @brief
	*need to lmdb to pageDB for creating record pair
	*/
	std::pair< page_traits::record_t*, page_traits::vertex_t> lmdb_to_pair(mdf::lmdb::Cursor *cursor, page_traits::serial_id_t &serial_id){
		page_traits::record_t* record;
		page_traits::vertex_t vertex;
		vertex.serial_id = serial_id++;
		printf("sid = %d\n", serial_id);
		if(cursor->valid()) {
			const string value = cursor->value_str_t();
			record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + value.size());
			record->size = value.size();
			//record->data = reinterpret_cast<char *>(value.c_str());
			memmove(record->data, value.c_str(), record->size);

			cursor->Next();
			return std::make_pair(record, vertex);
		}
		else {
			record = NULL;
			return std::make_pair(record, vertex);
		}
	}

	/** @brief
	*
	*/
	void lmdb_to_pageDB(const char* path,const char* filename) {
		puts("Start lmdb to pageDB \n");
		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB());
		
		

		lmdb->Open(path, mdf::lmdb::Mode::READ);
		mdf::lmdb::Cursor *cursor(lmdb->NewCursor());
		char buf[256];
		sprintf_s(buf, "%s/%s", path, filename);
		puts("Setting complete \n");
		cout << buf << endl;
		page_traits::serial_id_t sid = 0;
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(lmdb_to_pair, std::ref(cursor),std::ref(sid)), buf);
		puts("pageDB generated...\n");


	}


#if 0
	/** @brief
	*
	*/
	void testLMDB(const char* filename, const char* path) {
	

		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB());
		lmdb->Open(path, mdf::lmdb::Mode::READ);
		//mdf::lmdb::Transaction *txn(lmdb->NewTransaction());
		lmdb->get_stat();
	
		//txn->AddMapSize();
	//	txn->AddMapSize();
		//txn->Commit();
		lmdb->Close();
		
		puts("Commit Transaction \n");
	}

	/** @brief
	*
	*/
	void LMDBdummy(const char* path) {
		

		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB());
		lmdb->Open(path, mdf::lmdb::Mode::NEW);
		mdf::lmdb::Transaction *txn(lmdb->NewTransaction());
		size_t count = 0;
		size_t total_size = 0;
		puts("while start\n");
		
		printf("total_size = %d     , %d MB\n",total_size, total_size / SIZE_MB);
		puts("while end\n");

		//	txn->AddMapSize();
		txn->Commit();
		lmdb->Close();

		puts("Commit Transaction \n");
	}

	/** @brief
	*
	*/
	void page_to_lmdb2(const char* filename, const char* path) {
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
		txn->ChangeAddMapSize(SIZE_MB);

		//슬롯 사이즈만큼 돌린다.
		for (int i = 0; i < pages.size()-1; i++) {// 페이지 수 만큼 이터레이트
			printf(" page %d\n", i);
			size_t page_slot_size = pages[i].number_of_slots();

			for (int j = 0; j < page_slot_size; j++) { // 페이지 안의 슬롯 갯수 만큼 이터레이트
													   //get record
				auto record = pages[i].record_start(pages[i].slot(j));

				size_t sid = pages[i].slot(j).serial_id;

				std::string key(mdf::format_size_t(sid)); // lmdb key
				std::string value(record->payload, record->data_size); // lmdb value

				txn->Put(key, value);

			}
		}
		txn->Commit();
		lmdb->Close();
		puts("Commit Transaction \n");
	}


	void page_to_lmdb3(const char* filename, const char* path) {
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
		lmdb->Open(path, mdf::lmdb::Mode::WRITE);
		mdf::lmdb::Transaction *txn(lmdb->NewTransaction());
		txn->ChangeAddMapSize(SIZE_MB);

		//슬롯 사이즈만큼 돌린다.
		for (int i = pages.size()-1; i < pages.size(); i++) {// 페이지 수 만큼 이터레이트
			printf(" page %d\n", i);
			size_t page_slot_size = pages[i].number_of_slots();

			for (int j = 0; j < page_slot_size; j++) { // 페이지 안의 슬롯 갯수 만큼 이터레이트
													   //get record
				auto record = pages[i].record_start(pages[i].slot(j));

				size_t sid = pages[i].slot(j).serial_id;

				std::string key(mdf::format_size_t(sid)); // lmdb key
				std::string value(record->payload, record->data_size); // lmdb value

				txn->Put(key, value);

			}
		}
		txn->Commit();
		
		lmdb->Close();
		puts("Commit Transaction \n");
	}
#endif // temp..

}
