/**
 *   This file 'Subroutine.h' is part of ppi-server.
 *   Created on: 27.01.2011
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

#ifndef SUBROUTINE_H_
#define SUBROUTINE_H_

#include <string>
#include <vector>
#include <fstream>

using namespace std;

namespace subroutines
{
	/**
	 * create subroutine for measure.conf
	 */
	class Subroutine
	{
	public:
		/**
		 * constructor to create object
		 *
		 * @param out object to write output
		 * @param name name of subroutine
		 * @param type type of subroutine
		 */
		Subroutine(ofstream& out, const string& name, const string& type);
		/**
		 * return name of subroutine
		 *
		 * @param name of subroutine
		 */
		virtual string getName()
		{ return m_sName; };
		/**
		 * return type of subroutine
		 *
		 * @return type of subroutine
		 */
		virtual string getType()
		{ return m_sType; };
		/**
		 * write perm parameter for access
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pperm(const string& content, const string& desc= "");
		/**
		 * write description on command line
		 * with an sharp ('#') before
		 * when content is no null string
		 *
		 * @param content content of string (default:"")
		 */
		virtual void description(const string& content= "");
		/**
		 * write actual name of subroutine and type parameter when not be done,
		 * elsewhere this will be written before any other parameter should be written
		 */
		virtual void flush();
		/**
		 * virtual destructor with nothing to do
		 */
		virtual ~Subroutine()
		{};

	protected:
		/**
		 * write begin parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pbegin(const string& content, const string& desc= "");
		/**
		 * write while parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pwhile(const string& content, const string& desc= "");
		/**
		 * write end parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pend(const string& content, const string& desc= "");
		/**
		 * write from parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pfrom(const double& content, const string& desc= "");
		/**
		 * write from parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pfrom(const string& content, const string& desc= "");
		/**
		 * write one or more from parameter
		 *
		 * @param content content of parameter
		 */
		virtual void pfrom(const vector<double>& content);
		/**
		 * write one ore more from parameter
		 *
		 * @param content content of parameter
		 */
		virtual void pfrom(const vector<string>& content);
		/**
		 * write set parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pset(const double& content, const string& desc= "");
		/**
		 * write set parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pset(const string& content, const string& desc= "");
		/**
		 * write one or more set parameter
		 *
		 * @param content content of parameter
		 */
		virtual void pset(const vector<string>& content);
		/**
		 * write value parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pvalue(const double& content, const string& desc= "");
		/**
		 * write value parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pvalue(const string& content, const string& desc= "");
		/**
		 * write one or more value parameter
		 *
		 * @param content content of parameter
		 */
		virtual void pvalue(const vector<double>& content);
		/**
		 * write one or more value parameter
		 *
		 * @param content content of parameter
		 */
		virtual void pvalue(const vector<string>& content);
		/**
		 * write string parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pstring(const string& content, const string& desc= "");
		/**
		 * write one or more string parameter
		 *
		 * @param content content of parameter
		 */
		virtual void pstring(const vector<string>& content);
		/**
		 * write link parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void plink(const string& content, const string& desc= "");
		/**
		 * write one or more link parameter
		 *
		 * @param content content of parameter
		 */
		virtual void plink(const vector<string>& content);
		/**
		 * write lwhile parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void plwhile(const string& content, const string& desc= "");
		/**
		 * write default parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pdefault(const double& content, const string& desc= "");
		/**
		 * write default parameter
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pdefault(const string& content, const string& desc= "");
		/**
		 * write min parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		//virtual void pmin(const long& content, const string& desc= "");
		/**
		 * write min parameter for minimal value
		 *
		 * @param content content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pmin(const double& content, const string& desc= "");
		/**
		 * write max parameter for maximal value
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		//virtual void pmax(const long& content, const string& desc= "");
		/**
		 * write max parameter for maximal value
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pmax(const double& content, const string& desc= "");
		/**
		 * write day parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pday(const long& content, const string& desc= "");
		/**
		 * write hour parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void phour(const long& content, const string& desc= "");
		/**
		 * write sec parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void psec(const long& content, const string& desc= "");
		/**
		 * write millisec parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pmillisec(const long& content, const string& desc= "");
		/**
		 * write microsec parameter for microseconds
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pmicrosec(const long& content, const string& desc= "");
		/**
		 * write mtime parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pmtime(const string& content, const string& desc= "");
		/**
		 * write setnull parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void psetnull(const string& content, const string& desc= "");
		/**
		 * write begincommand parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pbegincommand(const string& content, const string& desc= "");
		/**
		 * write whilecommand parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pwhilecommand(const string& content, const string& desc= "");
		/**
		 * write endcommand parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pendcommand(const string& content, const string& desc= "");
		/**
		 * write file parameter
		 *
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void pfile(const string& content, const string& desc= "");
		/**
		 * write action parameter with one or more actions.<br />
		 * Seperated with an pipe ('|')
		 *
		 * @param content action(s) of parameter
		 * @param desc write this description after set parameter
		 */
		virtual void action(const string& content, const string& desc= "");
		/**
		 * write given parameter to output string
		 *
		 * @param parameter which parameter should written
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		void writeParam(const string& parameter, const string& content, const string& desc= "");
		/**
		 * write given parameter to output string
		 *
		 * @param parameter which parameter should written
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		void writeParam(const string& parameter, const long& content, const string& desc= "");
		/**
		 * write given parameter to output string
		 *
		 * @param parameter which parameter should written
		 * @param content content of parameter
		 * @param desc write this description after set parameter
		 */
		void writeParam(const string& parameter, const double& content, const string& desc= "");

	private:
		/**
		 * wheter name and type of subroutine is written
		 */
		bool m_bWritten;
		/**
		 * name of subroutine
		 */
		const string m_sName;
		/**
		 * type of subroutine
		 */
		const string m_sType;
		/**
		 * output file to write subroutine on command line
		 */
		ofstream& m_oOutput;
	};

}

#endif /* SUBROUTINE_H_ */
