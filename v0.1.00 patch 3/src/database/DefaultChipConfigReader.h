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

/*
 * DefaultChipConfigReader.h
 *
 *  Created on: 13.03.2009
 *      Author: Alexander Kolli
 */

#ifndef DEFAULTCHIPCONFIGREADER_H_
#define DEFAULTCHIPCONFIGREADER_H_

#include <string>
#include <map>

#include "../util/interlacedactionproperties.h"

using namespace std;
using namespace util;
using namespace design_pattern_world;

namespace ports
{

	class DefaultChipConfigReader
	{
	public:
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
			fraction_t* fraction;
			/**
			 * pointer to highest struct
			 */
			highest_t* highest;
			/**
			 * the next older structure should be after the current
			 */
			otime_t *older;

			otime_t* operator = (otime_t *other) {
				dbwrite= other->dbwrite;
				more= other->more;
				unit= other->unit;
				highest= other->highest;
				fraction= other->fraction;
				highest= other->highest;
	/*			if(other->fraction) {
					fraction= new fraction_t;
					fraction->bValue= other->fraction->bValue;
					fraction->dbafter= other->fraction->dbafter;
					fraction->dbinterval= other->fraction->dbinterval;
					fraction->direction= other->fraction->direction;
					fraction->value= other->fraction->value;
				}else
					fraction= NULL;
				if(other->highest) {
					highest= new highest_t;
					highest->bValue= other->highest->bValue;
					highest->between= other->highest->between;
					highest->hightime= other->highest->hightime;
					highest->highest= other->highest->highest;
					highest->lowtime= other->highest->lowtime;
					highest->lowest= other->highest->lowest;
				}else
					highest= NULL;*/
				older= other->older;
				return this;
			}
		};
		/**
		 * default values
		 */
		struct defValues_t {
			double dmin;
			double dmax;
			bool bFloat;
			double dcache;
			otime_t* older;
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
			otime_t *older;
		};

		/**
		 * create single instance of DefaultChipConfigReader
		 *
		 * @param path path where the config files be found
		 */
		static void init(const string& path);
		/**
		 * method get and or create singelton object of DefaultChipConfigReader
		 *
		 * @return instance of DefaultChipConfigReader
		 */
		static DefaultChipConfigReader* instance()
		{ return _instance; };
		/**
		 * define for all OWServer the name of the default-file
		 *
		 * @param server name of OWServer
		 * @param config name of default config-file
		 */
		void define(const string& server, const string& config);
		/**
		 * register the used chip from defined chip out of default.conf if set,
		 * elswhere if pdmin be defined, create an new chip
		 *
		 * @param server name of server
		 * @param chip unique id of chip
		 * @param pin witch pin inside the chip is used
		 * @param type specified type of chip
		 * @param family specified family code of chip
		 * @param pdmin minimal value witch chip can has
		 * @param pdmax maximal value witch chip can has
		 * @param pbFloat whether chip can has floating values
		 * @param pdCache which default cache the pin of chip have
		 */
		void registerChip(const string& server, const string& chip, const string& pin, const string& type, const string& family,
							const double* pdmin= NULL, const double* pdmax= NULL, const bool* pbFloat= NULL, const double* pdCache= NULL);
		/**
		 * register chip-id with pin for folder and subroutine
		 *
		 * @param subroutine name of subroutine
		 * @param folder name of folder
		 * @param server name of server
		 * @param chip unique id of chip
		 * @param pin witch pin inside the chip is used (default is standard pin defined in default.conf of server)
		 */
		void registerSubroutine(const string& subroutine, const string& folder, const string& server, const string& chip);
		/**
		 * return the defined default values from default.conf.<br />
		 * folder and subroutine set not be, or else both.<br />
		 * Method search the default values inside of given range.
		 * It found when the difference of <code>min</code> and <code>max</code> be smaller than in the default.conf.
		 * If <code>float</code> is false, the default must be also false or can be true if no false be set.
		 *
		 * @param min minimum of range
		 * @param max maximum of range
		 * @param bFloat whether the value in the subroutine can be an floating point variable
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @return default values with defined older_t structure
		 */
		const defValues_t getDefaultValues(const double min, const double max, const bool bFloat, const string& folder= "", const string& subroutine= "") const;
		/**
		 * return registered default chip of unique chip ID and server.<br />
		 *
		 *
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 */
		const chips_t* getRegisteredDefaultChip(const string& server, const string& chip) const;
		/**
		 * return registered default chip if exist.
		 * beginning search on backward parameter chip, type and than family
		 *
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 */
		const chips_t* getRegisteredDefaultChip(const string& server, const string& family, const string& type, const string& chip) const;
		/**
		 * return an older structure from the chip defined with folder and subroutine
		 * which last older is active. By this older structure, active flag will be set to false.
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @param nonactive whether older should be set to non active
		 * @return last active older structure
		 */
		const otime_t* getLastActiveOlder(const string& folder, const string& subroutine, const bool nonactive);
		/**
		 * read all values which be inside of chips for fractions or highest be saved
		 *
		 * @param pos which position should read
		 * @param older whether should read older values (true) or for current writing (false =default)
		 * @return values with time for fractions or highest in variable action.<br />
		 * 			By ending, action is 'no'. If action is fraction value and time is in variables highest.highest and hightime.
		 */
		write_t getLastValues(const unsigned int pos, const bool bolder= false);
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
		write_t allowDbWriting(const string& folder, const string& subroutine, const double value, time_t acttime, bool *newOlder= NULL);
		/**
		 * calcuĺating the next time in seconds always on begin for the next hour, day, week, month or year
		 *
		 * @param newer whether should be the time in the feature (true), or past (false)
		 * @param acttime actual time
		 * @param more how much hours, days .. and so on the next time be calculated
		 * @param spez specification whether variable more is for hour, day, week, ...
		 * @return seconds for the next calculated time
		 */
		static time_t calcDate(const bool newer, const time_t acttime, const int more, const char spez);
		/**
		 * describe whether all chips for folder and subroutine are defined to allow thin older database
		 *
		 * @param defined whether chips defined
		 */
		void chipsDefined(const bool defined)
		{ m_bChipsDef= defined; };
		/**
		 * describe whether all chips for folder and subroutine are defined
		 */
		bool chipsAreDefined()
		{ return m_bChipsDef; };
		/**
		 * destructor of object
		 */
		virtual ~DefaultChipConfigReader();

