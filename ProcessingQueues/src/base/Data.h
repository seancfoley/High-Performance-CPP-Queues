/*
 * Data.h
 *
 *  Created on: Sep 8, 2010
 *      Author: sfoley
 */

#ifndef DATA_H_
#define DATA_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

#include "primitiveTypes.h"

namespace hpqueue {

struct true_type { };
struct false_type { };

template<bool>
struct truth_type
{
	typedef false_type type;
};

template<>
struct truth_type<true> {
	typedef true_type type;
};

/**
 * This class is intended to be used for parameters in which we wish to allow non-trivial type conversions,
 * such as allowing to pass NULL for strings, and allowing us to pass null terminated char arrays for byte vectors.
 *
 * In the case where the object is a compiler-generated temporary, we initialize a field to store it.  Otherwise,
 * we simply store the pointer to the object being passed (vectors, strings and other standard language library objects of zero size
 * allocate no capacity so there is little cost from the unused field).
 */
template <typename T>
class parameter_wrapper {
	const T *originalValue;
	T convertedValue;

public:
	parameter_wrapper(const T &that, const false_type &isTemporary) : originalValue(&that) {}

	parameter_wrapper(const T &that, const true_type &isTemporary) : originalValue(&convertedValue), convertedValue(that) {}

	parameter_wrapper(const T &other) :
		originalValue((other.originalValue == &other.convertedValue) ? &convertedValue : other.originalValue),
		convertedValue(other.convertedValue) {}

	operator const T &() const {
		return *originalValue;
	}
};

/**
 * This class is intended to be used for parameters accepting string data to be able to accept strings, char *, or NULL.
 * Strings are passed by reference.
 */
class string_wrapper: public parameter_wrapper<std::string> {
public:
	string_wrapper(const std::string &that) : parameter_wrapper<std::string>(that, false_type()) {}

	string_wrapper(const char *that) : parameter_wrapper<std::string>(that == NULL ? "" : that, true_type()) {}
};

/**
 * This class is intended to be used for parameters accepting vector data to be able to accept vectors, a null-terminated array, or NULL.
 * Vectors are passed by reference.
 */
template <typename T>
class vector_wrapper: public parameter_wrapper<std::vector<T> > {
	static const T emptyValue[];

protected:
	static int getLen(const T *that) {
		const T *ptr = that;
		for(; *ptr; ptr++);
		return ptr - that;
	}

public:
	vector_wrapper(const std::vector<T> &that) : parameter_wrapper<std::vector<T> >(that, false_type()) {}

	vector_wrapper(const T *that) :
		parameter_wrapper<std::vector<T> >(
				std::vector<T>((that == NULL) ? emptyValue : that, (that == NULL) ? emptyValue : that + getLen(that))
		, true_type()) {}

	vector_wrapper(const T *that, int len) :
		parameter_wrapper<std::vector<T> >(
				std::vector<T>((len == 0 || that == NULL) ? emptyValue : that, (len == 0 || that == NULL) ? emptyValue : that + len), true_type()) {}
};

class byte_vector_wrapper: public vector_wrapper<INT_8> {
public:
	byte_vector_wrapper(const std::vector<INT_8> &that) : vector_wrapper<INT_8>(that) {}

	byte_vector_wrapper(const INT_8 *that) : vector_wrapper<INT_8>(that) {}

	byte_vector_wrapper(const INT_8 *that, int len) : vector_wrapper<INT_8>(that, len) {}

	byte_vector_wrapper(const char *that) : vector_wrapper<INT_8>((INT_8 *) that) {}

	byte_vector_wrapper(const char *that, int len) : vector_wrapper<INT_8>((INT_8 *) that, len) {}
};

template <typename T>
class DataConverter {
	friend struct Data;

	/**
	 * Deprecated.
	 * Use the non-static convertFromString(const std::string &val) instead.
	 */
	static T convertFromStringValue(const std::string &val) {
		std::istringstream stream(val);
		T result;
		stream >> std::skipws >> result;
		return result;
	}

public:
	/**
	 * This is non-static so we could possibly add options for conversion.
	 */
	std::string convertToStringValue(const T &val) {
		std::ostringstream stream;
		stream << val;
		return stream.str();
	}

	bool conversionFailed;

