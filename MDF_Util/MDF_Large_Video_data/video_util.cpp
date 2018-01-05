#include <iostream>
#include <fstream>
#include <string>
#include <windows.h> // for windows
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <mdf\datatype\pagedb.h>



/*
해당 Util은 Large Video Data를 처리하는 예제입니다.
임의의 (64MB 이상)동영상 데이터를 사용하여 예제를 사용해 볼수 있습니다.
*/


namespace video_util {
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
 
	


	std::pair< page_traits::record_t*, page_traits::vertex_t> read_large_file(std::ifstream& ifs, page_traits::serial_id_t &sid) {
		page_traits::record_t* record;
		page_traits::vertex_t vertex;

		//puts("read~~~");
		// 시작전 ifs 위치 확인.
		
	
		size_t length = 0;
		
		
		//ifs.open(filepath, ios::in | ios::binary);
		if (ifs.is_open()) {
			puts("Opne file");
		}
		else {
			puts("not open");
			record = NULL; // Record를 NULL로 설정
			return std::make_pair(record, vertex); // EOF
		}
		//puts("read~~~");
		ifs.seekg(0, ios::end);
		length = ifs.tellg();
		ifs.seekg(0, ios::beg);
		
		printf("length = %d \n", length,length);
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + length);
		//puts("read~~~");
		ifs.read(record->data, length);
		record->size = length;
		//puts("read~~~");

	
		vertex.serial_id = sid++;

