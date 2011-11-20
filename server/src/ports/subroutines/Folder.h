/**
 *   This file 'Folder.h' is part of ppi-server.
 *   Created on: 28.01.2011
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

#ifndef FOLDER_H_
#define FOLDER_H_

#include "Switch.h"
#include "Value.h"
#include "Set.h"
#include "Timer.h"
#include "Shell.h"
#include "Debug.h"
#include "ExternPort.h"
#include "Lirc.h"

namespace subroutines
{

	class Folder
	{
	public:
		/**
		 * constructor to define folder
		 *
		 * @param out object to write output
		 * @param name name of folder
		 */
		Folder(ofstream& out, const string& name);
		/**
		 * write description on command line
		 * with an sharp ('#') before
		 * when content is no null string
		 *
		 * @param content content of string (default:"")
		 */
		void description(const string& content= "");
		/**
		 * write actual folder name when not be done,
		 * elsewhere folder will be written when any subroutine be pulled
		 */
		void flush();
		/**
		 * create SWITCH subroutine
		 *
		 * @param name name of subroutine
		 * @return subroutine
		 */
		Switch* getSwitch(const string& name);
		/**
		 * create VALUE subroutine
		 *
		 * @param name name of subroutine
		 * @return subroutine
		 */
		Value* getValue(const string& name);
		/**
		 * create SET subroutine
		 *
		 * @param name name of subroutine
		 * @return subroutine
		 */
		Set* getSet(const string& name);
		/**
		 * create TIMER subroutine
		 *
		 * @param name name of subroutine
		 * @return subroutine
		 */
		Timer* getTimer(const string& name);
		/**
		 * create SHELL subroutine
		 *
		 * @param name name of subroutine
		 * @return subroutine
		 */
		Shell* getShell(const string& name);
		/**
		 * create DEBUG subroutine
		 *
		 * @param name name of subroutine
		 * @return subroutine
		 */
		Debug* getDebug(const string& name);
		/**
		 * create LIRC subroutine
		 *
		 * @param type type of external port
		 * @param name name of subroutine
		 * @return subroutine
		 */
		ExternPort* getExternPort(const string& type, const string& name);
		/**
		 * create LIRC subroutine
		 *
		 * @param name name of subroutine
		 * @return subroutine
		 */
		Lirc* getLirc(const string& name);
		/**
		 * destructor to delete all created objects
		 */
		virtual ~Folder();

	private:
		/**
		 * whether name and type of subroutine is written
		 */
		bool m_bWritten;
		/**
		 * name of subroutine
		 */
		const string m_sName;
		/**
		 * output file to write subroutine on command line
		 */
		ofstream& m_oOutput;
		/**
		 * all created subroutines to delete on ending of Folder
		 */
		vector<Subroutine*> m_vpObjs;
	};

}

#endif /* FOLDER_H_ */
