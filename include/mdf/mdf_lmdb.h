#ifndef _MDF_LMDB_H_
#define _MDF_LMDB_H_

#include <lmdb.h> // need lmdb.h lib
#include <cstdio>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <string>
#include <sstream>
#include <direct.h>
//#include "datatype\pagedb.h"
#include <functional>



#define DISABLE_COPY_AND_ASSIGN(classname) \
private:\
  classname(const classname&);\
  classname& operator=(const classname&)

#define E(expr) CHECK((rc = (expr)) == MDB_SUCCESS, #expr)
#define RES(err, expr) ((rc = expr) == (err) || (CHECK(!rc, #expr), 0))
#define CHECK(test, msg) ((test) ? (void)0 : ((void)fprintf(stderr, \
	"%s:%d: %s: %s\n", __FILE__, __LINE__, msg, mdb_strerror(rc)), abort()))

using namespace std;

namespace mdf {
	
	namespace lmdb {
		constexpr size_t SIZE_1KB = 1024u;
		constexpr size_t SIZE_1MB = SIZE_1KB * 1024u;
		constexpr size_t SIZE_1GB = SIZE_1MB * 1024u;
		
		using data_pair = std::pair<size_t /* size */, char* /* data */>; //반환값 
		using data_pair_func = std::function<data_pair()>; // 함수
		enum Mode { READ, WRITE, NEW };
		//void generate_lmdb(data_pair_func data_func, const char* path);
		class Cursor {
		public:
			explicit Cursor(MDB_txn* mdb_txn, MDB_cursor* mdb_cursor)
				: mdb_txn_(mdb_txn), mdb_cursor_(mdb_cursor), valid_(false) {
				SeekToFirst();
			}
			~Cursor() {
				mdb_cursor_close(mdb_cursor_);
				mdb_txn_abort(mdb_txn_);
			}
			void SeekToFirst() { Seek(MDB_FIRST); }
			void Next() { Seek(MDB_NEXT); }
			string key_str_t() {
				return string(static_cast<const char*>(mdb_key_.mv_data), mdb_key_.mv_size);
			}

			size_t& key_size_t() {
				return *(size_t *)mdb_key_.mv_data;
			}
			size_t value_size() {
				return static_cast<size_t>(mdb_value_.mv_size);
			}
			string value_str_t() {
				return string(static_cast<const char*>(mdb_value_.mv_data),
					mdb_value_.mv_size);
			}
			char& value_char_t() {
				return *reinterpret_cast<char *>(&mdb_value_.mv_data);
			}
			bool valid() { return valid_; }

		private:
			void Seek(MDB_cursor_op op) {
				int mdb_status = mdb_cursor_get(mdb_cursor_, &mdb_key_, &mdb_value_, op);
				if (mdb_status == MDB_NOTFOUND) {
					valid_ = false;
				}
				else {
					E(mdb_status);
					valid_ = true;
				}
			}

			int rc;
			MDB_txn* mdb_txn_;
			MDB_cursor* mdb_cursor_;
			MDB_val mdb_key_, mdb_value_;
			bool valid_;

			DISABLE_COPY_AND_ASSIGN(Cursor);
		};
		
		class Transaction {
		public:
			explicit Transaction(MDB_env* mdb_env)
				: mdb_env_(mdb_env) { }
			void Put(const string& key, const string& value);
			void ChangeAddMapSize(size_t size);
			void AddMapSize();
			void Commit();

		private:
			bool flag = true;
			int rc;
			MDB_env* mdb_env_;
			vector<string> keys, values;
			size_t addMapSize = SIZE_1MB * 64; /* Default add size 64MB */
			size_t total_size = 0;
			void setMapSize();

