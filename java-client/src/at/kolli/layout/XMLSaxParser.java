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
package at.kolli.layout;

import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.HashMap;

import org.eclipse.swt.layout.GridData;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import at.kolli.automation.client.NoStopClientConnector;

import org.apache.regexp.RE;

/**
 * XML parser class
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 04.12.2007
 * @since JDK 1.6
 */
public class XMLSaxParser extends DefaultHandler 
{
	/**
	 * whether the first tag &lt;layout&gt; is be set
	 * and the description beginning
	 */
	private boolean m_bLayout= false;
	/**
	 * whether the layout description is finished
	 */
	private boolean m_bFinishedLayout= false;
	/**
	 * whether the head tag is reading
	 */
	private boolean m_bHead= false;
	/**
	 * whether the reading of head is finished
	 */
	private boolean m_bFinishedHead= false;
	/**
	 * whether the body tag is reading
	 */
	private boolean m_bBody= false;
	/**
	 * whether the reading of body is finished
	 */
	private boolean m_bFinishedBody= false;
	/**
	 * line separator from OS for debugging edition
	 */
	static final String sNEWLINE= System.getProperty( "line.separator" );
	/**
	 * writer instance for debugging output
	 */
	static private Writer out= null;
	/**
	 * buffer to hold the strings between the tags
	 */
	private StringBuffer textBuffer = null;
	  /**
	   * calibrated text between tags from textBuffer
	   */
	  private StringBuffer m_sBuffer= null;
	/**
	 * the main &lt;layout&gt;-tag
	 */
	  private HtmTags m_oMain= null;	
	  /**
	   * actual tag which is be reading
	   */
	  private HtmTags m_oAktTag= null;
	  /**
	   * lightweight regular expression emulator for trim text between tags
	   */
	  private RE m_oRELabel= new RE("^[ \t\n]*(.*)[ \t\n]*$");
	  /**
	   * folder name for setting in all components
	   */
	  private String m_sFolder;
	  /**
	   * all components for faster access
	   */
	  private ArrayList<Component> m_aoComponents= new ArrayList<Component>();
	  /**
	   * ArrayList with permission<br>
	   * used as last in first out (LIFO) Collection
	   * if first permission only for reading, after them no permission
	   */
	  private ArrayList<permission> m_aoPermission= new ArrayList<permission>();
	  /**
	   * ArrayList with losted permission of components equalent to m_aoPermission
	   */
	  private ArrayList<HtmTags> m_aoPermissionTag= new ArrayList<HtmTags>();
	  /**
	   * comprised all meta data from header
	   */
	  private HashMap<String, String> m_mMetaBox= new HashMap<String, String>();

	  /**
	   * constructor to create instance of XMLSaxParser
	   * 
	   * @param folder folder name of current layout file
	   * 				which should be read
	   * @author Alexander Kolli
	   * @version 1.00.00, 04.12.2007
	   * @since JDK 1.6
	   */
 	  public XMLSaxParser(String folder)
	  {
		  m_sFolder= folder;
	  }
	  
 	  /**
 	   * method returning the folder for which the XMLSaxParser in use
 	   * 
 	   * @return folder name
 	   * @serial
 	   * @see
 	   * @author Alexander Kolli
 	   * @version 1.00.00, 09.12.2007
 	   * @since JDK 1.6
 	   */
 	  public String getActFolder()
 	  {
 		  return m_sFolder;
 	  }
 	  /**
 	   * returning the layout-tag which inherit all other
 	   * 
 	   * @return layout tag
 	   * @author Alexander Kolli
 	   * @version 1.00.00, 04.12.2007
 	   * @since JDK 1.6
 	   */
	  public HtmTags getMainTag()
	  {
		  return m_oMain;
	  }
  
