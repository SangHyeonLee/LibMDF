#ifndef _LibMDF_RELATIONAL_DATA_PROCESS_H_
#define _LibMDF_RELATIONAL_DATA_PROCESS_H_


	namespace relational_data {

		/** @brief Relational Relational PageDB�� �����մϴ�.
		* filename : CSV�� ���� ������ ������ Text file
		* pageDB_name : ������ DB�� �̸�
		*/
		int generate_page_db(const char* filename, const char* pageDB_name);
		/** @brief Relational Data�� �о� slot�� record�� �����ϴ� �Լ�.
		*
		*/
		void print_relational_pageDB(const char* DB_name);
		/** @brief Relational Data�� �о� slot�� record�� �����ϴ� �Լ�.
		*
		*/
		void print_index_of_col(const char* DB_name, size_t index);
		/** @brief Relational Data�� �о� slot�� record�� �����ϴ� �Լ�.
		*
		*/
		void pageDB_to_Dat(const char* DB_name, const char* filename);
		/** @brief Relational Data�� �о� slot�� record�� �����ϴ� �Լ�.
		*
		*/
		void print_indexOfTuple_col(const char* DB_name, const size_t sid, size_t col_index);
		/** @brief Relational Data�� �о� slot�� record�� �����ϴ� �Լ�.
		*
		*/
		void print_indexOfTuple(const char* DB_name, const size_t sid);
		/* @brief �� �������� ���� ������ ���
		*
		*/
		void print_page_numofslot(const char* DB_name);
	}





#endif