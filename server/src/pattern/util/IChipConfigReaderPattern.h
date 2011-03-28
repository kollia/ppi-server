/**
 *   This file 'IChipConfigReaderPattern.h' is part of ppi-server.
 *   Created on: 27.03.2011
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


#ifndef ICHIPCONFIGREADERPATTERN_H_
#define ICHIPCONFIGREADERPATTERN_H_

namespace design_pattern_world
{
	/**
	 * structure for highest data
	 */
	struct write_highest_t {
		/**
		 * time of highest value
		 */
		time_t hightime;
		/**
		 * highest value
		 */
		double highest;
		/**
		 * time of lowest value
		 */
		time_t lowtime;
		/**
		 * lowest value
		 */
		double lowest;
	};
	/**
	 * structure of highest and lowest writing
	 */
	struct write_t {
		/**
		 * writing after follow actions:
		 * no - no writing
		 * write - writing the actual value and time
		 * highest - write the lowest and highest time from structure write_highest_t
		 */
		string action;
		/**
		 * name of folder
		 */
		string folder;
		/**
		 * name of subroutine
		 */
		string subroutine;
		/**
		 * pointer to highest struct
		 */
		write_highest_t highest;

	};
	/**
	 * structure fraction data
	 */
	struct fraction_t {
		/**
		 * whether the current static value for fractions be set
		 */
		bool bValue;
		/**
		 * writing only an value by dbwrite fractions if the new value outside
		 * of this interval from the old (half over or half under)
		 */
		double dbinterval;
		/**
		 * writing by fractions also after this seconds
		 */
		time_t dbafter;
		/**
		 * writing value also if this time reached
		 */
		time_t write;
		/**
		 * last written value
		 */
		double writtenvalue;
		/**
		 * deepest or highest value
		 * which was measured
		 */
		double deepvalue;
		/**
		 * time of deepest or highest value
		 */
		time_t deeptime;
		/**
		 * for witch direction the values for action fraction is gone
		 */
		string direction;
	};
	/**
	 * structure for highest data
	 */
	struct highest_t {
		/**
		 * whether the highest, lowest value and between time for action highest be set
		 */
		bool bValue;
		/**
		 * time value specification
		 */
		char t;
		/**
		 * write only highest and lowest values between this time specified as variable t
		 */
		unsigned short between;
		/**
		 * write the highest and lowest values by or after this time
		 */
		time_t nextwrite;
		/**
		 * time of highest value
		 */
		time_t hightime;
		/**
		 * highest value
		 */
		double highest;
		/**
		 * time of lowest value
		 */
		time_t lowtime;
		/**
		 * lowest value
		 */
		double lowest;
	};
	/**
	 * structure of time writing
	 */
	struct otime_t {
		/**
		 * whether structure will be filled with values
		 */
		bool active;
		/**
		 * after how many days, weeks, month, year
		 * this older structure have to operate
		 */
		unsigned short more;
		/**
		 * specification for witch unit the variable more be
		 */
		char unit;
		/**
		 * writing after follow actions:
		 * all - writing all changes
		 * fractions - 	writing only the fractions of the values
		 *				parameter dbinterval have to be defined
		 * highest - save only the highest and lowest values inside the range
		 * kill - kill exist values from database
		 */
		string dbwrite;
		/**
		 * pointer to fraction struct
		 */
		auto_ptr<fraction_t> fraction;
		/**
		 * pointer to highest struct
		 */
		auto_ptr<highest_t> highest;
		/**
		 * the next older structure should be after the current
		 */
		SHAREDPTR::shared_ptr<otime_t> older;
	};
	/**
	 * default values
	 */
	struct defValues_t {
		double dmin;
		double dmax;
		bool bFloat;
		double dcache;
		SHAREDPTR::shared_ptr<otime_t> older;
	};
	/**
	 * structure of chip id's
	 */
	struct chips_t {
		string server;
		string family;
		string type;
		string id;
		string pin;
		vector<double> errorcode;
		double dmin;
		double dmax;
		bool bFloat;
		double dCache;
		bool bWritable;
		SHAREDPTR::shared_ptr<otime_t> older;
	};

	/**
	 * patter for DefaultChipConfigReader
	 * to use inside Database
	 */
	class IChipConfigReaderPattern
	{
	public:
		/**
		 * describe whether all chips for folder and subroutine are defined
		 */
		virtual bool chipsAreDefined()= 0;
		/**
		 * whether allow writing into database
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @param value whether can be writing this value
		 * @param acttime time of entry which was set
		 * @param newOlder if variable be set, method is to thin database and give back in this variable whether an new older structure be used
		 * @return an structure of write_t witch describe the writing values
		 */
		virtual write_t allowDbWriting(const string& folder, const string& subroutine, const double value,
																		time_t acttime, bool *newOlder= NULL)= 0;
		/**
		 * return an older structure from the chip defined with folder and subroutine
		 * which last older is active. By this older structure, active flag will be set to false.
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @param nonactive whether older should be set to non active
		 * @return last active older structure
		 */
		virtual const SHAREDPTR::shared_ptr<otime_t> getLastActiveOlder(const string& folder,
												const string& subroutine, const bool nonactive)= 0;
		/**
		 * read all values which be inside of chips for fractions or highest be saved
		 *
		 * @param pos which position should read
		 * @param older whether should read older values (true) or for current writing (false =default)
		 * @return values with time for fractions or highest in variable action.<br />
		 * 			By ending, action is 'no'. If action is fraction value and time is in variables highest.highest and hightime.
		 */
		virtual write_t getLastValues(const unsigned int pos, const bool bolder= false)= 0;
		/**
		 * dummy destruktor for pattern
		 */
		virtual ~IChipConfigReaderPattern() {};
	};
}

#endif /* ICHIPCONFIGREADERPATTERN_H_ */
