#include <iostream>
#include <chrono>
#include "relational_util.h"
/*
* 데이터 출처 : http://www.tpc.org/
* TPC-DS Dataset
*
*
*
*/

using namespace std;
void main() {
	/*std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

	relational_data::generate_page_db("./dataSet/web_sales.dat", "testDB");

	std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;

	std::cout << "Test() 함수를 수행하는 걸린 시간(초) : " << sec.count() << " seconds" << std::endl;*/



	//relational_data::print_relational_pageDB("testDB");
	//relational_data::print_index_of_col("testDB", 5);
	//relational_data::pageDB_to_Dat("testDB", "testDB.dat");
	relational_data::print_indexOfTuple("testDB", 700000);
	relational_data::print_indexOfTuple_col("testDB", 700000,5);

	relational_data::print_page_numofslot("testDB");
	system("pause");
}