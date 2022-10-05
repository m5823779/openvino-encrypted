#include <Windows.h>
#include <iostream> 

// read file
#include <fstream>
#include <filesystem>

#undef max
#undef min
#include <ie/inference_engine.hpp>

#include "crypto.h"

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

using namespace std;
using namespace InferenceEngine;

int model_id;

string user_input_str;
string mode;
string input_dir;
string output_dir;
string password;

string xml_path;
string bin_path;

void encrypt(string input_dir, string output_dir) {
	for (const auto& entry : std::filesystem::directory_iterator(input_dir)) {
		std::cout << "encrypt file : " << entry.path() << std::endl;
		ifstream fileBuffer(entry.path(), ios::in | ios::binary);

		if (fileBuffer.is_open()) {
			clock_t begin = clock();

			std::stringstream buffer;
			buffer << fileBuffer.rdbuf();
			std::string filename_string{ entry.path().filename().u8string() };
			encrypt_from_file(&buffer, password, filename_string, output_dir);

			clock_t end = clock();
			double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			cout << "Elasped time is " << std::to_string(elapsed_secs) << endl;
		}
		fileBuffer.close();
	}
}

void decrypt(const string& input_filename, std::vector<uint8_t>& inputModel, string& infos) {
    ifstream model_file(input_filename.c_str(), std::ios::binary);
    std::stringstream buffer;
    buffer << model_file.rdbuf();
    decrypt_from_file(&buffer, password, inputModel, infos);
}

void UserInput() {
    std::cout << "choose encrypt or decrypt mode [e / d] [default 'e']: ";
    std::getline(std::cin, user_input_str);
    mode = (user_input_str != "d" && user_input_str != "e") ? "e" : user_input_str;

    if (mode == "e") {
        std::cout << "start encrypt file ...\n";
        std::cout << "input directory  [default '../input']: ";
        std::getline(std::cin, user_input_str);
        input_dir = (user_input_str == "") ? "../input" : user_input_str;

        std::cout << "output directory [default '../output']: ";
        std::getline(std::cin, user_input_str);
        output_dir = (user_input_str == "") ? "../output" : user_input_str;
    }
    else {
        std::cout << "openvino read encrypted model";

        std::vector<std::filesystem::path> model_list;
        for (const auto& file : std::filesystem::recursive_directory_iterator("../")) {
            if (file.path().extension().string() == ".xml" && file.path().filename().string() != "plugins.xml") model_list.push_back(file);
        }

        if (!model_list.size()) {
            printf("\nCan not find any encrypted IR model (.xml.bin) please put it in root folder\n");
            exit(0);
        }
        else if (model_list.size() == 1) {
            printf("\nFind model: %s", model_list[0].string().c_str());
            xml_path = model_list[0].string();
        }
        else {
            std::cout << "\nFind model: ";
            for (int i = 0; i < model_list.size(); ++i) {
                printf("\n(%d): %s", i, model_list[i].string().c_str());
            }
            std::cout << "\nChoose model: ";
            std::getline(std::cin, user_input_str);
            model_id = (user_input_str == "") ? 0 : stoi(user_input_str);
            xml_path = model_list[min(model_id, model_list.size())].string();
        }

        bin_path = xml_path.substr(0, xml_path.find_last_of('.')) + ".bin";
    }

    std::cout << "password [enter 16, 24 or 32 words]: ";
    std::getline(std::cin, user_input_str);
    password = user_input_str;

    if (password.length() != 16 && password.length() != 24 && password.length() != 32) {
        cout << "password wrong format! please use 16, 24 or 32 words!" << endl;
        exit(0);
    }
}

int main() {
    UserInput();

    // encrypt
    if (mode == "e") {
        std::wstring stemp = std::wstring(output_dir.begin(), output_dir.end());
        CreateDirectory(stemp.c_str(), NULL);
        encrypt(input_dir, output_dir);
    }

    // decrypt
    else {
        InferenceEngine::Core ie;

        std::vector<uint8_t> model;
        std::vector<uint8_t> weights;

        string xml_infos;
        string bin_infos;

        decrypt(xml_path, model, xml_infos);
        decrypt(bin_path, weights, bin_infos);

        std::string strModel(model.begin(), model.end());
        CNNNetwork network = ie.ReadNetwork(strModel, make_shared_blob<uint8_t>({ Precision::U8, {weights.size()}, C }, weights.data()));

        InferenceEngine::InputsDataMap input_info = network.getInputsInfo();

        cout << "\ninput name: ";
        for (auto item : input_info)
            std::cout << item.first << "  ";

        cout << "\ninput shape: ";
        auto s = input_info.begin()->second->getTensorDesc().getDims();
        for (auto s : input_info.begin()->second->getTensorDesc().getDims())
            cout << s << "  ";

        return 0;
    }
}

