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
#ifndef EXTERNPORT_H_
#define EXTERNPORT_H_

#include <map>

#include "../pattern/util/imeasurepattern.h"

#include "../portserver/libs/interface/OWInterface.h"

#include "../util/properties/configpropertycasher.h"

#include "switch.h"

using namespace design_pattern_world::util_pattern;
using namespace server;
using namespace util;

namespace ports
{
	/**
	 * class define all DALLAS Semiconductor Devices.
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class ExternPort : public switchClass
	{
	public:
		/**
		 * create object of class ExternPort.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extended class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 * @param objectID count of folder when defined inside an object, otherwise 0
		 */
		ExternPort(string type, string folder, string subroutine, unsigned short objectID) :
		switchClass(type, folder, subroutine, objectID),
		m_bFirstAllocate(true),
		m_oValue(folder, subroutine, "value", true, false, this),
		m_dLastWValue(0),
		m_bDoSwitch(false),
		m_bFirstRead(true)
		{ };
		/**
		 * initialing object of ExternPort
		 *
		 * @param properties the properties in file measure.conf
		 * @return whether initialization was OK
		 */
		virtual bool init(IActionPropertyPattern* properties, const SHAREDPTR::shared_ptr<measurefolder_t>& pStartFolder);
		/**
		 * this method will be called from any measure thread to set as observer
		 * for starting own folder to get value from foreign folder
		 * if there the value was changing
		 *
		 * @param observer measure thread which containing the own folder
		 */
		virtual void setObserver(IMeasurePattern* observer);
		/**
		 * set subroutine for output doing actions
		 *
		 * @param whether should write output
		 */
		virtual void setDebug(bool bDebug);
		/**
		 * measure new value for subroutine
		 *
		 * @param actValue current value
		 * @return return measured value
		 */
		virtual IValueHolderPattern& measure(const ppi_value& actValue);
		/**
		 * write chip id and value to port server.<br />
		 * This method is called from method <code>measure()</code>
		 * and can be overloaded from any new extern port object
		 *
		 * @param chipID calculated id from measure.conf over portserver
		 * @param value value to set
		 * @param addinfo additional info for chip
		 * @return whether access to chip was given
		 */
		virtual bool write(const string& chipID, const double value, string& addinfo)
		{ return m_pOWServer->write(chipID, value, addinfo); };
		/**
		 * This method is called from method <code>measure()</code> and is only an dummy method.
		 * Because the the own value of this subroutine is set from portserver.
		 * It can be overloaded from any new extern port object
		 *
		 * @param value actual value set from portserver
		 */
		virtual void read(double* value)
		{};
		/**
		 * overwrite portBase class to define access from outside,
		 * if value defined for switching between 0 and 1
		 *
		 * @param who define whether intern (i:<foldername>) or extern (e:<username>) request
		 * @return current value with last changing time
		 */
		virtual IValueHolderPattern& getValue(const string who);
		/**
		 * check whether subroutine need an external owreader server
		 *
		 * @return whether need an server
		 */
		virtual bool needServer() const
		{ return true; };
		/**
		 * check whether object found for chip in subroutine correct server
		 *
		 * @return whether server found
		 */
		virtual bool hasServer() const
		{ return m_pOWServer != NULL ? true : false; };
		/**
		 * destructor of class ExternPort.<br />
		 * calls OW_finish() to reinitial the OWFS-server
		 */
		virtual ~ExternPort();

	protected:
		/**
		 * set min and max parameter to the range which can be set for this subroutine.<br />
		 * If the subroutine is set from 0 to 1 and no float, the set method sending only 0 and 1 to the database.
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
		 * override registration of folder and subroutine from portBase class
		 * for writing into and to thin database
		 * because this class do not need any registration of chip.
		 * It was registered into the server object
		 */
		virtual void registerSubroutine();

	private:
		/**
		 * type of server needed
		 */
		string m_sServer;
		/**
		 * whether have given display by not founding an server
		 */
		bool m_bFirstAllocate;
		/*
		 * calculation of value
		 */
		ListCalculator m_oValue;
		/**
		 * value by last pass of writing measure method
		 */
		double m_dLastWValue;
		/**
		 * whether is set an begin, while or end property
		 */
		bool m_bDoSwitch;
		/**
		 * whether measure method running first time.<br />
		 * this is necessary by reading subroutines to read first
		 * state of pin
		 */
		bool m_bFirstRead;
		/**
		 * ID of dallas chip for this subroutine
		 * without family code two digits
		 */
		string m_sChipID;
		/**
		 * two hex character family code
		 */
		string m_sChipFamily;
		/**
		 * type of chip
		 */
		string m_sChipType;
		/**
		 * whether pin on chip is to read
		 */
		bool m_bRead;
		/**
		 * server which read and write on one wire device
		 */
		OWI m_pOWServer;
		/**
		 * settings from measure config file for defined subroutine
		 */
		IActionPropertyPattern* m_pSettings;

		/**
		 * allocate chip id and type of subroutine to one wire server
		 *
		 * @return true if allocation and setting useChip was correct<br />
		 * 			If no server found, be also true to try later. Except for settings
		 * 			can not read correctly, return value be false.
		 */
		bool allocateServer();

	};

}

#endif /*EXTERNPORT_H_*/
