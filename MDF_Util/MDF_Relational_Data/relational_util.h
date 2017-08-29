#ifndef _LibMDF_RELATIONAL_DATA_PROCESS_H_
#define _LibMDF_RELATIONAL_DATA_PROCESS_H_


	namespace relational_data {

		/** @brief Relational Relational PageDB를 생성합니다.
		* filename : CSV와 같은 구분자 단위의 Text file
		* pageDB_name : 생성될 DB의 이름
		*/
		int generate_page_db(const char* filename, const char* pageDB_name);
		/** @brief Relational Data를 읽어 slot과 record를 리턴하는 함수.
		*
		*/
		void print_relational_pageDB(const char* DB_name);
		/** @brief Relational Data를 읽어 slot과 record를 리턴하는 함수.
		*
		*/
		void print_index_of_col(const char* DB_name, size_t index);
		/** @brief Relational Data를 읽어 slot과 record를 리턴하는 함수.
		*
		*/
		void pageDB_to_Dat(const char* DB_name, const char* filename);
		/** @brief Relational Data를 읽어 slot과 record를 리턴하는 함수.
		*
		*/
		void print_indexOfTuple_col(const char* DB_name, const size_t sid, size_t col_index);
		/** @brief Relational Data를 읽어 slot과 record를 리턴하는 함수.
		*
		*/
		void print_indexOfTuple(const char* DB_name, const size_t sid);
		/* @brief 각 페이지의 슬랏 개수를 출력
		*
		*/
		void print_page_numofslot(const char* DB_name);
	}





#endif