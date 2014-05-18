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

#include <set>

#include "../pattern/util/ICommandStructPattern.h"
#include "../pattern/util/IPPIDatabasePattern.h"

#include "../ports/subroutines/Folder.h"

using namespace subroutines;

class LircSupport {
public:
	/**
	 * structure of remote and code names
	 */
	struct t_remote
	{
		/**
		 * name of remote control for lirc
		 */
		string org_remote;
		/**
		 * alias name inside ppi-server for remote control
		 */
		string alias_remote;
		/**
		 * name of code in remote control for lirc
		 */
		string org_code;
		/**
		 * alias name of code inside ppi-server
		 */
		string alias_code;
		/**
		 * display name of code for user
		 */
		string display_code;
	};
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
	 * @param params setting options and parameters by starting
	 */
	int execute(const ICommandStructPattern* params);
	/**
	 * set all defined names from command line
	 * from option --learn
	 *
	 * @param names all defined names
	 */
	void setLearnNames(const vector<string> &names)
	{ m_vLearnNames= names; };

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
	 * structure of founded pre-defined namespaces
	 */
	struct set_t
	{
		unsigned short KEY;
		unsigned short KEY_NUMMERIC;
		unsigned short KEY_F;
		unsigned short KEY_FN_F;
		unsigned short KEY_BRL_DOT;
		unsigned short KEY_KP;
		unsigned short BTN;

		bool KEY_CHANNELUP;
		bool KEY_NEXT;

		bool KEY_PAGEUP;
		bool KEY_SCROLLUP;
		bool KEY_VOLUMEUP;
		bool KEY_BRIGHTNESSUP;
		bool KEY_KBDILLUMUP;
		bool BTN_GEAR_UP;

		bool KEY_LEFT;
		bool KEY_LEFTSHIFT;
		bool KEY_LEFTALT;
		bool KEY_LEFTBRACE;
		bool KEY_LEFTCTRL;
		bool KEY_LEFTMETA;
		bool BTN_LEFT;

		bool KEY_ZOOMIN;
		bool KEY_FORWARD;

		set_t()
		{
			KEY= 20;
			KEY_NUMMERIC= 20;
			KEY_F= 20;
			KEY_FN_F= 20;
			KEY_BRL_DOT= 20;
			KEY_KP= 20;
			BTN= 20;

			KEY_CHANNELUP= false;
			KEY_NEXT= false;

			KEY_PAGEUP= false;
			KEY_SCROLLUP= false;
			KEY_VOLUMEUP= false;
			KEY_BRIGHTNESSUP= false;
			KEY_KBDILLUMUP= false;
			BTN_GEAR_UP= false;

			KEY_LEFT= false;
			KEY_LEFTSHIFT= false;
			KEY_LEFTALT= false;
			KEY_LEFTBRACE= false;
			KEY_LEFTCTRL= false;
			KEY_LEFTMETA= false;
			BTN_LEFT= false;

			KEY_ZOOMIN= false;
			KEY_FORWARD= false;
		}
	};
	/**
	 * enum for witch direction the steps going
	 * and whether by ending step the value should begin by start
	 */
	enum direction_e
	{
		UP_STOP= 0,
		DOWN_STOP,
		UP_LOOP,
		DOWN_LOOP
	};
	/**
	 * enum whether sending should during for longer time
	 * or only one step
	 */
	enum sending_e
	{
		SEND_ONCE= 0,
	//	SEND_units,
		SEND= 2
	//	WAIT_after,
	//	BACK_time
	};
	/**
	 * default value for definition in measure.conf
	 */
	struct default_t
	{
		string group;
		unsigned short steps;
		direction_e direction;
		sending_e send;
		unsigned short digits;
		unsigned short setto;
	};
	/**
	 * structure for defined button
	 */
	struct button_def
	{
		/**
		 * remote control
		 */
		string remote;
		/**
		 * code of button
		 */
		string code;
		/**
		 * subroutine inside folder
		 */
		string subroutine;
		/**
		 * in which group the button is
		 */
		unsigned short group;
		/**
		 *  action to count steps in which direction
		 *      'UP STOP'         = 0
		 *      'DOWN STOP'       = 1
		 *      'UP LOOP'         = 2
		 *      'DOWN LOOP'       = 3
		 *      'set only number' = 4
		 */
		unsigned short direction;
		/**
		 * set do step number when direction is 4
		 */
		int to_value;
	};


