#ifndef __BASE_64_H__
#define __BASE_64_H__
/**************************************************************************************
 * FILE:base64.h
 * DATE:2017/12/21
 * AUTH:YangW
 * INTR:base64加密文件
**************************************************************************************/
#include <string>

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode(std::string const& encoded_string);


#endif // !__BASE_64_H__
