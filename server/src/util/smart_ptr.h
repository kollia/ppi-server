/**
 *   This file is part of ppi-server.
 *
 *   ppi-server is free software: you can redistribute it and/or modify
 *   it under the terms of the Lesser GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   ppi-server is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   Lesser GNU General Public License for more details.
 *
 *   You should have received a copy of the Lesser GNU General Public License
 *   along with ppi-server.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SMART_PTR_H_
#define SMART_PTR_H_

//****************************************
// std::auto_ptr<>()        - basic
// std::tr1::shared_ptr<>() - RCSP
//
// boost::scoped_array<>() - multi
// boost::shared_array<>() - RCSP
//****************************************
// include for std::auto_ptr
#include <memory>

#include <boost/shared_ptr.hpp>
#define SHAREDPTR boost

#include <boost/scoped_array.hpp>
#define SCOPEDARR boost

#include <boost/shared_array.hpp>
#define SHAREDARR boost

#endif /* SMART_PTR_H_ */
