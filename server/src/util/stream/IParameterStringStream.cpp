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

#include <iostream>

#include <boost/algorithm/string/replace.hpp>

#include "IParameterStringStream.h"

using namespace boost::algorithm;

namespace util {

IParameterStringStream::IParameterStringStream(const string& stream)
:	m_sStream(stream),
	m_bFail(false),
	m_bNull(false)
{

}

void IParameterStringStream::operator >> ( bool& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= false;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= false;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		string end;
		istringstream op(param);

		op.setf(ios_base::boolalpha);
		op >> value;
		bFail= op.fail();
		op >> end;
		if(	bFail &&
			end == ""	)
		{
			double dvalue;
			istringstream op2(param);

			// maybe parameter is no string of true or false but an number
			op2 >> dvalue;
			bFail= op2.fail();
			end= ""; // reading end can be not defined by EOF
			op2 >> end;
			if( bFail ||
				end != ""	)
			{
				bFail= true;

			}else if(	(	dvalue < 0 ||
							dvalue > 0 		) &&
						(	dvalue < 1 ||
							dvalue > 1		)	)
			{
				bFail= true;

			}else
				value= static_cast<bool>(dvalue);

		}if(end != "")
			bFail= true;
		if(bFail)
		{
			string::size_type npos;

			m_bFail= true;
			value= false;
			if(pos >= 0)
			{
				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( short& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof()	)
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail ||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( unsigned short& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail
			||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( int& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail
			||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( unsigned int& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail ||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( long& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail ||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( unsigned long& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail ||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( float& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail ||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> ( double& value)
{
	string param;
	streampos pos= m_sStream.tellg();

	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= 0;
		return;
	}
	m_sStream >> param;
	if(param == "")
	{
		value= 0;
		m_bNull= true;
		m_bFail= true;
		return;
	}else
	{
		bool bFail;
		istringstream op(param);

		op >> value;
		bFail= op.fail();
		param= ""; // reading param can be not defined by EOF
		op >> param;
		if( bFail ||
			param != ""	)
		{
			value= 0;
			m_bFail= true;
			if(pos >= 0)
			{
				string::size_type npos;

				npos= static_cast<string::size_type>(pos);
				m_sStream.seekg(npos, ios::beg);
			}else
				m_sStream.seekg(0, ios::end);
			m_sStream.clear();
		}
	}
}

void IParameterStringStream::operator >> (string& value)
{
	getString(value);
}

void IParameterStringStream::getString(string& value)
{
	short nBackS(0);
	string sStream;
	string sBuf;
	string::size_type nVLen, nBefLen, nCur;
	istringstream::pos_type pos= m_sStream.tellg();
	string::size_type npos;


	m_bFail= false;
	if(	m_bNull ||
		m_sStream.eof() )
	{
		m_bNull= true;
		m_bFail= true;
		value= "";
		return;
	}
	m_sStream >> sBuf;
	if(sBuf == "")
	{
		value= "";
		m_bNull= true;
		m_bFail= true;
		return;
	}
	if(sBuf.substr(0, 1) != "\"")
	{
		m_bFail= true;
		value= "";
		if(pos >= 0)
		{
			npos= static_cast<string::size_type>(pos);
			m_sStream.seekg(npos, ios::beg);
		}else
			m_sStream.seekg(0, ios::end);
		m_sStream.clear();
		return;
	}
	if(sBuf == "\"\"") // empty string
	{
		value= "";
		return;
	}
	pos= m_sStream.tellg();
	if(pos >= 0)
	{
		sStream= m_sStream.str();
		npos= static_cast<string::size_type>(pos);
		sStream= sStream.substr(npos);
	}
	if(sBuf.size() > 1)
	{
		nBefLen= sBuf.length();
		sStream= sBuf.substr(1) + sStream;
	}else
		nBefLen= 0;
	nVLen= sStream.length();
	nCur= 0;
	value= "";
	while(	nCur < nVLen &&
			(	sStream[nCur] != '"' ||
				nBackS % 2				)	)
	{
		if(sStream[nCur] == '\\')
			++nBackS;
		else
			nBackS= 0;
		value+= sStream[nCur];
		++nCur;
	}
	if(pos >= 0)
	{
		npos= static_cast<string::size_type>(pos);
		m_sStream.seekg(npos+nCur+3-nBefLen, ios::beg);
	}
	replace_all(value, "\\\\", "\\");
	replace_all(value, "\\\"", "\"");
	replace_all(value, "\\n", "\n");
}

bool IParameterStringStream::empty()
{
	bool bRv;
	streampos pos= m_sStream.tellg();
	string param;

	if(m_bNull)
		return true;
	bRv= m_sStream.eof();
	if(!bRv)
	{
		m_sStream >> param;
		if(param != "")
		{
			string::size_type npos;

			npos= static_cast<string::size_type>(pos);
			m_sStream.seekg(npos, ios::beg);
			m_sStream.clear();
			m_bNull= false;
		}else
			bRv= true;
	}
	return bRv;
}
}// namespace util
