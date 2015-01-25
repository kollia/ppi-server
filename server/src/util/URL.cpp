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
 * URL.cpp
 *
 *  Created on: 22.05.2009
 *      Author: Alexander Kolli
 */

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "stream/BaseErrorHandling.h"

#include "URL.h"

#include "../database/logger/lib/logstructures.h"
#include "../pattern/util/LogHolderPattern.h"
//#include "../logger/lib/LogInterface.h"

using namespace boost;
using namespace boost::algorithm;

namespace util {

	void URL::set(const string& src)
	{
		string::size_type nTypeLen;
		string address;
		vector<string> spl;

		m_nPort= 0;
		m_sHost= "";
		m_sPath= "";
		m_sFile= "";
		m_vsQuerys.clear();
		m_sAnchor= "";
		split(spl, src, is_any_of(":"));
		if(spl.empty())
			return;
		if(spl.size() > 1)
		{
			if(spl[0] == "file")
			{
				m_eType= file;
				nTypeLen= 4;

			}else if(spl[0] == "http")
			{
				m_eType= http;
				nTypeLen= 4;

			}else if(spl[0] == "https")
			{
				m_eType= https;
				nTypeLen= 5;

			}else if(spl[0] == "SOA")
			{
				m_eType= SOA;
				nTypeLen= 3;

			}else
			{
				m_eType= file;
				nTypeLen= 0;
				address= src;
			}
			if(src.substr(nTypeLen, 3) == "://")
				address= src.substr(nTypeLen + 3);
			else
			{
				address= src;
				if(nTypeLen != 0)
					m_eType= unknown;
			}
		}else
		{
			m_eType= file;
			address= src;
		}
		if(	m_eType != file &&
			m_eType != unknown	)
		{
			/*
			 * by files is for linux
			 * an question mark
			 * allowed in the path.
			 * In this case do not pass this block
			 */
			split(spl, address, is_any_of("?"));
			if(!spl.empty())
			{
				address= spl[0];
				if(spl.size() > 1)
				{
					string query;
					vector<string>::iterator it;
					string::size_type npos;

					it= spl.begin();
					++it;
					query= *it;
					++it;
					while(it != spl.end())
					{
						query+= "?" + *it;
						++it;
					}

					npos= query.find_last_of('#', query.length() -1 );
					if(npos != string::npos)
					{
						m_sAnchor= query.substr(npos + 1);
						query= query.substr(0, npos);
					}

					if(query != "")
					{
						split(m_vsQuerys, query, is_any_of("&"));
						if(query.substr(0, 1) == "&")
							m_vsQuerys.front()= "&" + m_vsQuerys.front();
						if(query.substr(query.length() - 1, 1) == "&")
							m_vsQuerys.back()+= "&";
					}
				}
			}
		}//if(m_eType != file)
		split(spl, address, is_any_of("/"));
		if(m_eType != file)
		{
			m_sHost= spl[0];
			spl.erase(spl.begin());
		}
		if(!spl.empty())
		{
			m_sFile= spl[spl.size() - 1];
			if(m_sFile.substr(0, m_sFile.length() - 1) == "/")
				m_sFile= "";//only path be given
		}
		if(address.length() > (m_sHost.length() + m_sFile.length()))
		{
			address= address.substr(m_sHost.length());
			m_sPath= address.substr(0, address.length() - m_sFile.length());
		}
		if(m_sHost != "")
		{
			bool bOK(true);
			istringstream oport;

			split(spl, m_sHost, is_any_of(":"));
			if(spl.size() == 2)
			{
				oport.str(spl[1]);
				oport >> m_nPort;
				if(oport.fail())
					bOK= false;
				else
					m_sHost= spl[0];

			}if(spl.size() != 1)
				bOK= false;
			if(!bOK)
			{
				m_eType= unknown;
				m_sPath= "";
				m_sFile= "";
				m_vsQuerys.clear();
				m_sAnchor= "";

			}else if(m_nPort == 0)
			{
				switch(m_eType)
				{
				case http:
					m_nPort= 80;
					break;
				case https:
					m_nPort= 443;
					break;
				case SOA:
					m_nPort= 80;
					break;
				default:
					m_nPort= 0;
					break;
				}
			}
		}
	}

