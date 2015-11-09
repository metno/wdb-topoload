/*
 wdb-topoload

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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <vector>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cmath>
#include <limits>
#include <iostream>
#include <ogr_spatialref.h>
#include <wdbLogHandler.h>
#include "GdalFile.h"
#include "geometry/Point.h"



extern WDB_LOG  *LOG;

namespace gdal
{
namespace math {
	int round(double val)
	{
		return std::floor(val + 0.5);
	}
}

GdalFile::ValueContainer::
ValueContainer(unsigned int xNumber, unsigned int yNumber  )
	:data_(0), xNumber_( xNumber), yNumber_( yNumber ), noDataValue_( std::numeric_limits<float>::max())
{
	data_ = (float *)CPLMalloc(sizeof(float)*xNumber_*yNumber_);
	if( data_ == NULL )
		throw NoMemmory();
}

GdalFile::ValueContainer::
~ValueContainer()
{
	if( data_ != NULL)
		CPLFree( data_ );
}

float
GdalFile::ValueContainer::
get( long x, long y )const
{
	if( x < 0 || y < 0 || x >= xNumber_ || y >= yNumber_ )
		return std::numeric_limits<float>::max();

	return data_[xNumber_*y+x];
}

void
GdalFile::ValueContainer::
convertToMeter( const std::string &unit )
{
	float scale=1;

	if( unit.empty() ) {
		LOG->infoStream() << "No unit specified, assuming 'm'.\n";
		return;
	}

	if( unit == "ft")
		scale = 3.2808399;
	else if( unit == "m" )
		return;
	else {
		LOG->fatalStream() << "FATAL: Unsupported unit '" << unit << "'\n";
		exit(1);
	}

	for( int i=0; i<(xNumber_*yNumber_); ++i ) {
		if( data_[i] != noDataValue_ )
			data_[i] *= scale;
	}
}

void
GdalFile::ValueContainer::
setMissingValueWhere( double noDataValue, float missingValue )
{

	//This test assumes the noDataValue is a integer.
	if( trunc(noDataValue) != noDataValue ||
		noDataValue >= std::numeric_limits<float>::max() || noDataValue <= -std::numeric_limits<float>::max() )
		throw NoDataValueError();

	noDataValue_=missingValue;
	float noData = (float) noDataValue;

	for( int i=0; i<(xNumber_*yNumber_); ++i ){
		if( data_[i] == noData )
			data_[i]=noDataValue_;
	}
}

GdalFile::
GdalFile(const boost::filesystem::path & rasterFile) :
	gdal(0), data_(0), convert(0)
{
	if ( ! boost::filesystem::exists(rasterFile))
		throw FileDoesNotExist();

	gdal = (GDALDataset *) GDALOpen( rasterFile.string().c_str(), GA_ReadOnly );

   if( gdal == NULL )
   	throw UnableToReadFile();

	if( gdal->GetGeoTransform( geoTransform_ ) != CE_None )
		throw Unsupported("Only datafiles with geotransforms is supported.");

	if( geoTransform_[2] != 0 || geoTransform_[4] != 0 )
		throw Unsupported("Only projections with north up is supported.");

	setProj();

	if( gdal->GetRasterCount() == 0 ) {
		throw NoRasterExist();
	}

	std::vector<geometry::Point> vertexs;

	double xRight = startX() + xIncrement()*xNumber();
	double yLower = startY() + yIncrement() * yNumber();

	boost::assign::push_back( vertexs )( geometry::Point( startX(), yLower ) ) //Lower left
	( geometry::Point( startX(), startY() ) ) //Upper left
	( geometry::Point( xRight, startY() ) ) //Upper right
	( geometry::Point( xRight, yLower ) ); //Lower right

	area_ = geometry::Polygon( vertexs.begin(), vertexs.end() );
}

GdalFile::
~GdalFile()
{
	gdal->Dereference();
}


std::string
GdalFile::
projDefinition()const
{
	return proj_;
}


int
GdalFile::
xNumber() const
{
	return gdal->GetRasterXSize();
}

int
GdalFile::
yNumber() const
{
	return gdal->GetRasterYSize();
}


double
GdalFile::
xIncrement() const
{
	return geoTransform_[1];
}

double
GdalFile::
yIncrement() const
{
	return geoTransform_[5];
}

double
GdalFile::
startX() const
{
	return geoTransform_[0];
}

double
GdalFile::
startY() const
{
	return geoTransform_[3];
}


void
GdalFile::
setProj()
{
	char *proj;
	char *wktStr = strdup( gdal->GetProjectionRef() );
	char *tmpWktStr = wktStr;
	srcProjection.importFromWkt( &tmpWktStr );
	free( wktStr );

	if( srcProjection.exportToProj4(&proj ) == OGRERR_NONE ) {
		proj_ = proj;
		boost::trim(proj_);
		CPLFree( proj );
	}

	if( latlongProjection.SetWellKnownGeogCS( "WGS84" ) == CE_None ) {
		convert = OGRCreateCoordinateTransformation( &latlongProjection, &srcProjection);

		if( convert == NULL )
			LOG->infoStream() << "Failed to create a coordinate transformation.\n";
	} else {
		LOG->infoStream() << "Failed to create a 'WGS84' projection\n";
	}
}

float
GdalFile::
elevation( double longitude, double latitude )const
{
	double x = longitude;
	double y = latitude;
	int ix, iy;
	if( convert == NULL )
		return std::numeric_limits<float>::max();

	if( convert->Transform(1, &x, &y) == FALSE )
		return std::numeric_limits<float>::max();

	LOG->debugStream() << "Convert: ( " << longitude << ", " << latitude << " ) -> ( " << x << ", " << y << " )";
	ix = math::round((x - geoTransform_[0])/geoTransform_[1]);
	iy = math::round((y - geoTransform_[3])/geoTransform_[5]);

	LOG->debugStream() << "index: (x,y): ( " << ix << ", " << iy << " )";
	float h=std::numeric_limits<float>::max();

	if( data_ )
		h = data_->get( ix, iy );

	if( h == std::numeric_limits<float>::max() )
		LOG->debugStream() << "No value for coordinate (" << longitude << " " << latitude <<").";

	return h;
}

const GdalFile::ValueContainer&
GdalFile::
data() const
{
	int hasNoDataValue=-1;
	double noData;

	if( data_ != NULL )
		return *data_;

	int n = gdal->GetRasterCount();

	if( n == 0 )
		throw NoRasterExist();

	if( n > 1 )
		LOG->warnStream() << "More than one rasterband in the dataset. Assume the first band is the elevation data.";

	GDALRasterBand *raster = gdal->GetRasterBand(1);
	noData = raster->GetNoDataValue( &hasNoDataValue );
	LOG->debugStream() << "Rasterunit: '"<< raster->GetUnitType() << "' HasNoData: " << hasNoDataValue << ".";

	if( hasNoDataValue )
		LOG->infoStream() << "NoDataValue: '"<< noData << "'.\n";
	else
		LOG->infoStream()<< "--- NoDataValue:  all value is valid values.";

	data_  = new ValueContainer( xNumber(), yNumber() );

	CPLErr err = raster->RasterIO(GF_Read, 0, 0, xNumber(), yNumber(), data_->data_, xNumber(), yNumber(),
			GDT_Float32, 0,0);

	if( err != CE_None )
		throw InternalError();

	if( hasNoDataValue )
		setMissingValue( noData );

	data_->convertToMeter( raster->GetUnitType() );

	return *data_;
}

void
GdalFile::
setMissingValue( double noDataValue, float missingValue )const
{
	try {
		data_->setMissingValueWhere( noDataValue, missingValue );
	}
	catch( ... ) {
		std::cerr << "Can't adjust for noDataValue, use whatever is in the dataset.\n";
	}
}

std::ostream&
operator<<( std::ostream &os, const GdalFile &gdal)
{
	os << "GdalFile"
		<< "\n proj: " << gdal.projDefinition()
		//<< "\n driver: " << gdal.gdal->GetDriver()->GetDescription()
		<<	"\n raster count: " << gdal.gdal->GetRasterCount()
		<< "\n xSize: " << gdal.xNumber()
		<< "\n ySize: " << gdal.yNumber()
		<< "\n xStart: " << gdal.startX()
		<< "\n yStart: " << gdal.startY()
		<< "\n xIncrement: " << gdal.xIncrement()
		<< "\n yIncrement: " << gdal.yIncrement()
		<< "\n Area: " << gdal.area().wkt()
		<< "\n geoTransform:";

	for( int i=0; i<6; ++i )
		os << " " << i << ": " << gdal.geoTransform_[i];
	return os;
}

}
