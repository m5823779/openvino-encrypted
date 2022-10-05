#pragma once
using namespace std;
int encrypt_from_file(stringstream* source, const string& password, string file, string output_dir);
int decrypt_from_file(stringstream* source, const string& password, std::vector<uint8_t>& model, string& infos);