	private:
		/**
		 * singelton instance of DefaultChipConfigReader
		 */
		static DefaultChipConfigReader* _instance;
		/**
		 * path where the configuration files all be found
		 */
		string m_sPath;
		/**
		 * whether all chips for folder and subroutine are defined to allow thin older database
		 */
		bool m_bChipsDef;
		/**
		 * map of all older structure from default.conf
		 * splits in folder > subroutine > range > float value > older strutures otime_t
		 */
		map<string, map<string, map<double , map<bool, defValues_t> > > > m_mmmmDefaultValues;
		/**
		 * map of all defined chips splits with server > family-id > type  > chip-id > pin > dbwriting-structure t_chips
		 */
		map<string, map<string, map<string, map<string, map<string, chips_t> > > > > m_mmmmmChips;
		/**
		 * map of all used chips,
		 * splits with server > chip > chips_t structure
		 */
		map<string, map<string, chips_t> > m_mmUsedChips;
		/**
		 * map of used chips in folder and subroutine,
		 * splits in folder > subroutine > pointer to chips_t structure
		 */
		map<string, map<string, chips_t*> > m_mmAllChips;
		/**
		 * default chip values,
		 * spliced in server > names of family/type/ID/pin > conde of names > pointer to pchips_t structure
		 */
		map<string, map<string, map<string, chips_t*> > > m_mDefaults;

		/**
		 * constructor of object to first define
		 */
		DefaultChipConfigReader(const string& path)
		:	m_sPath(path),
			m_bChipsDef(false)
			{ initialDefault(); };
		/**
		 * initial the default.conf for all chips how long be writing
		 * if the not defined in an other config file
		 */
		void initialDefault();
		/**
		 * save chip in map of all chips
		 *
		 * @param chip structure of chip witch should save
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 * @param pin witch pin inside the chip is used
		 */
		void saveChip(chips_t chip, const string& server, const string& family, const string& type, const string& ID, const string& pin);
		/**
		 * read Section in properties
		 *
		 * @param server name of current server
		 * @param property actual section
		 * @param bDefault whether reading is only for the default.conf
		 */
		void readSection(string server, IInterlacedPropertyPattern *property, const bool bDefault);
		/**
		 * read the older part's for section
		 *
		 * @param property actual section
		 * @param first whether the older section is the first, the method duplicate the section,
		 * 				because the first is for normally reading and all other to thin database
		 * @return older part
		 */
		otime_t* readOlderSection(IInterlacedPropertyPattern* property, const bool first);
		/**
		 * copy the given older structure into an new one
		 *
		 * @param older structure which should copy
		 * @return duplicated older structure
		 */
		otime_t* copyOlder(const otime_t* older);
		/**
		 * search for folder and subroutine the default older values what to do
		 * and return an dublicated older structure
		 *
		 * @param folder name of folder
		 * @param subroutine name of subroutine
		 * @param min minimal value of range
		 * @param max maximal value of range
		 * @param bFloat whether value can be an floating value
		 * @return new older structure
		 */
		otime_t* getNewDefaultChipOlder(const string& folder, const string& subroutine, const double min, const double max, const bool bFloat);
	};

}

#endif /* DEFAULTCHIPCONFIGREADER_H_ */