		ifs.close();
		if (!ifs.is_open()) puts("Create pair...");
		return std::make_pair(record, vertex);
	}
	void generatePageDB(const char* filepath, const char* pageDB_name)
	{
		/* begin */
		puts("@ Image DataSet - PageDB Geneartion\n");
	
		std::ifstream ifs(filepath, ios::in | ios::binary);
		if (ifs.is_open()) {
			puts("okay open!!");
		
		}
		else {
			puts("not open");

		}
		
		
		page_traits::serial_id_t sid = 0;
		// Generate pageDB 
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(read_large_file, std::ref(ifs), std::ref(sid)), pageDB_name);
		pagedb_generator.commit();
		puts("Completion");
	} // !generatePageDB

	void generatePageDB_dir(const char* dirPath,const char* pageDB_name) {
		struct DIR* dir;
		struct dirent *ent; // file info
		std::ifstream ifs;
		string full_path = dirPath;
		//string full_path;
		dir = opendir(dirPath);
		if (dir == NULL) {
			puts("It is not a directory.\n");
			return;
		}
		else {
			readdir(dir); // . dir
			readdir(dir); // .. dir
		}
		
		//(filepath, ios::in | ios::binary);
		
			
		//struct stat buf; // file size
		if ((ent = readdir(dir)) == NULL) {
			puts("no file");
			return;
		}
		else {
			full_path.append("/"); 
			full_path.append(ent->d_name);
			printf("filepath = %s \n", full_path.c_str());
			
		}
		ifs.open(full_path.c_str(), ios::in | ios::binary);
		if (!ifs.is_open()) {
			puts("failed file open");
			return;
		}
		/*
		printf("sid = %d\n", sid);
		printf("name = %s\n", ent->d_name);
		
		//stat(full_path.c_str(), &buf);
		const size_t temp_size = buf.st_size;
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + temp_size);
		
		std::ifstream ifs(full_path, ios::binary);

		image_to_record(ifs, record, temp_size);
		*/
		page_traits::serial_id_t sid = 0;
		// Generate pageDB 
		puts("generating..");
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(read_large_file, std::ref(ifs), std::ref(sid)), pageDB_name);
		ifs.close();
		while ((ent = readdir(dir)) != NULL) {
			full_path = dirPath;
			full_path.append("/");
			full_path.append(ent->d_name);
			printf("filepath = %s \n", full_path.c_str());
			ifs.open(full_path.c_str(), ios::in | ios::binary);
			if (!ifs.is_open()) {
				puts("failed file open");
				return;
			}
			puts("Updating..");
			pagedb_generator.updateDB(std::bind(read_large_file, std::ref(ifs), std::ref(sid)));
			ifs.close();
			puts("Update Completion");
		}
		pagedb_generator.commit();
		puts("Completion");

	}


	

	void get_rtable(const char* DB_name, rid_table_t& rtable) {
		char rid_name[256];
		sprintf_s(rid_name, "%s.rid_table", DB_name);
		/* Read RID Table */
		rtable = mdf::read_rid_table<rid_tuple_t, std::vector>(rid_name);
	}
	void get_pages(const char* DB_name, page_cont_t& pages) {
		char page_name[256];
		sprintf_s(page_name, "%s.pages", DB_name);
		/* Read pageDB */
		pages = mdf::read_pages<page_t, std::vector>(page_name);
	}
	/** @brief 해당 이름의 pageDB와 rid_Table을 로드합니다.
	*
	*/
	void get_pageDB(const char* DB_name, rid_table_t& rtable, page_cont_t& pages) {
		get_rtable(DB_name, rtable);
		get_pages(DB_name, pages);
	} // !get_pageDB

	void PageDBtoData_dir(const char* DB_name, const char* dirPath,const char* filename) {
		rid_table_t rtable;
		page_cont_t pages;
		get_pageDB(DB_name, rtable, pages);


	}
	void PageDBtoMkv(const char* DB_name, const char* filename) {
		rid_table_t rtable;
		page_cont_t pages;

		get_pageDB(DB_name, rtable, pages);
		std::fstream ofs;
		//std::string seq_filename;
		char seq_filename[256];
		
		size_t file_index = 0;
		
		printf("page size = %d \n", rtable.size());
		bool lp = false;
		if (rtable.at(0).auxiliary > 0) puts("okay");
		size_t i = 0; // iter index
		while(true) {

			size_t num = 0; // small page - slot size and large page - page size

			// sp or lp check
			if (rtable.at(i).auxiliary > 0) {
				puts("Large page");
				lp = true;
				num = rtable.at(i).auxiliary + 1; // total size..
			}
			else {
				puts("small page");
				lp = false;
				num = pages[i].number_of_slots(); // page[i] slot size

				// this area need to implement code
			}


			if (!lp) {
				// small page iter
				puts("small page");
				for (size_t j = 0; j < num; j++) {
					// page 안의 slot 개수만큼 반복.
				}
				i++; // 1개 증가.
			}
			else {
				//large page iter
				puts("Large page");
				sprintf_s(seq_filename, "%s%d.mkv",filename, file_index++);
				ofs.open(seq_filename,ios::out | ios::trunc | ios::binary);
				if (!ofs.is_open()) {
					puts("failed file open ");
					return;
				}
				for (size_t j = 0; j < num; j++) {
					printf("large iter %d\n", j);
					auto record = pages[i].record_start(pages[i].slot(0));
					ofs.write(record->payload, record->data_size);
					
					i++;
				}
				ofs.close(); // end of creating file.
			}
			printf("end~\n");
			
			
			if (i == rtable.size()) {
				
				puts("Complete");
				break;
			} 


		} // !while

		
	} // !PageDBtoData /
	void printRIDtable(const char* DB_name) {
		rid_table_t rtable;
		get_rtable(DB_name, rtable);
		for (size_t i = 0; i < rtable.size(); i++) {
			printf("[%02d] : ssid - [%d] auxiliary - [%02d]  \n", i, rtable[i].start_sid, rtable[i].auxiliary);
		}

	} //!printRIDtable

	void getDataSize(const char* DB_name, size_t sid) {
		// rtable 끝에서부터 조사하는 함수로 수정해야됨.

		rid_table_t rtable;
		page_cont_t pages;
		size_t file_size = 0;
		get_pageDB(DB_name, rtable, pages);
		size_t largeRecordSize = page_t::LargePageRecordSize;
		size_t i = 0;
		while (true) {
			if (rtable.at(i).auxiliary > 0) {
				// Large page
				size_t num = rtable.at(i).auxiliary;
				printf("num = %d \n", num);
				if (rtable.at(i).start_sid == sid) {
					// if sid == ssid
					puts("ssid == sid");
					printf("sid = %d \n", sid);
					i += num; // add offset
					printf("i += num , i = %d \n", i);
					file_size += (largeRecordSize * num);
					auto record = pages[i].record_start(pages[i].slot(0));
					file_size += record->data_size;
					break;
				}
				else {
					i += (num+1);
					if (rtable.size() == i) {
						puts("no exist");
						break;
					}
				}

			}
			else {
				//Small page
				//need to implement

			}

		} //!while
		//
		printf("file size = %d MB \n", ((file_size/1024)/1024));

	} //!getDataSize

}