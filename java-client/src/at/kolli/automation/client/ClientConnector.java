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
/**
 * written by Alexander Kolli
 * 
 */
package at.kolli.automation.client;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;

import org.apache.regexp.RE;

/**
 * connection interface to the ppi-server.<br />
 * <table>
 *   <th>
 *     <td colspan="2">
 *       throws IOException<br />
 *       can have follow error messages:
 *     </td>
 *   </th>
 *   <tr>
 *     <td>
 *       UNKNOWNHOSTEXCEPTION
 *     </td>
 *     <td>
 *       can not reach given host or port
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>
 *       UNDEFINEDSERVER
 *     </td>
 *     <td>
 *       unknown server running on host:port
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>
 *       SOCKETIOEXCEPTION
 *     </td>
 *     <td>
 *       cannot initial socket to write or read
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>
 *       READERIOEXCEPTION
 *     </td>
 *     <td>
 *       cannot read from ppi-server
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>
 *       WRITERIOEXCEPTION
 *     </td>
 *     <td>
 *       cannot write to ppi-server
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>
 *       FAILDSECONDCONNECTIONID
 *     </td>
 *     <td>
 *       can not make second connection to server
 *     </td>
 *   </tr>
 *   <tr>
 *     <td>
 *       ABORTEDCONNECTION
 *     </td>
 *     <td>
 *       connection to server was aborted
 *     </td>
 *   </tr>
 * </table>
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 30.11.2007
 * @since JDK 1.6
 */
public class ClientConnector 
{
	/**
	 * by starting ppi server this variable
	 * shows which process be starting
	 */
	private volatile String m_sProcessStart;
	/**
	 * by starting ppi server this variable
	 * shows how much percent is starting from process,
	 * or -1 by undefined
	 */
	private volatile Integer m_nPercentStart;
	/**
	 * host where the ppi-server running
	 */
	protected String m_sHost;
	/**
	 * port where the ppi-server running
	 */
	protected int m_nPort;
	/**
	 * created socket to the ppi-server
	 */
	private Socket m_oSock;
	/**
	 * created second socket to the ppi-server
	 */
	private Socket m_oSecSock= null;
	/**
	 * connection ID given from Server
	 */
	private long m_nConnectionID;
	/**
	 * whether have an second connection to server
	 */
	private volatile boolean m_bSecond= false;
	/**
	 * instance of own object
	 */
	private static ClientConnector _instance;
	/**
	 * writer on socket
	 */
	protected PrintWriter m_oPut;
	/**
	 * reader from socket
	 */
	protected BufferedReader m_oGet;
	/**
	 * second reader from socket
	 */
	protected BufferedReader m_oSecGet= null;

	/**
	 * constructor
	 * 
	 * @param host where the ppi-server running
	 * @param port where the ppi-serer running
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	protected ClientConnector(String host, int port)
	{
		m_sHost= host;
		m_nPort= port;
	}
	
	/**
	 * returning instance of own object
	 * 
	 * @return ClientConnector instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static ClientConnector instance()
	{
		return _instance;
	}
	
	/**
	 * creating and returning instance of own object
	 * 
	 * @param host where the ppi-server running
	 * @param port where the ppi-server running
	 * @return ClientConnector instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static ClientConnector instance(String host, int port)
	{
		if(_instance == null)
			_instance= new ClientConnector(host, port);
		return _instance;
	}

	/**
	 * by starting ppi server this variable
	 * shows which process be starting
	 * 
	 * @return name of process which starting, elsewhere an null string
	 */
	public String getServerStartingProcess()
	{
		return m_sProcessStart;
	}
	
	/**
	 * by starting ppi server this variable
	 * shows how much percent is starting from process,
	 * or -1 by undefined
	 * 
	 * @return percent of starting process name
	 */
	public Integer getServerStartingPercent()
	{
		return m_nPercentStart;
	}
	
