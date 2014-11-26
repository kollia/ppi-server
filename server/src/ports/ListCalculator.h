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
#ifndef LISTCALCULATOR_H_
#define LISTCALCULATOR_H_

#include <string>
#include <map>

#include "../pattern/util/IListObjectPattern.h"
#include "../pattern/util/imeasurepattern.h"

#include "../util/smart_ptr.h"
#include "../util/properties/measureStructures.h"
#include "../util/thread/Terminal.h"
#include "../util/CalculatorContainer.h"

#include "../util/stream/ppivalues.h"

using namespace std;

/**
 * this class calculate all inserted double values to an value
 * and allow variables from other subroutines
 *
 */
class ListCalculator : public CalculatorContainer
{
public:
	/**
	 * instanciate object with values 0
	 *
	 * @param folder where the string was set
	 * @param subroutine where the string was set
	 * @param param parameter type in the subroutine where the string
	 * @param need whether string be set in init() method or statement() have to exist
	 * @param boolean true if result for output should be TRUE or FALSE, otherwise result by output is an double
	 * @param obj subroutine object for debug output
	 */
	ListCalculator(const string& folder, const string& subroutine, const string& param,
					bool need, bool boolean, IListObjectPattern* obj);
	/**
	 * initial object with all list entrys
	 *
	 * @param pStartFolder first pointer to all defined folders
	 * @return whether rendering of 2. parameter calcString was correct
	 */
	bool init(const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, string calcString);
	/**
	 * render string for faster calculation
	 *
	 * @return whether rendering was correct
	 */
	virtual bool render();
	/**
	 * calculate string of statement
	 * and set last change time to 0
	 *
	 * @param dResult result of calculation
	 * @return whether the calculation was correct
	 */
	virtual bool calculate(double& dResult);
	/**
	 * return folder in which calculation string be defined
	 *
	 * @return folder name
	 */
	string getFolderName() const
	{ return m_sFolder; };
	/**
	 * return subroutine in which calculation string be defined
	 *
	 * @return subroutine name
	 */
	string getSubroutineName() const
	{ return m_sSubroutine; };
	/**
	 * return parameter name in the subroutine where the calculation string be defined
	 *
	 * @return parameter name
	 */
	string getParameterName() const
	{ return m_sParameter; };
	/**
	 * search pointer to subroutine from given folder:subroutine
	 *
	 * @param var variable of subroutine with folder, can give back other correct variable name
	 * @param nObjFolder count of folder when defined inside an object, otherwise 0
	 * @param own whether should also get portBase Class from own folder
	 * @return object of subroutine
	 */
	SHAREDPTR::shared_ptr<IListObjectPattern> getSubroutine(string* var, unsigned short nObjFolder, bool own);
	/**
	 * subroutine object of given folder:subroutine
	 *
	 * @param var variable of subroutine with folder, can give back other correct variable name
	 * @param nObjFolder count of folder when defined inside an object, otherwise 0
	 * @param own whether should also get portBase Class from own folder
	 */
	//IListObjectPattern* getSubroutine(string* var, unsigned short nObjFolder, bool own);
	/**
	 * return latest changing time of any variable in calculation
	 *
	 * @return changing time
	 */
	ppi_time getLastChanging();
	/**
	 * activate information for given measure thread (observer)
	 * in all folder:subroutines from
	 *
	 * @param observer measure thread which containing the own folder
	 */
	void activateObserver(IMeasurePattern* observer);
	/**
	 * remove observer inside ([folder:]<subroutine>)<br />
	 *
	 * @param observer measure thread which containing the own folder
	 */
	void removeObserver(IMeasurePattern* observer);
	/**
	 * set value for Variable which should not read from other list object
	 *
	 * @param var name of variable
	 * @param val value of variable
	 */
	void setSubVar(string var, const double val)
	{ setSubVar(var, &val); };
	/**
	 * set value for Variable which should not read from other list object
	 *
	 * @param var name of variable
	 * @param val value of variable
	 */
	void setSubVar(string var, const double* val);
	/**
	 * clear all setting variables which set with setSubVar()
	 */
	void clearSubVars()
	{ 	LOCK(m_CALCUALTEMUTEX);
		m_msSubVars.clear();
		UNLOCK(m_CALCUALTEMUTEX);	};
	/**
	 * write calculation output over message function getting in constructor.<br />
	 * Default setting is <code>false</code>.
	 *
	 * @param write wheter should write
	 */
	OVERWRITE void doOutput(const bool write)
	{ 	LOCK(m_CALCUALTEMUTEX);
		CalculatorContainer::doOutput(write);
		UNLOCK(m_CALCUALTEMUTEX);				};
	/**
	 * return whether output is set
	 *
	 * @return whether output is set
	 */
	OVERWRITE bool doOutput() const
	{ 	bool bRv;
		LOCK(m_CALCUALTEMUTEX);
		bRv= CalculatorContainer::doOutput();
		UNLOCK(m_CALCUALTEMUTEX);
		return bRv;								};
	/**
	 * destructor of object
	 */
	virtual ~ListCalculator()
	{ DESTROYMUTEX(m_CALCUALTEMUTEX); };

protected:
	/**
	 * folder where the string was set
	 */
	string m_sFolder;
	/**
	 * map of all folder aliases for actual folder
	 */
	map<string, string> m_mFolderAlias;
	/**
	 * subroutine where the string was set
	 */
	string m_sSubroutine;
	/**
	 * all new created subroutine object
	 * for returning value of sub-variable
	 * needed to hold reference
	 */
	vector<SHAREDPTR::shared_ptr<IListObjectPattern> > m_vNewSubObjs;
	/*
	 * map of all subroutine aliases for actual folder
	 */
	map<string, string> m_mSubroutineAlias;
	/**
	 * predefined string with folder and subroutine
	 */
	string m_sFolderSub;
	/**
	 * number of folder when inside an object defined
	 * otherwise 0
	 */
	unsigned short m_nObjFolderID;
	/**
	 * parameter type in the subroutine where the string
	 */
	string m_sParameter;
	/**
	 * original incomming statement
	 * for debug output
	 */
	vector<string> m_vsStatement;
	/**
	 * subroutine of taken last changing time
	 */
	string m_sLastChangingSub;
	/**
	 * first pointer to all defined folders
	 */
	SHAREDPTR::shared_ptr<measurefolder_t> m_pStartFolder;
	/**
	 * variables which are used in calculation
	 */
	map<string, SHAREDPTR::shared_ptr<IListObjectPattern> > m_msoVars;
	/**
	 * variables which should not read from other list object
	 */
	map<string, double> m_msSubVars;
	/**
	 * latest changing time of any variable after calculation
	 */
	ppi_time m_nLastChange;
	/**
	 * subroutine object to writing into terminal for output on command line
	 */
	IListObjectPattern* m_oOutput;
	/**
	 * mutex for thread-safe calculation
	 */
	pthread_mutex_t* m_CALCUALTEMUTEX;

