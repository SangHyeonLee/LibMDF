#ifndef _LibMDF_PROTOBUF_DATA_PROCESS_H_
#define _LibMDF_PROTOBUF_DATA_PROCESS_H_


namespace protobuf {
	/** @brief Create protobin file using protobuf 
	* filename : Protobin file name
	* size : Data size
	*/
	void createProtoBin(const char* filename, size_t size);

	/** @brief Read protobin file
	* filename : Protobin file name
	*/
	void readProtoBin(const char* filename);

	/** @brief Generate Proto PageDB
	* filename : Protobin file name to read
	* DB_name : PageDB name to be generated
	*/
	void generateProtoPageDB(const char* filename, const char* DB_name);

	/** @brief Read and print Proto PageDB  
	* DB_name : PageDB name to read
	*/
	void readProtoPageDB(const char* DB_name);

	/** @brief Convert protoPageDB to protobin file
	* DB_name : Proto pageDB to read
	* filename : Protobin file name to be generated
	*/
	void protoPageDBtoProtoBin(const char* DB_name, const char* filename);
}





#endif