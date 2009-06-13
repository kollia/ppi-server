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
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;

import com.sun.org.apache.regexp.internal.RE;


/**
 * Translator from codes setting in file language.txt to various languages
 * 
 * @package at.kolli.automation.client
 * @author Alexander Kolli
 * @version 1.00.00, 30.11.2007
 * @since JDK 1.6
 */
public class MsgTranslator {

	/**
	 * array of exist languages with two digits.<br />
	 * en, de, ...
	 */
	private ArrayList<String> m_asLangs= new ArrayList<String>();
	/**
	 * read hash map of all code as key and translation for all languages as value
	 */
	private HashMap<String, HashMap<String, String>> m_aTranslateMap= new HashMap<String, HashMap<String,String>>();
	/**
	 * hash map only of regulated language
	 * @see m_aTranslateMap
	 */
	private HashMap<String, String> m_aAktHash;
	/**
	 * instance of own single object
	 */
	private static MsgTranslator _instance= null;
	/**
	 * string of exception if the file can not read
	 */
	private String m_sError= "NONE";
	
	/**
	 * constructor to create the translate object
	 * 
	 * @param lang language with two digits
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	private MsgTranslator(String lang)
	{
		try{
			initialisation();

		}catch(FileNotFoundException ex)
		{
			m_sError= ex.getMessage();
			return;
		}catch(IOException ex)
		{
			m_sError= "cannot read file language.txt";
			return;
		}
		if(!m_asLangs.contains(lang))
		{
			System.out.println("language '" + lang + " is not defined,");
			System.out.println("use default language english ('en')");
			lang= "en";
		}
		setLanguage(lang);
	}
	
	/**
	 * create an instance of MsgTranslator class
	 * 
	 * @param lang language with two digits
	 * @return MsgTranslator instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static MsgTranslator init(String lang)
	{
		if(_instance == null)
			_instance= new MsgTranslator(lang);
		return _instance;
	}
	
	/**
	 * return the instance of the object
	 * 
	 * @return MsgTranslator instance
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public static MsgTranslator instance()
	{
		return _instance;
	}
	
	/**
	 * returns the error-string if the file translate.txt can not found
	 * 
	 * @return messeage of FileNotFound exception or NONE if all correct
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String getError()
	{
		return m_sError;
	}
	
	/**
	 * set's an new language in which should be translate
	 * 
	 * @param lang language with two digits
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public void setLanguage(String lang)
	{
		m_aAktHash= m_aTranslateMap.get(lang);
	}
	
	/**
	 * method reads the file translage.txt
	 * 
	 * @throws FileNotFoundException
	 * @throws IOException
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	private void initialisation() throws FileNotFoundException, IOException
	{
		String line;
		RE code= new RE("^[ \t]*(.+)[ \t]*:[ \t]*$");
		RE nullStr= new RE("^[ \t]*$");
		RE noEndLine= new RE("^(.*)/[ \t]*$");
		FileReader read= new FileReader("language.txt");
		BufferedReader get= new BufferedReader(read);
		ArrayList<HashMap<String, String>> hashArray= new ArrayList<HashMap<String,String>>();
		//InputStreamReader getStream= new InputStreamReader(new FileImageInputStream(""))
		
		line= get.readLine();
		if( line == null
			||
			line.compareTo("lang:") != 0	)
		{
			throw new IOException("file language.txt is incorrect");
		}
		line= get.readLine();
		while(	line != null
				&&
				!code.match(line))
		{
			if(!nullStr.match(line))
			{
				m_asLangs.add(line);
				hashArray.add(new HashMap<String, String>());
			}
			line= get.readLine();
		}
		while(line != null)
		{
			String codeStr= line;
			
			for(HashMap<String, String> obj : hashArray)
			{
				String inputStr;
				
				inputStr= "";
				line= get.readLine();
				while(line != null)
				{
					if(!nullStr.match(line))
					{
						if(!noEndLine.match(line))
						{
							inputStr+= line;
							break;
						}else
							inputStr+= noEndLine.getParen(1) + "\n";
					}
					line= get.readLine();					
				}
				if(inputStr.compareTo("") != 0)
					obj.put(codeStr, inputStr);
				if(line == null)
					break;
			}
			while(	line != null
					&&
					!code.match(line)	)
			{
				line= get.readLine();
			}
		}
		Iterator<HashMap<String, String>> it= hashArray.iterator();
		for(String lang : m_asLangs)
		{
			HashMap<String, String> obj= it.next();
			m_aTranslateMap.put(lang, obj);
		}
	}
	
	/**
	 * translate the given code in the default language
	 * 
	 * @param codeStr code which should be translate
	 * @param values values which should appear in the translated string
	 * @return the translated string
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String translate(String codeStr, String... values)
	{
		boolean bfirst= false;
		String sRv= "";
		String str= m_aAktHash.get(codeStr + ":");
		if(str == null)
		{
			return "unknown id(" + codeStr + ")";
		}
		StringTokenizer token= new StringTokenizer(str, "@");
		
		if(str.startsWith("@"))
			bfirst= true;
		
		for(String value : values)
		{
			if(bfirst)
				sRv+= value;
			if(!token.hasMoreTokens())
				break;
			sRv+= token.nextToken();
			if(!bfirst)
				sRv+= value;
		}
		if(token.hasMoreTokens())
			sRv+= token.nextToken();
		while(token.hasMoreTokens())
		{
			sRv+= "@";
			sRv+= token.nextToken();
		}
		return sRv;
	}
	
	/**
	 * translate an error given from server into an message
	 * 
	 * @param codeStr error from server (ERROR &lt;number&gt;)
	 * @param folder folder setting in server
	 * @param subroutine subroutines delimited with colons
	 * @return translated string
	 * @author Alexander Kolli
	 * @version 1.00.00, 30.11.2007
	 * @since JDK 1.6
	 */
	public String translatePortServerError(String codeStr, String folder, String subroutine)
	{
		String sRv;
		String codeNr;
		RE error= new RE("^ERROR ([0-9]+)( [0-9]+)*");
		
		if(folder == null)
			folder= "[UNKNOWN]";
		if(subroutine == null)
			subroutine= "[UNKNOWN]";
		
		if(!error.match(codeStr))
		{
			return translate("UNKNOWNERROR");
		}
		codeNr= error.getParen(1);
		switch(Integer.parseInt(codeNr))
		{
		case 3:
			sRv= translate("PORTSERVERERROR" + codeNr, error.getParen(2));
			break;
		case 4:
			sRv= translate("PORTSERVERERROR" + codeNr, folder);
			break;
		case 5:
			sRv= translate("PORTSERVERERROR" + codeNr, subroutine, folder);
			break;
		case 6:
			sRv= translate("PORTSERVERERROR" + codeNr, subroutine);
			break;
		case 11:
		case 12:
		case 15:
			sRv= translate("PORTSERVERERROR" + codeNr, folder);
			break;
		default:
			sRv= translate("PORTSERVERERROR" + codeNr);		
		}
		return sRv;
	}
}