	  /**
	   * end of document, only to display an newline on debugging session
	   * 
	   * @throws SAXException
	   * @override
	   * @author Alexander Kolli
	   * @version 1.00.00, 04.12.2007
	   * @since JDK 1.6
	   */
	  public void endDocument() throws SAXException
	  {
		  if(HtmTags.debug)
			  echoString( sNEWLINE );
	  }

	  /**
	   * return all meta data from the header
	   * 
	   * @return all meta data
	   */
	  public HashMap<String, String> getMetaBlock()
	  {
		  return m_mMetaBox;
	  }
	  
	  /**
	   * reading element with attributes in layout file
	   * 
	   * @param namespaceURI The Namespace URI, or the empty string if the element has no Namespace URI or if Namespace processing is not being performed.
	   * @param localName The local name (without prefix), or the empty string if Namespace processing is not being performed.
	   * @param qName The qualified name (with prefix), or the empty string if qualified names are not available.
	   * @param attrs The attributes attached to the element. If there are no attributes, it shall be an empty Attributes
	   * @throws SAXException
	   * @override
	   * @author Alexander Kolli
	   * @version 1.00.00, 04.12.2007
	   * @since JDK 1.6
	   */
	public void startElement(	String namespaceURI,
								String localName,   // local name
	                            String qName,       // qualified name
	                            Attributes attrs ) throws SAXException
	{                      
		HtmTags tag= null;
		ContentFields td= null;
		ContentRows tr= null;
		String eName = ( "".equals( localName ) ) ? qName : localName;
		
		if(	m_bFinishedLayout
			||
			(	m_bFinishedBody
				&&
				m_bFinishedHead	)	)
		{
			return;
		}
		createTextBuffer();
		//if(!eName.equals("br"))
			echoTextBuffer();
	
		if(HtmTags.debug)
			echoString( "<" + eName );
		if(m_bBody == false)
		{
			if(	m_bLayout == false
				&&
				eName.equals("layout")	)
			{
				m_oMain= new Layout();
				m_oAktTag= m_oMain;
				m_bLayout= true;
			}else if(	m_bLayout == true
						&&
						m_bHead == false
						&&
						eName.equals("head")
						&&
						m_bFinishedHead == false	)
			{
				tag= new Head();
				m_oAktTag.insert(tag);
				m_oAktTag= tag;
				m_bHead= true;
				
			}else if(	m_bHead == true
						&&
						m_bFinishedHead == false	)
			{
				if(eName.equals("title"))
				{
					tag= new Title();
					m_oAktTag.insert(tag);
					m_oAktTag= tag;
				}
			}else if(	m_bFinishedBody == false
						&&
						eName.equals("body")	)
			{
				tag= new Body();
				m_oAktTag.insert(tag);
				m_oAktTag= tag;
				m_bBody= true;
			}
		}else
		{
		    if(eName.equals("table"))
		    {
		    	tag= new Table();
		    	m_oAktTag.insert(tag);
		    	m_oAktTag= tag;
		    	
		    }else if(eName.equals("tr"))
		    {
		    	Table table= (Table)m_oAktTag;
		    	
		    	table.nextLine();
		    	
		    }else if(eName.equals("td"))
		    {
		    	td= new ContentFields();
		    	m_oAktTag.insert(td);
		    	
		    }else if(eName.equals("component"))
		    {
		    	Component component= new Component();
		    	
		    	tag= component;
		    	m_aoComponents.add(component);
		    	m_oAktTag.insert(tag);
		    	m_oAktTag= tag;
		    	
		    }else if(eName.equals("option"))
		    {
		    	tag= new Option();
		    	m_oAktTag.insert(tag);
		    	m_oAktTag= tag;
		    	
		    }else if(eName.equals("br"))
		    {
		    	tag= new Break();
		    	m_oAktTag.insert(tag);
		    	m_oAktTag= tag;
		    	//if(m_sBuffer != null)
		    	//	m_sBuffer.append("\n");
		    }
		    
			if(m_aoPermission.size() == 0)
				m_oAktTag.actPermission= permission.writeable;
			else
				m_oAktTag.actPermission= m_aoPermission.get(0);
			
		}
		
	    if( attrs != null )
	    {
	    	String metaName= "";
	    	
	      for( int i=0; i<attrs.getLength(); i++ )
	      {
	        String aName = attrs.getLocalName( i ); // Attr name
	        
	        if( "".equals( aName ) )  aName = attrs.getQName( i );
	        if(HtmTags.debug)
	        	echoString( " " + aName + "=\"" + attrs.getValue( i ) + "\"" );
	        if(	m_bHead == true
				&&
				m_bFinishedHead == false	)
	        {
	        	if(m_oAktTag instanceof Title)
		        {
		        	Title title= (Title)m_oAktTag;
		        	
		        	if(aName.equals("name"))
		        		title.name= attrs.getValue(i);
		        	
		        }else if(eName.equals("meta"))
	        	{
	        		if(aName.equals("name"))
	        			metaName= attrs.getValue(i);
	        		else if(aName.equals("content"))
	        			m_mMetaBox.put(metaName, attrs.getValue(i));
	        	}
	        }else if(m_oAktTag instanceof Option)
	    	{
	        	Option option= (Option)m_oAktTag;
	        	
	        	if(aName.equals("value"))
	        	{
	        		Double value;
	        		
	        		try{
	        			value= Double.valueOf(attrs.getValue(i));
	        		}catch(NumberFormatException ex)
	        		{
	        			value= new Double(0);
	        		}
	        		//doubleValue= value;
	        		option.value= value;//Double.valueOf(value);
	        	}
	        	
	    	}else if(m_oAktTag instanceof Component)
	        {
	        	Component component= (Component)m_oAktTag;
	        	
	        	if(aName.equals("type"))
	        	{
	        		String type= attrs.getValue(i);
	        		
	        		if(type.equals("spinner"))
	        		{
	        			component.arrowkey= 1;
	        			
	        		}   			
	        		component.type= type;
	        	}else if(aName.equals("name"))
	        		component.name= attrs.getValue(i);
	        	else if(aName.equals("value"))
	        	{
	        		component.value= attrs.getValue(i);
	        		
	        	}else if(aName.equals("width"))
	        		component.width= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("height"))
	        		component.height= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("size"))
	        		component.size= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("disabled"))
	        	{
	        		component.actLayout= Component.layout.disabled;
	        		component.normal= Component.layout.disabled;
	        		
	        	}else if(aName.equals("readonly"))
	        	{
	        		component.actLayout= Component.layout.readonly;
	        		component.normal= Component.layout.readonly;
	        		
	        	}else if(aName.equals("result"))
	        		component.result= attrs.getValue(i);
	        	else if(aName.equals("min"))
	        		component.min= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("max"))
	        		component.max= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("arrow"))
	        		component.arrowkey= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("scroll"))
	        		component.rollbarfield= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("format"))
	        		component.format= attrs.getValue(i);
	        	
	        }else if(m_oAktTag instanceof Table)
	        {
	        	Table table= (Table)m_oAktTag;
	        	
	        	if(aName.equals("border"))
	        		table.setBorder(Integer.parseInt(attrs.getValue(i)));
	        	else if(aName.equals("cellpadding"))
	        		table.cellpadding= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("cellspacing"))
	        		table.cellspacing= Integer.parseInt(attrs.getValue(i));
	        	else if(	aName.equals("rowspan")
	        				&&
	        				td != null				)
	        	{
	        		td.rowspan= Integer.parseInt(attrs.getValue(i));
	        	}else if(	aName.equals("colspan")
		    				&&
		    				td != null				)
		    	{
		    		td.colspan= Integer.parseInt(attrs.getValue(i));
		    	}else if(aName.equals("align"))
		    	{
		    		int type= GridData.BEGINNING;
		    		String value= attrs.getValue(i);
		    		
		    		if(value.equals("left"))
		    			type= GridData.BEGINNING;
		    		else if(value.equals("center"))
		    			type= GridData.CENTER;
		    		else if(value.equals("right"))
		    			type= GridData.END;
		    		
		    		if(td != null)
		    			td.align= type;
		    		else if(tr != null)
		    			tr.align= type;
		    		else
		    			table.align= type;
		    	}else if(aName.equals("valign"))
		    	{
		    		int type= GridData.BEGINNING;
		    		String value= attrs.getValue(i);
		    		
		    		if(value.equals("top"))
		    			type= GridData.BEGINNING;
		    		else if(value.equals("middle"))
		    			type= GridData.CENTER;
		    		else if(value.equals("bottom"))
		    			type= GridData.END;
		    		
		    		if(td != null)
		    			td.valign= type;
		    		else if(tr != null)
		    			tr.valign= type;
		    		else
		    			table.valign= type;
		    	}else if(aName.equals("width"))
		        {
		    		int width= Integer.parseInt(attrs.getValue(i));
		    		
		        	if(td != null)
		    			td.width= width;
		    		else if(tr != null)
		    			tr.width= width;
		    		else
		    			table.width= width;
		        	
		        }else if(aName.equals("height"))
		        {
		    		int height= Integer.parseInt(attrs.getValue(i));
		    		
		        	if(td != null)
		    			td.height= height;
		    		else if(tr != null)
		    			tr.height= height;
		    		else
		    			table.height= height;
		        	
		        }
	        }
	        if(	(	m_oAktTag instanceof Table
	        		||
	        		m_oAktTag instanceof ContentRows
	        		||
	        		m_oAktTag instanceof ContentFields	)
	        	&&
	        	aName.equals("permission")
    			&&
    			m_oAktTag.actPermission.compareTo(permission.None) != 0	)
	        {
	        	NoStopClientConnector client= NoStopClientConnector.instance();
	        	permission perm= client.permission(attrs.getValue(i));
	        	
	        	if(perm.compareTo(m_oAktTag.actPermission) < -1)
	        	{
	        		m_oAktTag.actPermission= perm;
	        		m_aoPermission.add(0, perm);
	        		m_aoPermissionTag.add(0, m_oAktTag);
	        	}
	        }
	      }
	    }
	    if(HtmTags.debug)
	    	echoString( ">" );
	    td= null;
	    tr= null;
	}

	
	/**
	 * Receive notification of the end of an element.
	 * 
	 * @param namespaceURI The Namespace URI, or the empty string if the element has no Namespace URI or if Namespace processing is not being performed.
	 * @param localName The local name (without prefix), or the empty string if Namespace processing is not being performed.
	 * @param qName The qualified name (with prefix), or the empty string if qualified names are not available.
	 * @throws SAXException
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
  public void endElement( String namespaceURI,
                          String localName,     // local name
                          String qName )        // qualified name
  throws SAXException
  {
	  String eName = ( "".equals( localName ) ) ? qName : localName;
	  
	  if(m_bFinishedLayout)
		  return;
	  createTextBuffer();
	  //if(!eName.equals("br"))
		  echoTextBuffer();
    
	  if(HtmTags.debug)
	    	echoString( "</" + eName + ">" );           // element name
    if(m_oAktTag != null)
    {
    	if(	m_aoPermissionTag.size() != 0
    		&&
    		!eName.equals("tr")
    		&&
    		!eName.equals("td")
    		&&
    		m_aoPermissionTag.get(0) == m_oAktTag	)
    	{
    		m_aoPermission.remove(0);
    		m_aoPermissionTag.remove(0);
    	}
    	
    	if(	eName.equals("layout")
    		||
    		eName.equals("head")
    		||
    		eName.equals("title")
    		||
    		eName.equals("body")
    		||
    		eName.equals("table")
    		||
    		eName.equals("component")
    		||
    		eName.equals("option")
    		||
    		eName.equals("br")			)
	    {
	    	if(!m_oAktTag.tagName.equals(eName))
	    	{
	    		if(HtmTags.debug)
	    			System.out.println();
	    		System.out.println("can not display correct folder");
	    		System.out.println("ERROR after end-tag </ " + eName + ">");
	    		throw new SAXException( "source layout error after end-tag " + eName);
	    	}
	    	if(eName.equals("head"))
	    		m_bFinishedHead= true;
	    	else if(eName.equals("body"))
	    		m_bFinishedBody= true;
	    	else if(eName.equals("layout"))
	    		m_bFinishedLayout= true;
	    	else if(m_bBody
	    			&&
	    			eName.equals("component")	)
	    	{
	    		((Component)m_oAktTag).setPermission();
	    	}
	    	m_oAktTag= m_oAktTag.getParentTag();
	    }
    }
  }

  /**
   * Receive notification of character data inside an element.
   * 
   * @param buf The characters
   * @param offset The start position in the character array.
   * @param len The number of characters to use from the character array.
   * @throws SAXException
   * @override
   * @author Alexander Kolli
   * @version 1.00.00, 04.12.2007
   * @since JDK 1.6
   */
  public void characters( char[] buf, int offset, int len )
  throws SAXException
  {
    String s = new String( buf, offset, len );
    if( textBuffer == null )
      textBuffer = new StringBuffer( s );
    else
      textBuffer.append( s );
  }

  
  /**
   * Display text in label composite, accumulated in the character buffer
   * 
   * @author Alexander Kolli
   * @version 1.00.00, 04.12.2007
   * @since JDK 1.6
   */
  private void echoTextBuffer()  
  {
	  Label label;
	  
	  if(m_sBuffer == null)
		  return;

	  label= new Label();	  
	  label.setText(m_sBuffer.toString());
	  m_oAktTag.insert(label);
	  m_sBuffer= null;
  }
  
  /**
   * copy accumulated characters from textBuffer calibrated into m_sBuffer
   * 
   * @throws SAXException
   * @author Alexander Kolli
   * @version 1.00.00, 04.12.2007
   * @since JDK 1.6
   */
  private void createTextBuffer() throws SAXException
  {  
	  if( textBuffer == null )
		  return;
	  if(HtmTags.debug)
	    	echoString( textBuffer.toString() );
	  if(m_bBody)
	  {
		  String text;

		  text= (textBuffer.toString());
		  text= text.replace("\n", " ");
		  text= text.replace("\t", " ");
		  if(m_oRELabel.match(text))
		  {
			  int count= m_oRELabel.getParenCount();
			  
			  if(count > 1)
			  {
				  text= m_oRELabel.getParen(1);
				  if(!text.equals(""))
				  {
					  //System.out.println("'"+textBuffer.toString()+"'");
					  if(m_sBuffer == null)
					  {
						  m_sBuffer= new StringBuffer(text);
					  }else
						  m_sBuffer.append(text);
				  }
			  }
		  }
	  }
	  textBuffer = null;
  }

  /**
   * write string in UTF format to shell output, only for debugging session
   * 
   * @param s string to write
   * @throws SAXException
   * @author Alexander Kolli
   * @version 1.00.00, 04.12.2007
   * @since JDK 1.6
   */
  private void echoString( String s )
  throws SAXException
  {
    try {
      if( null == out )
        out = new OutputStreamWriter( System.out, "UTF8" );
      out.write( s );
      out.flush();
    } catch( IOException ex )
    {
    	// Wrap I/O exceptions in SAX exceptions, to
    	// suit handler signature requirements
    	throw new SAXException( "I/O error", ex );
    }
  }
  
  /**
   * Gets all components which be set in the layout file,
   * for an faster access
   * 
   * @return array of components
   * @author Alexander Kolli
   * @version 1.00.00, 04.12.2007
   * @since JDK 1.6
   */
  public ArrayList<Component> getComponents()
  {
	  return m_aoComponents;
  }
}