	string URL::addPath(const string& first, string second, bool always/*= true*/)
	{
		bool fsep, ssep;
		string sRv;

		if(first == "")
			return second;
		if(second == "")
			return first;
		fsep= first.substr(first.length()-1, 1) == "/" ? true : false;
		if(second.substr(0, 2) == "./") second= second.substr(2);
		ssep= second.substr(0, 1) == "/" ? true : false;
		if(!always && ssep)
			return second;
		sRv= first;
		if(!fsep && !ssep)
			sRv+= "/";
		else if(fsep && ssep)
			sRv= sRv.substr(0, sRv.length()-1);
		sRv+= second;
		return sRv;
	}

	map<string, string> URL::readDirectory(const string& path, const string& beginfilter, const string& endfilter)
	{
		struct dirent *dirName;
		string file;
		map<string, string> files;
		int fileLen;
		int beginfilterLen= beginfilter.length();
		int endfilterLen= endfilter.length();
		DIR *dir;

		dir= opendir(&path[0]);
		if(dir == NULL)
		{
			string msg("### ERROR: cannot read in subdirectory '");

			msg+= path + "'\n";
			msg+= "    ERRNO: ";
			msg+= BaseErrorHandling::getErrnoString(errno);
			cout << msg << endl;
			TIMELOG(LOG_ALERT, "readdirectory", msg);
			return files;
		}
		while((dirName= readdir(dir)) != NULL)
		{
			if(dirName->d_type == DT_REG)
			{
				//printf ("%s\n", dirName->d_name);
				file= dirName->d_name;
				fileLen= file.length();
				if(	file.substr(0, beginfilterLen) == beginfilter &&
					(	fileLen == beginfilterLen
						||
						(	fileLen >= endfilterLen &&
							file.substr(fileLen - endfilterLen) == endfilter	)	)	)
				{
					string date(file.substr(beginfilterLen, 14));

					files[date]= file;
				}
			}
		}
		closedir(dir);
		return files;
	}

#if 0
	uid_t URL::getUserID(const string& user)
	{
		uid_t nID;
		string::size_type userLen= user.length();
		ifstream file("/etc/passwd");
		string line;
		string buffer;

		nID= -1;
		while(!file.eof())
		{
			getline(file, line);

			if(line.substr(0, userLen) == user)
			{
				vector<string> vec;

				split(vec, line, is_any_of(":"));
				nID= atoi(vec[2].c_str());
				break;
			}
		}
		return nID;
	}
#endif

	string URL::getPath(const string& filestring)
	{
		string::size_type  pos= filestring.find_last_of("/");

		if(pos == string::npos)
			return "./";
		return filestring.substr(0, pos + 1);
	}

	string URL::getProtocolSpec() const
	{
		string sRv;

		switch(m_eType)
		{
		case file:
			sRv= "file://";
			break;
		case http:
			sRv= "http://";
			break;
		case https:
			sRv= "https://";
			break;
		case SOA:
			sRv= "http://";
			break;
		default:
			sRv= "";
			break;
		}
		return sRv;
	}

	string URL::getAddress() const
	{
		string sRv;

		sRv= addPath(m_sHost, getAbsolutePath(), /*always*/true);
		sRv= addPath(sRv, m_sFile);
		return sRv;
	}

	string URL::getQuery() const
	{
		string sRv;
		vector<string>::const_iterator it;

		if(!m_vsQuerys.empty())
		{
			it= m_vsQuerys.begin();
			sRv= *it;
			++it;
			while(it != m_vsQuerys.end())
			{
				sRv+= "&" + *it;
				++it;
			}
		}
		if(m_sAnchor != "")
			sRv+= "#" + m_sAnchor;
		return sRv;
	}