			DISABLE_COPY_AND_ASSIGN(Transaction);



		};
		void Transaction::ChangeAddMapSize(size_t size) {
			addMapSize = size;
		}
		void Transaction::Put(const string& key, const string& value) {
			keys.push_back(key);
			values.push_back(value);
			total_size += sizeof(MDB_val) * 2 + key.size() + value.size();
		}
		void Transaction::Commit() {
			
			MDB_dbi mdb_dbi;
			MDB_val mdb_key, mdb_data;
			MDB_txn *mdb_txn;
			///if (flag) {
			///	setMapSize();
			///	flag = false;
			///}
			// Initialize MDB variables
			E(mdb_txn_begin(mdb_env_, NULL, 0, &mdb_txn));
			E(mdb_dbi_open(mdb_txn, NULL, 0, &mdb_dbi));
		
			
			for (int i = 0; i < keys.size(); i++) {
				mdb_key.mv_size = keys[i].size();
				mdb_key.mv_data = const_cast<char*>(keys[i].data());
				mdb_data.mv_size = values[i].size();
				mdb_data.mv_data = const_cast<char*>(values[i].data());
				
				// Add data to the transaction
				int put_rc = mdb_put(mdb_txn, mdb_dbi, &mdb_key, &mdb_data, 0);
				if (put_rc == MDB_MAP_FULL) {
					// Out of memory - double the map size and retry
					mdb_txn_abort(mdb_txn);
					mdb_dbi_close(mdb_env_, mdb_dbi);
					AddMapSize();
					Commit();
					return;
				}
				// May have failed for some other reason
				E(put_rc);
			}

			// Commit the transaction
			int commit_rc = mdb_txn_commit(mdb_txn);
			if (commit_rc == MDB_MAP_FULL) {
				// Out of memory - double the map size and retry
				mdb_dbi_close(mdb_env_, mdb_dbi);
				AddMapSize();
				Commit();
				return;
			}
			// May have failed for some other reason
			E(commit_rc);

			// Cleanup after successful commit
			mdb_dbi_close(mdb_env_, mdb_dbi);
			keys.clear();
			values.clear();
			puts("LMDB Commit Complete.. \n");
		}
		void Transaction::setMapSize() {
			struct MDB_envinfo current_info;
			
			//size_t new_size = current_info.me_mapsize + total_size;

			//DLOG(INFO) << "Doubling LMDB map size to " << (new_size >> 20) << "MB ...";
			E(mdb_env_set_mapsize(mdb_env_, total_size));
			printf("mapsize = %d\n", total_size/SIZE_1MB);
			//puts("mapsize increase!~~\n");
		}
		
		void Transaction::AddMapSize() {
			struct MDB_envinfo current_info;
			
			E(mdb_env_info(mdb_env_, &current_info));
			
			size_t new_size = current_info.me_mapsize + addMapSize;
			


			//DLOG(INFO) << "Doubling LMDB map size to " << (new_size >> 20) << "MB ...";
			E(mdb_env_set_mapsize(mdb_env_, new_size));
			printf("mapsize = %d\n", new_size/SIZE_1MB );
			//puts("mapsize increase!~~\n");
		}
		

		class LMDB {
		public:
			LMDB() : mdb_env_(NULL) { }
			~LMDB() { Close(); }
			void Open(const char* source, Mode mode);
			void Close() {
				if (mdb_env_ != NULL) {
					mdb_dbi_close(mdb_env_, mdb_dbi_);
					mdb_env_close(mdb_env_);
					mdb_env_ = NULL;
				}
			}
			Cursor* NewCursor();
			Transaction* NewTransaction();
			void get_stat();

		private:
			int rc;
			MDB_env* mdb_env_;
			MDB_dbi mdb_dbi_;
			

			DISABLE_COPY_AND_ASSIGN(LMDB);
		};
		void LMDB::get_stat() {
			MDB_stat stat;
			mdb_env_stat(mdb_env_, &stat);
			
			auto dbSize = stat.ms_psize * (stat.ms_leaf_pages + stat.ms_branch_pages + stat.ms_overflow_pages);
			printf("ms_psize = %d\n", stat.ms_psize);
			printf("ms_leaf_pages = %d\n", stat.ms_leaf_pages);
			printf("ms_branch_pages = %d\n", stat.ms_branch_pages);
			printf("ms_overflow_pages = %d\n", stat.ms_overflow_pages);
			printf("overflow_pages size = %d\n", (stat.ms_overflow_pages*stat.ms_psize) / SIZE_1MB);
			printf("dbsize = %d\n", dbSize/SIZE_1MB);
			printf("depth =%d\n", stat.ms_depth);
			printf("total entries %d\n", stat.ms_entries);
		}
		void LMDB::Open(const char* source, Mode mode) {
			E(mdb_env_create(&mdb_env_));
			if (mode == NEW) {
				_mkdir(source);
				//CHECK_EQ(_mkdir(source.c_str()), 0) << "mkdir " << source << " failed";
			}
			int flags = 0;
			if (mode == READ) {
				flags = MDB_RDONLY | MDB_NOTLS;
			}

			if (mode == WRITE) {
				flags = MDB_WRITEMAP | MDB_NOTLS;
			}
			int rc_stat = mdb_env_open(mdb_env_, source, flags, 0664);
#ifndef ALLOW_LMDB_NOLOCK
			E(rc_stat);
#else
			if (rc_stat == EACCES) {
				//LOG(WARNING) << "Permission denied. Trying with MDB_NOLOCK ...";
				// Close and re-open environment handle
				mdb_env_close(mdb_env_);
				E(mdb_env_create(&mdb_env_));
				// Try again with MDB_NOLOCK
				flags |= MDB_NOLOCK;
				E(mdb_env_open(mdb_env_, source, flags, 0664));
			}
			else {
				E(rc_stat);
			}
#endif
			//LOG_IF(INFO, Caffe::root_solver()) << "Opened lmdb " << source;
		}
		Cursor* LMDB::NewCursor() {
			MDB_txn* mdb_txn;
			MDB_cursor* mdb_cursor;
			E(mdb_txn_begin(mdb_env_, NULL, MDB_RDONLY, &mdb_txn));
			E(mdb_dbi_open(mdb_txn, NULL, 0, &mdb_dbi_));
			E(mdb_cursor_open(mdb_txn, mdb_dbi_, &mdb_cursor));
			return new Cursor(mdb_txn, mdb_cursor);
		}

