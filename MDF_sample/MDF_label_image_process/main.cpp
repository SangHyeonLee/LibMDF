
#include <iostream>
#include <stdio.h>
#include "util.h";

using namespace std;

void main() {
	puts("start main");
	string DB_name = "cifar10";
	string destPath = "./dest";
	//label_imageProcessing::cifar10_to_page_db();
	//label_imageProcessing::print_page(DB_name.c_str(), destPath.c_str());
	label_imageProcessing::print_label_page(DB_name.c_str(), destPath.c_str());
	puts("end main");
	//stbi_write_png(DB_name.c_str(), 32, 32, 3, (const void *)destPath.c_str(), 1024);
	//cv::Mat image888(32,32,CV_8UC3,cvScalar((0,1024),(1025,2048),(2049,3072)));
	system("pause");

}