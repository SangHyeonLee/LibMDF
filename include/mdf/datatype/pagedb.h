/** -------------------------------------------------------------------
*	@project	LibMDF
*	@location	mdf/datatype
*	@file		pagedb.h
*	@brief		Template based generic PageDB generator implementation
*	@original	Seyeon Oh (vee@dgist.ac.kr)		
*	@author		Sanghyeon Lee (sh_lee@dgist.ac.kr)
*	@version	1.1, 28/3/2017
* ----------------------------------------------------------------- */

#ifndef _MDF_DATATYPE_PAGEDB_H_
#define _MDF_DATATYPE_PAGEDB_H_

#include "universal_slotted_page.h"
#include "data_info.h"
#include <cstdio>
#include <vector>
#include <fstream>
#include <iterator>
#include <sstream>
#include <iomanip>


namespace mdf {
	inline std::string format_size_t(size_t n, int numberOfLeadingZeros = 0) {
		std::ostringstream s;
		s << std::setw(numberOfLeadingZeros) << std::setfill('0') << n;
		return s.str();
	}
	inline vector<std::string> split_string(const char* str) {
		vector<std::string> result;
		const char* split = "|";
		char *context = NULL;
		int i = 0;
		char *ptr = strtok_s((char *)str, split, &context);
		while (ptr != NULL) {
			result.push_back(ptr);
			i++;
			ptr = strtok_s(NULL, "|", &context);
		}
		return result;
	}
	

	inline vector<std::pair<size_t, std::string>> split_string2(const char* str) {
		vector<std::pair<size_t,std::string>> result;
		const char* split = "|";
		char *context = NULL;
		int i = 0;
		char *ptr = strtok_s((char *)str, split, &context);
		while (ptr != NULL) {
			std::string temp(ptr);
			printf("ptr ~~ %s \n", ptr);
			result.push_back(std::make_pair(temp.size(),temp.c_str()));
			i++;
			ptr = strtok_s(NULL, "|", &context);
		}
		return result;
	}
	inline std::string format_key_string(vector<std::string> args) {
		string result;
		for (int i = 0; i < args.size(); i++) {
			result.append(args.at(i));
			if (i < args.size() - 1) result.append("|");
		}
		return result;
	}
	inline size_t format_str(string str) {
		std::istringstream s(str);
		size_t num;
		s >> num;
		return num;
	}

#pragma pack(push, 1)
template <typename __serial_id_t, typename __auxiliary_t = std::size_t>
struct rid_tuple_template
{	// rid table
    using auxiliary_t = __auxiliary_t;
    using serial_id_t = __serial_id_t;
    serial_id_t start_sid;
	auxiliary_t auxiliary;
};
#pragma pack(pop)


//�������� 5���� �д´� 320�ް�.
template <typename PAGE_T,
    template <typename ELEM_T,
    typename = std::allocator<ELEM_T> >
    class CONT_T = std::vector >
CONT_T<PAGE_T> read_pages(const char* filepath, const std::size_t bundle_of_pages = 3)
{	//������ �д� �Լ�
    using page_t = PAGE_T;
    using cont_t = CONT_T<PAGE_T>;
	//puts("read pages\n");
    // Open a file stream
    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };

    // TODO: metadata implementation
	
    cont_t pages; // container for pages which will be returned.

    // Read pages
    { // ������ �б�
        const std::size_t chunk_size = sizeof(page_t) * bundle_of_pages;
        std::vector<page_t> buffer;
        buffer.resize(bundle_of_pages);
        uint64_t counter = 0;
        while (true) {
            ++counter;
            ifs.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
            std::size_t extracted = ifs.gcount() / sizeof(page_t);
            if (extracted == bundle_of_pages) {
                std::copy(buffer.begin(), buffer.end(), std::back_inserter(pages));
            }
            else {
                std::copy(buffer.begin(), buffer.begin() + extracted, std::back_inserter(pages));
                break;
            }
			
        }
    }
	puts("read pages\n");
    return pages;
}

template <typename RID_TUPLE_T,
    template <typename ELEM_T,
    typename = std::allocator<ELEM_T> >
    class CONT_T = std::vector >
    CONT_T<RID_TUPLE_T> read_rid_table(const char* filepath)
{ // rid ���̺� �д� �Լ�
    using rid_tuple_t = RID_TUPLE_T;
    using rid_table_t = CONT_T<RID_TUPLE_T>;
	//puts("@ start read rid table \n");
    // Open a file stream
    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };
	
    rid_table_t table; // rid table

	
	
	if (!ifs.is_open()) {
		puts("@ not open \n");
		//std::ofstream ofs{ "weuv_disk_based.rid_table", std::ios::out | std::ios::binary };
		//ofs.close();
		return table;
	}
    // Read table
    {
        rid_tuple_t tuple;
        while (!ifs.eof()) {
			//puts("@ insert \n");
            ifs.read(reinterpret_cast<char*>(&tuple), sizeof(rid_tuple_t));
            if (ifs.gcount() > 0)
                table.push_back(tuple);
        }
    }
	puts("@ read rid table \n");
    return table;
}

template <typename RID_TUPLE_T,
	template <typename ELEM_T,
	typename = std::allocator<ELEM_T> >
	class CONT_T = std::vector >
	CONT_T<RID_TUPLE_T> generate_rid_table()
{ // rid ���̺� �д� �Լ�
	using rid_tuple_t = RID_TUPLE_T;
	using rid_table_t = CONT_T<RID_TUPLE_T>;
	puts("@ Generate RID Table \n");

	rid_table_t table; // rid table
	return table;
}

