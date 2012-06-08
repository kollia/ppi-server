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

#include <string>

using namespace std;

enum PortExcept
{
	DEKLARATION= 0,
	UNKNOWN
};

class BaseException
{
	private:
		string m_sErrorHead;
		string m_sErrorText;

	public:
		BaseException(string heading, string error);
		~BaseException();

		const string getHeading();
		const string getErrorText();
		const string getMessage();
};

class PortException : public BaseException
{
	private:
		PortExcept m_status;

	public:
		PortException(string heading, PortExcept status, string error) :
			BaseException(heading, error),
			m_status(status) {};

		const string getMessage();
};