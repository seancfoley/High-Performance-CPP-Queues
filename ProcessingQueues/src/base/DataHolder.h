/*
 * DataHolder.h
 *
 *  Created on: Dec 6, 2010
 *      Author: sfoley
 */

#ifndef DATAHOLDER_H_
#define DATAHOLDER_H_

#include <sstream>
#include <iostream>

#include "Data.h"

namespace hpqueue {

/**
 * A holder class to be populated with data.
 */
template <typename T>
class DataHolder : public Data, DataConverter<T> {
protected:
	T value;

	/**
	 * override this function to handle blocking behaviour
	 */
	virtual T &getValue(bool block) {
		return value;
	}

	T &getCurrentValue() {
		return value;
	}

public:
	DataHolder() {}

	DataHolder(const T &value) : value(value) {}

	virtual ~DataHolder() {}

	T &getValue() {
		return getValue(true);
	}

	std::string getStringValue() {
		return getStringValue(true);
	}

	std::string getStringValue(bool block) {
		T value = getValue(block);
		return convertToStringValue(value);
	}

	void setValue(T &val) {
		value = val;
	}

};

/**
 * The same as the DataHolder class template, but for primitive types, such as a 64-bit integer key.
 */
template <typename T>
class PrimitiveDataHolder : public Data {
protected:
	T value;

	T &getCurrentValue() {
		return value;
	}

	/**
	 * override this function to handle blocking behaviour
	 */
	virtual T &getValue(bool block) {
		return value;
	}

public:

	PrimitiveDataHolder() : value()  {}

	PrimitiveDataHolder(T value) : value(value) {}

	virtual ~PrimitiveDataHolder() {}

	static std::string convertToStringValue(T value) {
		std::ostringstream stream;
		stream << value ;
		return stream.str();
	}

	T &getValue() {
		return getValue(true);
	}

	std::string getStringValue() {
		return getStringValue(true);
	}

	std::string getStringValue(bool block) {
		T value = getValue(block);
		return convertToStringValue(value);
	}

	void setValue(T val) {
		value = val;
	}
};

/**
 * The DataHolder template subclasses Data.  For primitive data types,
 * we specialize the template to subclass PrimitiveDataHolder instead.
 */

template <>
class DataHolder<INT_8> : public PrimitiveDataHolder<INT_8> {
public:
	DataHolder<INT_8>() {}
	DataHolder<INT_8>(INT_8 value) : PrimitiveDataHolder<INT_8>(value) {}
	virtual ~DataHolder<INT_8>() {}
};

template <>
class DataHolder<INT_16> : public PrimitiveDataHolder<INT_16> {
public:
	DataHolder<INT_16>() {}
	DataHolder<INT_16>(INT_16 value) : PrimitiveDataHolder<INT_16>(value) {}
	virtual ~DataHolder<INT_16>() {}
};

template <>
class DataHolder<INT_32> : public PrimitiveDataHolder<INT_32> {
public:
	DataHolder<INT_32>() {}
	DataHolder<INT_32>(INT_32 value) : PrimitiveDataHolder<INT_32>(value) {}
	virtual ~DataHolder<INT_32>() {}
};

template <>
class DataHolder<INT_64> : public PrimitiveDataHolder<INT_64> {
public:
	DataHolder<INT_64>() {}
	DataHolder<INT_64>(INT_64 value) : PrimitiveDataHolder<INT_64>(value) {}
	virtual ~DataHolder<INT_64>() {}
};

template <>
class DataHolder<UINT_8> : public PrimitiveDataHolder<UINT_8> {
public:
	DataHolder<UINT_8>() {}
	DataHolder<UINT_8>(UINT_8 value) : PrimitiveDataHolder<UINT_8>(value) {}
	virtual ~DataHolder<UINT_8>() {}
};

template <>
class DataHolder<UINT_16> : public PrimitiveDataHolder<UINT_16> {
public:
	DataHolder<UINT_16>() {}
	DataHolder<UINT_16>(UINT_16 value) : PrimitiveDataHolder<UINT_16>(value) {}
	virtual ~DataHolder<UINT_16>() {}
};

template <>
class DataHolder<UINT_32> : public PrimitiveDataHolder<UINT_32> {
public:
	DataHolder<UINT_32>() {}
	DataHolder<UINT_32>(UINT_32 value) : PrimitiveDataHolder<UINT_32>(value) {}
	virtual ~DataHolder<UINT_32>() {}
};

template <>
class DataHolder<UINT_64> : public PrimitiveDataHolder<UINT_64> {
public:
	DataHolder<UINT_64>() {}
	DataHolder<UINT_64>(UINT_64 value) : PrimitiveDataHolder<UINT_64>(value) {}
	virtual ~DataHolder<UINT_64>() {}
};

template <>
class DataHolder<double> : public PrimitiveDataHolder<double> {
public:
	DataHolder<double>() {}
	DataHolder<double>(double value) : PrimitiveDataHolder<double>(value) {}
	virtual ~DataHolder<double>() {}
};

template <>
class DataHolder<bool> : public PrimitiveDataHolder<bool> {
public:
	DataHolder<bool>() {}
	DataHolder<bool>(INT_64 value) : PrimitiveDataHolder<bool>(value) {}
	virtual ~DataHolder<bool>() {}
};

}

#endif /* DATAHOLDER_H_ */
