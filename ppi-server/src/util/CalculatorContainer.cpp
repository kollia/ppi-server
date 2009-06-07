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
#include "CalculatorContainer.h"

CalculatorContainer::CalculatorContainer()
{
	m_dValue= 0;
	m_dValueHolder= 0;
	m_dResultValue= 0;
	m_cOperator= '\0';
	m_cOperatorBefore= '\0';
}
bool CalculatorContainer::add(char cOperator)
{
	if(m_cOperator == '\0')
	{
		m_dResultValue= m_dValue;
		//m_dValueHolder= m_dValue;
		m_dValue= 0;
		m_cOperator= cOperator;
		return true;
	}
	if(	cOperator == '+'
		||
		cOperator == '-'	)
	{
		m_dResultValue= calc(m_dResultValue, m_cOperator, m_dValue);
		m_dValue= 0;
		m_cOperator= cOperator;
		if(m_cOperatorBefore != '\0')
		{
			m_dResultValue= calc(m_dResultValue, m_cOperatorBefore, m_dValueHolder);
			m_cOperatorBefore= '\0';
		}
		return true;
	}
	if(	cOperator == '*'
		||
		cOperator == '/'	)
	{
		if(m_cOperatorBefore != '\0')
		{
			m_dResultValue= calc(m_dResultValue, m_cOperator, m_dValue);
			m_dValue= 0;
			m_cOperator= cOperator;
			return true;
		}
		m_dValueHolder= m_dResultValue;
		m_dResultValue= m_dValue;
		m_dValue= 0;
		m_cOperatorBefore= m_cOperator;
		m_cOperator= cOperator;
		return true;
	}
	return false;
}

void CalculatorContainer::add(double value)
{
	m_dValue= value;
	return;
	if(m_cOperator == '\0')
		m_dValue= value;
	else if(	m_cOperator == '+'
				||
				m_cOperator == '-'	)
	{
		if(m_cOperatorBefore != '\0')
			m_dValueHolder= calc(m_dValueHolder, m_cOperator, m_dValue);
		else
			m_dValueHolder= m_dValue;
		m_dValue= value;
	}else if(m_cOperator == '*')
		m_dValue*= value;
	else if(m_cOperator == '/')
		m_dValue/= value;
}

double CalculatorContainer::getResult()
{
	double dRv;

	add('+');
	dRv= m_dResultValue;
	m_dValue= 0;
	m_dValueHolder= 0;
	m_dResultValue= 0;
	m_cOperator= '\0';
	m_cOperatorBefore= '\0';
	return dRv;
}

double CalculatorContainer::calc(const double oldValue, const char op, const double value)
{
	double dRv= oldValue;

	if(op == '+')
		dRv+= value;
	else if(op == '-')
		dRv-= value;
	else if(op == '*')
		dRv*= value;
	else if(op == '/')
		dRv/= value;
	else
		dRv= 0;
	return dRv;
}
