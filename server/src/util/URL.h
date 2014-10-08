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

/*
 * URL.h
 *
 *  Created on: 22.05.2009
 *      Author: Alexander Kolli
 */

#ifndef URL_H_
#define URL_H_

#include "unistd.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace util
{
	/**
	 * enum of different URL types
	 */
	enum URLtype_e
	{
		unknown= 0,
		file,
		http,
		https,
		SOA
	};
	/**
	 * object to hold any URL's
	 * which can contain an normally file on hard disk,
	 * or some internet address with different protocol's
	 */
	class URL
	{
		public:
			/**
			 * constructor to create empty object
			 */
			URL()
			: m_eType(unknown),
			  m_nPort(0)
			{};
			/**
			 * constructor to create object with URL
			 *
			 * @param src URL for which object should created
			 */
			URL(string src)
			: m_eType(unknown)
			{ set(src); };
			/**
			 * write new URI into object.<br />
			 * when URI is not from protocol type file,
			 * it will be split address with query by first question mark '?',
			 * the anchor by last grid '#'
			 * and hole query by all ampersands '&'.<br />
			 * when several query segments contain also an ampersand
			 * or the anchor definition is not beginning by the last grid,
			 * better define with this <code>set()</code> method
			 * only the address
			 * and all query segments with <code>addQuer()</code>.
			 * Also the anchor with method <code>anchor()</code>.
			 *
			 * @param src new URL
			 */
			void set(const string& src);
			/**
			 * add first with second path,
			 * only if the second not beginning
			 * with an slash
			 *
			 * @param first first beginning path (working directory)
			 * @param second second path or file
			 * @param always boolean whether second is an file.<br />
			 * 				If parameter is true (DEFAULT),
			 * 				the first path is only add when the second do not beginning
			 * 				with an slash
			 */
			static string addPath(const string& first, string second, bool always= true);
			/**
			 * read directory in defined path with an filter for begin and end of the files
			 *
			 * @param path subdirectory wich have to read
			 * @param beginfilter char string of beginning
			 * @param endfilter char atring of ending
			 * @return map of all files as value and the date as key
			 */
			static map<string, string> readDirectory(const string& path, const string& beginfilter, const string& endfilter);
			/**
			 * extract path from given path with file
			 *
			 * @param filestring file with path
			 * @return only path
			 */
			static string getPath(const string& filestring);
			/**
			 * return protocol of URI
			 *
			 * @return URL type
			 */
			URLtype_e getProtocol() const
			{ return m_eType; };
			/**
			 * return string of protocol specification
			 *
			 * @return specification
			 */
			string getProtocolSpec() const;
			/**
			 * return string of full address
			 * (HOST/PATH/ and when protocol from file also with FILE)
			 */
			string getAddress() const;
			/**
			 * return string of full address of object
			 * with specific URL protocol,
			 * but without parameters
			 *
			 * @return address string
			 */
			string getSpecAddress() const;
			/**
			 * return full address of object,
			 * without specific protocol type,
			 * but with query when exist
			 *
			 * @return address string
			 */
			string getAddressQuery() const;
			/**
			 * return when protocol is no file type
			 * an string of absolute path with
			 * query string when exist
			 *
			 * @return path with query string
			 */
			string getAbsolutePathQuery() const;
			/**
			 * return full address of object
			 * with specific protocol type
			 * and also query when exist
			 *
			 * @return URI string
			 */
			string getBaseUri() const;
			/**
			 * return only host of address
			 *
			 * @return host string
			 */
			 string getHost() const
			 { return m_sHost; };
			 /**
			  * return port from URL
			  * when URL type is no file
			  * otherwise 0
			  *
			  * @return port number
			  */
			 unsigned short getPort() const
			 { return m_nPort; };
			 /**
			  * return path of address
			  *
			  * @return path string
			  */
			 string getPath() const
			 { return m_sPath; };
			 /**
			  * return absolute path of address.<br />
			  * when not beginning with slash '/'
			  * add slash before. Or path not exist,
			  * return only an slash
			  *
			  * @return path string
			  */
			 string getAbsolutePath() const;
			 /**
			  * return file of address when exist
			  *
			  * @return file string
			  */
			 string getFile() const
			 { return m_sFile; };
			 /**
			  * return an string of query
			  * behind question mark when exist
			  * otherwise an null string
			  *
			  * @return query string
			  */
			 string getQuery() const;
			 /**
			  * add one query segment
			  * between ampersands '&'
			  *
			  * @param str query segment
			  */
			 void addQuery(const string& str)
			 { m_vsQuerys.push_back(str); };
			 /**
			  * set anchor string
			  * afer grid '#'
			  *
			  * @param str anchor string
			  */
			 void anchor(const string& str)
			 { m_sAnchor= str; };
			 /**
			  * encode full URI
			  */
			 void encode();
			 /**
			  * decode full URI
			  */
			 void decode();


		private:
			/**
			 * type of protocol or type file
			 */
			URLtype_e m_eType;
			/**
			 * host of internet address
			 */
			string m_sHost;
			/**
			 * port when URL is no file type
			 */
			unsigned short m_nPort;
			/**
			 * path of address
			 */
			string m_sPath;
			/**
			 * file of address
			 */
			string m_sFile;
			/**
			 * query parameters
			 * after file when exist
			 */
			vector<string> m_vsQuerys;
			/**
			 * anchor parameter
			 * after last grid sign '#'
			 */
			string m_sAnchor;

			/**
			 * encode given string
			 *
			 * @param str string to encode
			 * @param query whether string is query of the URI
			 * @param anchor wheter string is anchor of query
			 */
			void encode(string& str, bool query, bool anchor);
			/**
			 * decode given string
			 *
			 * @param str string to decode
			 */
			void decode(string& str);
	};

}

#endif /* URL_H_ */