	T convertFromString(const std::string &val) {//note: better than convertFromStringValue since this has the conversionFailed flag
		std::istringstream stream(val);
		T result;
		stream >> std::skipws >> result;
		conversionFailed = stream.fail();
		return result;
	}
};

template <>
std::string DataConverter<std::string>::convertToStringValue(const std::string &value);

template<>
std::string DataConverter<std::string>::convertFromStringValue(const std::string &value);

template<>
std::string DataConverter<std::string>::convertFromString(const std::string &value);

template<>
std::string DataConverter<bool>::convertToStringValue(const bool &val);

template<>
bool DataConverter<bool>::convertFromStringValue(const std::string &val);

template<>
bool DataConverter<bool>::convertFromString(const std::string &val);

/**
 * Represents any collection of data.
 */
struct Data {
	static const std::string emptyString;
	static const char emptyChars[];

	Data() {}

	virtual ~Data() {}

	/**
	 * Return a string representation of the data (like Object.toString() in java)
	 */
	virtual std::string getStringValue() = 0;

	static int chooseOption(const std::string &val, const std::vector<std::string> options) {
		std::string str(val);
		trim(str);
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		for(unsigned int i=0; i<options.size(); i++) {
			std::string option = options[i];
			trim(option);
			std::transform(option.begin(), option.end(), option.begin(), ::tolower);
			if(str == option) {
				return i;
			}
		}
		return -1;
	}

	/**
	 * Maps "true" and "false" to boolean values, ignoring case.
	 * Also maps "yes and "no", ignoring case.
	 * Everything that is neither "true", "false", "yes" nor "no" returns false.
	 */
	static bool getBoolValue(const std::string &val) {
		std::string str(val);
		trim(str);
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		return str == "yes" || DataConverter<bool>::convertFromStringValue(str);
	}

	static bool getBoolValue(const std::string &value, bool &succeeded) {
		std::string str(value);
		trim(str);
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		if(str == "yes") {
			return succeeded = true;
		} else {
			return getValue<bool>(str, succeeded);
		}
	}

	inline static INT_8 getByteValue(const std::string &value, bool &succeeded) {
		return getValue<INT_8>(value, succeeded);
	}

	inline static INT_8 getByteValue(const std::string &value) {
		return DataConverter<INT_8>::convertFromStringValue(value);
	}

	inline static INT_16 getShortValue(const std::string &value, bool &succeeded) {
		return getValue<INT_16>(value, succeeded);
	}

	inline static INT_16 getShortValue(const std::string &value) {
		return DataConverter<INT_16>::convertFromStringValue(value);
	}

	inline static INT_32 getIntValue(const std::string &value, bool &succeeded) {
		return getValue<INT_32>(value, succeeded);
	}

	inline static INT_32 getIntValue(const std::string &value) {
		return DataConverter<INT_32>::convertFromStringValue(value);
	}

	inline static INT_64 getLongValue(const std::string &value, bool &succeeded) {
		return getValue<INT_64>(value, succeeded);
	}

	inline static INT_64 getLongValue(const std::string &value) {
		return DataConverter<INT_64>::convertFromStringValue(value);
	}

	inline static UINT_8 getUnsignedByteValue(const std::string &value, bool &succeeded) {
		return getValue<UINT_8>(value, succeeded);
	}

	inline static UINT_8 getUnsignedByteValue(const std::string &value) {
		return DataConverter<UINT_8>::convertFromStringValue(value);
	}

	inline static UINT_16 getUnsignedShortValue(const std::string &value, bool &succeeded) {
		return getValue<UINT_16>(value, succeeded);
	}

	inline static UINT_16 getUnsignedShortValue(const std::string &value) {
		return DataConverter<UINT_16>::convertFromStringValue(value);
	}

	inline static UINT_32 getUnsignedIntValue(const std::string &value, bool &succeeded) {
		return getValue<UINT_32>(value, succeeded);
	}

	inline static UINT_32 getUnsignedIntValue(const std::string &value) {
		return DataConverter<UINT_32>::convertFromStringValue(value);
	}

	inline static UINT_64 getUnsignedLongValue(const std::string &value, bool &succeeded) {
		return getValue<UINT_64>(value, succeeded);
	}

	inline static UINT_64 getUnsignedLongValue(const std::string &value) {
		return DataConverter<UINT_64>::convertFromStringValue(value);
	}

	inline static std::string getYesNoStringValue(bool value) {
		return value ? "yes" : "no";
	}

	inline static std::string getStringValue(bool value) {
		return DataConverter<bool>().convertToStringValue(value);
	}

