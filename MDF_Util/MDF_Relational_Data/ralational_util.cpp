#include <iostream>
#include <windows.h> // for windows

#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
//#include <dirent.h>
#include <sys\stat.h>
#include <direct.h>
#include <io.h>
#include <thread>

#include "relational_util.h"
#include <mdf\datatype\pagedb.h>
/*
해당 Util은 텍스트 기반의 Relatione 데이터(TCP-DS Dat file)를 기반으로 한 예제이며 
web_sales.dat 파일을 활용하였습니다.
Blob을 사용한 이미지, 동영상 등등의 데이터를 저장한 경우 수정하여 사용해주시기 바랍니다.


*/



namespace relational_data {
	//Basic Setting
	const size_t attribute_size = 34; // need to use proper size 
	typedef struct attr { // need to use array..
		/*you need to use attribute_size, erase this coment*/
		//const size_t attribute_size = 34 
		uint32_t size[attribute_size];
	};
	using serial_id_t = uint64_t;
	using data_info_t = void;
	using record_offset_t = uint64_t;
	using data_size_t = uint64_t;
	using record_size_t = uint64_t;
	using data_payload_t = char[1];
	using key_payload_t = attr; // key_payload store label 
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


	
	
	/** @brief 튜플의 총 길이를 구하는 함수.
	*
	*/
	size_t get_full_size(vector<std::pair<size_t,std::string>> tuple) {
		size_t result = 0;
		for (int i = 0; i < tuple.size(); i++) result += tuple.at(i).first;
		return result;
	}
	/** @brief 텍스트 기반의 Relational Data를 속성 단위로 자르는 함수.
	*
	* This returns a store slot and record from tuple
	* specified function.
	* @param[in] string strOrigin 원본 튜플
	* @param[in] string strTok 각 속성을 구분짓는 구분자.
	* @param[in] page_traits::vertex_t& vertex 저장될 슬롯
	* @param[in] page_traits::record_t* record 각 속성 값을 저장할 레코드
	* @return 슬롯과 레코드가 저장된다.
	*/
	size_t strSplit(string strOrigin, string strTok, page_traits::vertex_t& vertex, page_traits::record_t* record)
	{
		int cutAt;  // 자르는 위치
		int index = 0;  // 문자열 인덱스

		uint32_t size_index = 0;
		size_t record_offset = 0;
		vector<std::pair<size_t, std::string>> strResult;  // 결과 return 변수

														   // strTok을 찾을 때까지 반복
		while ((cutAt = strOrigin.find_first_of(strTok)) != strOrigin.npos)
		{

			if (cutAt > -1)  // 자르는 위치가 -1보다 크면
			{

				
				memmove(record->data + record_offset, strOrigin.c_str(), cutAt);
				record_offset += cutAt;//vertex.payload.size[size_index - 1];
				vertex.payload.size[size_index] = record_offset;
				//printf("size[%d] = %d ", size_index, record_offset);
				size_index++;
				
			
			}
			// 원본은 자른 부분을 제외한 나머지
			strOrigin = strOrigin.substr(cutAt + 1);
		}

		if (strOrigin.length() > 0) // 원본이 아직 남았으면
		{
			// 나머지를 결과 배열에 추가
			//strResult.push_back(std::make_pair(cutAt, strOrigin.substr(0, cutAt)));
			memmove(record->data + record_offset, strOrigin.c_str(), cutAt);
			record_offset += cutAt;//vertex.payload.size[size_index - 1];
			vertex.payload.size[size_index] = record_offset;
			//printf("size[%d] = %d ", size_index, record_offset);
			size_index++;
			///memmove((record->data + record_offset), strOrigin.c_str(), cutAt);
			///string test((record->data + record_offset), cutAt);
			///printf("%s \n", test.c_str());
		}

		return record_offset; // full_size
	}
	/** @brief Relational Data를 읽어 slot과 record를 리턴하는 함수.
	*
	*/
	std::pair< page_traits::record_t*, page_traits::vertex_t> read_relational_file(std::ifstream& ifs, page_traits::serial_id_t &sid) {
		page_traits::record_t* record;
		page_traits::vertex_t vertex;
		std::string buffer;
		size_t full_size = 0;

		if (std::getline(ifs, buffer)) { 
			//라인을 얻어왔을 때.
			//printf("sid =%d \n", sid);
		}
		else { // EOF
			record = NULL; // Record를 NULL로 설정
			return std::make_pair(record, vertex); // EOF
			//printf("EOF \n"); 
		}
		

	///	printf("%s \nsize = %d\n", buffer.c_str(),buffer.size());
		
		// 레코드 메모리 할당을 위한 총 길이 계산. * 본 Util에서 full_size는 총길이 - 속성갯수(구분자 갯수)이며 
		// 상황에 따라 바꾸어 사용하시기 바랍니다.
		full_size = buffer.size() - attribute_size;
		
	//	printf("temp size = %d \n", temp.size());
		
	//	printf("temp full size = %d \n", full_size);

		//슬롯 SID 넣어줌
		printf("sid = %d\n", sid);
		vertex.serial_id = sid++;
		
		//레코드 메모리 할당
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + full_size);
		//레코드 사이즈를 넣어줌
		record->size = full_size;
		//구분자를 없애고 각 속성들을 레코드에 넣어준다.
		strSplit(buffer.c_str(), "|", vertex, record); 
		
