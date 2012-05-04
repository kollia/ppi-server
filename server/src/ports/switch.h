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
#ifndef SWITCHCLASS_H_
#define SWITCHCLASS_H_

#include "../util/structures.h"

#include "../util/properties/configpropertycasher.h"

#include "portbaseclass.h"
#include "ListCalculator.h"

using namespace ports;

class switchClass : public portBase
{
public:
		/**
		 * enum whether value be set
		 */
		enum setting
		{
			NONE= 0,
			BEGIN,
			WHILE,
			END
		};
		/**
		 * create object of class switchClass.
		 *
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 */
		switchClass(string folderName, string subroutineName);
		/**
		 * create object of class switchClass.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extendet class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 * @param defaultValue only for derived classes which are no SWITCH type the first value, otherwise it will taken from database or default 0
		 */
		switchClass(string type, string folderName, string subroutineName);
		/**
		 * initialing object of switchClass
		 *
		 * @param properties the properties in file measure.conf
		 * @param pStartFolder reference to all folder
		 * @return whether initialization was OK
		 */
		virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder)
			{ return init(properties, pStartFolder, false); };
		/**
		 * initialing object of switchClass
		 *
		 * @param properties the properties in file measure.conf
		 * @param pStartFolder reference to all folder
		 * @param bAlwaysBegin whether begin parameter should be always measured before while and end
		 * @return whether initialization was OK
		 */
		bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder, const bool bAlwaysBegin);
		/**
		 * this method will be called from any measure thread to set as observer
		 * for starting own folder to get value from foreign folder
		 * if there the value was changing
		 *
		 * @param observer measure thread which containing the own folder
		 */
		virtual void setObserver(IMeasurePattern* observer);
		/**
		 * measure whether switch value is set or not
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual double measure(const double actValue);
		/**
		 * measure also whether switch value is set or not,
		 * but show in second parameter, maybe for decided class need,
		 * whether actual value be set from BEGIN, WHILE or END
		 *
		 * @param actValue current value
		 * @param set whether value set from begin <code>switchClass::BEGIN</code>, while <code>switchClass::WHILE</code>, end <code>switchClass::END</code> or from none <code>switchClass::NONE</code>
		 * @param newValue new value when changed and is not the same than content inside
		 * @return return measured value
		 */
		double measure(const double actValue, setting& set, const double* newValue= NULL);
		/**
		 * destructor
		 */
		virtual ~switchClass();
		/**
		 * returns the address of the given folder name
		 *
		 * @param name name of the folder
		 * @param pStratFolder address of the first folder
		 * @return address of the folder in given parameter name
		 */
		static const SHAREDPTR::shared_ptr<measurefolder_t>  getFolder(const string &name, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * set subroutine for output doing actions
		 *
		 * @param whether should write output
		 */
		virtual void setDebug(bool bDebug);
#if 0
		/**
		 * get value from subroutine
		 *
		 * @param who define whether intern (i:<foldername>) or extern (e:<username>) request.<br />
		 * 				This time only defined for external reading over OwPort's.
		 * @return current value
		 */
		virtual double getValue(const string& who);
		/**
		 * set value in subroutine.<br />
		 * All strings from parameter 'from' beginning with an one character type,
		 * followed from an colon 'r:' by ppi-reader, 'e:' by an account connected over Internet
		 * or 'i:' by intern folder:subroutine.
		 *
		 * @param value value which should be set
		 * @param from which folder:subroutine or account changing the value
		 */
		virtual void setValue(const double value, const string& from);
#endif

	protected:
		/**
		 * set min and max parameter to the range which can be set for this subroutine.<br />
		 * If the subroutine is set from 0 to 1 and float false, the set method sending only 0 and 1 to the database.
		 * Also if the values defined in an bit value 010 (dec:2) set 0 and for 011 (dec:3) set 1 in db.
		 * Otherwise the range is only for calibrate the max and min value if set from client outher range.
		 * If pointer of min and max parameter are NULL, the full range of the double value can be used
		 *
		 * @param bfloat whether the values can be float variables
		 * @param min the minimal value
		 * @param max the maximal value
		 * @return whether the range is defined or can set all
		 */
		virtual bool range(bool& bfloat, double* min, double* max);

	private:
		/**
		 * whether own class is from type SWITCH
		 */
		bool m_bSwitch;
		/**
		 * value from last pass
		 */
		bool m_bLastValue;
		/**
		 * action to need only current values
		 */
		bool m_bCurrent;
		/**
		 * value for every subroutine or account over internet
		 * to show whether value was switched before when no current action be set.<br />
		 * This variable contain an map with first as name from subroutine or account
		 * and second an pair with two boolean to define as first whether the subroutine is also
		 * from type SWITCH or an account (true) or other subroutine (false) as first
		 * and wether the subroutine was defined before as second
		 */
		map<string, bool> m_msbSValue;
		/**
		 * whether parameter begin should always be measured
		 * before while and end
		 */
		bool m_bAlwaysBegin;
		/**
		 * begin calculation string
		 */
		ListCalculator m_oBegin;
		/**
		 * while calculation string
		 */
		ListCalculator m_oWhile;
		/**
		 * end calculation string
		 */
		ListCalculator m_oEnd;
		/**
		 * locking for set m_msbSValue;
		 */
		pthread_mutex_t *m_VALUELOCK;
};

#endif /*SWITCHCLASS_H_*/
