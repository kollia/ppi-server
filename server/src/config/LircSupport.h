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

#ifndef LIRCSUPPORT_H_
#define LIRCSUPPORT_H_

#include "../pattern/util/ICommandStructPattern.h"

class LircSupport {
public:
	/**
	 * constructor to set working directory
	 *
	 * @param workdir working directory
	 */
	LircSupport(const string workdir) :
		m_sWorkDir(workdir)
	{};
	/**
	 * configuration for lirc settings
	 *
	 * @param argc count of values
	 * @param argv values in an pointer array
	 */
	int execute(const ICommandStructPattern* params);

private:
	/**
	 * structure of all readed codes
	 */
	struct remotecodes_t
	{
		/**
		 * whether codes can be read successfully
		 */
		bool success;
		/**
		 * all founded remote controls inside lircd
		 */
		map<string, vector<string> > remotes;
		/**
		 * Remote witch are inherit of other remote
		 */
		map<string, vector<string> > kill;
		/**
		 * all remote controls
		 */
		map<string, string> remotealias;
		/**
		 * all receiving/transmitting codes from remote controls
		 */
		map<string, map<string, string> > rcodealias;
		/**
		 * all displaynames from code
		 */
		map<string, map<string, string> > dcodes;
	};

	/**
	 * working directory
	 */
	const string m_sWorkDir;

	/**
	 * read lircd file for configuration
	 *
	 * @param lircd path of lircd.conf file to read
	 * @return structure of all new defined folders and subroutines from remote controls and codes
	 */
	remotecodes_t readLircd(const string& lircd) const;
	/**
	 * write inside of file header information
	 *
	 * @param file in which file information should be written
	 * @param filename name of file without extension
	 */
	void writeHeader(ofstream& file, const string& filename) const;
	/**
	 * create measure file lirc.conf and for all remote controls an layout file (<remote>.desktop)
	 *
	 * @param transmit whether building files for an transmitter
	 * @param vertical how much rows for layout files should be as default defined
	 * @param userreadperm read permission defined in access.conf for configuration user
	 * @param userwriteperm write permission defined in access.conf for configuration user
	 * @param readperm name of group in access.conf which can reading the subroutine
	 * @param writeperm name of group in access.conf which can reading and changing the subroutine
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @param forremote create conf- and desktop-file only for this remote control when set
	 * @return whether creating was successful
	 */
	bool createConfigLayoutFiles(const bool transmit, const int vertical,
			const string& userreadperm, const string& userwriteperm,
			const string& readperm, const string& writeperm, const remotecodes_t& r, const string& forremote) const;
	/**
	 * make configuration file (<remote control>.conf)
	 *
	 * @param remote name of remote control
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @param readperm read permission of config user
	 * @param writeperm change permission of config user
	 * @param userreadperm read permission of normal user
	 * @param userwriteperm change permission of normal user
	 * @param transmit whether the configuration should be defined also for transmitter (true), or only reseiver (false)
	 * @return whether the creating of configuration file was correct
	 */
	bool createRemoteConfFile(const string& remote, const remotecodes_t& r,
			const string& readperm, const string& writeperm,
			const string& userreadperm, const string& userwriteperm, const bool transmit) const;
	/**
	 * make layout file (<remote control>.desktop)
	 *
	 * @param remote name of remote control
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @param vertical default vertical rows
	 * @param transmit whether the configuration should be defined also for transmitter (true), or only reseiver (false)
	 * @param perm permission to display side in client
	 * @return whether creating of layout file was correct
	 */
	bool createRemoteDesktopFile(const string& remote, const remotecodes_t& r, const int vertical,
			const bool transmit, const string& permission) const;

	/**
	 * create link attributes in subroutine, to which other subroutine should be linked
	 *
	 * @param file stream in which to write the subroutine content
	 * @param name name of subroutine
	 * @param remotelink beginning of link names
	 * @param linkcodes vector of all codes splitting with underline after remotelink
	 * @param firstlink name of first subroutine to which can link. If not exist or the same like parameter name, the lwhile attribute will calculate number of group
	 */
	void createSubroutineLink(ostream& file, const string& name, const string& remotelink, const vector<string>& linkcodes, const string& firstlink) const;
};

#endif /* LIRCSUPPORT_H_ */
