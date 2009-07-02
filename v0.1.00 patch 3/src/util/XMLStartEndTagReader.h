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
#ifndef XMLSTARTENDTAGREADER_H_
#define XMLSTARTENDTAGREADER_H_


#include <string>

using namespace std;

/**
 * Class reading from the incomming strings
 * and define the start- and end-tag.<br />
 *
 * @author Alexander Kolli
 * @version 1.0.0
 */
class XMLStartEndTagReader
{
private:
	/**
	 * displays whether looking for end-tag
	 */
	bool m_bEnd;
	/**
	 * displays whether the content should be read
	 */
	bool m_bRead;
	/**
	 * inherits the tag name of the first tag
	 */
	string m_sStartTag;
	/**
	 * displays whether the end-tag found in the lines
	 */
	bool m_bFoundEndTag;
	/**
	 * error if get one from server
	 */
	string m_sError;

public:
	/**
	 * Initializing the start variables from reader
	 */
	XMLStartEndTagReader();
	/**
	 * reads the line to now whether should be display
	 * and to know when the end is reachen
	 *
	 * @param line read line
	 * @return the right line
	 */
	string readLine(string line);
	/**
	 * displays whether the content should be read
	 *
	 * @return true if the content should read, otherwise false
	 */
	bool read();
	/**
	 * displays whether the end is reached
	 *
	 * @return true if the end is reachen, otherwise false
	 */
	bool end();
	/**
	 * display errorcode from server if any set
	 *
	 * @return error code
	 */
	string error();
	/**
	 * return the end-tag if it was not in the content,
	 * otherwise an empty string.
	 *
	 * @return end-tag
	 */
	string endTag();
};

#endif /*XMLSTARTENDTAGREADER_H_*/