template <typename RID_TUPLE_T,
	template <typename ELEM_T,
	typename = std::allocator<ELEM_T> >
	class CONT_T = std::vector >
	void issue_sp(CONT_T<RID_TUPLE_T>& table, size_t sid)
{ // rid ���̺� �д� �Լ�
	using rid_tuple_t = RID_TUPLE_T;
	using rid_table_t = CONT_T<RID_TUPLE_T>;
	//puts("@ Generate RID Table \n");
	rid_tuple_t tuple;
	tuple.start_sid = sid;
	tuple.auxiliary = 0; // small page: 0
	//printf("generate %d %d \n", tuple.start_sid, tuple.auxiliary);
	//if (sizeof(table) < 1) printf("what?");
	table.push_back(tuple);
	
	
	
	//printf("table size = %d \n", table.size());
	//next_ssid = sid_counter;
	//page->clear();
	//++num_pages;
	//rid_table_t table; // rid table
	//return table;
}
template <typename RID_TUPLE_T,
	template <typename ELEM_T,
	typename = std::allocator<ELEM_T> >
	class CONT_T = std::vector >
	void issue_lp_head(CONT_T<RID_TUPLE_T>& table, size_t sid, size_t num_related)
{ // rid ���̺� �д� �Լ�
	using rid_tuple_t = RID_TUPLE_T;
	using rid_table_t = CONT_T<RID_TUPLE_T>;
	//puts("@ Generate RID Table \n");
	rid_tuple_t tuple;
	tuple.start_sid = sid;
	tuple.auxiliary = num_related; // small page: 0
						 //printf("generate %d %d \n", tuple.start_sid, tuple.auxiliary);
						 //if (sizeof(table) < 1) printf("what?");
	table.push_back(tuple);

	//printf("table size = %d \n", table.size());
	//next_ssid = sid_counter;
	//page->clear();
	//++num_pages;
	//rid_table_t table; // rid table
	//return table;
}

template <typename RID_TUPLE_T,
	template <typename ELEM_T,
	typename = std::allocator<ELEM_T> >
	class CONT_T = std::vector >
	void issue_lp_ext(CONT_T<RID_TUPLE_T>& table, size_t sid, size_t num_ext_pages)
{ // rid ���̺� �д� �Լ�
	using rid_tuple_t = RID_TUPLE_T;
	using rid_table_t = CONT_T<RID_TUPLE_T>;
	//puts("@ Generate RID Table \n");
	rid_tuple_t tuple;
	tuple.start_sid = sid;
	for (size_t i = 1; i <= num_ext_pages; i++) {
		tuple.auxiliary = i;
		table.push_back(tuple);
	}
	//tuple.auxiliary = num_related; // small page: 0
								   //printf("generate %d %d \n", tuple.start_sid, tuple.auxiliary);
								   //if (sizeof(table) < 1) printf("what?");
	//table.push_back(tuple);

	//printf("table size = %d \n", table.size());
	//next_ssid = sid_counter;
	//page->clear();
	//++num_pages;
	//rid_table_t table; // rid table
	//return table;
}

template <typename RID_TUPLE_T,
template <typename ELEM_T,
typename = std::allocator<ELEM_T> >
class CONT_T = std::vector >
void write_rid_table(CONT_T<RID_TUPLE_T>& table, std::ostream& os)
{ // rid ���̺� �д� �Լ�
using rid_tuple_t = RID_TUPLE_T;
using rid_table_t = CONT_T<RID_TUPLE_T>;
puts("@ write rid_table");

for (int i = 0; i< table.size(); i++) {
	os.write(reinterpret_cast<char*>(&table[i].start_sid), sizeof(table[i].start_sid));
	os.write(reinterpret_cast<char*>(&table[i].auxiliary), sizeof(table[i].auxiliary));
}
}





template <typename PageTy>
struct page_traits {   //traits ���ϴ³�?
	using page_t = PageTy;
	ALIAS_SLOTTED_PAGE_TEMPLATE_TYPEDEFS(page_t);
	ALIAS_SLOTTED_PAGE_TEMPLATE_CONSTDEFS(page_t);
	//using rec_t = rec_template<serial_id_t>;
	using vertex_t = vertex_template<serial_id_t, key_payload_t>;
	using record_t = record_template<data_info_t, data_size_t, data_payload_t>;
	//using edge_t = edge_template<serial_id_t, edge_payload_t>;
	using page_builder_t = slotted_page_builder<serial_id_t, data_info_t, record_offset_t, data_size_t, record_size_t, PageSize, data_payload_t, key_payload_t>;
};

enum class generator_error_t { // ����üũ
	success,
	empty_data,
	fail_complete
};
// RID Table ������
template <typename PageTy,
	typename RIDTuplePayloadTy = std::size_t,
	template <typename _ElemTy,
	typename = std::allocator<_ElemTy> >
	class RIDTupleContTy = std::vector >
	class rid_table_generator { // rid ����¾�.
	public:
		using page_t = PageTy; // ������ Ÿ��
		using page_traits = page_traits<page_t>; // ��� ���� 
		using page_builder_t = typename page_traits::page_builder_t; // ������ ����
		ALIAS_SLOTTED_PAGE_TEMPLATE_TYPEDEFS(page_builder_t); 
		ALIAS_SLOTTED_PAGE_TEMPLATE_CONSTDEFS(page_builder_t);
		using rid_tuple_t = rid_tuple_template<typename page_traits::serial_id_t, RIDTuplePayloadTy>;
		using cont_t = RIDTupleContTy<rid_tuple_t>;
		using rid_table_t = cont_t;
		//using record_t = record_template<data_info_t, data_size_t, data_payload_t>;

		//using edgeset_t = std::vector<edge_t>;
		//using edge_iteration_result_t = std::pair<edgeset_t /* sorted vertex #'s edgeset */, serial_id_t /* max_vid */>;
		//using edge_iterator_t = std::function< edge_iteration_result_t() >;
		struct generate_result { // ���� ��� 
			generator_error_t error;
			rid_table_t table;
		};
		//generate_result generate(edge_iterator_t edge_iterator);
		//generate_result generate(edge_t* sorted_edges, ___size_t num_edges);

	protected:
		void init();
		void flush(rid_table_t& table);
		void issue_sp(rid_table_t& table);
		void issue_lp_head(rid_table_t& table, ___size_t num_related);
		void issue_lp_exts(rid_table_t& table, ___size_t num_ext_pages);

		serial_id_t next_ssid;
		serial_id_t sid_counter;
		___size_t  num_pages;
		std::shared_ptr<page_builder_t> page{ std::make_shared<page_builder_t>() };
};

