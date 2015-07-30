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


#ifndef SRC_GDAL_GDALRASTERINFO_H_
#define SRC_GDAL_GDALRASTERINFO_H_

#include <gdal_priv.h>
#include <cpl_conv.h> // for CPLMalloc()
#include <string>
#include <boost/noncopyable.hpp>

namespace gdal {
namespace priv {

class RasterBand {




};




class GdalRasterInfo: boost::noncopyable
{
public:

	GdalRasterInfo( GDALDataset *gdal): dataSet( gdal ){
		CPLErr err = dataSet->GetGeoTransform( geoTransform );
		hasGeoTransform = CPLErr == CE_None;
	}

	~GdalRasterInfo() {
		delete dataSet;
	}

	int rasterCount()const { return dataSet->GetRasterCount(); }

	RasterBand *rasterBand( int i );
	//std::string driver()const {  return dataSet->GetDriver()->GetName(); /*return dataSet->GetDriverName();*/}
	std::string projDef()const;
	int rasterXSize()const { return dataSet->GetRasterXSize(); }
	int rasterYSize()const { return dataSet->GetRasterYSize(); }
	int xSize()const { return rasterXSize() * rasterCount(); }
	int ySize()const { return rasterYSize() * rasterCount(); }
	std::string unit()const { return dataSet->GetUnitType(); }
	bool scaleAndOffset( double &scale, double &offset)const{
		int has;
		scale = dataSet->GetScale( &has );
		if( has ) offset = dataSet->GetOffset( &has );
		return has;
	}

	bool hasGeoTransform()const { return hasGeoTransform_; }
	double *geoTransorm()const{ return geoTransform_; }

private:
	GDALDataset *dataSet;
	bool hasGeoTransform_;
	double geoTransform_[6];
};

}
}





#endif
