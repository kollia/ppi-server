/**
 *   This file 'OwfsSupport.h' is part of ppi-server.
 *   Created on: 13.11.2011
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

#ifdef _OWFSLIBRARY
#ifndef OWFSSUPPORT_H_
#define OWFSSUPPORT_H_

#include "../pattern/util/ICommandStructPattern.h"
#include "../pattern/util/iinterlacedpropertypattern.h"

#include "../pattern/server/ichipaccesspattern.h"

using namespace design_pattern_world;

class OwfsSupport
{
public:
	/**
	 * constructor to set working directory
	 *
	 * @param workdir working directory
	 */
	OwfsSupport(const string workdir) :
		m_sWorkDir(workdir),
		m_nChip(0)
	{};
	/**
	 * produce subroutine examples
	 *
	 * @param params starting parameters
	 * @param oServerProperties property file of server.conf
	 */
	int execute(const ICommandStructPattern* params, IInterlacedPropertyPattern* oServerProperties);

private:
	struct defs_t
	{
		string name;
		string desc;
		string ID;
		string pin;
		string action;
	};
	/**
	 * working directory
	 */
	const string m_sWorkDir;
	/**
	 * actualy chip to read
	 */
	unsigned short m_nChip;
	/**
	 * vector for all first chip
	 * reading pro maximinit
	 */
	vector<unsigned short> m_nFirstChip;
	/**
	 * all configured chip's
	 */
	map<string, string> m_mConfigured;
	/**
	 * all defined subroutines
	 */
	map<string, defs_t> m_mSubroutines;

	/**
	 * show general map of chip's
	 *
	 * @param chip reference to chip reader (MaximChipAccess object)
	 * @param path which chip should be reading, or null string for reading root path
	 * @param found whether was found any chip
	 * @param result reding result of chips
	 * @return 0 by OK, otherwise -1
	 */
	int show(IChipAccessPattern* chip, string path, bool& found, vector<string>& result);
	/**
	 * write inside example file
	 *
	 * @param exampleFile name of example file
	 * @param chipPath defined path to chip
	 * @param chip reference to chip reader (MaximChipAccess object)
	 * @return whether writing was successful
	 */
	bool writeExample(const string& exampleFile, string chipPath, IChipAccessPattern* chip);
	/**
	 * writing header description inside given file handle
	 *
	 * @param file handle of file
	 * @param exampleFile name of example file
	 */
	void writeHead(ofstream& file, const string& exampleFile);
	/**
	 * used by reconfigure or delete any subroutine and write the hole example file new
	 *
	 * @param exampleFile name of example file
	 * @return whether writing was successful
	 */
	bool writeHoleExample(const string& exampleFile);
	/**
	 * write one subroutine in given file handle
	 *
	 * @param file handle of file
	 * @param defs definition structure of subroutine, gives back defs.action differing by bWritable
	 * @param bWritable whether action parameter of defs should be 'read' or 'write | read'
	 */
	void writeSubroutine(ofstream& file, defs_t& defs, const bool bWriteable);
};

#endif /* OWFSSUPPORT_H_ */
#endif /* _OWFSLIBRARY */