	/**
	 * working directory
	 */
	const string m_sWorkDir;
	/**
	 * pre defined codes
	 */
	map<string, set_t> m_mtPreDefined;
	/**
	 * defined parameter names from commandline
	 * from --learn option
	 */
	vector<string> m_vLearnNames;

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
	 * show which names all remote controls from lird.conf has
	 *
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @return whether routine of method was successful. Can be used as exit code
	 */
	int showRemotes(const remotecodes_t& r) const;
	/**
	 * create measure file lirc.conf and for all remote controls an layout file (<remote>.desktop)
	 *
	 * @param transmit whether building files for an transmitter
	 * @param vertical how much rows for layout files should be as default defined
	 * @param userreadperm read permission defined in access.conf for configuration user
	 * @param userwriteperm write permission defined in access.conf for configuration user
	 * @param ureadcw permission for normal user to read and config user to read and write
	 * @param readperm name of group in access.conf which can reading the subroutine
	 * @param writeperm name of group in access.conf which can reading and changing the subroutine
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @param forremote create conf- and desktop-file only for this remote control when set
	 * @return whether creating was successful
	 */
	bool createConfigLayoutFiles(const bool transmit, const int vertical,
			const string& userreadperm, const string& userwriteperm, const string& ureadcw,
			const string& readperm, const string& writeperm, const remotecodes_t& r, const string& forremote) const;
	/**
	 * learn method to create config files for remote control
	 * to make an work flow of pressed buttons
	 *
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @param writeperm change permission of config user
	 * @return whether creating of measure config files was correct
	 */
	bool learn(const remotecodes_t& r, const string& writeperm);
	/**
	 * write for learning one step on work flow
	 *
	 * @param output debug output on command line
	 * @param step actual next step in work flow and gives back also the next step
	 * @param toValue to which value the actual_step field should set
	 * @param folder actual folder to write in for configuration file
	 * @param db access to database with all actual values
	 * @param groupbuttons all defined button names in the same group
	 * @param aliases folder names for different remote with code
	 * @param buttons all buttons with behavior of direction, remote, code, ...
	 */
	void createConfigStep(ostringstream& output, unsigned short& step,
					const double toValue, Folder& folder, IPPIDatabasePattern* db,
					const set<string>& groupbuttons,map<string, string>& aliases, map<string, button_def>& buttons);
	/**
	 * create desktop file
	 *
	 * @return wheter creating was correct
	 */
	bool createLearnDesktopFiles();
	/**
	 * make configuration file (<remote control>.conf)
	 *
	 * @param remote name of remote control
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @param userreadperm read permission defined in access.conf for configuration user
	 * @param userwriteperm write permission defined in access.conf for configuration user
	 * @param ureadcw permission for normal user to read and config user to read and write
	 * @param readperm name of group in access.conf which can reading the subroutine
	 * @param writeperm name of group in access.conf which can reading and changing the subroutine
	 * @param transmit whether the configuration should be defined also for transmitter (true), or only reseiver (false)
	 * @return whether the creating of configuration file was correct
	 */
	bool createRemoteConfFile(const string& remote, const remotecodes_t& r,
					const string& userreadperm, const string& userwriteperm, const string& ureadcw,
					const string& readperm, const string& writeperm, const bool transmit) const;
	/**
	 * make layout file (<remote control>.desktop)
	 *
	 * @param remote name of remote control
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 * @param vertical default vertical rows
	 * @param perm permission to display side in client
	 * @return whether creating of layout file was correct
	 */
	bool createRemoteDesktopFile(const string& remote, const remotecodes_t& r, const int vertical, const string& perm) const;

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
	/**
	 * search from an link pre-defined original and alias remote and codes
	 *
	 * @param code link set in lircd.conf
	 * @param r read value from <code>readLircd(...)</code>
	 * @return structure of original and aliases
	 */
	t_remote getLinkDefinition(const string& code, const remotecodes_t& r) const;
	/**
	 * define first of pre defined codes
	 *
	 * @param r structure of all new defined folders and subroutines from remote controls and codes getting from <code>readLircd()</code>
	 */
	void searchPreDefined(const remotecodes_t& r);
	/**
	 * calculate default values for definition in measure.conf
	 *
	 * @param code original code in remote control
	 * @param remote alias remote control
	 * @return structure of defaults
	 */
	default_t getDefaults(const string& remote, const string& code) const;
};

#endif /* LIRCSUPPORT_H_ */