		return std::make_pair(record, vertex);
	}

	/** @brief Function of genetating PageDB.
	*
	*/
	int generate_page_db(const char* filename, const char* pageDB_name)
	{
		/* begin */
		puts("@ Image DataSet - PageDB Geneartion\n");
		/* init */
		
		std::ifstream file{filename};

		page_traits::serial_id_t sid = 0;
		//read_relational_file(file,sid);
		// Generate pageDB 
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(read_relational_file, std::ref(file), std::ref(sid)), pageDB_name);
		pagedb_generator.commit();
		return 0; // SUCCESS

	}
	/** @brief 
	*
	*/
	vector<string> get_page_tuples(page_traits::page_t& page) {
		printf("start get_tuple \n");
		size_t rec_size_t = sizeof(page_traits::record_size_t);
		vector<string> vec_result;
		///size_t slot_num = page.number_of_slots();
		//printf("slot_num =%d\n", slot_num);
		for (int i = 0; i < page.number_of_slots(); i++) {
			//printf("%d\n", i);
			auto slot = page.slot(i);
			///auto record_data = page.record_data(slot);
			///string tem(*record_data, page.record_start(page.slot(0))->data_size);
			///printf("tem\n%s\n", tem.c_str());
			size_t offset = slot.record_offset + rec_size_t;
			//printf("offset =%d \n", offset);
			size_t attr_size = slot.key_payload.size[0];

			string result;
			for (int j = 0; j < attribute_size; j++) {
				///reinterpret_cast<string&>(dataSection[offset]);
				if (j > 0) attr_size = slot.key_payload.size[j] - slot.key_payload.size[j - 1];
				string temp(*(page.record_offset_data(offset)), attr_size);

				///string temp = reinterpret_cast<string &>(page.data_section[offset], attr_size);
				///string test(page.data_section[slot.record_offset + rec_size_t], 100);
				///printf("%s\n", test.c_str());

				//printf("attr_size[%d] = %d , offset = %d \n", j, attr_size,offset);
				offset += attr_size;
				//printf("sid %d , %d = %s\n", i, j, temp.c_str());
				result.append(temp + "|");
			}
			vec_result.push_back(result);
			printf("sid #%d \n%s\n\n", slot.serial_id, result.c_str());
			
		}
		return vec_result;
	}
	/** @brief
	*
	*/
	string get_page_tuple(page_traits::page_t& page, size_t index) {
		///printf("start get_tuple \n");
		size_t rec_size_t = sizeof(page_traits::record_size_t);
		string result;
		auto slot = page.slot(index);
		///auto record_data = page.record_data(slot);
		///string tem(*record_data, page.record_start(page.slot(0))->data_size);
		///printf("tem\n%s\n", tem.c_str());
		size_t offset = slot.record_offset + rec_size_t;
		//printf("offset =%d \n", offset);
		size_t attr_size = slot.key_payload.size[0];
		for (int j = 0; j < attribute_size; j++) {
			///reinterpret_cast<string&>(dataSection[offset]);
			if (j > 0) attr_size = slot.key_payload.size[j] - slot.key_payload.size[j - 1];
			string temp(*(page.record_offset_data(offset)), attr_size);

			///string temp = reinterpret_cast<string &>(page.data_section[offset], attr_size);
			///string test(page.data_section[slot.record_offset + rec_size_t], 100);
			///printf("%s\n", test.c_str());

			//printf("attr_size[%d] = %d , offset = %d \n", j, attr_size,offset);
			offset += attr_size;
			//printf("sid %d , %d = %s\n", i, j, temp.c_str());
			result.append(temp + "|");
		}

		return result;

	}
	/** @brief 해당 이름의 pageDB와 rid_Table을 로드합니다.
	*
	*/
	void get_pageDB(const char* DB_name,rid_table_t& rtable, page_cont_t& pages) {
		char rid_name[256];
		char page_name[256];
		sprintf_s(rid_name, "%s.rid_table", DB_name);
		sprintf_s(page_name, "%s.pages", DB_name);
		/* Read RID Table */
		rtable = mdf::read_rid_table<rid_tuple_t, std::vector>(rid_name);
		/* Read pageDB */
		pages = mdf::read_pages<page_t, std::vector>(page_name);
	}

	/** @brief
	*
	*/
	void print_relational_pageDB(const char* DB_name) {
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
		

		///std::ofstream ofs;
		printf("Page Size = %d  \n ", page_size);
		for (size_t j = 0; j < page_size; j++) {
			/* get slot size*/
			size_t num = pages[j].number_of_slots();
			
			printf("Page ID = %d and Slot Size = %d \n", j, num);
			/* iterate slot size */
			get_page_tuples(pages[j]);
			printf("page[%d] print end \n", j);
		}
	}

	/** @brief
	*
	*/
	void get_page_tuple_col(page_traits::page_t& page, size_t index) {
		
		printf("start get_tuple \n");
		size_t rec_size_t = sizeof(page_traits::record_size_t);
		///size_t slot_num = page.number_of_slots();
		//iter number_of_slots...
		for (int i = 0; i < page.number_of_slots(); i++) {
			//get slot
			auto slot = page.slot(i);
			///auto record_data = page.record_data(slot);
			///string tem(*record_data, page.record_start(page.slot(0))->data_size);
			///printf("tem\n%s\n", tem.c_str());
			//default offset
			size_t offset = slot.record_offset + rec_size_t;
			size_t attr_size = slot.key_payload.size[0]; // default attr_size
			//printf("offset =%d \n", offset);
			if (index != 0) {
				offset += slot.key_payload.size[index - 1];
				attr_size = slot.key_payload.size[index] - slot.key_payload.size[index - 1];
			}
			string result(*(page.record_offset_data(offset)), attr_size);
			printf("sid =%d\n",slot.serial_id);
			printf("%s\n", result.c_str());
			
		}

	}
	string get_page_tuple_col(page_traits::page_t& page, size_t col_index, size_t index) {
		printf("start get_tuple \n");
		size_t rec_size_t = sizeof(page_traits::record_size_t);
		///size_t slot_num = page.number_of_slots();
		//iter number_of_slots...
		//for (int i = 0; i < page.number_of_slots(); i++) {
			//get slot
			auto slot = page.slot(index);
			///auto record_data = page.record_data(slot);
			///string tem(*record_data, page.record_start(page.slot(0))->data_size);
			///printf("tem\n%s\n", tem.c_str());
			//default offset
			size_t offset = slot.record_offset + rec_size_t;
			size_t attr_size = slot.key_payload.size[0]; // default attr_size
														 //printf("offset =%d \n", offset);
			if (col_index != 0) {
				offset += slot.key_payload.size[col_index - 1];
				attr_size = slot.key_payload.size[col_index] - slot.key_payload.size[col_index - 1];
			}
			string result(*(page.record_offset_data(offset)), attr_size);
			

			return result;
		//}
	}

	/** @brief
	*
	*/
	void print_index_of_col(const char* DB_name, size_t index) {
		if (index > attribute_size) {
			puts("error : out of index\n");
			return;
		}
		rid_table_t rtable;
		page_cont_t pages;
		get_pageDB(DB_name, rtable, pages); // get pageDB
		puts("start print.. \n");
		for (size_t j = 0; j < pages.size(); j++) {
			/* get slot size*/
			size_t num = pages[j].number_of_slots();

			printf("Page ID = %d and Slot Size = %d \n", j, num);
			/* iterate slot size */
			get_page_tuple_col(pages[j],index);
			printf("page[%d] print end \n", j);
		}

	}

	/** @brief
	*
	*/
	void write_page_tuples(std::ofstream& ofs, page_traits::page_t& page) {
		printf("start write_tuple \n");
		size_t rec_size_t = sizeof(page_traits::record_size_t);
		for (int i = 0; i < page.number_of_slots(); i++) {
			
			//auto slot = page.slot(i);
			string result = get_page_tuple(page,i);
			ofs << result.c_str() << endl;

		}
	}

	/** @brief
	*
	*/
	void pageDB_to_Dat(const char* DB_name, const char* filename) {
		rid_table_t rtable;
		page_cont_t pages;
		get_pageDB(DB_name, rtable, pages);
		std::ofstream ofs(filename, ios::trunc | ios::binary);
		for (size_t j = 0; j < pages.size(); j++) {
			printf("write page[%d]\n", j);
			write_page_tuples(ofs, pages[j]);
		}
		ofs.close();
	}


	/** @brief
	*
	*/
	string indexOfTuple(const char* DB_name,const size_t sid) {
		rid_table_t rtable;
		page_cont_t pages;
		get_pageDB(DB_name, rtable, pages);
		size_t page_index = 0; // 해당 sid가 있는 페이지 인덱스 
		size_t count = 0; // 이전 페이지들의 slot 총합
		size_t diff = 0; // sid와 count의 차이값. 
		size_t max_index = pages.size() - 1; // 페이지 사이즈.
		
		if (rtable[max_index].start_sid + pages[max_index].number_of_slots() < sid) return "Error : out of index.. ";
		
		for (int i = 0; i < pages.size(); i++) {
			///printf("rtable ssid = %d\n", rtable[i].start_sid);
			if (rtable[i].start_sid + pages[i].number_of_slots() > sid) {
				page_index = i;
				diff = sid - count;
				//printf("sid = %d count = %d , diff = %d \n", sid, count, diff);
				printf("page index = %d \n", page_index);
				return get_page_tuple(pages[page_index], diff);
			}
			else {
				count += pages[i].number_of_slots();
			}
		}
		return "error : unknown..";
	}
	void print_indexOfTuple(const char* DB_name, const size_t sid){
		string result = indexOfTuple(DB_name, sid);
		printf("sid = %d\n",sid);
		printf("\n%s\n\n", result.c_str());
	}
	string indexOfTuple_col(const char* DB_name,const size_t sid,size_t col_index) {
		rid_table_t rtable;
		page_cont_t pages;
		get_pageDB(DB_name, rtable, pages);
		size_t page_index = 0; // 해당 sid가 있는 페이지 인덱스 
		size_t count = 0; // 이전 페이지들의 slot 총합
		size_t diff = 0; // sid와 count의 차이값. 
		size_t max_index = pages.size() - 1; // 페이지 사이즈.
		
		if (rtable[max_index].start_sid + pages[max_index].number_of_slots() < sid) return "Error : out of index.. ";
		
		for (int i = 0; i < pages.size(); i++) {
			///printf("rtable ssid = %d\n", rtable[i].start_sid);
			if (rtable[i].start_sid + pages[i].number_of_slots() > sid) {
				page_index = i;
				diff = sid - count;
				//printf("sid = %d count = %d , diff = %d \n", sid, count, diff);
				printf("page index = %d \n", page_index);
				return get_page_tuple_col(pages[page_index], col_index, diff);
			}
			else {
				count += pages[i].number_of_slots();
			}
		}
		return "error : unknown..";
	}
	void print_indexOfTuple_col(const char* DB_name, const size_t sid, size_t col_index) {
		string result = indexOfTuple_col(DB_name, sid, col_index);
		printf("sid = %d col_index = %d \n", sid, col_index);
		printf("\n%s\n\n", result.c_str());
	}
	void print_page_numofslot(const char* DB_name) {
		rid_table_t rtable;
		page_cont_t pages;
		get_pageDB(DB_name, rtable, pages);
		for (int i = 0; i < pages.size(); i++) printf("page[%d] slot size = %d \n", i, pages[i].number_of_slots());
	}

}