	/**
	 * beginning connection to server
	 * 
	 * @param user binding to server with this user
	 * @param password binding to server with this password
	 * @return OK when connection was made right otherwise an error code from server
	 */
	public synchronized String openConnection(String user, String password) throws IOException
	{
		String res, put;

		if(	m_oSock != null &&
			!m_oSock.isClosed()	)
		{
			return "OK";
		}
		try{
			m_oSock= new Socket(m_sHost, m_nPort);
		}catch(UnknownHostException ex)
		{
			throw new IOException("UNKNOWNHOSTEXCEPTION");
			
		}catch(IOException ex)
		{
			throw new IOException("SOCKETIOEXCEPTION");
		}
		
		try{
			m_oPut= new PrintWriter(m_oSock.getOutputStream());
		}catch(IOException ex)
		{
			throw new IOException("WRITERIOEXCEPTION");
		}
		
		try{
			m_oGet= new BufferedReader(new InputStreamReader(m_oSock.getInputStream()));
		}catch(IOException ex)
		{
			try{
				m_oSock.close();
				m_oPut.close();
			}catch(IOException iex)
			{
				// ???
			}
			throw new IOException("READERIOEXCEPTION");
		}
		
		put= "GET v" + Float.toString(Definitions.SERVER_PROTOCOL);
		m_oPut.println(put);
		m_oPut.flush();
		try{
			String id;
			RE oldanswer= new RE("^port-server:([0-9]+)$");
			RE answer= new RE("^ppi-server:([0-9]+)( v([.0-9]+))$");
			boolean oldmatch= false;
			
			res= m_oGet.readLine();
			if(res == null)
			{
				try{
					m_oSock.close();
					m_oPut.close();
				}catch(IOException iex)
				{
					// ???
				}
				throw new IOException("READERIOEXCEPTION");
			}						 
			if(!answer.match(res))
			{
				// if get no right answer from ppi-server
				// check also whether server gives old answer
				oldmatch= oldanswer.match(res);
				if(!oldmatch)
				{
					try{
						m_oSock.close();
						m_oPut.close();
					}catch(IOException iex)
					{
						// ???
					}
					if(res.substring(0, 11).equals("WARNING 001"))
					{
						String[] split;						
						
						split= res.substring(12).split(" ");
						if(split.length < 2)
						{
							m_sProcessStart= "starting";
							m_nPercentStart= -1;
						}else
						{// is server busy
							m_sProcessStart= split[0];
							m_nPercentStart= Integer.valueOf(split[1]);
						}
						throw new IOException("PORTSERVERBUSY " + res.substring(12));
					}
					throw new IOException("UNDEFINEDSERVER");
				}
			}
			id= answer.getParen(1);
			m_nConnectionID= Long.decode(id);
			id= answer.getParen(3);
			if(oldmatch)
				Definitions.SERVER_PROTOCOL= 0F;
			else
				Definitions.SERVER_PROTOCOL= Float.parseFloat(id);
			m_oPut.println("U:" + user + ":" + password);
			m_oPut.flush();
			res= m_oGet.readLine();
			if(res == null)
			{
				throw new IOException("READERIOEXCEPTION");
			}
			if(!res.equals("OK"))
			{
				return res;
			}
		}catch(IOException ex)
		{
			String error;
			
			error= ex.getMessage();
			if(	!ex.getMessage().equals("UNDEFINEDSERVER") &&
				!ex.getMessage().substring(0, 14).equals("PORTSERVERBUSY")	)
			{
				error= "READERIOEXCEPTION";
			}
			try{
				m_oSock.close();
			}catch(IOException iex)
			{
				// ???
			}
			m_oPut.close();
			throw new IOException(error);
		}
		return "OK";
	}
	
