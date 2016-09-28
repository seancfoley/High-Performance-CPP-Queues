/*
 * Status.cpp
 *
 *  Created on: Jan 2, 2012
 *      Author: sfoley
 */

#include "SampleStatus.h"
#include "base/Data.h"

namespace hpqueue {

template <>
std::string DataConverter<Status>::convertToStringValue(const Status &value) {
	std::string str;
	value.appendTo(str);
	return str;
}

}