	/**
	 * creating of new objects of CalculatorContainer
	 * to overload when an other object needed
	 */
	virtual CalculatorContainer* newObject();
	/**
	 * static method to give in constructor
	 * for writing only inside of terminal (cout)
	 *
	 * @param bError whether message is an error of rendering
	 * @param file name of source file where the message occurs
	 * @param line in which line of source file the message occurs
	 * @param msg string of message
	 */
	void output(bool bError, const string& file, const int line, const string& msg);
	/**
	 * direct output called from method <code>output()</code>
	 *
	 * @param bError whether output should be an error
	 * @param msg output string
	 */
	virtual void out(const bool bError, const string& msg)
	{ m_oOutput->out() << msg; };
	/**
	 * this method is only to overload by an new child
	 * to get variables and will be called when parameter vars be set in constructor as true.<br />
	 * All created objects will be delete in destructor if own object destroy.
	 *
	 * @param var name of variable, can give back other correct variable name
	 * @param dResult result of var
	 * @param whether found correct variable
	 */
	virtual bool variable(string* var, double& dResult);
	/**
	 * clear time of last changes
	 */
	void clearTime();
	/**
	 * return latest changing time of any variable in calculation
	 *
	 * @return changing time
	 */
	ppi_time getLastChangingI();
};

#endif /*PORTCALCULATOR_H_*/