	/**
	 * open an second connection to server for hearing on changes
	 * 
	 * @param user binding to server with this user
	 * @param password binding to server with this password
	 * @return OK, when connection is or was created, elsewhere error code from server
	 */
	public synchronized String secondConnection(String user, String password) throws IOException
	{
		String res, put;
		PrintWriter oPut;

		if(m_bSecond)
			return "OK";
		try{
			m_oSecSock= new Socket(m_sHost, m_nPort);
		}catch(UnknownHostException ex)
		{
			throw new IOException("UNKNOWNHOSTEXCEPTION");
		}catch(IOException ex)
		{
			throw new IOException("SOCKETIOEXCEPTION");
		}
		
		try{
			oPut= new PrintWriter(m_oSecSock.getOutputStream());
		}catch(IOException ex)
		{
			try{
				m_oSecSock.close();
			}catch(IOException iex)
			{
				// ???
			}
			throw new IOException("WRITERIOEXCEPTION");
		}
		
		try{
			m_oSecGet= new BufferedReader(new InputStreamReader(m_oSecSock.getInputStream()));
		}catch(IOException ex)
		{
			try{
				m_oSecSock.close();
				oPut.close();
			}catch(IOException iex)
			{
				// ???
			}
			throw new IOException("READERIOEXCEPTION");
		}
		
		put= "GET v" + Float.toString(Definitions.SERVER_PROTOCOL);
		put+= " ID:" + Long.toString(m_nConnectionID);		
		oPut.println(put);
		oPut.flush();
		try{
			String id;
			String buf;
			RE answer;
			boolean match= false;
			 
			if(Definitions.SERVER_PROTOCOL < 1)
				answer= new RE("^port-server:([0-9]+)$");
			else
				answer= new RE("^ppi-server:([0-9]+) v[.0-9]+$");
			buf= m_oSecGet.readLine();
			if(buf != null)
				match= answer.match(buf);
			if(!match)
			{
				try{
					m_oSecGet.close();
					m_oSecSock.close();
				}catch(IOException iex)
				{
					// ???
				}
				oPut.close();
				throw new IOException("UNDEFINEDSERVER");
			}
			id= answer.getParen(1);
			if(m_nConnectionID != Long.decode(id))
			{
				try{
					m_oSecGet.close();
					m_oSecSock.close();
				}catch(IOException iex)
				{
					// ???
				}
				oPut.close();
				throw new IOException("FAILDSECONDCONNECTIONID");
			}
			oPut.println("U:" + user + ":" + password);
			oPut.flush();
			res= m_oSecGet.readLine();
			if(!res.equals("OK"))
			{
				try{
					m_oSecGet.close();
					m_oSecSock.close();
				}catch(IOException iex)
				{
					// ???
				}
				oPut.close();
				return res;
			}
		}catch(IOException ex)
		{
			try{
				if(m_oSecSock != null)
					m_oSecSock.close();
			}catch(IOException iex)
			{
				// ???
			}
			oPut.close();
			throw new IOException("READERIOEXCEPTION");
		}
		m_bSecond= true;
		return "OK";
	}
	
	/**
	 * whether have client an second connection to server for hearing
	 * 
	 * @return true if second connection is established
	 */
	public boolean haveSecondConnection()
	{
		return m_bSecond;
	}
		
