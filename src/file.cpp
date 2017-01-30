#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cstring>

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


const std::vector<uint32_t> FileToU32Vec(const std::string inFile)
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
	if(!contentStr.size())
	{
		std::cout << inFile << " is empty!\n";
		// return false;
	}
	if(contentStr.size() % 4 != 0)
	{
		std::cout << inFile << " isn't a multiple of 4 bytes!";
		// return false;
	}
	std::vector<uint32_t> contentVec(contentStr.size() / 4);
	std::memcpy(contentVec.data(), contentStr.data(), contentStr.size());

	return contentVec;
}


// const bool FileToU32Vec(const std::string inFile, std::vector<uint32_t> &contentVec)
// {
	// std::ifstream iFile(inFile.c_str(), std::ios::in | std::ios::binary);
	// if(iFile.is_open() == false)
	// {
		// std::cout << "File not found\n";
		// return false;
	// }

	// std::ostringstream contents;
    // contents << iFile.rdbuf();
	// iFile.close();

	// const std::string contentStr = contents.str();
	// if(!contentStr.size())
	// {
		// std::cout << inFile << " is empty!\n";
		// return false;
	// }
	// if(contentStr.size() % 4 != 0)
	// {
		// std::cout << inFile << " isn't a multiple of 4 bytes!\n";
		// return false;
	// }

	// contentVec.resize(contentStr.size() / 4);
	// std::memcpy(contentVec.data(), contentStr.data(), contentStr.size());

	// return true;
// }