	string URL::getAbsolutePath() const
	{
		string path;

		if(m_sPath == "")
			return "/";
		if(m_sPath.substr(0, 1) != "/")
			path= "/" + m_sPath;
		else
			path= m_sPath;
		return path;
	}

	string URL::getAbsolutePathQuery() const
	{
		string sRv, query;

		if(m_eType == file)
			return "";
		sRv= getAbsolutePath();
		query= getQuery();
		if(query != "")
			sRv+= "?" + query;
		return sRv;
	}

	string URL::getSpecAddress() const
	{
		string sRv;

		sRv= getProtocolSpec();
		sRv+= getAddress();
		return sRv;
	}

	string URL::getAddressQuery() const
	{
		string sRv(getAddress()), query;

		query= getQuery();
		if(query != "")
			sRv+= "?" + query;
		return sRv;
	}

	string URL::getBaseUri() const
	{
		string sRv(getSpecAddress()), query;

		query= getQuery();
		if(query != "")
			sRv+= "?" + query;
		return sRv;
	}

	void URL::encode()
	{
		encode(m_sHost, /*query*/false, /*anchor*/false);
		encode(m_sPath, /*query*/false, /*anchor*/false);
		encode(m_sFile, /*query*/false, /*anchor*/false);
		for(vector<string>::iterator it= m_vsQuerys.begin(); it != m_vsQuerys.end(); ++it)
			encode(*it, /*query*/true, /*anchor*/false);
		encode(m_sAnchor, /*query*/true, /*anchor*/true);
	}

	void URL::encode(string& str, bool query, bool anchor)
	{
		char n;
		vector<string> spl;
		vector<string>::iterator it;
		string::size_type c= 0;
		ostringstream on;
		string sNew;

		if(	query &&
			!anchor	)
		{
			// inside an query segment
			// do not encode first is same sign '='
			split(spl, str, is_any_of("="));
			if(spl.size() > 1)
			{
				it= spl.begin();
				str= *it;
				++it;
				sNew= *it;
				++it;
				while(it != spl.end())
				{
					sNew+= "=" + *it;
					++it;
				}
				encode(str, query, /*anchor*/true);
				encode(sNew, query, /*anchor*/true);
				str+= "=" + sNew;
				return;
			}
		}
		while(c < str.length())
		{
			bool encode(false);

			n= str.at(c);
			// do not encode alphabetic characters
			if(	n < '0' ||
				(	n > '9' &&
					n < 'A'		) ||
				(	n > 'Z' &&
					n < 'a'		) ||
				n > 'z'				)
			{
				// do not encode [-], [_], [.] or [~]
				if(	n != '-' &&
					n != '_' &&
					n != '.' &&
					n != '~' 	)
				{
					// do not encode [/]
					// when string is no query
					if(	query ||
						n != '/'	)
					{
						encode= true;
					}
				}
			}
			if(encode)
			{
				on.str("");
				on << hex << (int)n;
				sNew+= "%" + on.str();
			}else
				sNew+= n;
			++c;
		}
		str= sNew;
	}

	void URL::decode()
	{
		decode(m_sHost);
		decode(m_sPath);
		decode(m_sFile);
		for(vector<string>::iterator it= m_vsQuerys.begin(); it != m_vsQuerys.end(); ++it)
			decode(*it);
		decode(m_sAnchor);
	}

	void URL::decode(string& str)
	{
		string::size_type c= 0;
		string sNew;

		while(c < str.length())
		{
			if(str.at(c) == '%')
			{
				int n(0);
				istringstream shex;

				if(	(c + 2) < str.length())
				{
					string code(str.substr(c + 1, 2));
					shex.str(code);
					shex >> hex >> n;
					if(!shex.fail())
					{
						sNew+= n;
						c+= 2;
					}else
						sNew+= str.at(c);
				}else
					sNew+= str.at(c);
			}else
				sNew+= str.at(c);
			++c;
		}
		str= sNew;
	}
}
