/*
 demLoad

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

#ifndef POLYGON_H_
#define POLYGON_H_

#include "Point.h"
#include <vector>
#include <stdexcept>

namespace geometry
{

class Polygon
{
public:
	Polygon() {}
	template<class PointIterator>
	Polygon(PointIterator start, PointIterator stop);
	~Polygon();

	typedef std::vector<Point> Container;
	typedef Container::size_type size_type;

	size_type noOfVertex() const { return vertex_.size(); }

	const Container & vertex() const { return vertex_; }

	std::string wkt() const;

private:
	Container vertex_;
};

template<class PointIterator>
Polygon::Polygon(PointIterator start, PointIterator stop) :
	vertex_(start, stop)
{
	if ( vertex_.empty() )
		throw std::logic_error("A polygon must have more than two vertex");
	if ( vertex_.front() == vertex_.back() )
		vertex_.pop_back();
	if ( vertex_.size() <= 2 )
		throw std::logic_error("A polygon must have more than two vertex");
}

}

std::ostream & operator << (std::ostream & s, const geometry::Polygon & p);

#endif /* POLYGON_H_ */
