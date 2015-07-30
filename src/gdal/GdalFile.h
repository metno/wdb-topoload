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

#ifndef GDALFILE_H_
#define GDALFILE_H_

#include "GdalException.h"
#include <gdal_priv.h>
#include <cpl_conv.h> // for CPLMalloc()
#include <ogr_spatialref.h>
#include <iostream>
#include <geometry/Polygon.h>
#include <PlaceSpecification.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/noncopyable.hpp>
#include <iterator>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <limits>

namespace gdal
{

namespace priv
{
class GdalRasterInfo;
}

class GdalFile : boost::noncopyable
{
public:
	GdalFile(const boost::filesystem::path & rasterFile);
	~GdalFile();

	class ValueContainer : boost::noncopyable {
		friend class GdalFile;
	public:
		const float *data()const { return (const float*) data_; }
		unsigned int size()const { return xNumber_ * yNumber_;}
		float get( long x, long y )const;
		void setMissingValueWhere( double noDataValue, float missingValue=-32767 );
		void convertToMeter( const std::string &unit );
	private:
		ValueContainer( unsigned int xNumber, unsigned int yNumber );
		~ValueContainer();

		unsigned int xNumber_;
		unsigned int yNumber_;
		float * data_;
		float noDataValue_;

	};

	const ValueContainer &data() const;

	std::string projDefinition() const;

	int xNumber() const;
	int yNumber() const;

	double xIncrement() const;
	double yIncrement() const;

	double startX() const;
	double startY() const;

	float elevation( double longitude, double latitude )const;

	geometry::Polygon area() const { return area_; }

   friend std::ostream& operator<<( std::ostream &os, const GdalFile &gdal);

private:
	GDALDataset *gdal;
   double geoTransform_[6];
   std::string proj_;
   OGRSpatialReference srcProjection;
   OGRSpatialReference latlongProjection;
   OGRCoordinateTransformation *convert;

	PlaceSpecification ps_;
	geometry::Polygon area_;

	mutable ValueContainer *data_;

	void setProj();
	void setMissingValue( double noDataValue, float missingValue=-32767 )const;
};

std::ostream& operator<<( std::ostream &os, const GdalFile &gdal);
}

#endif
