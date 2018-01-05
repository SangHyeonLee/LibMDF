#ifndef _LibMDF_VIDEO_DATA_PROCESS_H_
#define _LibMDF_VIDEO_DATA_PROCESS_H_

namespace video_util {

	/** @brief Generate PageDB
	* filename : Large data file to read at least size 64MB
	* DB_name : PageDB name to be generated
	*/
	void generatePageDB(const char* filepath, const char* pageDB_name);
	/** @brief PageDB convert to law data (in this function use mkv extension)
	* DB_name : PageDB name to read 
	* filename : Law data name to be generated
	*/
	void PageDBtoMkv(const char* DB_name, const char* filename);
	/** @brief Generate PageDB using directory for many files
	* dirPath : Directory path containing many files
	* DB_name : PageDB name to be generated
	*/
	void generatePageDB_dir(const char* dirPath, const char* pageDB_name);
	/** @brief Get data size of the corresponding sid
	* DB_name : PageDB name to read
	* sid : index of sid
	*/
	void getDataSize(const char* DB_name, size_t sid);
	/** @brief Print RID Table information
	* DB_name : PageDB name to be generated
	*/
	void printRIDtable(const char* DB_name);
}

#endif