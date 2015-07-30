
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "gdal/GdalFile.h"
#include "configuration/GdalLoadConfiguration.h"
#include <wdb/LoaderDatabaseConnection.h>
#include <wdb/errors.h>
#include <wdbLogHandler.h>
#include <wdbException.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <sstream>

WDB_LOG  *LOG=0;

using namespace std;

std::ostream&
printInfo(std::ostream & s, const gdal::GdalFile & topography)
{
	s << "Dimensions: (" << topography.xNumber() << ", " << topography.yNumber() << ")" << endl;
	s << "Increment:  (" << topography.xIncrement() << ", " << topography.yIncrement() << ")" << endl;
	s << "start pos:  (" << topography.startX() << ", " << topography.startY() << ")" << endl;
	s << "proj4text:   " << topography.projDefinition() << endl;

	geometry::Polygon::Container a = topography.area().vertex();
	s << "Upper Left  ( " << a[1].x() << ", " << a[1].y() << " )" << endl;
	s << "Lower Left  ( " << a[0].x() << ", " << a[0].y() << " )" << endl;
	s << "Upper Right ( " << a[2].x() << ", " << a[2].y() << " )" << endl;
	s << "Lower Right ( " << a[3].x() << ", " << a[3].y() << " )" << endl;
	return s << endl;
}

std::ostream & version(std::ostream & s)
{
    s << PACKAGE_STRING << endl;
    return s;
}

std::ostream & help(std::ostream & s, const wdb::GdalLoadConfiguration & conf)
{
	version(s);
	s << '\n';
    s << "Usage: wdb-rasterload [OPTIONS] raster_files...\n\n";
//    s << "If you don't give any files on command line, stdin will be read instead.\n\n"; // doesn't work yet
    s << "Options:\n";
	s << conf.shownOptions();
	return s;
}

std::ostream & listInfo(std::ostream & s, const gdal::GdalFile & f)
{
	s << "GdalInfo:\n" << f << "\n\n";


	printInfo( s, f );

	s << "Topography data for area " << f.area() << "\n";
	s << "Increment distance " << f.xIncrement() << ", " << f.yIncrement() << "\n";
	s << "Proj definition " << f.projDefinition() << endl;
	return s << endl;
}

std::string
getReferenceTime( const wdb::GdalLoadConfiguration &conf, const std::vector<boost::filesystem::path> &files, WDB_LOG &log )
{
	using namespace boost::posix_time;
	using namespace boost::gregorian;
	using namespace boost::filesystem;

	if( ! conf.loading().referenceTime.empty() ) {
		return conf.loading().referenceTime;
	} else if( ! files.empty() && is_regular_file(files.front())){
		ptime refTime = from_time_t(last_write_time( files.front() ) );
		refTime = ptime( refTime.date(), hours(0));

		ostringstream ot;
		time_duration t = refTime.time_of_day();
		date d = refTime.date();

		ot << d.year() << "-"<< d.month() << "-" << d.day() << " "
			<< t.hours() << ":" << t.minutes() << ":" << t.seconds() << "+00";

		log.infoStream() << "Reference time is based on mtime to the file '" << files.front() << "'. Reference time: " << ot.str() <<".";

		return ot.str();
	}

	log.errorStream() << "Failed to create a reference time.";
	exit( 1 );
	return "";
}

void
test(const std::vector<boost::filesystem::path> &files)
{
   gdal::GdalFile gdal( files.front() );
   cerr << gdal << "\n\n";

   printInfo( cout, gdal );

   const gdal::GdalFile::ValueContainer &data(gdal.data());

   float h = gdal.elevation( 86.922623, 27.986065 );//elevation 8495m
   cerr << "Elevation 'Mount Everest': ( 86.922623, 27.986065 ): " << h << endl << endl;

   h = gdal.elevation( 8.309198, 61.637023 );//elevation 2296m
   cerr << "Elevation 'Galdhøpiggen': ( 8.309198, 61.637023 ): " << h << endl << endl;

   h = gdal.elevation( 88.147535, 27.7024914 ); //8586
   cerr << "Elevation 'Kanchenjunga': ( 88.146667, 27.7025 ): " << h << endl << endl;

   h = gdal.elevation( 19.14453506, 69.60725813 );//elevation > ~ 1072m
   cerr << "Elevation 'Tromsdalstinden': (  19.14453506, 69.60725813 ): " << h << endl << endl;


   h = gdal.elevation( 10.708382, 59.924696 );//elevation > ~ 47m
   cerr << "Elevation 'Vigelands parken': ( 10.708382, 59.924696 ): " << h << endl << endl;

   h = gdal.elevation( 88.1217530125, 27.1622067243); //2738m
   cerr << "Elevation: ( 88.1217530125, 27.1622067243 ): " << h << endl << endl;
   exit(0);
}

