/*
 wdb-rasterload

 Copyright (C) 2009 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 E-mail: wdb@met.no

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA
*/

#ifndef __EXCEPTION_SPEC_H__
#define __EXCEPTION_SPEC_H__

#include <exception>
#include <string>

#define EXCEPTION_SPEC_BASE(BaseName)\
class BaseName : public std::exception \
{\
	std::string msg_;\
public:\
	BaseName(const std::string & msg) : msg_(msg) {}\
	virtual ~BaseName() throw() {}\
	virtual const char* what() const throw() { return msg_.c_str(); }\
};

#define EXCEPTION_SPEC(BaseException, ExceptionName, message) \
	class ExceptionName : public BaseException { \
	public: \
	ExceptionName() : BaseException(message) {} \
	}

#define EXCEPTION_SPEC_CUSTOM_MESSAGE(BaseException, ExceptionName) \
class ExceptionName : public BaseException { \
public: \
	ExceptionName(const std::string & msg) : BaseException(msg) {} \
}

#endif
