#ifndef _LibMDF_PROTOBUF_DATA_PROCESS_H_
#define _LibMDF_PROTOBUF_DATA_PROCESS_H_


namespace protobuf {
	/** @brief protobin ������ ����� �Լ�.
	* filename : protobin ������ �̸�
	* size : ������ ����
	*/
	void createProtoBin(const char* filename, size_t size);

	/** @brief protobin ������ �д� �Լ�.
	* filename : protobin ������ �̸�
	* 
	*/
	void readProtoBin(const char* filename);

	void generateProtoPageDB(const char* filename, const char* DB_name);
	void readProtoPageDB(const char* DB_name);
	/** @brief protoPageDB�� protobin ���Ϸ� ��ȯ�մϴ�.
	* DB_name : pageDB�� �̸�
	* filename : protobin ������ �̸�
	*/
	void protoPageDBtoProtoBin(const char* DB_name, const char* filename);
}





#endif