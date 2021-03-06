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
import java.util.LinkedList;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.FontData;
import org.eclipse.swt.layout.GridData;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import org.apache.regexp.RE;

import at.kolli.automation.client.MsgTranslator;
import at.kolli.layout.FontObject.colors;

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
	 * whether title will be actually defined
	 */
	private boolean m_bTitleDef= false;
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
	  private ArrayList<IComponentListener> m_aoComponents= new ArrayList<IComponentListener>();
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
	   * all definitions of body tags or td tags from table
	   */
	  private HashMap<String, HtmTags> m_mClasses= new HashMap<String, HtmTags>();

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
		
		//System.out.println("method startElement(" + eName + ")");
		if(	m_bFinishedLayout
			||
			(	m_bFinishedBody
				&&
				m_bFinishedHead	)	)
		{
			return;
		}

		if(HtmTags.debug)
			echoString( "<" + eName );
		if(m_bTitleDef)
		{
			String content;
			
			content= "<" + eName;
			for( int i=0; i<attrs.getLength(); i++ )
		    {
				String aName = attrs.getLocalName( i ); // Attr name
				String value;
		        
		        if( "".equals( aName ) )  aName = attrs.getQName( i );
				value= " " + aName + "=\"" + attrs.getValue( i ) + "\"";
		        content+= value;
		        if(HtmTags.debug)
		        	echoString(value);
		    }
			content+=  ">";
			if(HtmTags.debug)
				echoString(">");
			((Title)m_oAktTag).name+= content;
			return;
		}
		createTextBuffer();
		echoTextBuffer();
	
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
					m_bTitleDef= true;
				}
			}else if(	m_bFinishedBody == false
						&&
						eName.equals("body")	)
			{
				tag= new Body();
				m_oAktTag.insert(tag);
				m_oAktTag= tag;
				m_bBody= true;
			}else
			{
				if(HtmTags.debug)
					echoString("\nfind unknown tag <" + eName + ">\n");
			}
		}else
		{
		    if(	eName.equals("table") ||
		    	eName.equals("fonttable")	)
		    {
		    	tag= new Table();
		    	m_oAktTag.insert(tag);
		    	m_oAktTag= tag;
		    	if(eName.equals("fonttable"))
		    		createFontTable();
		    	
		    }else if(eName.equals("tr"))
		    {
		    	Table table= (Table)m_oAktTag;
		    	
		    	tr= table.nextLine();
		    	
		    }else if(eName.equals("td"))
		    {
		    	td= new ContentFields();
		    	m_oAktTag.insert(td);
		    	
		    }else if(eName.equals("input"))
		    {
		    	Component component= new Component();
		    	
		    	tag= component;
		    	m_aoComponents.add(component);
		    	m_oAktTag.insert(tag);
		    	m_oAktTag= tag;
		    	
		    }else if(eName.equals("select"))
		    {
		    	Component component= new Component();
		    	
		    	component.type= "combo";
		    	tag= component;
		    	m_aoComponents.add(component);
		    	m_oAktTag.insert(tag);
		    	m_oAktTag= tag;
		    	
		    }else if(	m_oAktTag instanceof Component &&
		    			eName.equals("option")				)
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
		    }else if(eName.equals("hr"))
		    {
		    	Label hr= new Label();
		    	
		    	hr.separator= SWT.SEPARATOR | SWT.HORIZONTAL;
		    	m_oAktTag.insert(hr);
		    	m_oAktTag= hr;
		    	
		    }else if(eName.equals("font"))
		    {
		    	Style style= new Style();
		    	
		    	m_oAktTag.insert(style);
		    	m_oAktTag= style;
		    	
		    }else if(eName.equals("b"))
		    {
		    	Style style= new Style();
		    	
		    	style.bold= true;
		    	m_oAktTag.insert(style);
		    	m_oAktTag= style;
		    	
		    }else if(eName.equals("i"))
		    {
		    	Style style= new Style();
		    	
		    	style.italic= true;
		    	m_oAktTag.insert(style);
		    	m_oAktTag= style;
		    	
		    }else if(eName.equals("u"))
		    {
		    	Style style= new Style();
		    	
		    	style.underline= true;
		    	m_oAktTag.insert(style);
		    	m_oAktTag= style;
		    	
		    }else if(eName.equals("fieldset"))
		    {
		    	FieldSet fieldset= new FieldSet();
		    	
		    	fieldset.cellpadding= 0;
		    	m_oAktTag.insert(fieldset);
		    	m_oAktTag= fieldset;
		    	
		    }else if(eName.equals("legend"))
		    {
		    	Legend legend;
	    		
	    		if(!m_oAktTag.tagName.equals("fieldset"))
	    			throw new SAXException("tag of <legend> can be defined only after <fieldset>");
	    		legend= new Legend();
	    		m_oAktTag.insert(legend);
	    		m_oAktTag= legend;
		    	
		    }else
		    {
		    	System.out.println("");
		    	System.out.println("wrong tag <" + eName + "> be set");
		    	throw new SAXException( "wrong tag <" + eName + "> be set");
		    }
		    
			if(m_aoPermission.size() == 0)
				m_oAktTag.setPermission(permission.writable);
			else
				m_oAktTag.setPermission(m_aoPermission.get(0));
			
		}
		
	    if( attrs != null )
	    {
	    	String metaName= "";
	    	String attribval;
	    	
	      for( int i=0; i<attrs.getLength(); i++ )
	      {
	        String aName = attrs.getLocalName( i ); // Attr name
	        
	        if( "".equals( aName ) )
	        	aName = attrs.getQName( i );
	        aName= aName.toLowerCase();
	        attribval= attrs.getValue(i);
	        if(HtmTags.debug)
	        	echoString( " " + aName + "=\"" + attribval + "\"" );
	        if(	m_bHead == true
				&&
				m_bFinishedHead == false	)
	        {
	        	if(eName.equals("meta"))
	        	{
	        		if(aName.equals("name"))
	        			metaName= attribval;
	        		else if(aName.equals("content"))
	        			m_mMetaBox.put(metaName, attribval);
	        	}
	        }else if(m_oAktTag instanceof Body)
	        {
	        	if(aName.equals("href"))
	        	{
	        		((Body)m_oAktTag).href= attrs.getValue(i);
	        		m_aoComponents.add((Body)m_oAktTag);
	        	}else if(aName.equals("class"))
	        		m_mClasses.put(attrs.getValue(i), m_oAktTag);
	        	else if(aName.equals("bgcolor"))
	        		((Body)m_oAktTag).bgcolor= attrs.getValue(i);
	        	else
	        	{
					if(HtmTags.debug)
						echoString("\nfind unknown attribute " + aName + " in tag <" + eName + ">\n");
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
	        	}else
				{
					if(HtmTags.debug)
						echoString("\nfind unknown attribute " + aName + " in tag <" + eName + ">\n");
				}
	        	
	    	}else if(m_oAktTag instanceof Label)
	    	{
	    		if(aName.equals("align"))
	    		{
	    			String typ= attribval;
	    			
	    			if(typ.equals("left"))
	    				((Label)m_oAktTag).separator|= SWT.LEFT;
	    			else if(typ.equals("center"))
	    				((Label)m_oAktTag).separator|= SWT.CENTER;
	    			else if(typ.equals("right"))
	    				((Label)m_oAktTag).separator|= SWT.RIGHT;
	    			else if(HtmTags.debug)
						echoString("\nfind unknown content '" + typ + "' of attribute " + aName + " in tag <" + eName + ">\n");
	    			
	    		}else if(aName.equals("width"))
	    		{
	    			Integer value;

	        		try{
	        			value= Integer.valueOf(attrs.getValue(i));
	        		}catch(NumberFormatException ex)
	        		{
	        			value= new Integer(-1);
	        		}
	        		((Label)m_oAktTag).width= value;
	        		
	    		}else if(HtmTags.debug)
					echoString("\nfind unknown attribute " + aName + " in tag <" + eName + ">\n");
	    		
	    	}else if(m_oAktTag instanceof Component)
	        {
	        	Component component= (Component)m_oAktTag;
	        	
	        	if(aName.equals("type"))
	        	{
	        		String type= attribval;
	        		
	        		if(type.equals("spinner"))
	        		{
	        			component.arrowkey= 1;
	        			
	        		}
	        		if(	!type.equals("button") &&
	        			!type.equals("leftbutton") &&
	        			!type.equals("rightbutton") &&
	        			!type.equals("upbutton") &&
	        			!type.equals("downbutton") &&
	        			!type.equals("togglebutton") &&
	        			!type.equals("checkbox") &&
	        			!type.equals("radio") &&
	        			!type.equals("text") &&
	        			!type.equals("label") &&
	        			!type.equals("spinner") &&
	        			!type.equals("slider") &&
	        			!type.equals("range") &&
	        			!type.equals("hidden")			)
	        		{
	        			type= "text";
	        		}
	        		component.type= type;
	        	}else if(aName.equals("name"))
	        		component.name= attrs.getValue(i);
	        	else if(aName.equals("value"))
	        	{
	        		if(!component.value.equals(""))
	        			component.value+= " ";
	        		component.value+= attrs.getValue(i);
	        		
	        	}else if(aName.equals("format"))
	        	{
	        		if(!component.value.equals(""))
	        			component.value= attrs.getValue(i) + " " + component.value;
	        		else
	        			component.value= attrs.getValue(i);
	        		
	        	}else if(aName.equals("width"))
	        		component.width= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("height"))
	        		component.height= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("size"))
	        		component.size= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("disabled"))
	        		component.setLayoutPermission(Component.layout.disabled);
	        	else if(aName.equals("readonly"))
	        		component.setLayoutPermission(Component.layout.readonly);
	        	else if(aName.equals("result"))
	        		component.result= attrs.getValue(i);
	        	else if(aName.equals("min"))
	        	{
	        		if(	component.type.equals("slider") ||
	        			component.type.equals("range")		)
	        		{
	        			try{
	        				component.min= Integer.parseInt(attrs.getValue(i));
	        				
	        			}catch(NumberFormatException ex)
	        			{
	        				component.m_smin= attrs.getValue(i);
	        			}
	        		}else
	        			component.min= Integer.parseInt(attrs.getValue(i));
	        		
	        	}else if(aName.equals("max"))
	        	{
	        		if(	component.type.equals("slider") ||
	        			component.type.equals("range")		)
	        		{
	        			try{
	        				component.max= Integer.parseInt(attrs.getValue(i));
	        				
	        			}catch(NumberFormatException ex)
	        			{
	        				component.m_smax= attrs.getValue(i);
	        			}
	        		}else
	        			component.max= Integer.parseInt(attrs.getValue(i));
	        		
	        	}else if(aName.equals("arrow"))
	        		component.arrowkey= Integer.parseInt(attrs.getValue(i));
	        	else if(aName.equals("step"))
	        		component.rollbarfield= Integer.parseInt(attrs.getValue(i));
	        	else
				{
					if(HtmTags.debug)
						echoString("\nfind unknown attribute " + aName + " in tag <" + eName + ">\n");
				}
	        	
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
		    		
		    	}else if(aName.equals("bgcolor"))
		    	{
		    		if(	td == null &&
		    			tr == null		)
		    		{
		    			((Table)m_oAktTag).bgcolor= attrs.getValue(i);
		    			
		    		}else if(	tr != null &&
		    					td == null		)
		    		{
		    			tr.bgcolor= attrs.getValue(i);
		    			
		    		}else if(td != null)
		    			td.bgcolor= attrs.getValue(i);
		    		
		    	}else if(aName.equals("align"))
		    	{
		    		int type= GridData.BEGINNING;
		    		String value= attribval;
		    		
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
		    		String value= attribval;
		    		
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
		        	
		        }else if(aName.equals("permission"))
		        {
		        	if(td != null)
		        		td.permgroup= attrs.getValue(i);
		        	else if(tr != null)
		        		tr.permgroup= attrs.getValue(i);
		        	else
		        		table.permgroup= attrs.getValue(i);		        	
		        }else if(	aName.equals("href") &&
		        			td != null				)
	        	{
		        	td.href= attrs.getValue(i);
		        	m_aoComponents.add(td);
		        	
	        	}else if(	aName.equals("class") &&
	        				td != null				)
	        	{
	        		m_mClasses.put(attrs.getValue(i), td);
	        	}else
				{
					if(HtmTags.debug)
						echoString("\nfind unknown attribute " + aName + " inside tag <table>\n");
				}		        	
	        }else if(m_oAktTag instanceof Style)
	        {
	        	if(aName.equals("face"))
	        		((Style)m_oAktTag).font= attrs.getValue(i);
	        	else if(aName.equals("size"))
	        	{
	        		((Style)m_oAktTag).size= Integer.parseInt(attrs.getValue(i));
	        		
	        	}else if(aName.equals("color"))
	        		((Style)m_oAktTag).color= attrs.getValue(i);
	        	else if(aName.equals("type"))
	        	{
	        		colors colorID;
	        		String color= attrs.getValue(i);
	        			        		
					try{
						
						colorID= colors.valueOf(color);
		        		((Style)m_oAktTag).colortype= colorID;
						
					}catch(IllegalArgumentException ex)
					{
						MsgTranslator.instance().errorPool("FAULT_font_color_type", color, getActFolder());
					}
	        		
	        	}else if(aName.equals("style"))
	        	{
	        		String text;	        		

	        		((Style)m_oAktTag).bold= false;
	        		((Style)m_oAktTag).italic= false;
	        		text= attrs.getValue(i).toLowerCase();
	        		if(text.contains("bold"))
	        			((Style)m_oAktTag).bold= true;
	        		if(text.contains("italic"))
	        			((Style)m_oAktTag).italic= true;
	        	}
	        }
	      }
	    }
	    if(HtmTags.debug)
	    	echoString( ">" );
	}
	
	/**
	 * create table with all font's on system
	 * 
	 * @throws SAXExecption for wrong tag handling
	 * @author Alexander Kolli
	 * @version 0.02.00, 06.03.2012
	 * @since JDK 1.6
	 */
	private void createFontTable() throws SAXException
	{
		HtmTags parentTag;
		HtmTags curTag, tdTag;
		String sysFont;
		FontData[] fontDatas;
		LinkedList<FontData[]> otherFonts;
		
		fontDatas= FontObject.getSystemFont();
		sysFont= fontDatas[0].name;
		((Table)m_oAktTag).nextLine();
		parentTag= m_oAktTag;
			curTag= new ContentFields();
			((ContentFields)curTag).bgcolor= "white";
			((ContentFields)curTag).align= GridData.CENTER;
			parentTag.insert(curTag);
			parentTag= tdTag= curTag;
			
				curTag= new Style();
				((Style)curTag).color= "black";
				((Style)curTag).font= sysFont;
				((Style)curTag).italic= true;
				parentTag.insert(curTag);
				parentTag= curTag;
					curTag= new Label();
					((Label)curTag).setText(sysFont);
					parentTag.insert(curTag);
				parentTag= tdTag;
				parentTag.insert(new Break());
				
				curTag= new Style();
				((Style)curTag).color= "black";
				((Style)curTag).font= sysFont;
				((Style)curTag).bold= true;
				((Style)curTag).size= 20;
				parentTag.insert(curTag);
				parentTag= curTag;
					curTag= new Label();
					((Label)curTag).setText("defined Font - " + sysFont);
					parentTag.insert(curTag);
		
		otherFonts= FontObject.getotherFonts();
		if(otherFonts.size() > 0)
		{
			for (FontData[] fontData : otherFonts)
			{
				((Table)m_oAktTag).nextLine();
				parentTag= m_oAktTag;
					curTag= new ContentFields();
					((ContentFields)curTag).bgcolor= "white";
					((ContentFields)curTag).align= GridData.CENTER;
					parentTag.insert(curTag);
					parentTag= tdTag= curTag;
					
						curTag= new Style();
						((Style)curTag).color= "black";
						((Style)curTag).italic= true;
						parentTag.insert(curTag);
						parentTag= curTag;
							curTag= new Label();
							((Label)curTag).setText(fontData[0].name);
							parentTag.insert(curTag);
						parentTag= tdTag;
						parentTag.insert(new Break());
						
						curTag= new Style();
						((Style)curTag).color= "black";
						((Style)curTag).font= fontData[0].name;
						((Style)curTag).bold= true;
						((Style)curTag).size= 20;
						parentTag.insert(curTag);
						parentTag= curTag;
							curTag= new Label();
							((Label)curTag).setText("defined Font - " + fontData[0].name);
							parentTag.insert(curTag);
						parentTag= tdTag;
						parentTag.insert(new Break());				
			}
		}
	}
	/**
	 * Return HashMap with all defined tags
	 * which has an class definition
	 * 
	 * @return class names with tags
	 * @author Alexander Kolli
	 * @version 0.02.00, 01.02.2012
	 * @since JDK 1.6
	 */
	public HashMap<String, HtmTags> getClassDefinitions()
	{
		return m_mClasses;
	}

	
	/**
	 * Receive notification of the end of an element.
	 * 
	 * @param namespaceURI The Namespace URI, or the empty string if the element has no Namespace URI or if Namespace processing is not being performed.
	 * @param localName The local name (without prefix), or the empty string if Namespace processing is not being performed.
	 * @param qName The qualified name (with prefix), or the empty string if qualified names are not available.
	 * @throws SAXException
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
	  //System.out.println("method endElement(" + eName + ")");
		
	  if(m_bFinishedLayout)
		  return;
	  createTextBuffer();
	  echoTextBuffer();
    
	  if(HtmTags.debug)
	    	echoString( "</" + eName + ">" ); // element name
	  //if(eName.equals("td") ||
	//		  eName.equals("tr"))
	//	  System.out.println();
		if(	m_bTitleDef &&
			!eName.equals("title")	)
		{
			((Title)m_oAktTag).name+= "</" + eName + ">";
			return;
			
		}
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
		
		if(	eName.equals("layout") ||
			eName.equals("head") ||
			eName.equals("title") ||
			eName.equals("body") ||
			eName.equals("table") ||
			eName.equals("fonttable") ||
			eName.equals("input") ||
			eName.equals("select") ||
			eName.equals("option") ||
			eName.equals("font") ||
			eName.equals("b") ||
			eName.equals("i") ||
			eName.equals("u") ||
			eName.equals("br") ||
			eName.equals("hr") ||
			eName.equals("fieldset") ||
			eName.equals("legend")		)
	    {    		
	    	if(	!m_oAktTag.tagName.equals(eName) &&
	    		!m_oAktTag.tagName.equals("fonttable") &&
	    		eName.equals("legend")	&&
	    		(	eName.equals("select") &&
	    			!m_oAktTag.tagName.equals("input")	)	)
	    	{
	    		if(HtmTags.debug)
	    			System.out.println();
	    		System.out.println("\ncan not display correct folder");
	    		System.out.println("ERROR after end-tag </ " + eName + ">");
	    		throw new SAXException( "source layout error after end-tag " + eName);
	    	}
	    	if(eName.equals("head"))
	    		m_bFinishedHead= true;
	    	else if(eName.equals("title"))
	    		m_bTitleDef= false;
	    	else if(eName.equals("body"))
	    		m_bFinishedBody= true;
	    	else if(eName.equals("layout"))
	    		m_bFinishedLayout= true;
	    	m_oAktTag= m_oAktTag.getParentTag();
	    	
	    }else if(	m_oAktTag instanceof Table &&
	    			(	eName.equals("tr") ||
	    				eName.equals("td")		)	)
	    {
	    	((Table)m_oAktTag).tagEnd(eName);
	    }
		if(HtmTags.debug)
			System.out.println();
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
	  String before;
	  String s = new String( buf, offset, len );

	s= s.replaceAll("\n", " ");
	s= s.replaceAll("\t", " ");
    
    do{
    	before= s;
    	s= before.replaceAll("  ", " ");
    }while(before != s);

    if(HtmTags.debug)
    	echoString(s);
    if(m_bTitleDef)
    	((Title)m_oAktTag).name+= s;
    else
    {
	    if( textBuffer == null )
	      textBuffer = new StringBuffer( s );
	    else
	      textBuffer.append( s );
    }
    	
  }

  
  /**
   * Display text in label composite, accumulated in the character buffer
   * 
   * @throws SAXExecption for wrong tag handling
   * @author Alexander Kolli
   * @version 1.00.00, 04.12.2007
   * @since JDK 1.6
   */
  private void echoTextBuffer() throws SAXException
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
        out = new OutputStreamWriter( System.out, "UTF-8" );
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
  public ArrayList<IComponentListener> getComponents()
  {
	  return m_aoComponents;
  }
}
