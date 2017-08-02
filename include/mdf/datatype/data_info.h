/*
* Under development...
*
*
*
*
*
*/

#ifndef _MDF_DATATYPE_DATA_INFO_H_
#define _MDF_DATATYPE_DATA_INFO_H_

#include <memory>
#include <iostream>

using namespace std;
namespace dataInfo {
#pragma pack(push, 1)
	struct imageInfo {
		uint16_t label;
		uint16_t channel;
		uint32_t width;
		uint32_t height;
		std::string ext_name;

		void init() {
			label = 0;
			channel = 0;
			width = 0;
			height = 0;
			ext_name.clear();
		}
		void set_info(uint16_t label, uint16_t channel, uint32_t width, uint32_t height, std::string ext_name) {
			this->label = label;
			this->channel = channel;
			this->width = width;
			this->height = height;
			this->ext_name = ext_name;
		}
		void set_label() {

		}
		bool is_null() {
			return this == NULL;
		}
		void get_info(imageInfo data) {
			this->label = data.label;
			this->channel = data.channel;
			this->width = data.width;
			this->height = data.height;
			this->ext_name = data.ext_name;
		}
		void print_info() {
			printf("label %d channel %d height %d width %d \n", this->label, this->channel, this->height, this->width);

		}
	};


	struct record {
		dataInfo::imageInfo imageinfo;
		size_t data_size;
		char data[1];
		void assign(string filepath) {

			if (data_size == 0) return;
			memmove(&data[0], filepath.data(), data_size);
		}
		void init() {
			imageinfo.init();
			data_size = 0;
			memset(data, 0, data_size);
		}

	};
#pragma pack(pop)
}


#endif