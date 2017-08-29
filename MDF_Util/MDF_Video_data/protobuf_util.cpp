#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

#include <mdf\datatype\pagedb.h>
#include "addressbook.pb.h"


/*
해당 Util은 ProtoBuf를 기반으로 한 예제이며
Google Protocol Buffer 사이트의 C++ 예제를 활용하였습니다.
본 예제를 참고하여 PageDB에 적용하기 위해서는 사용하시는 Proto 파일을 적용하여 사용하시면 됩니다.

*/

using namespace std;
//using namespace protobuf;
namespace protobuf {
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



	void PromptForAddress(Person* person, int id) {

		person->set_id(id);
		cout << "Enter person ID number: " << id << endl;

		cout << "Enter name: ";

		string name;
		name.append("test" + to_string(id));
	
		*person->mutable_name() = name.c_str();
		cout << "Enter name: " << name.c_str() << endl;
		string email;
		email.append(name.c_str());
		email.append("@test.com");
		cout << "Enter email address (blank for none): " << email.c_str() << endl;

		if (!email.empty()) {
			person->set_email(email);
		}


		while (true) {


			int num = rand() % 30000;
			string number = to_string(num);



			Person::PhoneNumber* phone_number = person->add_phones();
			phone_number->set_number(number);
			cout << "Enter a phone number (or leave blank to finish): " << number << endl;


			size_t what_type = rand() % 3;


			switch (what_type)
			{
			case 0:
				phone_number->set_type(Person::MOBILE);
				cout << "Is this a mobile, home, or work phone? " << "Mobile" << endl;
				break;
			case 1:
				phone_number->set_type(Person::HOME);
				cout << "Is this a mobile, home, or work phone? " << "Home" << endl;
				break;
			case 2:
				phone_number->set_type(Person::WORK);
				cout << "Is this a mobile, home, or work phone? " << "Work" << endl;
				break;
			default:
				cout << "Unknown phone type.  Using default." << endl;
				break;
			}

			size_t goOrStop = rand() % 4;
			if (goOrStop > 0) {
				cout << "goOrStop is " << goOrStop << "  Stop " << endl;
				break;
			}
		}
	}
	/** @brief size 만큼의 데이터의 protoBin 파일을 filename으로 생성.
	*
	*/
	void createProtoBin(const char* filename, size_t size) {
		// Verify that the version of the library that we linked against is
		// compatible with the version of the headers we compiled against.
		GOOGLE_PROTOBUF_VERIFY_VERSION;
		srand((unsigned int)time(NULL));
		
		//char* filename = "myPhonebook.bin";
		AddressBook address_book;
		

		int id = 0;
		

#if 0 // must overwrite..
		{
			// Read the existing address book.

			fstream input(filename, ios::in | ios::binary );
			if (!input) {
				cout << filename << ": File not found.  Creating a new file." << endl;
			}
			else if (!address_book.ParseFromIstream(&input)) {
				cerr << "Failed to parse address book." << endl;
				return;

			}
		}
#endif
		// Add an address.
		for (id = 0; id < size; id++) {
			PromptForAddress(address_book.add_people(), id);
		}
		{
			// Write the new address book back to disk.
			fstream output(filename, ios::out | ios::trunc | ios::binary);
			if (!address_book.SerializeToOstream(&output)) {
				cerr << "Failed to write address book." << endl;
				return;
			}
		}

		// Optional:  Delete all global objects allocated by libprotobuf.
		google::protobuf::ShutdownProtobufLibrary();

	}

	void ListPeople(const AddressBook& address_book) {
		for (int i = 0; i < address_book.people_size(); i++) {
			const Person& person = address_book.people(i);

			cout << "Person ID: " << person.id() << endl;
			cout << "  Name: " << person.name() << endl;

			if (person.email().size() > 0) {
				cout << "  E-mail address: " << person.email() << endl;
			}

			for (int j = 0; j < person.phones_size(); j++) {
				const Person::PhoneNumber& phone_number = person.phones(j);

				switch (phone_number.type()) {
				case Person::MOBILE:
					cout << "  Mobile phone #: ";
					break;
				case Person::HOME:
					cout << "  Home phone #: ";
					break;
				case Person::WORK:
					cout << "  Work phone #: ";
					break;
				}
				cout << phone_number.number() << endl;
			}
		}
	}

