/*
 * DemLoadConfiguration.h
 *
 *  Created on: Aug 17, 2009
 *      Author: vegardb
 */

#ifndef GDALLOADCONFIGURATION_H_
#define GDALLOADCONFIGURATION_H_

#include <wdb/LoaderConfiguration.h>

namespace wdb
{

class GdalLoadConfiguration : public wdb::load::LoaderConfiguration
{
public:
	GdalLoadConfiguration(const std::string & defaultDataProvider) :
		wdb::load::LoaderConfiguration(defaultDataProvider)
	{}
};

}

#endif