std::string
placedefinition( const wdb::GdalLoadConfiguration &conf,
		const gdal::GdalFile &f,
		const boost::filesystem::path &file,
		wdb::load::LoaderDatabaseConnection &db )
{
	string placeName;
	bool add=false;
	string exWhat;

	try {
		placeName = db.getPlaceName(f.xNumber(), f.yNumber(), f.xIncrement(), f.yIncrement(), f.startX(), f.startY(), f.projDefinition());
	}
	catch ( wdb::empty_result & )	{
		add=true;
	}
	catch( const wdb::load::LoadError &ex ) {
		add = true;
		exWhat = ex.what();
	}

	if( add ) {
		if( ! conf.loading().loadPlaceDefinition ) {
			LOG->errorStream() << "There is no placedefinition for this field in the database.\n"
					<<"If you want to add a placedefinition, rerun the command with --loadPlaceDefinition\n"
					<<"and if you want to give it a name other than the default name\n"
					<<"'" << boost::filesystem::basename(file) << "' use --placename.\n";
			exit( 1 );
		}

		if ( conf.loading().placeName.empty() )
			placeName = boost::filesystem::basename(file);
		else
			placeName = conf.loading().placeName;
		if( ! exWhat.empty())
		LOG->infoStream() << exWhat << ".";
		LOG->infoStream() << "Trying to add placedefinition: '" << placeName << "'.";
		try {
			placeName = db.addPlaceDefinition(placeName, f.xNumber(), f.yNumber(), f.xIncrement(), f.yIncrement(), f.startX(), f.startY(), f.projDefinition());
			LOG->infoStream() << "Added placedefinition '" << placeName << "'.";
		}
		catch( const std::exception &ex ) {
			LOG->fatalStream() << "Failed to add placedefinition: '" << placeName << "'. "
			<< ex.what() << ".";
			exit(1);
		}
	}
	return placeName;
}

int main(int argc, char ** argv)
{
   GDALAllRegister();
   string refTime;
	wdb::GdalLoadConfiguration conf("wdb-topoload");
	conf.parse(argc, argv);

	if ( conf.general().help )
	{
		help(cout, conf);
		return 0;
	}
	if ( conf.general().version )
	{
		version(cout);
		return 0;
	}

	wdb::WdbLogHandler logHandler( conf.logging().loglevel, conf.logging().logfile );
   LOG = &WDB_LOG::getInstance( "wdb.topoload.main" );
   LOG->debug( "Starting topoload" );

   const std::vector<std::string> & file = conf.input().file;
   std::vector<boost::filesystem::path> files;
   copy(file.begin(), file.end(), back_inserter(files));

   //test( files );

	wdb::load::LoaderDatabaseConnection db(conf);

	refTime = getReferenceTime( conf, files, *LOG );
	bool listInsteadOfLoad = conf.output().list;

	for ( std::vector<boost::filesystem::path>::const_iterator it = files.begin(); it != files.end(); ++ it )
	{
		LOG->infoStream() << "loading file " << it->string();

		gdal::GdalFile f(* it);

		if ( listInsteadOfLoad ) {
			listInfo(cout , f);
		} else	{
			std::string placeName;
			placeName = placedefinition( conf,f, *it, db );

			try {
				db.write(
						f.data().data(),
						f.data().size(),
						"topography", // will be ignored and overridden by whatever is in config.
						placeName,
						refTime,
						"-infinity",
						"infinity",
						"altitude",
						"height above reference ellipsoid",
						0, 0,
						0,
						0);
			}
			catch ( std::exception & e ) {
				if( string(e.what()).find("duplicate") == string::npos )
					LOG->errorStream() << "Error when loading file " << it->string() << ": " << e.what()<<".";
				else
					LOG->infoStream() << "Allready loaded: '" << *it << "'\n";
			}
		}
	}
}
