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
#ifndef ICALCULATORPATTERN_H_
#define ICALCULATORPATTERN_H_

#include <string>

using namespace std;

namespace design_pattern_world
{

	/**
	 * pattern to create calculation
	 *
	 */
	class ICalculatorPattern
	{
	public:
		/**
		 * set string to calculate
		 *
		 * @param str string which should be calculated
		 */
		virtual void statement(const string& str)= 0;
		/**
		 * return actual string statement
		 */
		virtual string getStatement() const= 0;
		/**
		 * return all defined child containers
		 *
		 * @return child containers
		 */
		virtual const vector<ICalculatorPattern*> getChilds() const= 0;
		/**
		 * return all variables which are defined inside the calculation string
		 *
		 * @return all variables
		 */
		virtual vector<string> getVariables() const= 0;
		/**
		 * whether object has no statement to calculate
		 */
		virtual bool isEmpty() const= 0;
		/**
		 * whether rendering was correct
		 */
		virtual bool isCorrect() const= 0;
		/**
		 * whether calculation string has only fix numbers
		 */
		virtual bool onlyNumbers() const= 0;
		/**
		 * render string for faster calculation
		 *
		 * @return whether rendering was correct
		 */
		virtual bool render()= 0;
		/**
		 * allow if sentence inside of statement
		 *
		 * @param allow whether if sentence is allowed
		 */
		virtual void allowIfSentence(bool allow)= 0;
		/**
		 * whether string statement is rendered
		 */
		virtual bool isRendered() const= 0;
		/**
		 * write calculation output over message function getting in constructor
		 *
		 * @param write wheter should write
		 */
		virtual void doOutput(bool write)= 0;
		/**
		 * return whether output is set
		 *
		 * @return whether output is set
		 */
		virtual bool doOutput() const= 0;
		/**
		 * calculate string of statement
		 *
		 * @param dResult result of calculation
		 * @return whether the calculation was correct
		 */
		virtual bool calculate(double& dResult)= 0;
		/**
		 * this method will be called when any variable be used
		 *
		 * @param var name of variable
		 * @param dResult result of var
		 * @param whether found correct variable
		 */
		virtual bool variable(const string& var, double& dResult)= 0;
		/**
		 * get last render error if exist
		 */
		virtual string getRenderError() const= 0;
		/**
		 * dummy destructor of design pattern
		 */
		virtual ~ICalculatorPattern() {};

	};

}

#endif /* ICALCULATORPATTERN_H_ */
