/*
 * Data.cpp
 *
 *  Created on: Jun 9, 2010
 *      Author: sfoley
 */

#include <sstream>
#include <ios>

#include "Data.h"

namespace hpqueue {

const std::string Data::emptyString;
const char Data::emptyChars[] = {'\0'};

template<>
const INT_8 vector_wrapper<INT_8>::emptyValue[] = { 0 };

template <>
std::string DataConverter<std::string>::convertToStringValue(const std::string &value) {
	return value;
}

template<>
std::string DataConverter<std::string>::convertFromStringValue(const std::string &value) {
	return value;
};

template<>
std::string DataConverter<std::string>::convertFromString(const std::string &value) {
	return value;
};

template<>
std::string DataConverter<bool>::convertToStringValue(const bool &val) {
	std::ostringstream stream;
	stream << std::boolalpha << val;
	return stream.str();
};

template<>
bool DataConverter<bool>::convertFromStringValue(const std::string &val) {
	std::string str(val);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::istringstream stream(str);
	bool result;
	stream >> std::skipws >> std::boolalpha >> result;
	return result;
};

template<>
bool DataConverter<bool>::convertFromString(const std::string &val) {
	std::string str(val);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::istringstream stream(str);
	bool result;
	stream >> std::skipws >> std::boolalpha >> result;
	conversionFailed = stream.fail();
	return result;
};

template struct DataConverter<bool>;
template struct DataConverter<INT_32>;
template struct DataConverter<INT_64>;
template struct DataConverter<INT_8>;
template struct DataConverter<INT_16>;
template struct DataConverter<UINT_32>;
template struct DataConverter<UINT_64>;
template struct DataConverter<UINT_8>;
template struct DataConverter<UINT_16>;
template struct DataConverter<double>;
template struct DataConverter<std::string>;

}

