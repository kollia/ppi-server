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
#ifndef PORTBASECLASS_H_
#define PORTBASECLASS_H_

#include <string>
#include <set>
#include <map>

#include <boost/regex.hpp>

#include "../util/debug.h"
#include "../util/debugsubroutines.h"
#include "../util/structures.h"

#include "../util/stream/ppivalues.h"
#include "../util/properties/configpropertycasher.h"

#include "../pattern/util/IMeasureSet.h"
#include "../pattern/util/IListObjectPattern.h"
#include "../pattern/util/imeasurepattern.h"

#include "ListCalculator.h"

using namespace std;
using namespace util;
using namespace design_pattern_world::util_pattern;

namespace ports
{
	class portBase : virtual public IListObjectPattern,
					 virtual public IMeasureSet
	{
		public:
	#ifdef DEBUG
			unsigned long m_count;
	#endif //DEBUG
			enum Pin
			{
				NONE= 0,
				// COM-interface
				DTR,
				RTS,
				TXD,
				CTS,
				DSR,
				RI,
				DCD,

				// description
				SETPIN,
				GETPIN,
				GETSETPIN,

				// port
				COM,
				LPT
			};
			struct Pins
			{
				unsigned long nPort;
				string sPort;
				Pin ePin;
				string sPin;

				int operator == (const Pins &other) const
				{
					if(	nPort == other.nPort
						&&
						ePin == other.ePin	)
					{
						return 1;
					}
					return 0;
				};
				int operator<(const Pins &other) const
				{
					if(	nPort < other.nPort
						||
						(	nPort == other.nPort
							&&
							ePin < other.ePin	)	)
					{
						return 1;
					}
					return 0;
				};
			};
			struct portpin_address_t
			{
				unsigned long nPort;
				int nPin;
				Pin eDescript;
				Pin ePort;
			};

		private:
			/**
			 * whether subroutine is only for switching
			 * between 0 and 1
			 */
			bool m_bSwitch;
			/**
			 * count number of subroutine
			 * inside folder
			 */
			unsigned short m_nCount;
			/**
			 * number of folder when inside an object defined
			 * otherwise 0
			 */
			unsigned short m_nObjFolderID;
			/**
			 * wether access to device is correct.<br />
			 * If pointer is <code>NULL</code> object do not know access from database
			 */
			auto_ptr<bool> m_pbCorrectDevice;
			/**
			 * min valuue which can be set.<br />
			 * default min is 1 and max is 0 -> full range can be used
			 */
			double m_dMin;
			/**
			 * max value which can be set.<br />
			 * default min is 1 and max is 0 -> full range can be used
			 */
			double m_dMax;
			/**
			 * whether an floating value can be used
			 */
			bool m_bFloat;
			/**
			 * whether range of value is correct defined
			 */
			bool m_bDefined;
			/**
			 * whether set DEBUG over server for this folder
			 */
			bool m_bDebug;
			/**
			 * whether subroutine need information from other subroutines
			 * when any value which set inside begin/while/end was changed
			 */
			bool m_bInfo;
			/**
			 * whether subroutine should holding last changing time
			 */
			bool m_bTime;
			/**
			 * whether object should write changed values into database
			 */
			bool m_bWriteDb;
			/**
			 * value which hold this subroutine,
			 * with time from last changing
			 */
			ValueHolder m_dValue;
			/**
			 * all clients with values to know whether
			 * switch subroutine was active between the last request
			 */
			map<string, short> m_mdValue;
			/**
			 * type of this measure object
			 */
			string m_sType;
			/**
			 * folder name in which this subroutine is running
			 */
			string m_sFolder;
			/**
			 * name of the subroutine
			 */
			string m_sSubroutine;
			/**
			 * contain groups which have read or write permission,
			 * Separately with colons, in this subroutine
			 */
			string m_sPermission;
			/**
			 * all other folder threads which should be informed when value was changed
			 */
			map<IMeasurePattern*, vector<string> > m_mvObservers;
			/**
			 * header text for display error message on screen or log-file
			 */
			string m_sErrorHead;
			/**
			 * header text for display warning message on screen or log-file
			 */
			string m_sWarningHead;
			/**
			 * header text for display message on screen or log-file
			 */
			string m_sMsgHead;
			/**
			 * mutex lock for value
			 */
			pthread_mutex_t *m_VALUELOCK;
			/**
			 * mutex lock for debug
			 */
			pthread_mutex_t *m_DEBUG;
			/**
			 * mutex for variable m_bDeviceAccess
			 */
			pthread_mutex_t *m_CORRECTDEVICEACCESS;
			/**
			 * mutex to set or remove observers
			 */
			pthread_mutex_t *m_OBSERVERLOCK;

			/**
			 * private copy constructor for no allowed copy
			 *
			 * @param x object for coppy
			 */
			portBase(const portBase& x);
			/**
			 * private assignment operator for not allowed allocation
			 *
			 * @param x opbject for assignment
			 * @return own object
			 */
			portBase& operator=(const portBase& x);
			/**
			 * create binary string of two numbers for debug output
			 *
			 * @param value double value to convert
			 * @return binary string
			 */
			string switchBinStr(double value);
			/**
			 * stream object to writing into terminal for output on command line
			 */
			ostringstream m_sStreamObj;


		public:
			/**
			 * creating object of extendet class
			 *
			 * @param type type of the current subroutine (extendet class)
			 * @param folder name of the folder in which this soubroutine running
			 * @param subroutine name from this subroutine
			 * @param objectID count of folder when defined inside an object, otherwise 0
			 */
			portBase(const string& type, const string& folderName, const string& subroutineName,
							unsigned short objectID);
			/**
			 * initial extended object to check whether write into database and define range of value.<br />
			 * Before called this method all parameters for method range have be set.
			 *
			 * @param properties the properties of subroutine from file measure.conf
			 * @param pStartFolder reference to all folder
			 * @return whether initalization was ok
			 */
			virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
			/**
			 * whether subroutine is defined as binary
			 *
			 * @return whether subroutine is binary defined
			 */
			bool binary()
			{ return m_bSwitch; };
			/**
			 * check whether subroutine need an external owreader server
			 *
			 * @return whether need an server
			 */
			virtual bool needServer() const
			{ return false; };
			/**
			 * check whether object found for chip in subroutine correct server.<br />
			 * Only when server needed.
			 *
			 * @return whether server found
			 */
			virtual bool hasServer() const
			{ return false; }
			/**
			 * return true when subroutine need information from other subroutines by changing.<br />
			 * otherwise false.
			 *
			 * @return whether subroutine need information from other subroutines
			 */
			virtual bool needObserver() const
			{ return m_bInfo; };
			/**
			 * this method will be called from any measure thread to set as observer
			 * for starting own folder to get value from foreign folder
			 * if there the value was changing
			 *
			 * @param observer measure thread which containing the own folder
			 */
			virtual void setObserver(IMeasurePattern* observer);
			/**
			 * fill observer vector to inform other folder if value changed
			 *
			 * @param observer measure thread which containing the own folder
			 * @param folder name of folder which should be informed
			 * @param subroutine name of subroutine which should be informed
			 * @param parameter name of parameter where the own subroutine is occured
			 */
			virtual void informObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter);
			/**
			 * remove from observer from information when value changed.<br />
			 * This method remove the observer only when all 'folder:subroutine parameter' values
			 * be removed.
			 *
			 * @param observer measure thread which containing the own folder
			 * @param folder name of folder which should be removed from information
			 * @param subroutine name of subroutine which should be removed from information
			 * @param parameter name of parameter where the own subroutine is occured
			 */
			virtual void removeObserver(IMeasurePattern* observer, const string& folder, const string& subroutine, const string& parameter);
			/**
			 * set whether subroutine has correct access to device
			 *
			 * @param access whether own object have correct access to device
			 */
			virtual void setDeviceAccess(const bool access);
			/**
			 * ask subroutine whether she has an correct access to device
			 *
			 * @return whether the access is correct
			 */
			virtual bool hasDeviceAccess() const;
			/**
			 * returning all groups, seperatly with colons
			 *
			 * @return string of groups
			 */
			virtual string getPermissionGroups();
			//bool doForAfterContact();
			//void setAfterContact(const map<unsigned long, unsigned> &ports, const set<Pins> &pins);
			//void noAfterContactPublication();
			/**
			 * set subroutine for output doing actions
			 *
			 * @param whether should write output
			 */
			virtual void setDebug(bool bDebug);
			/**
			 * return whether is subroutine set for output doing actions
			 *
			 * @return whether subroutine do output
			 */
			virtual bool isDebug();
			/**
			 * returning ostringstream object which should written on right time
			 * by next pass into Terminal for output on command line
			 *
			 * @return string stream for writing by next pass
			 */
			virtual ostringstream& out();
			/**
			 * writing into string stream into terminal
			 * when definition WRITEDEBUGALLLINES not be set
			 */
			virtual void writeDebugStream();
			/**
			 * returning the type of the current object
			 *
			 * @return name of type of the subroutine
			 */
			virtual string getSubroutineType()
			{ return m_sType; };
			/**
			 * returning the name of the folder
			 * in which this subroutine running
			 *
			 * @return name of folder
			 */
			string getFolderName();
			/**
			 * returning the name of this subroutine
			 *
			 * @return name of the subroutine
			 */
			string getSubroutineName();
			/**
			 * return true if subroutine is only for switching between 0 and 1
			 *
			 * @return whether subroutine is for switching
			 */
			bool onlySwitch();
			/**
			 *  external command to send stopping to all subroutines when needed
			 *
			 * @param bWait calling routine should wait until the thread is stopping
			 */
			virtual void stop(const bool bWait)
			{ portBase::stop(&bWait); };
			/**
			 *  external command to send stopping to all subroutines.<br />
			 *  dummy routine can be overloaded when need
			 *
			 * @param bWait calling routine should wait until the thread is stopping
			 */
			virtual void stop(const bool *bWait= NULL)
			{ /* dummy routine can be overloaded when need*/ };
			/**
			 * measure new value for subroutine
			 *
			 * @param actValue current value
			 * @return measured value with last changing time when not changed by self
			 */
			virtual IValueHolderPattern& measure(const ppi_value& actValue)=0;
			/**
			 * get value from subroutine
			 *
			 * @param who define whether intern (i:<foldername>) or extern (e:<username>) request.<br />
			 * 				This time only defined for external reading over OwPort's.
			 * @return current value with last changing time
			 */
			virtual IValueHolderPattern& getValue(const string& who);
			/**
			 * set value in subroutine.<br />
			 * All strings from parameter 'from' beginning with an one character type,
			 * followed from an colon 'r:' by ppi-reader, 'e:' by an account connected over Internet
			 * or 'i:' by intern folder:subroutine.
			 *
			 * @param value value which should be set with last changing time when set, otherwise method create own time
			 * @param from which folder:subroutine or account changing the value
			 */
			virtual void setValue(const IValueHolderPattern& value, const string& from);
			/**
			 * set double value into measure list
			 *
			 * @param folder folder name from the running thread
			 * @param subroutine name of the subroutine in the folder
			 * @param value value which should write into database with last changing time when set, otherwise method create own time
			 * @param account from which account over Internet the value will be set
			 * @return whether subroutine can be set correctly
			 */
			virtual bool setValue(const string& folder, const string& subroutine,
							const IValueHolderPattern& value, const string& account);
			/**
			 * return count of subroutine in folder
			 *
			 * @return count of subroutine
			 */
			virtual unsigned short getActCount()
			{ return m_nCount; };
			/**
			 * return number of folder when defined inside an object
			 * otherwise 0
			 */
			unsigned short getObjectFolderID() const
			{ return m_nObjFolderID; };
			/**
			 * set measure thread which run this object with method <code>measure()</code>
			 *
			 * @param thread measure thread
			 */
			virtual void setRunningThread(IMeasurePattern* thread)
			{ m_poMeasurePattern= thread; };
			/**
			 * check how much folder are running
			 *
			 * @return count of folder running
			 */
			string getFolderRunningID();
			/**
			 * return measure thread which run this object with method <code>measure()</code>
			 *
			 * @return measure thread
			 */
			IMeasurePattern* getRunningThread()
			{ return m_poMeasurePattern; };
			/**
			 * return type of subroutine
			 *
			 * @return type name
			 */
			string getType() const
			{ return m_sType; };
			/**
			 * return info whether subroutine need last changing time
			 *
			 * @return whether need time
			 */
			virtual bool needChangingTime() const
			{ return m_bTime; };
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
			/**
			 * destructor
			 */
			virtual ~portBase();

