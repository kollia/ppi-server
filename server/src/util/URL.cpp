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

#include "URL.h"

#include "../pattern/util/LogHolderPattern.h"
//#include "../logger/lib/LogInterface.h"

using namespace boost;
using namespace boost::algorithm;

namespace util {

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
			msg+= strerror(errno);
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
				if(	file.substr(0, beginfilterLen) == beginfilter
					&&
					(	fileLen == beginfilterLen
						||
						(	//fileLen == (beginfilterLen + 14 + endfilterLen)
							//&&
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

	string URL::getPath(const string& filestring)
	{
		string::size_type  pos= filestring.find_last_of("/");

		if(pos == string::npos)
			return "./";
		return filestring.substr(0, pos + 1);
	}
}
