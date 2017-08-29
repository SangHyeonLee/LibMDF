
#include <iostream>
#include <windows.h> // for windows

#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys\stat.h>
#include <direct.h>
#include <io.h>
#include <thread>
#include <lodepng.h>
#include "util.h"
#include <mdf\datatype\pagedb.h>

#include <mdf\mdf_lmdb.h> // need to download lmdb.h file

/* Util의 Dataset은 cifar10 데이터입니다.

*/
namespace label_imageProcessing {

	
	//Basic Setting
	using serial_id_t = uint64_t; //sid
	using data_info_t = void; // 
	using record_offset_t = uint64_t; // must 
	using data_size_t = uint64_t; //
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
	
	// cifar10 data info /png/
	const int kCIFARSize = 32;
	const int kCIFARImageNBytes = 3072;
	const int kCIFARBatchSize = 10000;
	const int kCIFARTrainBatches = 5;

	/** @brief
	*
	*/
	std::pair< page_traits::record_t*, page_traits::vertex_t> read_image(std::ifstream& file, page_traits::serial_id_t &sid, int &batchSize) {
		page_traits::record_t* record;
		page_traits::vertex_t vertex;
		if (batchSize == 10000) {
			record = NULL;
			batchSize = 0;
			return make_pair(record, vertex);
		}
		//int label; 
	
		char label_char;
		
		file.read(&label_char, 1);
		//label = label_char;
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + kCIFARImageNBytes);
		record->size = kCIFARImageNBytes;
		file.read(record->data, kCIFARImageNBytes);
		vertex.serial_id = sid;
		vertex.payload = static_cast<uint32_t>(label_char);
		//printf("sid = %d, label = %d\n", sid, static_cast<uint32_t>(label_char));
		sid++; batchSize++;
		return make_pair(record, vertex);
	}
	

	/** @brief cifar10 데이터를 pageDB로 변환합니다.
	*
	*/
	int cifar10_to_page_db() {
		int count = 1;
		char batchFileName[256];
		sprintf_s(batchFileName, "./cifar10/data_batch_%d.bin", count++);
		std::ifstream data_file(batchFileName,std::ios::in | std::ios::binary);
		page_traits::serial_id_t sid = 0;
		int batchSize = 0;
		printf("%s\n", batchFileName);
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(read_image, std::ref(data_file), std::ref(sid), std::ref(batchSize)), "cifar10");
		data_file.close();
		for (int i = 1; i < 5; i++) {
			sprintf_s(batchFileName, "./cifar10/data_batch_%d.bin", count++);
			printf("Update ~~ %s\n", batchFileName);
			data_file.open(batchFileName, std::ios::in | std::ios::binary);
			pagedb_generator.updateDB(std::bind(read_image, std::ref(data_file), std::ref(sid), std::ref(batchSize)));
			data_file.close();
		}
		pagedb_generator.commit();
		return 0;
	}

	/** @brief
	*
	*/
	void binary_to_png(const string bin, const char* filename) {
		std::vector<std::uint8_t> PngBuffer(bin.size());

		for (std::int32_t I = 0; I < 1024; ++I)
		{
			//std::size_t OldPos = (32 - I - 1) * (32 * 4) + 4 * J;
			std::size_t NewPos = I * 3;
			PngBuffer[NewPos + 0] = bin[I]; //rand()%255;// //B is offset 2
			PngBuffer[NewPos + 1] = bin[1024 + (I)];//rand() % 255;// //G is offset 1
			PngBuffer[NewPos + 2] = bin[2048 + (I)];//rand() % 255;// //R is offset 0
		}
		std::vector<std::uint8_t> ImageBuffer;
		lodepng::encode(ImageBuffer, PngBuffer, 32, 32, LodePNGColorType::LCT_RGB, 8U);
		lodepng::save_file(ImageBuffer, filename);
	}

	/** @brief
	*
	*/
	void iter_page(page_cont_t &pages, size_t page_id, size_t slotSize, const char* destPath) {
		char name[256];
		for (size_t i = 0; i < slotSize; i++) {
			//printf("slot sid = %d\n", pages[j].slot(i).serial_id);
			//ofs.open(name, ios::trunc | ios::binary);
			page_traits::slot_t slot = pages[page_id].slot(i);
			sprintf_s(name, "%s/%d/p%d.png", destPath, slot.key_payload, slot.serial_id); // image name
			auto record = pages[page_id].record_start(slot);
			string image(record->payload, record->data_size);


			printf("%s\n", name);
			//------------
			binary_to_png(image, name);

		}
	}
	//print pageDB. 
	/** @brief
	*
	*/
	void print_label_page(const char* DB_name, const char* destPath) {
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
		vector<thread> thread(page_size);
		
		std::ofstream ofs;
		printf("Page Size = %d  \n ", page_size);
		

		char name[256]; //  image name buffer
		_mkdir(destPath); // create dir
		for (int i = 0; i < 10; i++) {
			//make directory
			sprintf_s(name, "%s/%d", destPath, i);
			_mkdir(name);
		}
						  /* iterate page size */
		for (size_t j = 0; j < page_size; j++) {
			/* get slot size*/
			size_t num = pages[j].number_of_slots();
			printf("Page ID = %d and Slot Size = %d \n", j, num);
			/* iterate slot size */
			thread.at(j) = std::thread(&iter_page, pages, j, num, destPath);

			printf("page[%d] thread start \n", j);
		}
		for (int i = 0; i < thread.size(); i++) thread.at(i).join();
		
		printf("End Threads \n\n");
		puts("Strart RID info \n");
		for (size_t a = 0; a < rtable.size(); a++) {
			printf("rid_table %d %d \n", rtable[a].start_sid, rtable[a].auxiliary);
		}
	}
	

	/** @brief
	*
	*/
	void print_page_slot(const char* DB_name) {
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
		for (int i = 0; i < page_size; i++) {
			size_t num = pages[i].number_of_slots();
			printf("page[%d] slot size = %d \n ", i, num);
		}

	}

	/** @brief
	*
	*/
	void print_page(const char* DB_name, const char* destPath) {

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
		for (int i = 0; i < 10; i++) {
			//make directory
			sprintf_s(name, "%s/%d", destPath, i);
			_mkdir(name);
		}
		for (size_t j = 0; j < page_size; j++) {
			/* get slot size*/
			size_t num = pages[j].number_of_slots();
			printf("Page ID = %d and Slot Size = %d \n", j, num);
			/* iterate slot size */
			for (size_t i = 0; i < num; i++) {
				page_traits::slot_t slot = pages[j].slot(i);
				
				sprintf_s(name, "%s/%d/p%d.png", destPath,slot.key_payload,slot.serial_id); // image name
																  //printf("slot sid = %d\n", pages[j].slot(i).serial_id);
																  //ofs.open(name, ios::trunc | ios::binary);
																  
				auto record = pages[j].record_start(pages[j].slot(i));
				
				string image(record->payload, record->data_size);


				printf("%s\n", name);
				//------------
				binary_to_png(image, name);

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
		size_t MB = 1024U * 1024U;
		mdf::lmdb::LMDB *lmdb(new mdf::lmdb::LMDB());
		lmdb->Open(path, mdf::lmdb::Mode::NEW);
		mdf::lmdb::Transaction *txn(lmdb->NewTransaction());
		txn->ChangeAddMapSize(PageSize);
		//슬롯 사이즈만큼 돌린다.
		for (int i = 0; i < pages.size()-2; i++) {// 페이지 수 만큼 이터레이트
			size_t page_slot_size = pages[i].number_of_slots();

			for (int j = 0; j < page_slot_size; j++) { // 페이지 안의 슬롯 갯수 만큼 이터레이트
													   //get record
				auto record = pages[i].record_start(pages[i].slot(j));
				auto slot = pages[i].slot(j);
				//size_t sid = slot.serial_id;
				vector<std::string> key_values;

				key_values.push_back(mdf::format_size_t(slot.serial_id));
				key_values.push_back(mdf::format_size_t(slot.key_payload));

				std::string key(mdf::format_key_string(key_values)); // lmdb key
				std::string value(record->payload, record->data_size); // lmdb value

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

}