		Transaction* LMDB::NewTransaction() {
			return new Transaction(mdb_env_);
		}
		void generate_lmdb(data_pair_func data_func,size_t map_size ,const char* path) {
			puts("Start generate lmdb \n");
			//lmdb 생성 할것
			int i = 0, j = 0, rc;
			MDB_env *env;
			MDB_dbi dbi;
			MDB_val key, data;
			MDB_txn *txn;
			MDB_stat mst;
			MDB_cursor *cursor, *cur2;
			MDB_cursor_op op;
			puts("get func<data pair> \n");
			data_pair result = data_func();
			if (!result.first) {
				puts("do not have record\n");
				return;
			}
			puts("exist record... \n");
			size_t count = 0;
			size_t total_size = 0;
		

			// Setting lmdb..
			puts("Setting lmdb \n");
			E(mdb_env_create(&env));
			E(mdb_env_set_maxreaders(env, 1));
			E(mdb_env_set_mapsize(env, map_size));
			//printf("MapSize = %d\n", map_size);
			E(mdb_env_open(env, path, MDB_FIXEDMAP /*|MDB_NOSYNC*/, 0664));

			E(mdb_txn_begin(env, NULL, 0, &txn));
			E(mdb_dbi_open(txn, NULL, 0, &dbi));

			key.mv_size = sizeof(size_t);
			
			key.mv_data = &count;
			///size_t *count2 = (size_t *)malloc(sizeof(size_t));
			///&count2 = &count;
			///key.mv_data = count2;
			///key.mv_data = (size_t *)count;
			//memmove(key.mv_data,&count,key.mv_size);
			data.mv_size = result.first;
			total_size += result.first;
			data.mv_data = result.second;
			/*if (total_size > map_size) {
				printf("total size = %d , mapsize = %d\n", total_size, map_size);
				map_size *= 2;
				
				printf("MapSize = %d\n", map_size);
				E(mdb_env_set_mapsize(env, map_size));
			}*/
			do { // iterate to insert record into lmdb
				//넣고
				puts("Iterate func... \n");
				mdb_put(txn, dbi, &key, &data, MDB_NOOVERWRITE);

				count++;
				result = data_func();
				if (!result.first) {
					puts("EOF record\n");
					break;
				}
				//key.mv_size = count; // fixed size key size
				///size_t *count2 = (size_t *)malloc(sizeof(size_t));
				///count2 = &count;
				///key.mv_data = count2;
				///key.mv_data = (size_t *)count;
				
				
				key.mv_data = &count;
				//key.mv_data = (size_t *)malloc(sizeof(size_t));
				//memmove(key.mv_data, &count, key.mv_size);
				printf("first(data_size) = %d\n", result.first);
				data.mv_size = result.first;
				total_size += result.first;
				data.mv_data = result.second;
				/*if (total_size > map_size){
					printf("total size = %d , mapsize = %d\n", total_size, map_size);
					map_size *= 2;
					printf("MapSize = %d\n", map_size);
					E(mdb_env_set_mapsize(env, map_size));
				}*/
				

			} while (true);
			puts("End lmdb \n");
			E(mdb_txn_commit(txn));
			E(mdb_env_stat(env, &mst));
		}

		void get_lmdb_data(const char* path) {
			int i = 0, j = 0, rc;
			MDB_env *env;
			MDB_dbi dbi;
			MDB_val key, data;
			MDB_txn *txn;
			MDB_stat mst;
			MDB_cursor *cursor, *cur2;
			MDB_cursor_op op;
			size_t count = 0;
			E(mdb_env_create(&env));
			E(mdb_env_set_maxreaders(env, 1));
			
			E(mdb_env_open(env, path, MDB_FIXEDMAP /*|MDB_NOSYNC*/, 0664));
			puts("start print \n");

			E(mdb_txn_begin(env, NULL, MDB_RDONLY, &txn));
			E(mdb_dbi_open(txn, NULL, 0, &dbi));
			E(mdb_cursor_open(txn, dbi, &cursor));
			
			while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
				printf("count = %d   ", ++count);
				printf("key: %d %d\n",
					*(size_t*)key.mv_data, key.mv_size);
				
			} // 출력해주는놈 
			puts("why??\n");
			CHECK(rc == MDB_NOTFOUND, "mdb_cursor_get"); // 이거 머지?
			mdb_cursor_close(cursor); // 커서 닫고 
			mdb_txn_abort(txn); // 닫고 
		}
		
		
		
	}

}

#endif _MDF_LMDB_H_ // !_MDF_LMDB_H_