#define RID_TABLE_GENERATOR_TEMPLATE template <typename PageTy, typename RIDTuplePayloadTy, template <typename _ElemTy,	typename > class RIDTupleContTy >
#define RID_TABLE_GENERATOR rid_table_generator<PageTy, RIDTuplePayloadTy, RIDTupleContTy>

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::init()
{
	next_ssid = 0;
	sid_counter = 0;
	num_pages = 0;
}


RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::flush(rid_table_t& table)
{
	if (!page->is_empty())
		issue_sp(table);
}




RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::issue_sp(rid_table_t& table)
{
	rid_tuple_t tuple;
	tuple.start_sid = next_ssid;
	tuple.auxiliary = 0; // small page: 0
	table.push_back(tuple);
	next_ssid = sid_counter;
	page->clear();
	++num_pages;
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::issue_lp_head(rid_table_t& table, ___size_t num_related)
{
	rid_tuple_t tuple;
	tuple.start_sid = next_ssid;
	tuple.auxiliary = static_cast<typename rid_tuple_t::auxiliary_t>(num_related); // head page: the number of related pages
	table.push_back(tuple);
	// This function does not update a member variable 'last_vid' 
	page->clear();
	++num_pages;
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::issue_lp_exts(rid_table_t& table, ___size_t num_ext_pages)
{
	rid_tuple_t tuple;
	tuple.start_sid = next_ssid;
	for (___size_t i = 1; i <= num_ext_pages; ++i) {
		tuple.auxiliary = i; // ext page: page offset from head page
		table.push_back(tuple);
	}
	next_ssid = sid_counter + 1;
	page->clear();
	num_pages += num_ext_pages;
}

#undef RID_TABLE_GENERATOR
#undef RID_TABLE_GENERATOR_TEMPLATE
//page ������ 
template <typename PageBuilderTy, typename RIDTableTy>
class pagedb_generator
{
public:
	using builder_t = PageBuilderTy;
	ALIAS_SLOTTED_PAGE_TEMPLATE_TYPEDEFS(builder_t);
	ALIAS_SLOTTED_PAGE_TEMPLATE_CONSTDEFS(builder_t);
	using rid_table_t = RIDTableTy;
	using rid_tuple_t = typename rid_table_t::value_type;
	using record_t = record_template<data_info_t, data_size_t, data_payload_t>;
	using vertex_t = vertex_template<serial_id_t, key_payload_t>;

	pagedb_generator(rid_table_t& rid_table_);
	using record_pair_t = std::pair<record_t* /* record */, vertex_t /*  */>; //��ȯ�� 
	using record_result_t = std::function<record_pair_t()>; // �Լ�
	

	//using edgeset_t = std::vector<edge_t>;
	//using edge_iteration_result_t = std::pair<edgeset_t /* sorted vertex #'s edgeset */, serial_id_t /* max_vid */>;
	//using edge_iterator_t = std::function< edge_iteration_result_t() >;
	//using vertex_iteration_result_t = std::pair<bool /* success or failure */, vertex_t /* vertex */>;
	//using vertex_iterator_t = std::function< vertex_iteration_result_t() >;


	/*
	�ϴ� �⺻���� Ű - ��� ��Ÿ�Ϸ� �ڵ��Ұ�.
	
	*/
	//std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generateDB(record_result_t record_function, std::ostream& os)

	// Enabled if vertex_payload_t is void type.
	template <typename PayloadTy = key_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type generateDB(record_result_t record_function,const char* filename);
	// Enabled if vertex_payload_t is non-void type.
	template <typename PayloadTy = key_payload_t>
	typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type generateDB(record_result_t record_function, const char* filename);

	template <typename PayloadTy = key_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type updateDB(record_result_t record_function);
	
	
	// Enabled if vertex_payload_t is non-void type.
	template <typename PayloadTy = key_payload_t>
	typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type updateDB(record_result_t record_function);
	
	generator_error_t commit();

	/*
	// Enabled if vertex_payload_t is void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type generate(edge_iterator_t edge_iterator, std::ostream& os);
	// Enabled if vertex_payload_t is non-void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type generate(edge_iterator_t edge_iterator, vertex_iterator_t vertex_iterator, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os);

	// Enabled if vertex_payload_t is void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value>::type generate(edge_t* sorted_edges, ___size_t num_edges, std::ostream& os);
	// Enabled if vertex_payload_t is non-void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<!std::is_void<PayloadTy>::value>::type generate(edge_t* sorted_edges, ___size_t num_edges, vertex_t* sorted_vertices, ___size_t num_vertices, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os);
	*/

protected:
	void init();
	void insert_vertex(std::ostream& os, const vertex_t& vertex, record_t* record);
	void small_page_generate(std::ostream& os, const vertex_t& vertex, record_t* record);
//	void iteration_per_vertex(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
	void flush(std::ostream& os);
	//void small_page_iteration(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
	void large_page_generate(std::ostream& os, const vertex_t& vertex, record_t* record);
	void issue_page(std::ostream& os, page_flag_t flags);
//	void update_list_buffer(edge_t* edges, ___size_t num_edges);
	void update_buffer(record_t* record);


	rid_table_t& rid_table;
	std::ofstream page_os;
	std::ofstream rid_table_os;
	___size_t  sid_counter;
	___size_t  num_pages;
	std::vector<adj_list_elem_t> list_buffer;
	adj_list_elem_t* buffer;
	std::shared_ptr<builder_t> page{ std::make_shared<builder_t>() };
};

#define PAGEDB_GENERATOR_TEMPALTE template <typename PageBuilderTy, typename RIDTableTy>
#define PAGEDB_GENERATOR pagedb_generator<PageBuilderTy, RIDTableTy>


PAGEDB_GENERATOR_TEMPALTE
PAGEDB_GENERATOR::pagedb_generator(rid_table_t& rid_table_) : rid_table{ rid_table_ }
{

}





PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::init()
{
	sid_counter = 0;
	num_pages = 0;
}

//������ ��� ���� void.
PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generateDB(record_result_t record_function,const char* filename)
{
	/* �����Ϸ�.*/
	
	puts("@ start GenerateDB ----------------------------------");
	serial_id_t sid = 0;
	char pages_filename[256];
	char rid_table_filename[256];

	sprintf_s(pages_filename, "%s.pages",filename);
	sprintf_s(rid_table_filename, "%s.rid_table", filename);
	
	page_os.open(pages_filename, std::ios::out | std::ios::binary);
	rid_table_os.open(rid_table_filename, std::ios::out | std::ios::binary);
	//record_t* record;
	//serial_id_t max_vid;

	// Init phase
	this->init();
	record_pair_t result = record_function(); //  �Լ����� ���ڵ� ��� �޾ƿ� 
	//result 1 = record , 2 = vertex
	if (result.first == NULL)
		return generator_error_t::empty_data; // initialize failed or EOF;
	//sid = result.first[0].src;
	//max_vid = result.second;
	if ((builder_t::SlotSize + sizeof(result.first) + result.first->size) > builder_t::DataSectionSize) {
		//large rid
		//issue_sp(rid_table, 0);
	}
	else {
		issue_sp(rid_table, 0);
	}
	//int count = 0; // test 
	//char test2[256]; //test
	// Iteration
	do
	{
		/* it is create pic test.

		record_t* test = result.first;
		sprintf_s(test2, "test%d.jpeg", count++); // test
		printf("%s\n", test2);
		std::ofstream ofs(test2, ios::trunc | ios::binary);// test
		ofs.write(reinterpret_cast<const char*>(test->data), test->size);// test
		ofs.close();
		*/
		
		
		insert_vertex(page_os, result.second, result.first);
		//iteration_per_vertex(os, vertex_t{ vid }, result.first.data(), result.first.size());
		// �� �Լ��� ���� ���� ������ ����°ɷ� ���� �ֵ� small ������ ���ش�.
		///���� ���� �־��ְ�.
		sid += 1; // sid �������ְ� 

		result = record_function(); // �ٽ� �־��ְ� 
		if (result.first == NULL) // ���ڵ尡 ������ �����ش�.
			break; // parsing error?


	} while (true);

	//while (max_vid >= vid)
	//	iteration_per_vertex(os, vertex_t{ vid++ }, nullptr, 0);
	/*
	puts("@ flush page os \n");
	flush(page_os);
	page_os.close();
	puts("@ close os \n");
	
	puts("@ write rid table \n");
	printf("rid_table size ---- %d\n", rid_table.size());
	for(int i = 0 ; i< rid_table.size(); i++)
		printf("rid_table %d %d \n", rid_table[i].start_sid, rid_table[i].auxiliary);


	//print2_rid_table(rid_table);
	write_rid_table(rid_table, rid_table_os);
	rid_table_os.close();
	puts("@ close os \n");*/
	puts("@ end GenerateDB ----------------------------------");
	return generator_error_t::success;
}

//non - void key payload ff
PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generateDB(record_result_t record_function, const char* filename)
{
	/* �����ؾߵ�  vertex label �۾��Ұ�.*/

	puts("@ start GenerateDB ----------------------------------\n");
	serial_id_t sid = 0;
	char pages_filename[256];
	char rid_table_filename[256];

	sprintf_s(pages_filename, "%s.pages", filename);
	sprintf_s(rid_table_filename, "%s.rid_table", filename);
	page_os.open(pages_filename, std::ios::out | std::ios::binary);
	rid_table_os.open(rid_table_filename, std::ios::out | std::ios::binary);
	//record_t* record;
	//serial_id_t max_vid;

	// Init phase
	this->init();
	record_pair_t result = record_function(); //  �Լ����� ���ڵ� ��� �޾ƿ� 
											  //result 1 = record , 2 = vertex
	if (result.first == NULL)
		return generator_error_t::empty_data; // initialize failed;
											  //sid = result.first[0].src;
											  //max_vid = result.second;
	if ((builder_t::SlotSize + sizeof(result.first) + result.first->size) > builder_t::DataSectionSize) {
		//large rid
		issue_sp(rid_table, 0);
	}
	else {
		issue_sp(rid_table, 0);
	}
	//int count = 0; // test 
	//char test2[256]; //test
	// Iteration
	do
	{
		/* it is create pic test.
		
		record_t* test = result.first;
		sprintf_s(test2, "test%d.jpeg", count++); // test
		printf("%s\n", test2);
		std::ofstream ofs(test2, ios::trunc | ios::binary);// test
		ofs.write(reinterpret_cast<const char*>(test->data), test->size);// test
		ofs.close();
		*/


		insert_vertex(page_os, result.second, result.first);
		//iteration_per_vertex(os, vertex_t{ vid }, result.first.data(), result.first.size());
		// �� �Լ��� ���� ���� ������ ����°ɷ� ���� �ֵ� small ������ ���ش�.
		///���� ���� �־��ְ�.
		sid += 1; // sid �������ְ� 
		//printf("sid = %d\n", sid);
		result = record_function(); // �ٽ� �־��ְ� 
		if (result.first == NULL) // ���ڵ尡 ������ �����ش�.
			break; // parsing error?


	} while (true);

	
	
	puts("@ end GenerateDB ----------------------------------\n");
	return generator_error_t::success;
}

PAGEDB_GENERATOR_TEMPALTE
generator_error_t PAGEDB_GENERATOR::commit() {
	puts("@ flush page os");
	flush(page_os);
	page_os.close();
	puts("@ close os");
	puts("@ write rid table");
	printf("rid_table size - %d\n", rid_table.size());
	for (int i = 0; i< rid_table.size(); i++)
		printf("rid_table line[%d] %d %d \n",i, rid_table[i].start_sid, rid_table[i].auxiliary);

	write_rid_table(rid_table, rid_table_os);
	rid_table_os.close();
	return generator_error_t::success;
}
//������ ��� ���� void.
PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::updateDB(record_result_t record_function)
{
	/* �����Ϸ�.*/

	puts("@ start GenerateDB ----------------------------------\n");
	serial_id_t sid = 0;
	
	record_pair_t result = record_function(); //  �Լ����� ���ڵ� ��� �޾ƿ� 
											  //result 1 = record , 2 = vertex
	if (result.first == NULL) {
		puts("Update Error:Empty Data... \n");
		return generator_error_t::empty_data; // initialize failed;
	}
										  
	
	do
	{
	
		insert_vertex(page_os, result.second, result.first);
		//iteration_per_vertex(os, vertex_t{ vid }, result.first.data(), result.first.size());
		// �� �Լ��� ���� ���� ������ ����°ɷ� ���� �ֵ� small ������ ���ش�.
		///���� ���� �־��ְ�.
		sid += 1; // sid �������ְ� 

		result = record_function(); // �ٽ� �־��ְ� 
		if (result.first == NULL) // ���ڵ尡 ������ �����ش�.
			break; // parsing error?


	} while (true);

	
	puts("@ end UpdateDB ----------------------------------\n");
	return generator_error_t::success;
}

//non-void key payload
PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::updateDB(record_result_t record_function)
{
	/* �����Ϸ�.*/

	puts("@ start GenerateDB ----------------------------------\n");
	serial_id_t sid = 0;

	record_pair_t result = record_function(); //  �Լ����� ���ڵ� ��� �޾ƿ� 
											  //result 1 = record , 2 = vertex
	if (result.first == NULL) {
		puts("Update Error:Empty Data... \n");
		return generator_error_t::empty_data; // initialize failed;
	}


	do
	{

		insert_vertex(page_os, result.second, result.first);
		//iteration_per_vertex(os, vertex_t{ vid }, result.first.data(), result.first.size());
		// �� �Լ��� ���� ���� ������ ����°ɷ� ���� �ֵ� small ������ ���ش�.
		///���� ���� �־��ְ�.
		sid += 1; // sid �������ְ� 

		result = record_function(); // �ٽ� �־��ְ� 
		if (result.first == NULL) // ���ڵ尡 ������ �����ش�.
			break; // parsing error?


	} while (true);


	puts("@ end UpdateDB ----------------------------------\n");
	return generator_error_t::success;
}
/*
PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generate(edge_iterator_t edge_iterator, vertex_iterator_t vertex_iterator, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os)
{
	serial_id_t vid;
	serial_id_t max_vid;

	// Init phase
	this->init();
	edge_iteration_result_t edge_iter_result = edge_iterator();
	vertex_iteration_result_t vertex_iter_result = vertex_iterator();
	if (0 == edge_iter_result.first.size())
		return generator_error_t::init_failed_empty_edgeset; // initialize failed;
	bool& wv_enabled = vertex_iter_result.first;
	vertex_t& wv = vertex_iter_result.second;
	vid = edge_iter_result.first[0].src;

	max_vid = edge_iter_result.second;

	// Iteration
	do
	{
		if (!wv_enabled || wv.vertex_id != vid)
		{
			iteration_per_vertex(os, vertex_t{ vid, default_slot_payload }, edge_iter_result.first.data(), edge_iter_result.first.size());
		}
		else
		{
			iteration_per_vertex(os, wv, edge_iter_result.first.data(), edge_iter_result.first.size());
			vertex_iter_result = vertex_iterator();
		}
		vid += 1;

		edge_iter_result = edge_iterator();
		if (0 == edge_iter_result.first.size())
			break; // eof

		if (edge_iter_result.first[0].src > vid)
		{
			for (serial_id_t id = vid; id < edge_iter_result.first[0].src; ++id)
				iteration_per_vertex(os, vertex_t{ id, default_slot_payload }, nullptr, 0);
			vid = edge_iter_result.first[0].src;
		}

		if (edge_iter_result.second > max_vid)
			max_vid = edge_iter_result.second;
	} while (true);

	while (max_vid >= vid)
	{
		if (!wv_enabled || wv.vertex_id != vid)
			iteration_per_vertex(os, vertex_t{ vid, default_slot_payload }, nullptr, 0);
		else
		{
			iteration_per_vertex(os, wv, nullptr, 0);
			vertex_iter_result = vertex_iterator();
		}
		vid += 1;
	}

	flush(os);
	return generator_error_t::success;
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value>::type PAGEDB_GENERATOR::generate(edge_t* sorted_edges, ___size_t num_total_edges, std::ostream& os)
{
	___size_t off = 0;
	serial_id_t src;
	serial_id_t max;
	auto edge_iterator = [&]() -> edge_iteration_result_t
	{
		edgeset_t edgeset;
		if (off == num_total_edges)
			return std::make_pair(edgeset, 0); // eof

		src = sorted_edges[off].src;
		max = (sorted_edges[off].src > sorted_edges[off].dst) ? sorted_edges[off].src : sorted_edges[off].dst;
		edgeset.push_back(sorted_edges[off++]);
		for (___size_t i = off; i < num_total_edges; ++i)
		{
			if (sorted_edges[off].src == src)
			{
				if (sorted_edges[off].dst > max)
					max = sorted_edges[off].dst;
				edgeset.push_back(sorted_edges[off++]);
			}
			else break;
		}
		return std::make_pair(edgeset, max);
	};

	this->generate(edge_iterator, os);
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value>::type PAGEDB_GENERATOR::generate(edge_t* sorted_edges, ___size_t num_total_edges, vertex_t* sorted_vertices, ___size_t num_vertices, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os)
{
	___size_t e_off = 0;
	serial_id_t src;
	serial_id_t max;
	auto edge_iterator = [&]() -> edge_iteration_result_t
	{
		edgeset_t edgeset;
		if (e_off == num_total_edges)
			return std::make_pair(edgeset, 0); // eof

		src = sorted_edges[e_off].src;
		max = (sorted_edges[e_off].src > sorted_edges[e_off].dst) ? sorted_edges[e_off].src : sorted_edges[e_off].dst;
		edgeset.push_back(sorted_edges[e_off++]);
		for (___size_t i = e_off; i < num_total_edges; ++i)
		{
			if (sorted_edges[e_off].src == src)
			{
				if (sorted_edges[e_off].dst > max)
					max = sorted_edges[e_off].dst;
				edgeset.push_back(sorted_edges[e_off++]);
			}
			else break;
		}
		return std::make_pair(edgeset, max);
	};

	___size_t v_off = 0;
	auto vertex_iterator = [&]() -> vertex_iteration_result_t
	{
		if (v_off == num_vertices)
			return std::make_pair(false, vertex_t{});
		return std::make_pair(true, sorted_vertices[v_off++]);
	};

	this->generate(edge_iterator, vertex_iterator, default_slot_payload, os);
}
*/
/*
PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::iteration_per_vertex(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
	if (num_edges > builder_t::MaximumEdgesInHeadPage)
		this->large_page_iteration(os, vertex, edges, num_edges);
	else
		this->small_page_iteration(os, vertex, edges, num_edges);
	++sid_counter;
}
*/

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::flush(std::ostream& os)
{
	if (!page->is_empty())
		issue_page(os, slotted_page_flag::SP);
}


PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::insert_vertex(std::ostream& os, const vertex_t& vertex, record_t* record)
{
	//puts("@ Insert vertex \n");
	//if(free_space < )

	//printf("SlotSIze = %d , record = %d, data = %d , datasectionSize = %d\n", builder_t::SlotSize, sizeof(record), record->size, builder_t::DataSectionSize);
	
	//this->large_page_iteration(os, vertex, edges, num_edges)
	if ((builder_t::SlotSize + sizeof(record) + (size_t)record->size) > builder_t::DataSectionSize)
		this->large_page_generate(os, vertex, record);
	else
		this->small_page_generate(os, vertex, record);
	++sid_counter;
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::small_page_generate(std::ostream& os, const vertex_t& vertex, record_t* record)
{
	//puts("small page gen \n");
	auto scan_result = page->scan(); // ������ ��ĵ
	//puts("small page scan \n");
	bool& slot_available = scan_result.first; // ����? �Ұ���?
	auto& capacity = scan_result.second; // ���� ����
	//puts("small page gen 2 \n");
										 // ���� ���ڵ� �ǰ� + ������ �߰��ص� ���������Ǵ°�?
	if (!slot_available || (capacity < record->size)) {
		issue_page(os, slotted_page_flag::SP); // �ȵǸ� ������ �߰�
		issue_sp(rid_table, vertex.serial_id);
		printf("issue new page \n");
		
	}
		
	//puts("small page gen3 \n");
	vertex.to_slot(*page); // �������� ���� �߰�
	//puts("small page gen4 \n");
	//if (num_edges == 0)
		//return;

	auto offset = page->number_of_slots() - 1;
	//puts("small page gen5 \n");
	//printf("record size = %d\n", record->size);
	update_buffer(record);
	//�������� ���ڵ� �߰�.
	//puts("small page gen6 \n");
	page->add_list_sp(*buffer);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::large_page_generate(std::ostream& os, const vertex_t& vertex, record_t* record)
{
	puts("why lp???? \n");
	//����ū ���������� üũ �غ��ߵ�. �ϴ� RID Table ���� �ذ��Ұ�.
	/* ���� �ʿ���.*/
	record_t* temp_record;
	size_t total_size = record->size;
	printf("total_size = %d \n", total_size);
	const size_t full_record_size = DataSectionSize - sizeof(vertex_t) - sizeof(record_t);
	printf("full_record_size = %d \n", full_record_size);
	size_t current_offset = 0;
	size_t remaining_data_size = record->size;
	puts("setting okay...");
	//std::string data(record->data, record->size);
	//free(record);
	//std::istream ifs(data.data(),ios::binary);
	size_t num_page = (total_size / full_record_size) + 1; // ������ + 1
	printf("num_page = %d\n", num_page);
	if (!page->is_empty())  // �������� �Ⱥ������ �� �������� �Ҵ��Ѵ�.
		issue_page(os, slotted_page_flag::SP);

	//___size_t required_ext_pages = static_cast<___size_t>(std::floor((num_edges - MaximumEdgesInHeadPage) / MaximumEdgesInExtPage)) + 1;

	// Processing a head page
	{
		puts("processing a head page");
		///constexpr ___size_t data_size = record->size; // ���ڵ� ��Ż ������
		vertex.to_slot(*page);// �������� ���̱� vertex;
		temp_record = (record_t *)malloc(sizeof(record_t) + full_record_size); // �޸� �Ҵ�
		temp_record->size = full_record_size; // �ӽ� ���ڵ忡 ������ �Ҵ� 
		printf("temp record size = %d \n", full_record_size);
		//ifs.read(temp_record->data, full_record_size); // ifs �б�
		std::string temp(record->data, full_record_size);
		puts("temp gen");
		memmove(temp_record->data, temp.c_str(), full_record_size);
		puts("memmove ~");
		current_offset += full_record_size; // ������ ������.
		puts("offset change");
		//delete &temp; // �����ش�.
		//puts("del temp");
		update_buffer(temp_record); // ���� ������Ʈ
		puts("update buffer");
		page->add_list_lp_head(full_record_size,*buffer); // �������� �ٿ��ֱ�
		puts("page->add_list_lp_head");
		remaining_data_size -= full_record_size; // ���� ������ ����
		printf("remaining_data_size = %d \n", remaining_data_size);
		issue_page(os, slotted_page_flag::LP_HEAD); // ������ �̽�
		puts("issue_page");
		issue_lp_head(rid_table, vertex.serial_id, num_page - 1); // rid table ������Ʈ.
		puts("issue_lp_head");

		delete temp_record; // �޸� ����.
		puts("temp_record del");
	//	void add_list_lp_head(___size_t record_size, adj_list_elem_t* elem_arr);
		/// Add list for extended part of Large pages (LP-ext)
	//	void add_list_lp_ext(adj_list_elem_t& elem_arr);

		//update_buffer(edges, num_edges_in_page); // ���ڵ� ������Ʈ
	//	page->add_list_lp_head(num_edges, list_buffer.data(), num_edges_in_page); // �������� �־��ش�.
		//issue_page(os, slotted_page_flag::LP_HEAD); // ������ ���θ���.
	}

	// Processing a extended pages
	///___size_t remained_edges = 0;//���� ���ڵ� ������ ������
	//___size_t offset = MaximumEdgesInHeadPage;
	{
		puts("Processing a extended pages");
		for (size_t i = 1; i <= num_page - 1; i++) {
			printf("i = %d  and num_page = %d \n", i,num_page);
			if (i == num_page - 1) {
				puts("last page ~~");
				vertex.to_slot(*page);// �������� ���̱� vertex;
				temp_record = (record_t *)malloc(sizeof(record_t) + remaining_data_size); // �޸� �Ҵ�
				temp_record->size = remaining_data_size; // �ӽ� ���ڵ忡 ������ �Ҵ� 
				//ifs.read(temp_record->data, remaining_data_size); // ifs �б�
				std::string temp(record->data+current_offset, remaining_data_size);
				memmove(temp_record->data, temp.c_str(), remaining_data_size);
				printf("current_offset = %d \n", current_offset);
				current_offset += remaining_data_size; // ������ ������.
				printf("current_offset = %d \n", current_offset);
				printf("remaining_data_size = %d \n", remaining_data_size);

				if (current_offset == total_size) puts("Okay... perfect!");
				//delete &temp; // �����ش�.
				update_buffer(temp_record); // ���� ������Ʈ
				puts("okay..");
				page->add_list_lp_ext(*buffer); // �������� �ٿ��ֱ�
				puts("yes..");
				//remaining_data_size -= full_record_size; // ���� ������ ����
				issue_page(os, slotted_page_flag::LP_EXTENDED); // ������ �̽�
				puts("dd??");
				issue_lp_ext(rid_table, vertex.serial_id, num_page - 1); // rid table ������Ʈ.
				puts("hmm");
				delete temp_record; // �޸� ����.
				puts("Okay end iter...");
				break;
			}
			puts("ext page gen start..");
			vertex.to_slot(*page);// �������� ���̱� vertex;
			puts("vertex.to_slot");
			temp_record = (record_t *)malloc(sizeof(record_t) + full_record_size); // �޸� �Ҵ�
			puts("mallc temp_record");
			temp_record->size = full_record_size; // �ӽ� ���ڵ忡 ������ �Ҵ� 
			
												  
			//ifs.read(temp_record->data, full_record_size); // ifs �б�

			std::string temp(record->data+current_offset, full_record_size);
			puts("temp gen");
			
			memmove(temp_record->data, temp.c_str(), full_record_size);
			puts("memmove");
			current_offset += full_record_size; // ������ ������.
			printf("current_offset = %d \n", current_offset);
			//delete &temp; // �����ش�.

			update_buffer(temp_record); // ���� ������Ʈ
			page->add_list_lp_ext(*buffer); // �������� �ٿ��ֱ�
			remaining_data_size -= full_record_size; // ���� ������ ����
			printf("remaing_data_size = %d \n", remaining_data_size);
			issue_page(os, slotted_page_flag::LP_EXTENDED); // ������ �̽�

			delete temp_record; // �޸� ����.

		}
		// ���������� �󸶳� ���°�? ( �ִ�ġ�� �ִ��� : ���� �뷮��ŭ �ִ��� )
//		___size_t num_edges_per_page = (remained_edges >= MaximumEdgesInExtPage) ? MaximumEdgesInExtPage : remained_edges; 
	//	vertex.to_slot_ext(*page); // �������� ���� �ھƳֱ�
//		update_buffer(edges + offset, num_edges_per_page); // ���ڵ� ������Ʈ
	//	page->add_list_lp_ext(list_buffer.data(), num_edges_per_page); // �߰�
	//	offset += num_edges_per_page; // 
	//	remained_edges -= num_edges_per_page;
		
	}
	puts("Okay end func");
}

/*
PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::small_page_iteration(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
	auto scan_result = page->scan();
	bool& slot_available = scan_result.first;
	auto& capacity = scan_result.second;

	if (!slot_available || (capacity < num_edges))
		issue_page(os, slotted_page_flag::SP);

	vertex.to_slot(*page);

	if (num_edges == 0)
		return;

	auto offset = page->number_of_slots() - 1;
	update_list_buffer(edges, num_edges);

	page->add_list_sp(offset, list_buffer.data(), num_edges);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::large_page_iteration(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
	if (!page->is_empty())
		issue_page(os, slotted_page_flag::SP);

	//___size_t required_ext_pages = static_cast<___size_t>(std::floor((num_edges - MaximumEdgesInHeadPage) / MaximumEdgesInExtPage)) + 1;

	// Processing a head page
	{
		constexpr ___size_t num_edges_in_page = MaximumEdgesInHeadPage;
		vertex.to_slot(*page);
		update_list_buffer(edges, num_edges_in_page);
		page->add_list_lp_head(num_edges, list_buffer.data(), num_edges_in_page);
		issue_page(os, slotted_page_flag::LP_HEAD);
	}

	// Processing a extended pages
	___size_t remained_edges = num_edges - MaximumEdgesInHeadPage;
	___size_t offset = MaximumEdgesInHeadPage;
	{
		___size_t num_edges_per_page = (remained_edges >= MaximumEdgesInExtPage) ? MaximumEdgesInExtPage : remained_edges;
		vertex.to_slot_ext(*page);
		update_list_buffer(edges + offset, num_edges_per_page);
		page->add_list_lp_ext(list_buffer.data(), num_edges_per_page);
		offset += num_edges_per_page;
		remained_edges -= num_edges_per_page;
		issue_page(os, slotted_page_flag::LP_EXTENDED);
	}
}
*/
PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::issue_page(std::ostream& os, page_flag_t flags)
{
	page->flags() = flags;
	builder_t* raw_ptr = page.get();
	os.write(reinterpret_cast<char*>(raw_ptr), PageSize);
	page->clear();
	//printf("new page[%d]\n", num_pages);
	++num_pages;
}
/*
PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::update_list_buffer(edge_t* edges, ___size_t num_edges)
{
list_buffer.clear();
adj_list_elem_t elem;
for (___size_t i = 0; i < num_edges; ++i)
{
edges[i].template to_adj_elem<builder_t>(rid_table, &elem);
list_buffer.push_back(elem);
}
}
*/

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::update_buffer(record_t* record)
{
	//puts("buffer update \n");
	//adj_list_elem_t* buf;
	std::ifstream ifs(record->data, ios::binary);

/* ok clear
char test2[256];
sprintf_s(test2, "test%d.jpeg", rand() % 10000); // test
printf("%s\n", test2);
std::ofstream ofs(test2, ios::trunc | ios::binary);// test
ofs.write(reinterpret_cast<const char*>(record->data), record->size);

*/
	

	buffer = (adj_list_elem_t*)malloc(sizeof(adj_list_elem_t) + record->size); // �� ����
	memset(buffer, 0, sizeof(adj_list_elem_t) + record->size); // �ʱ�ȭ
	//printf("buffer~~ record = %d\n",record->size); // ���
	buffer->data_size = record->size; // ���� ������� ���ڵ� ũ��.
	memmove(buffer->payload, record->data, record->size);
	//printf("buf size = %d\n", buffer->data_size);
	//ifs.read(reinterpret_cast<char*>(buffer->payload), buffer->data_size);
	
	//puts("testt\n");
	//buffer = buf;
//	puts("end update \n");

	//it is create pic test
	/*
	char test2[256];
	sprintf_s(test2, "test%d.jpeg",rand()%10000); // test
	printf("%s\n", test2);
	std::ofstream ofs(test2, ios::trunc | ios::binary);// test
	ofs.write(reinterpret_cast<const char*>(buffer->payload), buffer->data_size);// test
	ofs.close();
	*/
	
	
	
	//return *buf;
	
}

template <typename PageTy, typename RIDTuplePayloadTy = std::size_t, template <typename _ElemTy, typename = std::allocator<_ElemTy> > class RIDContainerTy = std::vector>
struct generator_traits {
	using page_t = PageTy;
	using page_traits = page_traits<PageTy>;
	using rid_table_generator_t = rid_table_generator<PageTy, RIDTuplePayloadTy, RIDContainerTy>;
	using rid_tuple_t = typename rid_table_generator_t::rid_tuple_t;
    using rid_table_t = typename rid_table_generator_t::rid_table_t;
	using pagedb_generator_t = pagedb_generator<typename page_traits::page_builder_t, typename rid_table_generator_t::rid_table_t>;
};
/*
template <typename RIDTableTy>
void write_rid_table(RIDTableTy& rid_table, std::ostream& os)
{
for (auto& tuple : rid_table) {
os.write(reinterpret_cast<char*>(&tuple.start_sid), sizeof(tuple.start_sid));
os.write(reinterpret_cast<char*>(&tuple.auxiliary), sizeof(tuple.auxiliary));
}
}
*/


template <typename PageTy>
void print_page(PageTy& page, FILE* out_file = stdout)
{
	fprintf(out_file, "number of slots in the page: %llu\n", static_cast<std::uint64_t>(page.number_of_slots()));
	fprintf(out_file, "page type: %s\n",
		(page.is_sp()) ?
		"small page" : (page.is_lp_head()) ?
		"large page (head)" : "large page (extended)");
	for (int i = 1; i <= PageTy::PageSize; ++i) {
		fprintf(out_file, "0x%02llX ", static_cast<std::uint64_t>(page[i - 1]));
		if (i % 8 == 0)
			fprintf(out_file, "\n");
	}
}

template <typename RIDTableTy>
void print_rid_table(RIDTableTy& rid_table, FILE* out_file = stdout)
{
	for (auto& tuple : rid_table)
		fprintf(out_file, "%llu\t|\t%llu\n", static_cast<std::int64_t>(tuple.start_sid), static_cast<std::uint64_t>(tuple.auxiliary));
}

#undef PAGEDB_GENERATOR
#undef PAGEDB_GENERATOR_TEMPALTE

} // !nameaspace mdf

#endif // !_MDF_DATATYPE_PAGEDB_H_
