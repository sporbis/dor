#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <openssl/md5.h>
#include <string>

namespace fs = std::filesystem;

std::string read_file(fs::path path)
{
	std::ifstream f(path, std::ios::in | std::ios::binary);
	const auto size = fs::file_size(path);
	std::string result(size, '\0');
	f.read(result.data(), size);
	return result;
}

std::string generate_hash(std::string filename)
{
	FILE* inFile = fopen(filename.c_str(), "rb");

	if (inFile == NULL) {
		printf("%s can't be opened.\n", filename.c_str());
		return NULL;
	}

	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	unsigned char digest[MD5_DIGEST_LENGTH];

	MD5_Init(&mdContext);
	while ((bytes = fread(data, 1, 1024, inFile)) != 0) {
		MD5_Update(&mdContext, data, bytes);
	}
	MD5_Final(digest, &mdContext);

	fclose(inFile);

	char md5string[33];
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(&md5string[i * 2], "%02x", (unsigned int)digest[i]);
	}

	return std::string(md5string);
}

void create_checksum(std::string target)
{
	int totalCount = 0;
	int okCount = 0;

	for (const auto& entry : fs::recursive_directory_iterator(target)) {
		fs::path path = entry.path();
		std::string filename = path.filename().u8string();

		if (fs::is_regular_file(entry) && filename[0] != '.' && path.extension() != ".md5") {
			std::string md5File = path.string() + ".md5";

			if (fs::exists(md5File)) {
				std::cout << path.string() << " SKIPPED" << std::endl;
			}
			else {
				std::ofstream outfile(md5File);
				outfile << generate_hash(path.u8string());
				outfile.close();

				okCount++;
				std::cout << path.string() << " OK" << std::endl;
			}

			totalCount++;
		}
	}

	std::cout << "********************" << std::endl;
	std::cout << "OK: " << okCount << std::endl;
	std::cout << "Skipped: " << totalCount - okCount << std::endl;
	std::cout << "********************" << std::endl;
}

void verify(std::string target)
{
	int totalCount = 0;
	int okCount = 0;

	for (const auto& entry : fs::recursive_directory_iterator(target)) {
		if (fs::is_regular_file(entry) && entry.path().extension() == ".md5") {
			fs::path md5File = entry.path();
			fs::path targetFile = md5File.parent_path() / md5File.stem();
			std::string md5String = generate_hash(targetFile.u8string());
			std::string originalMd5 = read_file(md5File);

			if (originalMd5 == md5String) {
				okCount++;
				std::cout << targetFile << " OK" << std::endl;
			}
			else {
				std::cout << targetFile << "FAILED" << std::endl;
			}

			totalCount++;
		}
	}

	std::cout << "********************" << std::endl;
	std::cout << "OK: " << okCount << std::endl;
	std::cout << "Failed: " << totalCount - okCount << std::endl;
	std::cout << "********************" << std::endl;
}

int main(int argc, char* argv[])
{
	cxxopts::Options options("dor", "A command line program that validates and checks files for data corruption.");

	options.add_options()
		("c,create", "Create checksum file for each file in the path.", cxxopts::value<std::string>())
		("v,verify", "Verify checksum files in the path.", cxxopts::value<std::string>())
		("h,help", "Print usage");

	auto result = options.parse(argc, argv);

	if (result.count("create")) {
		create_checksum(result["create"].as<std::string>());
	}
	else if (result.count("verify")) {
		verify(result["verify"].as<std::string>());
	}
	else {
		std::cout << options.help() << std::endl;
	}

	return 0;
}
