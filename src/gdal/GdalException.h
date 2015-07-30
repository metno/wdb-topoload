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

#ifndef __GDALEXCEPTION_H__
#define __GDALEXCEPTION_H__

#include "../ExceptionSpec.h"

namespace gdal
{

EXCEPTION_SPEC_BASE(GdalException)
EXCEPTION_SPEC(GdalException, FileDoesNotExist, "File does not exist");
EXCEPTION_SPEC(GdalException, NoRasterExist, "No useful raster data in file.");
EXCEPTION_SPEC(GdalException, NotEnoughDataInFile, "File does not contain enough data");
EXCEPTION_SPEC(GdalException, UnableToReadFile, "Unable to read file");
EXCEPTION_SPEC(GdalException, InternalError, "Internal error");
EXCEPTION_SPEC(GdalException, NoDataValueError, "Can not detect the no datavalue.");
EXCEPTION_SPEC(GdalException, NoMemmory, "Not enough memmory to allocate buffer.");

EXCEPTION_SPEC_CUSTOM_MESSAGE(GdalException, Unsupported);
EXCEPTION_SPEC_CUSTOM_MESSAGE(GdalException, InvalidProjection);

}

#endif
