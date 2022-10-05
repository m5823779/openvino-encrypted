#ifndef _CRYPTO_UTIL_H_
#define _CRYPTO_UTIL_H_

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string>

// cryptopp
#include <aes.h>
#include <hex.h>
#include <files.h>
#include <default.h>
#include <filters.h>
#include <osrng.h>

#include "crypto.h"

using namespace CryptoPP;

enum AESKeyLength { AES_KEY_LENGTH_16 = 16, AES_KEY_LENGTH_24 = 24, AES_KEY_LENGTH_32 = 32 };

class CCryptoUtil {
public:
	static int encrypt4aes(const std::string& inData, const std::string& strKey, std::string& outData, std::string& errMsg) {
		outData = "";
		errMsg = "";

		if (inData.empty() || strKey.empty()) {
			errMsg = "indata or key is empty!!";
			return -1;
		}

		unsigned int iKeyLen = strKey.length();
		if (iKeyLen != AES_KEY_LENGTH_16 && iKeyLen != AES_KEY_LENGTH_24 && iKeyLen != AES_KEY_LENGTH_32) {
			errMsg = "aes key invalid!!";
			return -2;
		}

		CryptoPP::byte iv[AES::BLOCKSIZE];
		int iResult = 0;
		try {
			CBC_Mode<AES>::Encryption e;
			e.SetKeyWithIV((CryptoPP::byte*)strKey.c_str(), iKeyLen, iv);
			StringSource ss(inData, true, new StreamTransformationFilter(e, new StringSink(outData)));
		}
		catch (const CryptoPP::Exception& e) {
			errMsg = "Encryptor throw exception!!";
			iResult = -3;
		}
		return iResult;
	}

	static int decrypt4aes(const std::string& inData, const std::string& strKey, std::string& outData, std::string& errMsg) {
		outData = "";
		errMsg = "";

		if (inData.empty() || strKey.empty()) {
			errMsg = "indata or key is empty!!";
			return -1;
		}

		unsigned int iKeyLen = strKey.length();

		if (iKeyLen != AES_KEY_LENGTH_16 && iKeyLen != AES_KEY_LENGTH_24 && iKeyLen != AES_KEY_LENGTH_32) {
			errMsg = "aes key invalid!!";
			return -2;
		}

		CryptoPP::byte iv[AES::BLOCKSIZE];
		int iResult = 0;

		try {
			CBC_Mode<AES>::Decryption d;
			d.SetKeyWithIV((CryptoPP::byte*)strKey.c_str(), iKeyLen, iv);
			StringSource ss(inData, true, new StreamTransformationFilter(d, new StringSink(outData)));
		}
		catch (const CryptoPP::Exception& e) {
			errMsg = "Encryptor throw exception";
			iResult = -3;
		}
		return iResult;
	}

};
#endif//_CRYPTO_UTIL_H_

int encrypt_from_file(stringstream* source, const string& password, string file, string output_dir) {
	std::string strKey = password; // 16 words

	std::string strResult;
	std::string strErrMsg;
	int iResult = CCryptoUtil::encrypt4aes(source->str(), strKey, strResult, strErrMsg);
	if (iResult) {
		std::cout << "CCryptoUtil::encrypt4aes failed,errMsg:" << strErrMsg;
		return -1;
	}

	string output_path = output_dir;
	output_path.append("/encrypted_");
	output_path.append(file);
	ofstream outputBuffer(output_path, ios::out | ios::binary);
	outputBuffer << strResult;
	std::cout << "Output file : " << output_path << endl;

	return 0;
}

int decrypt_from_file(stringstream* source, const string& password, std::vector<uint8_t>& model, string& infos) {
	std::string strKey = password; // 16 words

	std::string strResult;
	std::string strErrMsg;
	int iResult = CCryptoUtil::decrypt4aes(source->str(), strKey, strResult, strErrMsg);
	if (iResult) {
		std::cout << "CCryptoUtil::decrypt4aes failed,errMsg:" << strErrMsg;
		return -2;
	}

	std::cout << "Decrypt successed!" << std::endl;
	vector<unsigned char> output(strResult.begin(), strResult.end());
	for (int i = 0; i < output.size(); i++) {
		model.push_back(output[i]);
		infos.push_back(output[i]);
	}
	return 0;
}