		protected:
#ifdef __followSETbehaviorToFolder
			bool m_bFollow;
			boost::regex m_oToFolderExp;
			boost::regex m_oToSubExp;
#endif // __followSETbehaviorToFolder
			/**
			 * value from last pass inside <code>measure()</code> method.<br />
			 * need to hold as reference for return value
			 */
			ValueHolder m_oMeasureValue;
			/**
			 * value from last <code>getValue</code> method.<br />
			 * need to hold as reference for return value
			 */
			ValueHolder m_oGetValue;

			static void lockApplication(bool bSet);
			/**
			 * registration of folder and subroutine
			 * with server 'measureRoutine', chip '###DefChip <folder> <subroutine>', pin '0', and family code 'measure'
			 * for writing into and to thin database
			 */
			virtual void registerSubroutine();
			/**
			 * define range of value. see also public range method for better description
			 */
			void defineRange();
			/**
			 * initial subroutine to get value from an other subroutine
			 *
			 * @param type type of subroutine defined in constructor
			 * @param properties the properties in file measure.conf
			 * @param pStartFolder reference to all folder
			 * @return whether initialization was OK
			 */
			bool initLinks(const string& type, IPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
			/**
			 * get the linked value
			 *
			 * @param type type of subroutine defined in constructor
			 * @param val returned value from other subroutine, or same as incoming when Link show to own
			 * @param bCountDown whether getting linked value is an count down (only for TIMER subroutines) do not inform liked values
			 * @return true when the value is from an other subroutine, else false
			 */
			bool getLinkedValue(const string& type, ValueHolder& val, const double& maxCountDownValue= 0);
			/**
			 * return message header with folder and subroutine name
			 * and also error or warning type when parameter be set
			 */
			string getSubroutineMsgHead(bool *error= NULL);
			/**
			 * return message header with folder and subroutine name
			 * and also error or warning type
			 */
			string getSubroutineMsgHead(bool error);

		private:
			IMeasurePattern* m_poMeasurePattern;
			/**
			 * all links to share with other subroutines
			 */
			vector<ListCalculator*> m_vpoLinks;
			/**
			 * pointer to first folder
			 */
			SHAREDPTR::shared_ptr<measurefolder_t> m_pFolders;
			/**
			 * all folder thread which need to create running folder ID specification
			 */
			vector<SHAREDPTR::shared_ptr<IMeasurePattern> > m_vpFolderSpecs;
			/**
			 * while expression to set link from m_vsLinks
			 */
			ListCalculator m_oLinkWhile;
			/**
			 * last set value inside method <code>getLinkedValue()</code>
			 */
			double m_dLastSetValue;
			/**
			 * which link for observer be set
			 */
			vector<string>::size_type m_nLinkObserver;
	};
}

#endif /*PORTBASECLASS_H_*/
