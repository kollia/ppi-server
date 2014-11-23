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

#include <cmath>

#include <iostream>
#include <sstream>

#include <boost/algorithm/string/trim.hpp>

#include "CalculatorContainer.h"

using namespace boost;

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
	if(	cOperator == '*' ||
		cOperator == '/' ||
		cOperator == '%'	)
	{
		if(	m_cOperator == '*' ||
			m_cOperator == '/' ||
			m_cOperator == '%' ||
			m_cOperatorBefore != '\0'	)
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

double CalculatorContainer::calc(double value1, const char op, const double value2)
{
	if(op == '+')
		value1+= value2;
	else if(op == '-')
		value1-= value2;
	else if(op == '*')
		value1*= value2;
	else if(op == '/')
	{
		if(value2 == 0)
		{
			if(m_bOutput)
				outputF(false, __FILE__, __LINE__, "{### division thru 0 : set resutl to 0}");
			value1= 0;
		}else
			value1/= value2;
	}else if(op == '%')
		value1= fmod(value1, value2);
	else
		value1= 0;
	return value1;
}

bool CalculatorContainer::render()
{
	CalculatorContainer calc;
	string::size_type nLen;
	double value;
	string full;
	string::size_type nPos= 0;
	int breaks= 0;
	ostringstream msg;

	// for debugging stop by an defined block
	// an block inside an calculation beginning with ( and ending with )
	// the variable of calcStringBegin should be some first characters of the block
	// without the block beginning of (
	/*string calcStringBegin("0.0000102102");
	if(	m_sStatement.length() >= calcStringBegin.length() &&
		m_sStatement.substr(0, calcStringBegin.length()) == calcStringBegin	)
	{
		cout << "CalculatorContainer::render() will be stopping by calculation of '" << m_sStatement << "'" << endl;
		cout << __FILE__ << " line:" << __LINE__ << endl;
	}*/
	if(m_bRendered)
		return m_bCorrect;
	full= m_sStatement;
	trim(full);
	nLen= full.length();
	if(full == "")
	{
		if(m_bNeed)
			output(true, __FILE__, __LINE__, "no calculation string be set");
		m_bRendered= true;
		m_bCorrect= false;
		return false;
	}
	if(m_bIfAllow)
	{// search first for if sentences
		while(nPos < nLen)
		{
			if(full[nPos] == '(')
				++breaks;
			else if(full[nPos] == ')')
				--breaks;
			else if(	full[nPos] == '?' &&
						breaks == 0				)
			{// calculate boolean for if sentence before question mark ('?')
				int x;

				m_poIf= newObject();
				m_poIf->m_bBool= true;
				m_poIf->m_bOutput= m_bOutput;
				m_poIf->m_bIfAllow= m_bIfAllow;
				m_poIf->statement(full.substr(0, nPos));
				m_bCorrect= m_poIf->render();
				if(!m_bCorrect)
				{
					m_sError= m_poIf->m_sError;
					removeObject(m_poIf);
					delete m_poIf;
					m_poIf= NULL;
					return false;
				}
				++nPos;
				x= nPos;
				while(nPos < nLen)
				{
					if(full[nPos] == '(')
						++breaks;
					else if(full[nPos] == ')')
						--breaks;
					else if(	full[nPos] == ':' &&
								breaks == 0				)
					{ // calculate result for true value (before colon (':'), or for false value (after colon (':')
						m_poThen= newObject();
						m_poThen->m_bBool= m_bBool;
						m_poThen->m_bOutput= m_bOutput;
						m_poThen->m_bIfAllow= m_bIfAllow;
						m_poThen->statement(full.substr(x, nPos - x));
						m_bCorrect= m_poThen->render();
						if(!m_bBool)
							m_bBool= m_poThen->m_bBool;
						if(!m_bCorrect)
						{
							m_sError= m_poThen->m_sError;
							removeObject(m_poThen);
							delete m_poThen;
							m_poThen= NULL;
							return false;
						}
						m_poElse= newObject();
						m_poElse->m_bBool= m_bBool;
						m_poElse->m_bOutput= m_bOutput;
						m_poElse->m_bIfAllow= m_bIfAllow;
						m_poElse->statement(full.substr(nPos + 1));
						m_bCorrect= m_poElse->render();
						if(!m_bBool)
							m_bBool= m_poElse->m_bBool;
						if(!m_bCorrect)
						{
							m_sError= m_poElse->m_sError;
							removeObject(m_poElse);
							delete m_poElse;
							m_poElse= NULL;
							return false;
						}
						findVariables();
						m_bRendered= true;
						return true;
					}
					++nPos;
				}// while(nPos < nLen)
				string msg("find no ");

				if(m_poThen == NULL)
					msg+= "then/";
				msg+= "else sentence in statement '"+m_sStatement+"'";
				outputF(true, __FILE__, __LINE__, msg);
				return false;
			}
			++nPos;
		}// while(nPos < nLen)
	} // if(m_bIfAllow)

	// create result of string

	/**
	 * whether the operator (minus or plus) is for an value.<br>
	 * if beginning to parsing and there is first an minus
	 * or plus, the operator is for the value.
	 * After an value is found bforVal should be false
	 * because the next operator have to be for calculation
	 */
	bool bforVal= true;
	bool bNewCont= false;
	string var, op;
	CalculatorContainer* lastCont;

	breaks= 0;
	nPos= 0;
	while(nPos < nLen)
	{
		var= "";
		if(full[nPos] == '(')
		{
			int nstart;

			nstart= nPos;
			++nPos;
			++breaks;
			while(nPos < nLen)
			{
				if(full[nPos] == '(')
					++breaks;
				else if(full[nPos] == ')')
				{
					--breaks;
					if(breaks == 0)
						break;
				}
				++nPos;
			}
			if(breaks != 0)
			{
				outputF(true, __FILE__, __LINE__, "found no correct count of closing breaks");
				m_bCorrect= false;
				return false;
			}
			m_vndoing.push_back(3); // new container
			m_voContainers.push_back(newObject());
			bNewCont= true;
			lastCont= dynamic_cast<CalculatorContainer*>(m_voContainers.back());
			lastCont->m_bBool= m_bBool;
			lastCont->m_bOutput= m_bOutput;
			lastCont->m_bIfAllow= m_bIfAllow;
			lastCont->statement(full.substr(nstart+1, nPos-1-nstart));
			m_bCorrect= lastCont->render();
			if(!m_bBool)
				m_bBool= lastCont->m_bBool;
			if(!m_bCorrect)
			{
				m_sError= lastCont->m_sError;
				m_voContainers.pop_back();
				removeObject(lastCont);
				delete lastCont;
				return false;
			}
			full= full.substr(nPos+1);
			nLen= full.length();
			nPos= -1;
			bforVal= false;

		}else if(	(	!bforVal &&
						(	full[nPos] == '+' ||
							full[nPos] == '-'	)	) ||
					full[nPos] == '/' ||
					full[nPos] == '*' ||
					full[nPos] == '%'					)
		{
			op= full[nPos];
			m_vcOperators.push_back(op);
			var= full.substr(0, nPos);
			bforVal= true;
			if(bNewCont)
			{
				full= full.substr(nPos + 1);
				nPos= -1;
				nLen= full.length();
				bNewCont= false;
			}

		}else if(	m_bComparison &&
					(	full[nPos] == '|' ||
						full[nPos] == '&' ||
						full[nPos] == '>' ||
						full[nPos] == '<' ||
						full[nPos] == '=' ||
						full[nPos] == '!'		)	)
		{
			m_bBool= true;
			op= full[nPos];
			var= full.substr(0, nPos);
			if(	(	full[nPos] == '>' ||
					full[nPos] == '<' ||
					full[nPos] == '!'	) &&
				full[nPos+1] == '='				)
			{
				op+= '=';
				++nPos;
			}
			bforVal= true;
			if(bNewCont)
			{
				full= full.substr(nPos + 1);
				nPos= -1;
				nLen= full.length();
				bNewCont= false;
			}
			m_vcOperators.push_back(op);

		}else if(	bforVal &&
					full[nPos] != ' '	)
		{// for beginning if there an minus or plus value
		 // calculate them to the value
		 // when ther any caracter found
		 // it's the plus or minus or any other number or variable
		 // the next plus or minus is an operator
			bforVal= false;// the next have to be an value

		}
		trim(var);
		if(	var != "" )
		{
			//if(!bNewCont)
			{
				if(searchResult(var, value))
				{
					m_vndoing.push_back(1); // value
					m_vdValues.push_back(value);
				}else
				{
					if(variable(&var, value))
					{
						m_vndoing.push_back(2); // variable
						m_vsVariables.push_back(var);

					}else
					{
						string msg("variable '"+var+"' is not set correctly");

						if(!m_bIfAllow && var.find("?"))
							msg+= " (look like an if sentence, but not allowed)";
						outputF(true, __FILE__, __LINE__, msg);
						m_bCorrect= false;
						return false;
					}
				}
			}

			full= full.substr(nPos + 1);
			nPos= -1;
			nLen= full.length();
			bforVal= true;
			bNewCont= false;
		}
		++nPos;
	}
	trim(full);
	if(full != "")
	{
		if(searchResult(full, value))
		{
			m_vndoing.push_back(1); // value
			m_vdValues.push_back(value);
		}else
		{
			if(variable(&full, value))
			{
				m_vndoing.push_back(2); // variable
				m_vsVariables.push_back(full);

			}else
			{
				string msg("variable '"+full+"' is not set correctly");

				if(!m_bIfAllow && full.find("?"))
					msg+= " (look like an if sentence, but not allowed)";
				outputF(true, __FILE__, __LINE__, msg);
				m_bCorrect= false;
				return false;
			}
		}
	}
	//cout << "doing steps:  " << m_vndoing.size() << endl;
	//cout << "operators:    " << m_vcOperators.size() << endl;
	//cout << "values:       " << m_vdValues.size() << endl;
	//cout << "variables:    " << m_vsVariables.size() << endl;
	//cout << "new container:" << m_voContainers.size() << endl;
	//cout << "---------------------------------------" << endl;
	if((m_vndoing.size()-1) != m_vcOperators.size())
	{
		ostringstream err;

		err << endl;
		err << "    by statement '" << m_sStatement << "'" << endl;
		err << "       doing steps:  " << m_vndoing.size() << endl;
		err << "       operators:    " << m_vcOperators.size() << "    ";
		for(vector<string>::iterator op= m_vcOperators.begin(); op != m_vcOperators.end(); ++op)
			err << *op << " ";
		err << endl;
		err << "       values:       " << m_vdValues.size() << "    ";
		for(vector<double>::iterator val= m_vdValues.begin(); val != m_vdValues.end(); ++val)
			err << *val << " ";
		err << endl;
		err << "       variables:    " << m_vsVariables.size() << "    ";
		for(vector<string>::iterator var= m_vsVariables.begin(); var != m_vsVariables.end(); ++var)
			err << *var << " ";
		err << endl;
		err << "       new container:" << m_voContainers.size() << endl;
		err << "    -----------------------------------------------------------" << endl;

		if((m_vndoing.size()-1) < m_vcOperators.size())
			outputF(true, __FILE__, __LINE__, "it does not exist enough values in statement" + err.str());
		else
			outputF(true, __FILE__, __LINE__, "it exist to much operators in statement" + err.str());
		m_bCorrect= false;
		return false;
	}
	findVariables();
	m_bRendered= true;
	return true;
}

bool CalculatorContainer::variable(string* var, double &dResult)
{
	outputF(true, __FILE__, __LINE__, "object of CalculatorContainer is not defined vor any variables (like '"+*var+"')");
	dResult= 0;
	return false;
}

void CalculatorContainer::findVariables()
{
	vector<string> variables;

	if(!m_bCorrect)
		return;
	m_vsAllVariables= m_vsVariables;
	for(vector<ICalculatorPattern*>::iterator it= m_voContainers.begin(); it != m_voContainers.end(); ++it)
	{
		variables= (*it)->getVariables();
		m_vsAllVariables.insert(m_vsAllVariables.end(), variables.begin(), variables.end());
	}
	if(m_poIf)
	{
		variables= m_poIf->getVariables();
		m_vsAllVariables.insert(m_vsAllVariables.end(), variables.begin(), variables.end());
	}
	if(m_poThen)
	{
		variables= m_poThen->getVariables();
		m_vsAllVariables.insert(m_vsAllVariables.end(), variables.begin(), variables.end());
	}
	if(m_poElse)
	{
		variables= m_poElse->getVariables();
		m_vsAllVariables.insert(m_vsAllVariables.end(), variables.begin(), variables.end());
	}
}

bool CalculatorContainer::searchResult(string var, double &dResult)
{
	bool bOk(true), bSetCol(false);
	string::size_type nLen;
	string::size_type nPos= 0;
	istringstream sResult(var);

	dResult= 0;
	nLen= var.length();
	while(nPos < nLen)
	{
		if(	!(	isdigit(var[nPos]) ||
				(	nPos == 0 && /*only for first digit*/
					nLen > 1 &&  /*have to be longer then one*/
					(	var[0] == '.' || /*maybe a float beginning with no 0*/
						var[0] == '-' ||
						var[0] == '+' 		)	) ||
				(	nPos > 0 &&
					bSetCol == false &&
					var[nPos] == '.' 		)			)	)
		{
			// string is no correct number
			bOk= false;
			break;
		}
		if(var[nPos] == '.')
			bSetCol= true;
		++nPos;
	}
	if(!bOk)
	{// when string is no number look whether string are the characters true or false
		transform(var.begin(), var.end(), var.begin(), (int(*)(int)) toupper);
		if(var == "TRUE")
		{
			dResult= 1;
			return true;

		}else if(var == "FALSE")
		{
			dResult= 0;
			return true;
		}
	}
	if(bOk)
		sResult >> dResult;
	return bOk;
}

bool CalculatorContainer::calculate(double& dResult)
{
	bool correct;

	if(m_sStatement == "")
	{
		dResult= 0;
		return false;
	}
	if(m_bOutput)
	{
		bool rendered(m_bRendered);

		m_bRendered= true;
		outputF(false, __FILE__, __LINE__, "calculate('"+m_sStatement+"')");
		outputF(false, __FILE__, __LINE__, "\n");
		m_bRendered= rendered;
	}
	if(!m_bCorrect)
	{
		dResult= 0;
		if(	m_bNeed == false &&
			m_sStatement == ""	)
		{
			return false;
		}
		output(true, __FILE__, __LINE__, "ERROR: " + m_sError + "\n");
		return false;
	}
	if(!m_bRendered)
	{
		m_bCorrect= render();
		if(!m_bCorrect)
		{
			dResult= 0;
			m_bRendered= true;
			return false;
		}
	}
	correct= calculateI(dResult);
	if(m_bOutput && !m_poIf)
		outputF(false, __FILE__, __LINE__, "\n");
	return correct;
}

const vector<ICalculatorPattern*> CalculatorContainer::getChilds() const
{
	vector<ICalculatorPattern*> vRv;

	vRv= m_voContainers;
	if(m_poIf)
		vRv.push_back(m_poIf);
	if(m_poThen)
		vRv.push_back(m_poThen);
	if(m_poElse)
		vRv.push_back(m_poElse);
	return vRv;
}

bool CalculatorContainer::calculateI(double& dResult)
{
	bool correct;
	double result;
	string comp;
	vector<double>::iterator itValue;
	vector<string>::iterator itVariable;
	vector<ICalculatorPattern*>::iterator itContainer;
	CalculatorContainer* itContainerContent;
	vector<string>::iterator itOperator;

	if(m_poIf)
	{

		if(m_bOutput)
		{
			outputF(false, __FILE__, __LINE__, "if: ");
			m_poIf->m_nSpaces= m_nSpaces + 4;
		}
		dResult= 0;
		correct= m_poIf->calculateI(result);
		if(m_bOutput)
			outputF(false, __FILE__, __LINE__, "\n");
		if(!correct)
			return false;
		if(result < 0 || result > 0)
		{
			if(m_bOutput)
			{
				outputF(false, __FILE__, __LINE__, "  then: ");
				m_poThen->m_nSpaces= m_nSpaces + 8;
			}
			correct= m_poThen->calculateI(result);
		}else
		{
			if(m_bOutput)
			{
				outputF(false, __FILE__, __LINE__, "  else: ");
				m_poElse->m_nSpaces= m_nSpaces + 8;
			}
			correct= m_poElse->calculateI(result);
		}
		if(m_bOutput)
			outputF(false, __FILE__, __LINE__, "\n");
		if(!correct)
			return false;
		dResult= result;
		return true;
	}
	itValue= m_vdValues.begin();
	itVariable= m_vsVariables.begin();
	itContainer= m_voContainers.begin();
	if(m_voContainers.size())
		itContainerContent= dynamic_cast<CalculatorContainer*>(*itContainer);
	else
		itContainerContent= NULL;
	itOperator= m_vcOperators.begin();
	for(vector<short>::iterator doing= m_vndoing.begin(); doing != m_vndoing.end(); ++doing)
	{
		switch(*doing)
		{
		case 1: // double value
			m_dValue= *itValue;
			if(m_bOutput)
			{
				ostringstream str;
				string out;
				string::size_type nLen, n;

				str << fixed << m_dValue;
				out= str.str();
				nLen= out.length();
				for(n= nLen - 1; n > 0; --n)
				{
					if(out[n] != '0')
						break;
				}
				if(out[n] == '.')
					--n;
				++n;
				outputF(false, __FILE__, __LINE__, out.substr(0, n));
			}
			++itValue;
			break;
		case 2: // variable type
			correct= variable(&(*itVariable), m_dValue);
			if(!correct)
			{
				outputF(true, __FILE__, __LINE__, "[cannot found var '"+(*itVariable)+"']");
				m_dValue= 0;
			}
			if(m_bOutput)
			{
				ostringstream str;
				string out;
				string::size_type nLen, n;

				str << fixed << m_dValue;
				out= str.str();
				nLen= out.length();
				for(n= nLen - 1; n > 0; --n)
				{
					if(out[n] != '0')
						break;
				}
				if(out[n] == '.')
					--n;
				++n;
				str.str("");
				str << "[" << *itVariable << "=" << out.substr(0, n) << "] ";
				outputF(false, __FILE__, __LINE__, str.str());
			}
			++itVariable;
			break;
		case 3: // new container
			if(m_bOutput)
			{
				outputF(false, __FILE__, __LINE__, "(");
				itContainerContent->m_nSpaces= m_nSpaces + 1;
			}
			correct= itContainerContent->calculateI(m_dValue);
			if(m_bOutput)
				outputF(false, __FILE__, __LINE__, ") ");
			if(!correct)
				m_dValue= 0;
			++itContainer;
			if(itContainer != m_voContainers.end())
				itContainerContent= dynamic_cast<CalculatorContainer*>(*itContainer);
			else
				itContainerContent= NULL;
			break;
		}
		if(itOperator != m_vcOperators.end())
		{
			if(	*itOperator == "+" ||
				*itOperator == "-" ||
				*itOperator == "*" ||
				*itOperator == "/" ||
				*itOperator == "%"		)
			{
				if(m_bOutput)
					outputF(false, __FILE__, __LINE__, *itOperator+" ");
				add((*itOperator)[0]);
				++itOperator;

			}else if(	(*itOperator)[0] == '=' ||
						(*itOperator)[0] == '<' ||
						(*itOperator)[0] == '>' ||
						(*itOperator)[0] == '!'		)
			{
				if(m_bOutput)
				{
					string str(*itOperator);

					//if(*itOperator == "=")
					//	str+= "=";
					str+= " ";
					outputF(false, __FILE__, __LINE__, str);
				}
				result= getResult();
				comp= *itOperator;
				++itOperator;

			}else if(	*itOperator == "|" ||
						*itOperator == "&"		)
			{
				if(comp == "")
				{
					dResult= getResult();
					if(	dResult > 0 ||
						dResult < 0		)
					{
						dResult= 1;
					}else
						dResult= 0;
				}else
				{
					double od= getResult();
					dResult= compare(result, comp, od);
					comp= "";
				}
				if(*itOperator == "&")
				{
					if(	!(	dResult > 0 ||
							dResult < 0		)	)
					{
						if(m_bOutput)
							outputF(false, __FILE__, __LINE__, "{break by FALSE}");
						dResult= 0;
						return true;
					}
				}else // (*itOperator == "|")
				{
					if(	dResult > 0 ||
						dResult < 0		)
					{
						if(m_bOutput)
							outputF(false, __FILE__, __LINE__, "{break by TRUE}");
						dResult= 1;
						return true;
					}
				}
				if(m_bOutput)
				{
					string msg(" ");

					msg+= *itOperator + " ";
					outputF(false, __FILE__, __LINE__, msg);
				}
				comp= "";
				++itOperator;
			}
		}
	}
	if(comp != "")
	{
		dResult= compare(result, comp, getResult());
		if(	dResult > 0 ||
			dResult < 0		)
		{
			if(m_bOutput)
				outputF(false, __FILE__, __LINE__, "{result TRUE}");
			dResult= 1;
		}else
		{
			if(m_bOutput)
				outputF(false, __FILE__, __LINE__, "{result FALSE}");
		}
	}else
	{
		dResult= getResult();
		if(m_bOutput)
		{
			ostringstream str;

			str << " {result ";
			if(m_bBool)
			{
				if(	dResult > 0 ||
					dResult < 0		)
				{
					str << "TRUE";
				}else
					str << "FALSE";
			}else
				str << dResult;
			str << "}";
			outputF(false, __FILE__, __LINE__, str.str());
		}
	}
	return true;
}

void CalculatorContainer::doOutput(const bool write/*= true*/)
{
	m_bOutput= write;
	if(m_poIf)
		m_poIf->doOutput(write);
	if(m_poThen)
		m_poThen->doOutput(write);
	if(m_poElse)
		m_poElse->doOutput(write);
	for(vector<ICalculatorPattern*>::iterator it= m_voContainers.begin(); it != m_voContainers.end(); ++it)
		dynamic_cast<CalculatorContainer*>(*it)->doOutput(write);
}

double CalculatorContainer::compare(const double value1, const string& op, const double value2)
{
	double result= 0;

	if(op == "=")
	{
		// do not compare double and double with is equal sign ('==')
		if(value1 < value2 || value1 > value2)
			result= 0;
		else
			result= 1;

	}else if(op == "!=")
	{
		// do not compare double and double with not equal sign ('!=')
		if(value1 < value2 || value1 > value2)
			result= 1;

	}else if(op == ">=")
	{
		// do not compare double and double with is equal sign ('==')
		if(value1 > value2)
			result= 1;
		else if(value1 < value2)
			result= 0;
		else
			result= 1;

	}else if(op == "<=")
	{
		// do not compare double and double with is equal sign ('==')
		if(value1 < value2)
			result= 1;
		else if(value1 > value2)
			result= 0;
		else
			result= 1;

	}else if(op == ">")
	{
		if(value1 > value2)
			result= 1;

	}else if(op == "<")
	{
		if(value1 < value2)
			result= 1;
	}
	return result;
}

void CalculatorContainer::outputF(bool bError, const string& file, const int line, const string& msg)
{
	if(!m_bRendered && msg != "\n")
		m_sError= msg;
	output(bError, file, line, msg);
}

void CalculatorContainer::output(bool bError, const string& file, const int line, const string& msg)
{
	ostringstream o;

	if(m_funcMessage)
	{
		m_funcMessage(true, __FILE__, __LINE__, msg);
		return;
	}
	if(!m_bRendered)
	{
		if(bError)
			o << "ERROR: ";
		else
			o << "WARNING: ";
		o << msg << endl;
		out(bError, o.str());
		return;
	}
	if(	m_bOutput || 
		(	bError &&
			m_bShowErrors	)	)
	{
		if(msg == "\n")
		{
			o << endl;
			for(unsigned short n= 0; n < m_nSpaces; ++n)
				o << " ";
		}else
			o << msg << flush;
		out(bError, o.str());
	}
}

bool CalculatorContainer::onlyNumbers() const
{
	if(	isRendered() &&
		!isEmpty() &&
		isCorrect() &&
		m_vdValues.size() > 0 &&
		m_vsVariables.size() == 0	)
	{
		for(vector<ICalculatorPattern*>::const_iterator it= m_voContainers.begin(); it != m_voContainers.end(); ++it)
		{
			if(!(*it)->onlyNumbers())
				return false;
		}
		return true;
	}
	return false;
}

void CalculatorContainer::clear()
{
	m_sStatement= "";
	m_bCorrect= true;
	m_bBool= m_bCBool;
	m_bRendered= false;
	if(m_poIf)
	{
		removeObject(m_poIf);
		delete m_poIf;
		m_poIf= NULL;
	}
	if(m_poThen)
	{
		removeObject(m_poThen);
		delete m_poThen;
		m_poThen= NULL;
	}
	if(m_poElse)
	{
		removeObject(m_poElse);
		delete m_poElse;
		m_poElse= NULL;
	}
	m_vndoing.clear();
	m_vdValues.clear();
	m_vsVariables.clear();
	m_vsAllVariables.clear();
	m_vcOperators.clear();
	for(vector<ICalculatorPattern*>::iterator it= m_voContainers.begin(); it != m_voContainers.end(); ++it)
	{
		removeObject(dynamic_cast<CalculatorContainer*>(*it));
		delete *it;
	}
	m_voContainers.clear();
	m_nSpaces= 0;
	m_sError= "";
}