	/**
	 * ask server which files be set in client sub-directory
	 * @param filter filter files with this extension	 * 
	 * @return array of HashMap of files as key and date as value, or ERROR as key and error number as day from Date
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public HashMap<String, Date> getDirectory(String filter) throws IOException
	{
		String getString;
		Calendar cal= Calendar.getInstance();
		HashMap<String, Date> array= new HashMap<String, Date>();
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		RE descriptor= new RE("^/(.*) -> ([0-9]{1,2})/([0-9]{1,2})/([0-9]{2}) ([0-9]{1,2}):([0-9]{1,2}):([0-9]{1,2})");
		
		synchronized (m_oSock)
		{
			m_oPut.println("DIR " + filter);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				while(getString.compareTo("done") != 0)
				{
					if(error.match(getString))
					{
						cal.set(1970, 1, Integer.getInteger(error.getParen(1)).intValue());
						array.put("ERROR", cal.getTime());
						break;
					}
					if(descriptor.match(getString))
					{
						int Year= Integer.parseInt("20" +descriptor.getParen(4));
						int Month= Integer.parseInt(descriptor.getParen(2));
						int Day= Integer.parseInt(descriptor.getParen(3));
						int Houer= Integer.parseInt(descriptor.getParen(5));
						int Min= Integer.parseInt(descriptor.getParen(6));
						int Sec= Integer.parseInt(descriptor.getParen(7));
						
						cal.set(Year, (Month - 1), Day, Houer, Min, Sec);
						array.put(descriptor.getParen(1), cal.getTime());
					}
					getString= m_oGet.readLine();
				}
			}catch(IOException ex)
			{
				throw new IOException("READERIOEXCEPTION");
			}catch(NullPointerException ex)
			{
				throw new IOException("ABORTEDCONNECTION");
			}
		}
		
		return array;
	}

	/**
	 * set double value on given path
	 * 
	 * @param path  of subroutine<br />
	 * 				The path is made up of &lt;folder&gt;:&lt;subroutine&gt;:[subroutine:...]
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @return OK, when connection is or was created, elsewhere error code from server
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String setValue(String path, double value) throws IOException
	{
		Pathholder tStruct= splitPath(path);
		
		return setValue(tStruct.folder, tStruct.subroutine, value);
	}
	
	/**
	 * set double value on given path
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine name of subroutine 
	 * @param value	witch should be set<br />
	 * 				If the value should be an boolean for an SWITCH-subroutine, set 1 for true and 0 for false
	 * @return OK, or error code from server
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String setValue(String folder, String subroutine, double value) throws IOException
	{
		String getString;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");

		synchronized (m_oSock)
		{	
			m_oPut.println("SET " + folder + ":" + subroutine + " " + value);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				if(getString == null)
				{
					throw new IOException("ABORTEDCONNECTION");
				}
				if(error.match(getString))
				{
					return getString;
				}
			}catch(IOException ex)
			{
				throw new IOException("READERIOEXCEPTION");
			}
		}
		return "OK";
	}

	/**
	 * split an path from folder and subroutines, which be delimited with colons
	 * 
	 * @param path folder and subroutines delimited with colons
	 * @return an object from PathHolder which contains separately folder and subroutines
	 * @see PathHolder
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public Pathholder splitPath(String path)
	{
		Pathholder tStruct= new Pathholder();
		String split[]= path.split(":");

		if(	split == null ||
			split.length == 0	)
		{
			tStruct.folder= "";
			tStruct.subroutine= "";
			
		}else if(split.length == 1)
		{
			tStruct.folder= split[0];
			tStruct.subroutine= ""; // get later error from server
			
		}else
		{
			tStruct.folder= split[0];
			tStruct.subroutine= split[1];
		}
		return tStruct;
	}

	/**
	 * read XML-file from server
	 * 
	 * @param file string of filename
	 * @return ArrayList with strings from the layout xml-file or an error code from server inside array
	 */
	public ArrayList<String> getContent(String file) throws IOException
	{
		ArrayList<String> xmlFile= new ArrayList<String>();
		RE endParser= new RE("<[ \t]*/[ \t]*layout[ \t]*>");
		RE error= new RE("<[ \t]*error[ \t]*.*[ \t]*number[ \t]*=[ \t]*('|\")([ 0-9]+)('|\")[ \t]*/[ \t]*>");
		String getString= null;

		synchronized (m_oSock)
		{
			m_oPut.println("CONTENT " + file);
			m_oPut.flush();
			
			try{
				do{
					getString= m_oGet.readLine();
					if(getString == null)
					{
						throw new IOException("ABORTEDCONNECTION");
					}
					if(error.match(getString))
					{
						xmlFile.add(getString);
						break;
					}
					xmlFile.add(getString);
					
				}while(!endParser.match(getString));
				
			}catch(IOException ex)
			{
				String errormsg;
				
				errormsg= "READERIOEXCEPTION";
				if(ex.getMessage().equals("ABORTEDCONNECTION"))
					errormsg= ex.getMessage();
				throw new IOException(errormsg);
				
			}catch(NullPointerException ex)
			{
				throw new IOException("ABORTEDCONNECTION");
			}
		}
		return xmlFile;
	}
	
	/**
	 * change user account for new permission
	 * 
	 * @param user user name for new account
	 * @param password password for user
	 * @return string of OK when changing was successfully, otherwise error code from server
	 */
	public String changeUser(String user, String password) throws IOException
	{
		String res;
		
		res= command("CHANGE " + user + ":" + password);
		if(res == null)
			throw new IOException("ABORTEDCONNECTION");
		return res;
	}
	
	/**
	 * sending request for permission groups
	 * 
	 * @param groups groups of permission, separately with colons
	 * @return strings of write, reade, none or error code of server 
	 */
	public String permission(String groups) throws IOException
	{
		return command("PERMISSION " + groups);
	}
	
	/**
	 * sending request for subroutines in folder to hearing for changes.<br />
	 * this command should only send if open an second connection
	 * to server where you can send an hearing request with the command <code>request()</code>
	 * 
	 * @param folder name of folder
	 * @param subroutine name of subroutine
	 * @return OK when hear command was requested correctly, or an error code from server
	 */
	public String hear(String folder, String subroutine) throws IOException
	{
		String res;
		
		res= command("HEAR " + folder + ":" + subroutine);
		if(res == null)
			throw new IOException("ABORTEDCONNECTION");
		if(!res.equals("done"))
		{
			return res;
		}
		return "OK";
	}

