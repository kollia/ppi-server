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

#include "../util/smart_ptr.h"

#include "../util/properties/interlacedactionproperties.h"

#include "../pattern/util/IChipConfigReaderPattern.h"

using namespace std;
using namespace util;
using namespace design_pattern_world;

namespace ports
{

	class DefaultChipConfigReader : public virtual IChipConfigReaderPattern
	{
	public:

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
		 * return the defined default cache from default.conf.<br />
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
		double getDefaultCache(const double min, const double max, const bool bFloat, const string& folder= "", const string& subroutine= "") const;
		/**
		 * return registered default chip of unique chip ID and server.
		 *
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 */
		const chips_t* getRegisteredDefaultChip(const string& server, const string& chip) const;
		/**
		 * return cache value from registered default chip of unique chip ID and server.
		 *
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 */
		const double* getRegisteredDefaultChipCache(const string& server, const string& chip) const;
		/**
		 * return registered default chip if exist.
		 * beginning search on backward parameter chip, type and than family
		 *
		 * @param server name of server
		 * @param family specified family code of chip
		 * @param type specified type of chip
		 * @param chip unique id of chip
		 */
		const chips_t* getRegisteredDefaultChip(const string& server,
						const string& family, const string& type, const string& chip) const;
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
												const string& subroutine, const bool nonactive);
		/**
		 * read all values which be inside of chips for fractions or highest be saved
		 *
		 * @param pos which position should read
		 * @param older whether should read older values (true) or for current writing (false =default)
		 * @return values with time for fractions or highest in variable action.<br />
		 * 			By ending, action is 'no'. If action is fraction value and time is in variables highest.highest and hightime.
		 */
		virtual write_t getLastValues(const unsigned int pos, const bool bolder= false);
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
																		time_t acttime, bool *newOlder= NULL);
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
		virtual bool chipsAreDefined()
		{ return m_bChipsDef; };
		/**
		 * delete object of DefaultChipConfigReader
		 */
		static void deleteObj();
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
		SHAREDPTR::shared_ptr<otime_t> readOlderSection(IInterlacedPropertyPattern* property, const bool first);
		/**
		 * copy the given older structure into an new one
		 *
		 * @param older structure which should copy
		 * @return duplicated older structure
		 */
		SHAREDPTR::shared_ptr<otime_t> copyOlder(const SHAREDPTR::shared_ptr<otime_t> &older) const;
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
		SHAREDPTR::shared_ptr<otime_t> getNewDefaultChipOlder(const string& folder, const string& subroutine, const double min, const double max, const bool bFloat) const;
	};

}

#endif /* DEFAULTCHIPCONFIGREADER_H_ */