	inline static std::string getStringValue(INT_32 value) {
		return DataConverter<INT_32>().convertToStringValue(value);
	}

	inline static std::string getStringValue(INT_64 value) {
		return DataConverter<INT_64>().convertToStringValue(value);
	}

	inline static std::string getStringValue(INT_8 value) {
		return DataConverter<INT_8>().convertToStringValue(value);
	}

	inline static std::string getStringValue(INT_16 value) {
		return DataConverter<INT_16>().convertToStringValue(value);
	}

	inline static std::string getStringValue(UINT_32 value) {
		return DataConverter<UINT_32>().convertToStringValue(value);
	}

	inline static std::string getStringValue(UINT_64 value) {
		return DataConverter<UINT_64>().convertToStringValue(value);
	}

	inline static std::string getStringValue(UINT_8 value) {
		return DataConverter<UINT_8>().convertToStringValue(value);
	}

	inline static std::string getStringValue(UINT_16 value) {
		return DataConverter<UINT_16>().convertToStringValue(value);
	}

	inline static std::string getStringValue(double value) {
		return DataConverter<double>().convertToStringValue(value);
	}

	static std::string &lowerCaseAppend(std::string &toStr, const std::string &appendStr) {
		for(size_t i=0; i<appendStr.length(); i++) {
			toStr.append(1, ::tolower(appendStr[i]));
		}
		return toStr;
	}

	static char *trimEnd(char *str) {
		char *ptr = str + strlen(str) - 1;
		if(ptr >= str && isspace(*ptr)) {
			for(--ptr; ptr >= str && isspace(*ptr); ptr--);
			ptr[1] = '\0';
		}
		return str;
	}

	static std::string &trimEnd(std::string &str) {
		int index = str.length();
		if(index > 0) {
			--index;
			if(isspace(str[index])) {
				for(--index; index >= 0 && isspace(str[index]); index--);
				str.resize(index + 1);
			}
		}
		return str;
	}

	static char *trimStart(char *str) {
		char *ptr;
		for(ptr = str; *ptr && isspace(*ptr); ptr++);
		return ptr;
	}

	static std::string &trimStart(std::string &str) {
		size_t index = 0;
		size_t len = str.length();
		if(index < len && isspace(str[index])) {
			for(index = 1; index < len && isspace(str[index]); index++);
			str.erase(0, index);
		}
		return str;
	}

	static char *trim(char *str) {
		return trimStart(trimEnd(str));
	}

	static std::string &trim(std::string &str) {
		return trimStart(trimEnd(str));
	}

	static std::string trimTrailingLFs(const std::string &str, bool leaveOne = false) {
		size_t len = str.length();
		if(len > 0 && str[len - 1] == '\n') {
			int pos = len - 2;
			for(; pos > 0 && str[pos] == '\n'; pos--);
			if(len - pos > (leaveOne ? 1 : 0)) {
				return str.substr(0, pos + (leaveOne ? 2 : 1));
			}
		}
		return str;
	}

	static std::string multiLineInsertLF(const std::string &str) {
		if(str.length() > 0) {
			if(str.find("\n") != std::string::npos) {
				std::string replacedString("\n");
				return replacedString.append(str);
			}
		}
		return str;
	}

	static std::string indent(const std::string &str, const std::string &indentation, bool indentFirstLine, bool indentOnlyMultiLine) {
		std::string replacedString(str);
		if(str.length()) {
			size_t pos = 0;
			if(indentFirstLine && !indentOnlyMultiLine) {
				if(replacedString[0] != '\n') {
					replacedString.insert(0, indentation);
					pos += indentation.length();
				}
				indentFirstLine = false;
			}
			do {
				pos = replacedString.find("\n", pos);
				if(pos == std::string::npos || ++pos == replacedString.length()) {
					break;
				}
				if(indentFirstLine) {
					if(replacedString[0] != '\n') {
						replacedString.insert(0, indentation);
						pos += indentation.length();
					}
					indentFirstLine = false;
				}
				if(replacedString[pos] != '\n') {
					replacedString.insert(pos, indentation);
					pos += indentation.length();
				}
			} while(true);
		}
		return replacedString;
	}

private:
	template <typename T>
	inline static T getValue(const std::string &value, bool &succeeded) {
		DataConverter<T> converter;
		T result = converter.convertFromString(value);
		succeeded = !converter.conversionFailed;
		return result;
	}

};

}

#endif /* DATA_H_ */
