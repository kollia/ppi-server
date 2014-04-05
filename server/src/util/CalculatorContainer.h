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

#include <string>
#include <vector>

#include "../pattern/util/ICalculatorPattern.h"

using namespace design_pattern_world;
using namespace std;

/**
 * this class calculate all inserted double values to an value
 *
 */
class CalculatorContainer : virtual public ICalculatorPattern
{
public:
	/**
	 * Instantiate object with values 0
	 *
	 * @param need whether string be set in init() method or statement() have to exist (defualt= true)
	 * @param boolean whether should show true and false for result by output
	 * @param message static void function to display on screen or in log file<br />for display only on screen use <code>CalculatorContainer::message</code>.
	 */
	CalculatorContainer(bool need= true, bool boolean= false, void (*message)(const bool, const string&, const int, const string&)= NULL)
	: 	m_bRendered(false),
	  	m_bCorrect(true),
	  	m_bBool(boolean),
	  	m_bCBool(boolean),
	  	m_bComparison(false),
	  	m_bIfAllow(false),
	  	m_bShowErrors(false),
	  	m_bOutput(false),
	  	m_nSpaces(0),
		m_dValue(0),
		m_dValueHolder(0),
		m_dResultValue(0),
		m_cOperatorBefore('\0'),
		m_cOperator('\0'),
		m_bNeed(need),
		m_poIf(NULL),
		m_poThen(NULL),
		m_poElse(NULL),
		m_funcMessage(message)
	{ };
	/**
	 * set string to calculate
	 *
	 * @param str string which should be calculated
	 */
	virtual void statement(const string& str)
	{ clear(); m_sStatement= str; };
	/**
	 * return actual string statement
	 */
	virtual string getStatement() const
	{ return m_sStatement; };
	/**
	 * whether object has no statement to calculate
	 * or rendering wasn't correct
	 */
	virtual bool isEmpty() const
	{ return m_sStatement == "" || !m_bCorrect ? true : false; };
	/**
	 * whether rendering was correct
	 */
	virtual bool isCorrect() const
	{ return m_bCorrect; };
	/**
	 * whether calculation string has only fix numbers
	 */
	virtual bool onlyNumbers() const;
	/**
	 * render string for faster calculation
	 *
	 * @return whether rendering was correct
	 */
	virtual bool render();
	/**
	 * whether string statement is rendered
	 */
	virtual bool isRendered() const
	{ return m_bRendered; };
	/**
	 * whether CalculatorContainer should only show errors.<br />
	 * This behavior is also activated by <code>doOutput()</code>.<br />
	 * Default setting is <code>false</code>.
	 *
	 * @param show whether should shown errors
	 */
	virtual void showErrors(const bool show)
	{ m_bShowErrors= show; };
	/**
	 * return whether should shown errors
	 *
	 * @return whether should shown errors
	 */
	virtual bool showErrors()
	{ if(m_bOutput || m_bShowErrors) return true; else return false; };
	/**
	 * write calculation output over message function getting in constructor.<br />
	 * Default setting is <code>false</code>.
	 *
	 * @param write wheter should write
	 */
	virtual void doOutput(const bool write);
	/**
	 * return whether output is set
	 *
	 * @return whether output is set
	 */
	virtual bool doOutput() const
	{ return m_bOutput; };
	/**
	 * calculate the two double values
	 * with the operator
	 *
	 * @param value1 first double value
	 * @param op operator character (+, - , * or /)
	 * @param value2 second value to add, substract, ... with operator to oldValue
	 * @return result of calculation
	 */
	double calc(double value1, const char op, const double value2);
	/**
	 * calculate string of statement
	 *
	 * @param dResult result of calculation
	 * @return whether the calculation was correct
	 */
	virtual bool calculate(double& dResult);
	/**
	 * allow comparison (=, !=, < , <=, >, >=) between values
	 * or (&, |) between comparison
	 *
	 * @param allow whether comparison is allowed (default:false)
	 */
	virtual void allowComparison(bool allow)
	{ m_bComparison= allow; };
	/**
	 * allow if sentence inside of statement.<br />
	 * When in the statement variables allowed with an colon
	 * or you want to set inside the if sentence an second one,
	 * write this inside of brackets.
	 *
	 * @param allow whether if sentence is allowed (default:false)
	 */
	virtual void allowIfSentence(bool allow)
	{ m_bIfAllow= allow; };
	/**
	 * return all variables which are defined inside the calculation string
	 *
	 * @return all variables
	 */
	virtual vector<string> getVariables() const
			{ return m_vsAllVariables; };
	/**
	 * compare value1 with value2
	 *
	 * @param value1 first value
	 * @param op comparator between values
	 * @param value2 second value
	 * @return double 1 when comparison was true, elsewhere 0
	 */
	double compare(const double value1, const string& op, const double value2);
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
	void add(double value)
	{ m_dValue= value; };
	/**
	 * returning the calculated value
	 * and clear the value which be inherit
	 *
	 * @return calculated value
	 */
	inline double getResult();
	/**
	 * refresh object for new rendering
	 */
	void clear();
	/**
	 * set render error
	 */
	virtual void setRenderError(const string& error)
	{ m_sError= error; }
	/**
	 * get last render error if exist
	 */
	virtual string getRenderError() const
	{ return m_sError; }
	/**
	 * destructor of class
	 */
	virtual ~CalculatorContainer()
	{ clear(); };

protected:
	/**
	 * creating of new objects of CalculatorContainer
	 * to overload when an other object needed
	 */
	virtual CalculatorContainer* newObject()
		{ return new CalculatorContainer(m_funcMessage); };
	/**
	 * remove an created object.<br />
	 * if the overloaded class save also the new containers in an vector
	 * or what else, this object should also delete the class object from the container
	 * but should not delete the object, because than this case create an segmentation fault
	 */
	virtual void removeObject(const CalculatorContainer* const object)
	{ /* dummy method do nothing */ };
	/**
	 * method to writing only inside of terminal (cout)
	 *
	 * @param bError whether message is an error of rendering
	 * @param file name of source file where the message occurs
	 * @param line in which line of source file the message occurs
	 * @param msg string of message
	 */
	virtual void output(bool bError, const string& file, const int line, const string& msg);
	/**
	 * direct output called from method <code>output()</code>
	 *
	 * @param bError whether output should be an error
	 * @param msg output string
	 */
	virtual void out(const bool bError, const string& msg)
	{ if(bError) cerr << msg; else cout << msg; };
	/**
	 * this method is only to overload by an new child, otherwise variables not be allowed.<br />
	 * This method will be also called by rendering, when overload class give back (by rendering)
	 * an error, method <code>calculate()</code> do not calculate string by next call.
	 * If you do not know the value by first creation, but you want render immediately, ask for
	 * <code>isRendered()</code>. When method is <code>false</code> give back true.
	 * In this case by every calculation and <code>doOutput()</code> be set calculation is written
	 * only by this fault variable an fail message.
	 *
	 * @param var name of variable, can give back other correct variable name
	 * @param dResult result of var
	 * @param whether found correct variable
	 */
	virtual bool variable(string* var, double& dResult);
	/**
	 * return all defined child containers
	 *
	 * @return child containers
	 */
	virtual const vector<ICalculatorPattern*> getChilds() const;
	/**
	 * search for all variables in any container
	 */
	virtual void findVariables();

private:
	/**
	 * whether statement is rendered
	 */
	bool m_bRendered;
	/**
	 * whether object of CalculatorConstainer has an correct statement
	 */
	bool m_bCorrect;
	/**
	 * whether result is an boolean.<br />
	 * Only for more pretty output
	 */
	bool m_bBool;
	/**
	 * whether result from constructor is to calculate for boolean.<br />
	 * Only for more pretty output
	 */
	const bool m_bCBool;
	/**
	 * whether an comparison is allowed
	 */
	bool m_bComparison;
	/**
	 * whether if sentence should be allowed
	 */
	bool m_bIfAllow;
	/**
	 * whether should shown errors
	 */
	bool m_bShowErrors;
	/**
	 * whether should write output over message function (getting in constructor),
	 * show also errors
	 */
	bool m_bOutput;
	/**
	 * how much spaces should set by calculation output before string
	 */
	unsigned short m_nSpaces;
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
	/**
	 * whether parameter have to be set
	 */
	bool m_bNeed;
	/**
	 * sting of statement to calculate
	 */
	string m_sStatement;
	/**
	 * calculation of if sentence
	 */
	CalculatorContainer* m_poIf;
	/**
	 * calculation when if sentence was true
	 */
	CalculatorContainer* m_poThen;
	/**
	 * calculation when if sentence was false
	 */
	CalculatorContainer* m_poElse;
	/**
	 * action for calculating with next step
	 * <table>
	 *   <tr>
	 *     <td>
	 *       1
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       double value
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>
	 *       2
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       variable type
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>
	 *       3
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       new container
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>
	 *       4
	 *     </td>
	 *     <td>
	 *       -
	 *     </td>
	 *     <td>
	 *       next step is / or *
	 *     </td>
	 *   </tr>
	 */
	vector<short> m_vndoing;
	/**
	 * all values for calculating
	 */
	vector<double> m_vdValues;
	/**
	 * all variables for calculating
	 */
	vector<string> m_vsVariables;
	/**
	 * all variables found inside any container
	 */
	vector<string> m_vsAllVariables;
	/**
	 * vector of all container
	 */
	vector<ICalculatorPattern*> m_voContainers;
	/**
	 * vector of all operators
	 */
	vector<string> m_vcOperators;
	/**
	 * rendering error for next calls to output
	 */
	string m_sError;

	CalculatorContainer(const CalculatorContainer&);
	CalculatorContainer& operator=(const CalculatorContainer&);
	/**
	 * object to write for log messages
	 *
	 * @param bError whether message is an error of rendering
	 * @param file name of source file where the message occurs
	 * @param line in which line of source file the message occurs
	 * @param msg string of message
	 */
	void (*m_funcMessage)(const bool bError, const string& file, const int line, const string& msg);
	/**
	 * calculate string of statement
	 *
	 * @param dResult result of calculation
	 * @return whether the calculation was correct
	 */
	bool calculateI(double& dResult);
	/**
	 * look whether variable is an double number, boolean or an variable name (when undefined)
	 *
	 * @param var string from statement which should controlled
	 * @param dResult value from var
	 * @return whether var was an correct number or boolean, elsewhere should be an variable
	 */
	bool searchResult(string var, double& dResult);
	/**
	 * intern method to writing output and set m_sError for all next calls
	 *
	 * @param bError whether message is an error of rendering
	 * @param file name of source file where the message occurs
	 * @param line in which line of source file the message occurs
	 * @param msg string of message
	 */
	void outputF(bool bError, const string& file, const int line, const string& msg);
};

#endif /*CALCULATORCONTAINER_H_*/
