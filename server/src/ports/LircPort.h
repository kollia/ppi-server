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
#ifndef LIRCPORT_H_
#define LIRCPORT_H_

#include <map>

#include "ExternPort.h"

namespace ports
{
	/**
	 * class define all DALLAS Semiconductor Devices.
	 *
	 * @author Alexander Kolli
	 * @version 1.0.0.
	 */
	class LircPort : public ExternPort
	{
	public:
		/**
		 * create object of class LircPort.<br />
		 * Constructor for an extendet object
		 *
		 * @param type type of object from extended class
		 * @param folder in which folder the routine running
		 * @param subroutine name of the routine
		 * @param objectID count of folder when defined inside an object, otherwise 0
		 */
		LircPort(string type, string folder, string subroutine, unsigned short objectID) :
			ExternPort(type, folder, subroutine, objectID),
			m_bONCE(false),
			m_oCount(folder, subroutine, "count", true, false, this)
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
		,m_bWritten(false)
		,m_bPressed(false)
		,after(folder, subroutine, "after", true, false, this)
		,digits(folder, subroutine, "digits", true, false, this)
#endif // DEBUG_ACTIVATEDLIRCOUTPUT
		{ };
		/**
		 * initialing object of LircPort
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
		virtual auto_ptr<IValueHolderPattern> measure(const ppi_value& actValue);
		/**
		 * destructor do nothing
		 */
		virtual ~LircPort() {};

	protected:
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
		virtual bool write(const string& chipID, const double value, string& addinfo);

	private:
		/**
		 * whether subroutine is defined to write only one signal
		 * or more with parameter count in measure.conf
		 */
		bool m_bONCE;
		/*
		 * calculation of count parameter
		 */
		ListCalculator m_oCount;
#ifdef DEBUG_ACTIVATEDLIRCOUTPUT
		/**
		 * whether any signal was written
		 */
		bool m_bWritten;
		/**
		 * whether signal is actualy send
		 */
		bool m_bPressed;
		/**
		 * only for test time
		 */
		ListCalculator after;
		/**
		 * how much digits be set on real case
		 */
		ListCalculator digits;
		/*
		 * length of time measure
		 */
		timeval m_tTime;
		/**
		 * array of time results
		 */
		map<double, pair<double, double> > m_mtResult;
#endif // DEBUG_ACTIVATEDLIRCOUTPUT

	};

}

#endif /*LIRCPORT_H_*/