	void readProtoBin(const char* filename) {
		GOOGLE_PROTOBUF_VERIFY_VERSION;
		//char* filename = "myPhonebook.bin";

		AddressBook address_book;

		{
			// Read the existing address book.
			fstream input(filename, ios::in | ios::binary);
			if (!address_book.ParseFromIstream(&input)) {
				cerr << "Failed to parse address book." << endl;
				return;
			}
		}
		//address_book.people(0).SerializeAsString();
		ListPeople(address_book);

		// Optional:  Delete all global objects allocated by libprotobuf.
		google::protobuf::ShutdownProtobufLibrary();
	}
	std::pair< page_traits::record_t*, page_traits::vertex_t> read_proto(AddressBook address_book, page_traits::serial_id_t &sid) {
		page_traits::record_t* record;
		page_traits::vertex_t vertex;
		
		
		size_t record_size = 0;
		

		if (sid < address_book.people_size()) {
			//사이즈가 더 크면.
			//printf("sid =%d \n", sid);
		}
		else { // EOF
			record = NULL; // Record를 NULL로 설정
			return std::make_pair(record, vertex); // EOF
												   //printf("EOF \n"); 
		}
		

		printf("sid = %d\n", sid);
		const Person& person = address_book.people(sid);
		printf("people num = %d id = %d \n", sid, person.id());
		string peopleData = person.SerializeAsString();
		 
		record_size = peopleData.size();


	
		
		vertex.serial_id = sid++;

		//레코드 메모리 할당
		record = (page_traits::record_t *)malloc(sizeof(page_traits::record_t) + peopleData.size());
		
		//데이터 쓰기.
		memmove(record->data, peopleData.c_str(), record_size);
		//레코드 사이즈를 넣어줌
		record->size = record_size;
		printf("size %d \n", record_size);

		return std::make_pair(record, vertex);
	}
	void generateProtoPageDB(const char* filename, const char* DB_name) {
		/* begin */
		puts("@ Image DataSet - PageDB Geneartion\n");
		/* init */
		AddressBook address_book;
		{
			// Read the existing address book.
			fstream input(filename, ios::in | ios::binary);
			if (!address_book.ParseFromIstream(&input)) {
				cerr << "Failed to parse address book." << endl;
				return;
			}
		}

		page_traits::serial_id_t sid = 0;
		
		// Generate pageDB 
		generator_traits::pagedb_generator_t pagedb_generator{ mdf::generate_rid_table<generator_traits::rid_tuple_t, std::vector>() };
		pagedb_generator.generateDB(std::bind(read_proto, std::ref(address_book), std::ref(sid)), DB_name);
		pagedb_generator.commit();
		
	}
	/** @brief 해당 이름의 pageDB와 rid_Table을 로드합니다.
	*
	*/
	void get_pageDB(const char* DB_name, rid_table_t& rtable, page_cont_t& pages) {
		char rid_name[256];
		char page_name[256];
		sprintf_s(rid_name, "%s.rid_table", DB_name);
		sprintf_s(page_name, "%s.pages", DB_name);
		/* Read RID Table */
		rtable = mdf::read_rid_table<rid_tuple_t, std::vector>(rid_name);
		/* Read pageDB */
		pages = mdf::read_pages<page_t, std::vector>(page_name);
	}
	void readProtoPageDB(const char* DB_name) {
		rid_table_t rtable;
		page_cont_t pages;
		AddressBook address_book;
		get_pageDB(DB_name, rtable, pages); // get pageDB

		for (size_t i = 0; i < pages.size(); i++) {
			/* get slot size*/
			size_t num = pages[i].number_of_slots();
			printf("Page ID = %d and Slot Size = %d \n", i, num);
			/* iterate slot size */
			for (size_t j = 0; j < num; j++) {
				auto record = pages[i].record_start(pages[i].slot(j));
				//string data(record->payload, record->data_size);
				address_book.add_people()->ParseFromArray(record->payload, record->data_size);
			}
			printf("page[%d] print end \n", i);

		}
		ListPeople(address_book);
	}


	void protoPageDBtoProtoBin(const char* DB_name, const char* filename) {
		rid_table_t rtable;
		page_cont_t pages;
		
		get_pageDB(DB_name, rtable, pages);
		GOOGLE_PROTOBUF_VERIFY_VERSION;
		puts("start protoPageDBtoProtoBin\n");
		AddressBook address_book;

		for (size_t i = 0; i < pages.size(); i++) {
			/* get slot size*/
			size_t num = pages[i].number_of_slots();
			
			/* iterate slot size */
			for (size_t j = 0; j < num; j++) {
				auto record = pages[i].record_start(pages[i].slot(j));
				address_book.add_people()->ParseFromArray(record->payload, record->data_size);
			}

		}

		{
			// Write the new address book back to disk.
			fstream output(filename, ios::out | ios::trunc | ios::binary);
			if (!address_book.SerializeToOstream(&output)) {
				cerr << "Failed to write address book." << endl;
				return;
			}
		}

		// Optional:  Delete all global objects allocated by libprotobuf.
		google::protobuf::ShutdownProtobufLibrary();
		puts("Complete... \n");
	}

}