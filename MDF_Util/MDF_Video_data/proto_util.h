#ifndef _LibMDF_PROTOBUF_DATA_PROCESS_H_
#define _LibMDF_PROTOBUF_DATA_PROCESS_H_


namespace protobuf {
	/** @brief protobin 파일을 만드는 함수.
	* filename : protobin 파일의 이름
	* size : 데이터 갯수
	*/
	void createProtoBin(const char* filename, size_t size);

	/** @brief protobin 파일을 읽는 함수.
	* filename : protobin 파일의 이름
	* 
	*/
	void readProtoBin(const char* filename);

	void generateProtoPageDB(const char* filename, const char* DB_name);
	void readProtoPageDB(const char* DB_name);
	/** @brief protoPageDB를 protobin 파일로 변환합니다.
	* DB_name : pageDB의 이름
	* filename : protobin 파일의 이름
	*/
	void protoPageDBtoProtoBin(const char* DB_name, const char* filename);
}





#endif