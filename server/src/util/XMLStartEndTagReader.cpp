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

#include <boost/algorithm/string/trim.hpp>

#include "XMLStartEndTagReader.h"

using namespace boost;

XMLStartEndTagReader::XMLStartEndTagReader()
{
	m_bEnd= false;
	m_bRead= false;
	m_bFoundEndTag= false;
	m_sError= "";
}

string XMLStartEndTagReader::error()
{
	return m_sError;
}

string XMLStartEndTagReader::readLine(string line)
{
	bool bFoundError= false;
	int lineLen= line.length();
	int spos= -1;
	int epos= -2;
	string etag;

	if(m_bFoundEndTag)
	{
		m_bRead= false;
		return "";
	}
	for(unsigned short i= 0; i < lineLen; ++i)
	{
		if(m_bEnd)
		{
			if(	epos > -1
				&&
				line[i] == '>'	)
			{
				trim(etag);
				if(m_sStartTag == etag)
				{
					epos= i;
					m_bFoundEndTag= true;
					break;
				}else if(etag.substr(0, 13) == "error number=")
				{
					m_sError= "ERROR ";
					m_sError= etag.substr(14, 3);
					m_bFoundEndTag= true;
				}
				epos= -2;
				etag= "";
			}
			if(epos > -1)
			{
				etag+= line[i];
				epos= i;
			}
			if(	epos == -1
				&&
				line[i] == '/'	)
			{
				epos= i;
			}
			if(	epos == -1
				&&
				line[i] != ' '
				&&
				line[i] != '\t'
				&&
				line[i] != '/'	)
			{
				epos= -2;
				etag= "";
			}
			if(line[i] == '<')
			{
				epos= -1;
				etag= "";
			}

		}else
		{
			if(	spos > -1
				&&
				line[i] == '>'	)
			{
				m_bEnd= true;
				m_bRead= true;
				trim(m_sStartTag);
			}else
			{
				if(bFoundError)
				{
					if(line[i] == '\"')
						bFoundError= false;
					else
						m_sError+= line[i];
				}
				if(spos > -1)
				{
					m_sStartTag+= line[i];
					if(m_sStartTag == "error number=\"")
					{
						bFoundError= true;
						m_bFoundEndTag= true;
						m_sError= "ERROR ";
					}
				}
				if(line[i] == '<')
				{
					spos= i;
					m_sStartTag= "";
				}
			}
		}
	}
	if(m_bRead)
	{
		if(	spos > -1
			||
			epos > -1	)
		{
			if(spos < 0)
				spos= 0;
			if(epos < 0)
				line= line.substr(spos);
			else
				line= line.substr(spos, epos+1);
		}
		return line;
	}
	return "";
}

bool XMLStartEndTagReader::read()
{
	return m_bRead;
}

bool XMLStartEndTagReader::end()
{
	return m_bFoundEndTag;
}

string XMLStartEndTagReader::endTag()
{
	string sRv;

	if(!m_bFoundEndTag)
	{
		sRv= "</";
		sRv+= m_sStartTag;
		sRv+= ">\n";
	}
	return sRv;
}
