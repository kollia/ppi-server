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
#ifndef CALCULATORCONTAINER_H_
#define CALCULATORCONTAINER_H_

/**
 * this class calculate all inserted double values to an value
 *
 */
class CalculatorContainer
{
private:
	/**
	 * the last setting actual value
	 */
	double m_dValue;
	/**
	 * the Value before
	 * if the next value be for multiple or division
	 */
	double m_dValueHolder;
	/**
	 * the hole result value
	 */
	double m_dResultValue;
	/**
	 * the operator from before
	 * if the next operator is * or /
	 */
	char m_cOperatorBefore;
	/**
	 * the last operator
	 */
	char m_cOperator;

public:
	/**
	 * instanciate object with values 0
	 */
	CalculatorContainer();
	/**
	 * calculate the two double values
	 * with the operator
	 */
	static double calc(const double oldValue, const char op, const double value);
	/**
	 * method add's the next operator for calculate
	 *
	 * @param cOperator operator character (+, - , * or /)
	 */
	bool add(char cOpertor);
	/**
	 * method add's the next value for calculate
	 *
	 * @param value double value
	 */
	void add(double value);
	/**
	 * returning the calculated value
	 * and clear the value which be inherit
	 *
	 * @return calculated value
	 */
	double getResult();
};

#endif /*CALCULATORCONTAINER_H_*/