	/**
	 * sending request for subroutines in folder to hearing for changes.<br />
	 * this command should only send if open an second connection
	 * to server where you can send an hearing request with the command <code>request()</code>
	 * 
	 * @param result name of folder and subroutine, separated with an colon
	 * @return OK when hear command was requested correctly, or an error code from server
	 */
	public String hear(String result) throws IOException
	{
		Pathholder tStruct= splitPath(result);
		
		return hear(tStruct.folder, tStruct.subroutine);
	}
	
	/**
	 * hear on server for changing subroutines
	 * 
	 * @return folder and subroutine with changed value, or the error from server
	 * @throws IOException
	 */
	public String hearing() throws IOException
	{
		String res= null;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		synchronized (m_oSecSock)
		{
			try{
				res= m_oSecGet.readLine();
				if(res == null)
					throw new IOException("ABORTEDCONNECTION");
				if(error.match(res))
				{
					return res;
				}			
			}catch(IOException ex)
			{				
				throw new IOException("READERIOEXCEPTION");
				
			}catch(NullPointerException ex)
			{
				throw new IOException("ABORTEDCONNECTION");
			}
		}
		return res;
	}
	/**
	 * clear hearing on server for the request was sending before
	 * 
	 * @return OK when hearing was cleared, otherwise the string of false when no second connection exits, or the error code from server
	 */
	public String clearHearing() throws IOException
	{

		String res;
		
		if(!haveSecondConnection())
			return "false";
		res= command("NEWENTRYS");
		if(res == null)
			throw new IOException("ABORTEDCONNECTION");
		if(!res.equals("done"))
			return res;
		return "OK";
	}
	
	/**
	 * sending any command to server and waiting for answer
	 * 
	 * @param command full command which should sending to server
	 * @return answer if the request was successfully, or throws IOException
	 */
	protected String command(String command) throws IOException
	{
		String res= null;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");
		
		synchronized (m_oSock)
		{	
			m_oPut.println(command);
			m_oPut.flush();
			try{
				res= m_oGet.readLine();
				if(res == null)
				{
					throw new IOException("ABORTEDCONNECTION");
				}
				if(error.match(res))
					return res;
				
			}catch(IOException ex)
			{
				throw new IOException("READERIOEXCEPTION");
				
			}catch(NullPointerException ex)
			{
				throw new IOException("ABORTEDCONNECTION");
			}
		}
		return res;
	}
	/**
	 * get value from given path, which be set currently on server
	 * 
	 * @param path folder and subroutines delimited with colons
	 * @param value double value getting from server
	 * @return OK when get an right double value from server, otherwise the string false or an error code from server
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String getValue(String path, DoubleHolder value) throws IOException
	{
		Pathholder tStruct= splitPath(path); 
		
		return getValue(tStruct.folder, tStruct.subroutine, value);
	}

	/**
	 * get value from given folder and subroutines, which be set currently on server
	 * 
	 * @param folder the folder which be set on the server
	 * @param subroutine one ore more subroutines, delimited with colons
	 * @param value double value getting from server
	 * @return OK when get an right double value from server, otherwise the string false or an error code from server
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String getValue(String folder, String subroutine, DoubleHolder value) throws IOException
	{
		double res;
		String getString= null;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)?$");

		synchronized (m_oSock)
		{	
			m_oPut.println("GET " + folder + ":" + subroutine);
			m_oPut.flush();
			try{
				getString= m_oGet.readLine();
				if(getString == null)
					throw new IOException("ABORTEDCONNECTION");
				if(error.match(getString))
					return getString;
				
			}catch(IOException ex)
			{
				throw new IOException("READERIOEXCEPTION");
				
			}catch(NullPointerException ex)
			{
				throw new IOException("READERIOEXCEPTION");
			}
		}
		try{
			res= Double.parseDouble(getString);
			value.set(res);
			
		}catch(NumberFormatException ex)
		{
			value.set(0);
			return "false";
		}
		return "OK";
	}

	/**
	 * close connection to server
	 * 
	 * @return true if connection is correctly closed, otherwise false
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public synchronized void closeConnection()
	{
		m_oPut.println("ending");
		m_oPut.flush();
		m_bSecond= false;
		synchronized (this)
		{	
			try{
				m_oGet.close();
				if(m_oSecGet != null)
					m_oSecGet.close();
				m_oPut.close();
				m_oSock.close();
				if(m_oSecSock != null)
					m_oSecSock.close();
			}catch(IOException ex)
			{
				ex.printStackTrace();
			}
		}
		return;
	}
	
}
