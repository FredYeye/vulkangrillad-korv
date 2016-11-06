#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "file.hpp"


const std::vector<uint8_t> FileToU8Vec(const std::string inFile)
{
	std::ifstream iFile(inFile.c_str(), std::ios::in | std::ios::binary);
	if(iFile.is_open() == false)
	{
		std::cout << "File not found" << std::endl;
		exit(0);
	}

	std::ostringstream contents;
    contents << iFile.rdbuf();
	iFile.close();

	const std::string contentStr = contents.str();
	const std::vector<uint8_t> contentVec(contentStr.begin(), contentStr.end());

	return contentVec;
}
