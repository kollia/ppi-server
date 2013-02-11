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

import java.awt.AWTException;
import java.awt.GraphicsEnvironment;
import java.awt.Robot;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.eclipse.swt.SWT;
import org.eclipse.swt.browser.LocationEvent;
import org.eclipse.swt.browser.LocationListener;
import org.eclipse.swt.browser.ProgressEvent;
import org.eclipse.swt.browser.ProgressListener;
import org.eclipse.swt.browser.StatusTextEvent;
import org.eclipse.swt.browser.StatusTextListener;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.events.FocusListener;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Scale;
import org.eclipse.swt.widgets.Slider;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;

import org.apache.regexp.RE;

import at.kolli.automation.client.MsgClientConnector;
import at.kolli.automation.client.MsgTranslator;
import at.kolli.automation.client.NodeContainer;
import at.kolli.dialogs.DisplayAdapter;
import at.kolli.layout.IComponentListener;

/**
 * class representing all components which can displayed into the body-tag
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 1.00.00, 08.12.2007
 * @since JDK 1.6
 */
public class Component  extends HtmTags implements IComponentListener 
{
	/**
	 * enum type of layouts<br /
	 * <table>
	 *   <tr>
	 *     <td>
	 *       normal
	 *     </td>
	 *     <td>
	 *       component can be changed from user
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>
	 *       readonly
	 *     </td>
	 *     <td>
	 *       Set the component to read only. Component can not change from user.
	 *       If this attribute is not set, but user have no permission to change,
	 *       the component is also set to read only as default
	 *     </td>
	 *   </tr>
	 *   <tr>
	 *     <td>
	 *       disabled
	 *     </td>
	 *     <td>
	 *       set the component to disabled. Component can not changed from user
	 *       and is grey deposited.
	 *       If this attribute is not set, but user have no permission to read,
	 *       the component is also set to disabled as default
	 *     </td>
	 *   </tr>
	 * </table>
	 */
	public enum layout	{	normal,
							readonly,
							disabled	}
	/**
	 * whether permission right checked from server.<br />
	 * (permission only will be set inside method askPermission)
	 */
	private boolean permissionSet= false;
	/**
	 * layout type from layout file if permission is correctly
	 * and permission on server is not checked
	 */
	private layout normal= layout.normal;
	/**
	 * disabled or read only attribute of component.<br />
	 * is only set when component type is no TEXT
	 */
	private layout actLayout= layout.normal;
	/**
	 * type attribute of component<br />
	 * button, togglebutton, checkbox, radio, text, spinner, slider, scale or combo
	 */
	public String type= "text";
	/**
	 * name attribute of component.<br />
	 * only for type radio, describe the group in which the radio button is
	 */
	public String name= "";
	/**
	 * value attribute of component.<br />
	 * for type radio or checkbox it describe the value which should be set
	 * type button or togglebutton the name inside the button
	 * type text the suffix behind the number
	 */
	public String value= "";
	/**
	 * when component is from type text
	 * variable is an text-box
	 */
	private Text m_oText= null;
	/**
	 * when component is from type label
	 * variable is an normal text without any boy
	 */
	private Label m_oLabel= null;
	/**
	 * whether text widget has the focus.<br />
	 * This will be set to true when mouse listener get up
	 * and to false when focus listener lost the focus
	 */
	private boolean m_bTextFocus= false;
	/**
	 * whether last url in browser was an about:blank
	 */
	private boolean m_bWasBlank= false;
	/**
	 * when component is from type button
	 * variable as an valid button object
	 */
	private Button m_oButton= null;
	/**
	 * width attribute of component.<br />
	 * the width of the components beside the types check and radio
	 * by the types slider or scale it shows an horizontal bar
	 * if the height is not set or lower
	 */
	public int width= -1;
	/**
	 * height attribute of component.<br />
	 * for type slider and scale it be shown an vertical bar
	 * if the width not be set or lower
	 */
	public int height= -1;
	/**
	 * size attribute of component<br />
	 * only for type COMBO
	 * describe how much rows should be displayed
	 * DEFAULT: 1
	 */
	public int size= 1;
	/**
	 * min attribute of component.<br />
	 * minimal-value for component slider and range
	 */
	public int min= 0;
	/**
	 * min attribute of component.<br />
	 * minimal-value for component slider and range
	 */
	public String m_smin= "";
	/**
	 * max attribute of component.<br />
	 * maximal-value for component slider and range
	 */
	public int max= 1500000;
	/**
	 * max attribute of component.<br />
	 * maximal-value for component slider and range
	 */
	public String m_smax= "";
	/**
	 * arrow attribute of component.<br />
	 * value for get higher or lower by pressing the arrow keys
	 * only for component slider
	 */
	public int arrowkey= 1;
	/**
	 * scroll attribute of component.<br />
	 * higher or lower value by clicking into the scroll bar
	 * for components slider and scale
	 */
	public int rollbarfield= 1;
	/**
	 * format attribute of component.<br />
	 * format of displayed number beginning with '#' than a number whitch
	 * representing the digits before decimal point and maby followd with an point
	 * for any decimal places or with an number how much decimal places should be showen
	 */
	public String format= "#1.";
	/**
	 * how much digits after decimal point the component with type SPINNER have
	 */
	public int digits= 0;
	/**
	 * result attribute of component.<br />
	 * describing the position in the measure folders
	 * the subroutines only can be from type SWITCH for boolean or VALUE for numbers
	 * exp. 'folder:subroutine'.
	 */
	public String result= "";
	/**
	 * structure of result string
	 * 
	 * @author Alexander Kolli
	 *
	 */
	private class textFormat
	{
		/**
		 * result attribute for specific result 
		 */
		public String resultStr= "";
		/**
		 * result value for actual result string
		 */
		public Double result= null;
		/**
		 * whether permission right checked from server.<br />
		 * (permission only will be set inside method askPermission)
		 */
		public boolean permissionSet= false;
		/**
		 * permission from server
		 */
		public permission actPermission= permission.None;
		/**
		 * disabled or read only attribute of component.
		 */
		private layout actLayout= layout.normal;
		/**
		 * whether result string is an correct folder:subroutine name
		 */
		public boolean correctResultString= true;
		/**
		 * whether server has correct access to device for this component
		 */
		public boolean deviceAccess= true;
		/**
		 * error code when result string not reachable
		 */
		public String errorCode= "";
		/**
		 * error message when result string not reachable
		 */
		public String errorMessage= "";
		/**
		 * value attribute of component before number.<br />
		 * only for type text
		 */
		public String beforeValue= "";
		/**
		 * digits before decimal point. Calculated from format attribute.
		 */
		public int numBefore= 1;
		/**
		 * digits behind decimal point. Calculated from format attribute.
		 */
		public int numBehind= -1;
		/**
		 * RE pattern in first textFormat entry to read changes from user
		 */
		public RE changePattern= null;
	}
	/**
	 * result attribute of component with type TEXT.<br />
	 * can have more then one results.
	 */
	private ArrayList<textFormat> m_aResult;
	/**
	 * variable is true if the result attribute on server is not reachable,
	 * incorrect or not set.<br />
	 * is only set when component type is no TEXT
	 */
	private boolean m_bCorrectName= true;
	/**
	 * whether server has correct access to device for this component.<br />
	 * is only set when component type is no TEXT
	 */
	private boolean m_bDeviceAccess= true;
	/**
	 * actual value which was read from server
	 */
	private double m_nAktValue= 0;
	/**
	 * whether result of component is an soft button
	 */
	private boolean m_nSoftButton= false;
	/**
	 * which soft button is defined
	 */
	private String m_sSoftButtonName= "";
	/**
	 * when the name of soft button is 'browser_url'
	 * this variable holds the URL for setting 
	 */
	private String m_sSoftButtonUrl= "";
	/**
	 * other tag component combine with soft button
	 */
	private HtmTags m_oSoftButtonTag= null;
	/**
	 * component as SWT widget
	 */
	private Control m_oComponent= null;
	/**
	 * Hash table for all entries inside the component with type COMBO
	 * from option-tags names for values
	 */
	private HashMap<String, Double> m_asComboValueEntrys= new HashMap<String, Double>();
	/**
	 * Hash table for all entries inside the component with type COMBO
	 * from option-tags as values for names
	 */
	private HashMap<Double, String> m_asComboNameEntrys= new HashMap<Double, String>();
	/**
	 * whether class have an listener
	 */
	private boolean haveListener= false;
	/**
	 * Listener {@link SWT}.MouseDown for component button
	 */
	private Listener m_eListener1;
	/**
	 * Listener {@link SWT}.MouseUp for component button
	 */
	private Listener m_eListener2;
	/**
	 * mouse up listener for moving after press
	 */
	private MouseListener m_eMouseUpListener= null;
	/**
	 * SelectionListener for components
	 */
	private SelectionListener m_eSelectionListener= null;
	/**
	 * LocationListener for external browser widget
	 */
	private LocationListener m_eLocationListener= null;
	/**
	 * whether button is active.<br />
	 * This variable will be used for soft buttons back and forward 
	 */
	private boolean m_bActiveButton= false;
	/**
	 * FocusListener for text widget when as soft button defined
	 */
	private FocusListener m_eFocusListener= null;
	/**
	 * MouseListener for text widget when soft button is an browser_url
	 */
	private MouseListener m_eMouseListener= null;
	/**
	 * StatusTextListener for text widget when soft button is an browser_status
	 */
	private StatusTextListener m_eStatusListener= null;
	/**
	 * ProgressListener for button-tag with type checkbox, radio or togglebutton
	 * when soft button is an browser_load
	 */
	private ProgressListener m_eProgressListener= null;
	
	/**
	 * create instance of all components whitch can display in the window
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public Component()
	{
		super("input");
	}
	
	/**
	 * method add entries to the component with type COMBO
	 * 
	 * @param entry name of entry witch will be displayed in the window
	 * @param value value of entry witch should be send to the server
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void addComboEntrys(String entry, double value)
	{
		m_asComboValueEntrys.put(entry, value);
		m_asComboNameEntrys.put(value, entry);
	}

	/**
	 * execute method to create the composite for display
	 * 
	 * @param composite parent composite
	 * @param font object of defined font and colors
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, FontObject font, HashMap<String, HtmTags> classes) throws IOException
	{
		boolean disabled= false;
		boolean readonly= false;
		MsgClientConnector client= MsgClientConnector.instance();
		//Composite widgetCompo= new Composite(composite, SWT.NONE);
		//GridLayout gridLayout= new GridLayout();
		GridData gridData= new GridData();

		gridData.horizontalAlignment= align;
		gridData.verticalAlignment= valign;
		//widgetCompo.setLayoutData(gridData);
		//gridData= new GridData();
		if(HtmTags.debug)
		{
			System.out.println("execute component '" + type + "'");
			if(!value.equals(""))
			{
				System.out.println("    with value '" +value + "'");
				System.out.print("    and ");
			}else
				System.out.print("    with ");
			if(result.equals(""))
				System.out.println("no result");
			else
				System.out.println("result '" + result + "'");
		}
		if(!this.type.equals("text"))
		{// by type TEXT, it will be ask later in the specific if sentence
			if(!isOnlySoftButton(classes))
				askPermission(null);
			if(	actLayout.compareTo(layout.disabled) == 0 ||
				!m_bDeviceAccess								)
			{
				disabled= true;
			}
			if(actLayout.compareTo(layout.readonly) == 0)
				readonly= true;
		}

		if(this.type.equals("hidden"))
		{
			if(!this.result.equals(""))
				m_bCorrectName= true;
			return; // nothing to implement
		}
		
		if(	this.type.equals("checkbox") ||
			this.type.equals("radio") ||
			this.type.equals("button") ||
			this.type.equals("togglebutton") ||
			this.type.equals("upbutton") ||
			this.type.equals("leftbutton") ||
			this.type.equals("rightbutton") ||
			this.type.equals("downbutton")		)
		{
			int type= 0;
			Double akt= null;
			String firstButtonValue= "";
			
			if(	this.value.equals("") &&
				(	this.type.equals("checkbox") ||
					this.type.equals("radio")		)	)
			{// when no value for checkbox or radio button be set
			 // set 1 as default
				this.value= "1";
			}
			if(	!getPermission().equals(permission.None) &&
				!m_nSoftButton								)
			{
				akt= client.getValue(this.result, /*bthrow*/true);
			}
			if(	!client.getErrorCode().equals("ERROR 016") &&
				akt == null &&
				m_nSoftButton == false							)
			{
				if(HtmTags.debug)
				{	
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component " + this.type;
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			
			if(this.type.equals("checkbox"))
			{
				type= SWT.CHECK;
			}else if(this.type.equals("radio"))
			{
				type= SWT.RADIO;
			}else if(this.type.equals("button"))
			{
				type= SWT.PUSH;
			}else if(this.type.equals("togglebutton"))
			{
				type= SWT.TOGGLE;
			}else if(this.type.equals("upbutton"))
			{
				type= SWT.ARROW | SWT.UP;
			}else if(this.type.equals("leftbutton"))
			{
				type= SWT.ARROW | SWT.LEFT;
			}else if(this.type.equals("rightbutton"))
			{
				type= SWT.ARROW | SWT.RIGHT;
			}else if(this.type.equals("downbutton"))
			{
				type= SWT.ARROW | SWT.DOWN;
			}else
			{
				System.out.println("no defined input widget for type "+this.type);
				Composite cp= new Composite(composite, SWT.NONE);
				font.setDevice(cp);
				return;
			}
			if(	(	type == SWT.PUSH ||
					type == SWT.TOGGLE	) &&
				!m_lContent.isEmpty()			)
			{
				double item= -1;
				
				for(HtmTags tag : m_lContent)
				{
					if(tag instanceof Option)
					{
						Option option= (Option)tag;
						String entry= option.getOptionString();
						
						if(option.value != null)
							item= option.value;
						else
							++item;
						addComboEntrys(entry, item);
						if(	(	akt == null &&
								firstButtonValue.equals("")	) ||
							(	akt != null &&
								akt.equals(m_asComboValueEntrys.get(entry))	)
																)
						{
							firstButtonValue= entry;
						}
					}
				}
			}
			
			disabled= disabled || readonly ? true : false;
			
			m_oButton= new Button(composite, type);
			m_oButton.setEnabled(!disabled);
			if(!m_asComboNameEntrys.isEmpty())
			{
				if(width == -1)
				{
					int maxLen= 0;
					
					for (String key : m_asComboValueEntrys.keySet())
					{
						int len= key.length();
						
						if(len > maxLen)
							maxLen= len;
					}
					maxLen-= firstButtonValue.length();
					if(maxLen > 0)
					{
						StringBuffer newValue= new StringBuffer();
						
						for(int i= 0; i < maxLen; ++i)
							newValue.append(" ");
						newValue.append(firstButtonValue);
						for(int i= 0; i < maxLen; ++i)
							newValue.append(" ");
						this.value= newValue.toString();
						
					}else
						this.value= firstButtonValue;
				}else
					this.value= firstButtonValue;
			}
			if(width != -1)
				gridData.widthHint= width;
			if(height != -1)
				gridData.heightHint= height;
			m_oButton.setLayoutData(gridData);
			m_oComponent= m_oButton;
			if(	type == SWT.PUSH ||
				type == SWT.TOGGLE	)
			{
				m_oButton.setText(this.value);
			}
			if(type == SWT.RADIO)
			{
				try{
					if(akt != null && akt == Double.parseDouble(this.value.trim()))
						m_oButton.setSelection(true);
					else
						m_oButton.setSelection(false);
				}catch(NumberFormatException ex)
				{
					m_oButton.setSelection(false);
				}
			}else
			{
				if(akt != null && akt > 0)
					m_oButton.setSelection(true);
				else
					m_oButton.setSelection(false);
			}
			// define font object after setText for PUSH and TOGGLE button
			// because when color changes font object delete text and paint
			// own one
			font.setDevice(m_oButton);
			
		}else if(	this.type.equals("text") ||
					(	this.type.equals("label") &&
						m_lContent.isEmpty()		)	)
		{
			boolean bread= false, bLabel= this.type.equals("label");
			int style;
			RE floatStr= new RE("([\\\\]*)#([0-9]+)(.([0-9]*))?");
			textFormat formatObj;
			String[] spl;
			String sNumberStr= "([^0-9]*)([0-9]*";
			String sBehindStr= "(\\.[0-9]+)?";
			
			m_aResult= new ArrayList<Component.textFormat>();
			spl= this.result.split(",");
			for (String res : spl)
			{
				formatObj= new textFormat();
				formatObj.resultStr= res.trim();
				m_aResult.add(formatObj);
			}
			if(!isOnlySoftButton(classes))
			{
				permission highPerm, perm;
				
				highPerm= permission.None;
				for (textFormat obj : m_aResult)
				{
					perm= askPermission(obj);
					if(highPerm == permission.None)
						highPerm= perm;
					else
					{
						if(	highPerm == permission.readable &&
							perm == permission.writeable		)
						{
							highPerm= permission.writeable;
						}
					}
				}
				setPermission(highPerm, null);
			}
			if(	actLayout.compareTo(layout.disabled) == 0 ||
				!m_bDeviceAccess								)
			{
				disabled= true;
			}
			if(actLayout.compareTo(layout.readonly) == 0)
				readonly= true;
			style= readonly ? SWT.SINGLE | SWT.READ_ONLY : SWT.SINGLE;
			style= !disabled && !readonly ? style | SWT.BORDER : style;
			if(bLabel)
			{
				m_oLabel= new Label(composite, style);
				font.setDevice(m_oLabel);
			}else
			{
				m_oText= new Text(composite, style);
				font.setDevice(m_oText);
			}
			if(	!getPermission().equals(permission.None) &&
				!m_nSoftButton								)
			{
				for (textFormat obj : m_aResult)
				{
					obj.result= client.getValue(obj.resultStr, /*throw*/true);
					if(client.hasError())
					{
						obj.errorCode= client.getErrorCode();
						obj.errorMessage= client.getErrorMessage();
					}
				}
			}
			if(!m_nSoftButton)
			{
				int count= 0;
				// calculating the digits before decimal point
				// and after. save in m_numBefore and m_numBehind
				do{
					if(floatStr.match(this.value))
					{
						String v, b;
						String sPattern;
						
						bread= true;
						if((count+1) > m_aResult.size())
						{
							formatObj= new textFormat();
							formatObj.correctResultString= false;
							m_aResult.add(formatObj);
							
						}else
							formatObj= m_aResult.get(count);
						formatObj.beforeValue+= this.value.substring(0, floatStr.getParenStart(0));
						sPattern= sNumberStr;
						if(	count > 0 &&
							formatObj.beforeValue.substring(0, 1).equals("\\")	)
						{
							if(formatObj.beforeValue.length() > 1)
								formatObj.beforeValue= formatObj.beforeValue.substring(1);
							else
								formatObj.beforeValue= "";
						}
						this.format= floatStr.getParen(0);
						if(floatStr.getParen(1) != null)
						{
							if(floatStr.getParenLength(1) % 2 != 0)
							{// no right number holder, take the next one
								formatObj.beforeValue+= floatStr.getParen(1).substring(0, floatStr.getParenLength(1)-1);
								formatObj.beforeValue+= this.format.substring(floatStr.getParenLength(1));
								this.value= this.value.substring(formatObj.beforeValue.length()+1);
								bread= true;
								continue;
							}
							formatObj.beforeValue+= floatStr.getParen(1);
							this.format= this.format.substring(floatStr.getParenLength(1));
						}
						this.value= this.value.substring(floatStr.getParenEnd(0));
						try{
							v= floatStr.getParen(2);
							if(	v != null &&
								v.length() > 0	)
							{
								formatObj.numBefore= Integer.parseInt(v);
							}
							b= floatStr.getParen(3);
							v= floatStr.getParen(4);
							if(	v != null &&
								v.length() > 0	)
							{
								formatObj.numBehind= Integer.parseInt(v);
								sPattern+= sBehindStr;
								
							}else if(	b != null &&
										b.equals(".")	)
							{
								formatObj.numBehind= -1;
								sPattern+= sBehindStr;
							}else
								formatObj.numBehind= 0;
							if(	b != null &&
								b.length() > 0 &&
								!b.substring(0, 1).equals(".")	)
							{
								this.value= b + this.value;
							}
							
						}catch(NumberFormatException ex)
						{
							// do nothing,
							// take default value from member
							bread= true;
						}
						if(	!formatObj.errorCode.equals("ERROR 016") &&
							formatObj.result == null										)
						{
							if(HtmTags.debug)
							{
								String message= "";
								
								if(!this.result.equals(""))
									message= "cannot reach subroutine '" + formatObj.resultStr + "' from ";
								message+= "component input text";
								if(!this.name.equals(""))
									message+= " with name " + this.name + " ";
								if(this.result.equals(""))
									message+= " have no result attribute";
								System.out.println(message);
								System.out.println(formatObj.errorMessage);
							}
							formatObj.correctResultString= false;
						}
						sPattern+= ")";
						formatObj.beforeValue= formatObj.beforeValue.replaceAll("\\\\\\\\", "\\\\");
						formatObj.changePattern= new RE(sPattern);
						++count;
						
					}else
					{
						this.value= " " + this.value;
						bread= false;
					}
				}while(bread);
				this.value= this.value.replaceAll("\\\\\\\\", "\\\\");

			}

			if(width == -1)
				width= 100;
			gridData.widthHint= width;
			if(height != -1)
				gridData.heightHint= height;
			if(bLabel)
			{
				m_oLabel.setLayoutData(gridData);
				m_oComponent= m_oLabel;
				m_oLabel.setText(calculateInputValue());
				m_oLabel.setEnabled(!disabled);
				
			}else
			{
				m_oText.setLayoutData(gridData);
				m_oComponent= m_oText;
				if(!m_nSoftButton)
					m_oText.setText(calculateInputValue());
				m_oText.setEnabled(!disabled);
			}
			
		}else if(this.type.equals("slider"))
		{
			final Slider slider;
			GridData data= new GridData();
			int style= SWT.HORIZONTAL;
			int value= this.min;
			Double akt= null;
			
			if(	!getPermission().equals(permission.None) &&
				!m_nSoftButton								)
			{
				akt= client.getValue(this.result, /*bthrow*/true);
			}
			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component slider";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}else
			{
				double d;
				
				if(akt == null)
					d= 0;
				else
					d= akt;				
				value= (int)d;
			}

			if(	width != -1 ||
				height != -1	)
			{
				if(width != -1)
					data.widthHint= width;
				if(height != -1)
					data.heightHint= height;
				if(width < height)
					style= SWT.VERTICAL;
				
			}else
				data.widthHint= 100;
			
			disabled= disabled || readonly ? true : false;
			slider= new Slider(composite, style);
			slider.setLayoutData(data);
			slider.setSize(width, height);
			if(!m_smin.equals(""))
			{
				akt= client.getValue(m_smin, /*bthrow*/false);
				if(akt != null)
				{
					min= akt.intValue();
					
				}else if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component slider";
					if(!this.name.equals(""))
						message+= " with min value " + this.m_smin + " ";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
			}
			slider.setMinimum(min);
			min= slider.getMinimum();
			if(!m_smax.equals(""))
			{
				akt= client.getValue(m_smax, /*bthrow*/false);
				if(akt != null)
				{
					max= akt.intValue();
					
				}else if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component slider";
					if(!this.name.equals(""))
						message+= " with max value " + this.m_smin + " ";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
			}
			// toDo:	search reason why maximum need 
			//			10 values more than minimum
			slider.setMaximum(max+10);
			max= slider.getMaximum();
			font.setDevice(slider);
			// Pfeiltaste
			slider.setIncrement(arrowkey);
			// klick auf Schieberegler
			slider.setPageIncrement(rollbarfield);
			// Aktuaelle Position
			slider.setSelection(value);
			slider.setEnabled(!disabled);
			m_oComponent= slider;
			slider.setMaximum(max);
			
		}else if(this.type.equals("range"))
		{
			Scale scale;
			GridData data= new GridData();
			int style= SWT.HORIZONTAL;
			int value= this.min;
			Double akt= null;
			
			if(	!getPermission().equals(permission.None) &&
				!m_nSoftButton								)
			{
				akt= client.getValue(this.result, /*bthrow*/true);
			}
			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component range";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}else
			{
				if(akt != null)
				{
					double d= akt;
					
					value= (int)d;
				}else
					value= 0;
			}
			
			if(	width != -1 ||
				height != -1	)
			{
				if(width != -1)
					data.widthHint= width;
				if(height != -1)
					data.heightHint= height;
				if(width < height)
					style= SWT.VERTICAL;
					
			}else
				data.widthHint= 100;

			disabled= disabled || readonly ? true : false;
			scale= new Scale(composite, style);
			scale.setLayoutData(data);
			if(!m_smin.equals(""))
			{
				akt= client.getValue(m_smin, /*bthrow*/false);
				if(akt != null)
				{
					min= akt.intValue();
					
				}else if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component range";
					if(!this.name.equals(""))
						message+= " with min value " + this.m_smin + " ";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
			}
			scale.setMinimum(min);
			if(!m_smax.equals(""))
			{
				akt= client.getValue(m_smax, /*bthrow*/false);
				if(akt != null)
				{
					max= akt.intValue();
					
				}else if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component range";
					if(!this.name.equals(""))
						message+= " with max value " + this.m_smin + " ";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
			}
			scale.setMaximum(max);
			scale.setSize(width, height);
			// Pfeiltaste
			scale.setIncrement(arrowkey);
			// klick auf Schieberegler
			scale.setPageIncrement(rollbarfield);
			// Aktuaelle Position
			scale.setEnabled(!disabled);

			if(value != 0) // do not set 0 for selection, because in this case
				scale.setSelection(value); // scale will be set to full range
			value= scale.getMinimum();
			value= scale.getMaximum();
			value= scale.getSelection();
			m_oComponent= scale;
			m_nAktValue= value;
			
		}else if(	this.type.equals("label") ||
					(	this.type.equals("combo") &&
						this.size == 1					)	)
		{
			boolean bLabel= this.type.equals("label");
			Combo combo= null;
			Label label= null;
			//Combo combo= new Combo(composite, SWT.READ_ONLY | SWT.DROP_DOWN);
			Double akt= null;
			permission perm;
			
			if(bLabel)
			{
				String[] spl;
				textFormat formatObj;
				
				label= new Label(composite, SWT.NONE);
				font.setDevice(label);
				m_aResult= new ArrayList<textFormat>();
				spl= this.result.split(",");
				for (String res : spl)
				{
					formatObj= new textFormat();
					formatObj.resultStr= res.trim();
					m_aResult.add(formatObj);
				}
				
			}else
			{
				combo= new Combo(composite, SWT.READ_ONLY | SWT.DROP_DOWN | SWT.BORDER);
				font.setDevice(combo);
			}
			perm= getPermission();
			if(	!perm.equals(permission.None) &&
				!m_nSoftButton								)
			{
				akt= client.getValue(this.result, /*bthrow*/true);
				if(bLabel)
				{
					for (textFormat obj : m_aResult)
						obj.actPermission= perm;
				}
			}
			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component combo";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			if(bLabel)
			{
				label.setLayoutData(gridData);
				label.setEnabled(true);
				m_oComponent= label;
			}else
			{
				combo.setLayoutData(gridData);
				combo.setEnabled(!disabled);
				m_oComponent= combo;
			}
			if(width != -1)
				gridData.widthHint= width;
			else if(bLabel &&
					width == -1)
			{
				gridData.widthHint= 100;
			}
			if(height != -1)
				gridData.heightHint= height;
			if(	!m_lContent.isEmpty())
			{
				boolean bfirst= true;
				double value= -1;
				
				for(HtmTags tag : m_lContent)
				{
					if(tag instanceof Option)
					{
						Option option= (Option)tag;
						String entry= option.getOptionString();

						if(option.value != null)
							value= option.value;
						else
							++value;
						addComboEntrys(entry, value);
						if(!bLabel)
							combo.add(entry);
						if(	(	akt == null &&
								bfirst == true	) ||
							(	akt != null &&
								akt.equals(m_asComboValueEntrys.get(entry))	)
																)
						{
							if(bLabel)
								label.setText(entry);
							else
								combo.setText(entry);
							bfirst= false;
						}
					}
				}
			}
			
		}else if(	this.type.equals("combo")
					&&
					this.size > 1				)
		{
			int style;
			List list;
			int item= 0;
			double value= -1;
			Double akt= null;
			
			style= m_lContent.size() > this.size ? SWT.SINGLE | SWT.V_SCROLL : SWT.SINGLE;
			style= !readonly && !disabled ? style | SWT.BORDER : style;
			list= new List(composite, style);
			font.setDevice(list);
			if(	!getPermission().equals(permission.None) &&
				!m_nSoftButton								)
			{
				akt= client.getValue(this.result, /*bthrow*/true);
			}
			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component combo";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			m_oComponent= list;
			if(width != -1)
				gridData.widthHint= width;
			gridData.heightHint= list.getItemHeight()  * this.size + this.size;
			//data.heightHint= fontData.getHeight()  * this.size;
			//data.heightHint= (list.computeSize(SWT.DEFAULT, SWT.DEFAULT, true).x/4) * this.size;
			list.setLayoutData(gridData);
			list.setEnabled(!disabled);
			if(	!m_lContent.isEmpty()
				&&
				akt != null				)
			{
				for(HtmTags tag : m_lContent)
				{
					if(tag instanceof Option)
					{
						Option option= (Option)tag;
						String entry= option.getOptionString();
						
						if(option.value != null)
							value= option.value;
						else
							++value;
						addComboEntrys(entry, value);
						list.add(entry);
						if(akt == value)
							list.select(item);
						++item;
					}
				}
				this.min= 0;
				this.max= item - 1;
			}
			
		}else if(this.type.equals("spinner"))
		{
			int style= readonly ? SWT.READ_ONLY  : SWT.NONE;
			style= !readonly && !disabled ? style | SWT.BORDER : style;
			Spinner spinner= new Spinner(composite, style);
			Double akt= null;
			
			if(	!getPermission().equals(permission.None) &&
				!m_nSoftButton								)
			{
				akt= client.getValue(this.result, /*bthrow*/true);
			}
			if(	!client.getErrorCode().equals("ERROR 016")
				&&
				akt == null									)
			{
				if(HtmTags.debug)
				{
					String message= "";
					
					if(!this.result.equals(""))
						message= "cannot reach subroutine '" + this.result + "' from ";
					message+= "component spinner";
					if(!this.name.equals(""))
						message+= " with name " + this.name + " ";
					if(this.result.equals(""))
						message+= " have no result attribute";
					System.out.println(message);
					System.out.println(client.getErrorMessage());
				}
				m_bCorrectName= false;
			}
			m_oComponent= spinner;
			if(width != -1)
				gridData.widthHint= width;
			if(height != -1)
				gridData.heightHint= height;
			spinner.setLayoutData(gridData);
			spinner.setEnabled(!disabled);
			if(this.min > -1)
				spinner.setMinimum(this.min);
			if(this.max > -1)
				spinner.setMaximum(this.max);
			if(akt == null)
				akt= new Double(0);
			spinner.setValues((int)akt.doubleValue(), min, max, this.digits, arrowkey, rollbarfield);
			spinner.setEnabled(!disabled);
			
		}else
		{
			m_bCorrectName= false;
			if(HtmTags.debug)
				System.out.println("Component type '"+this.type+"' is no correct entry");
		}
	}
	
	/**
	 * disable or enable component whether server has correct access to device
	 * 
	 * @param subroutine string of subroutine as &lt;folder&gt;:&lt;subroutine&gt;
	 * @param enable whether server has access to device
	 * @return whether given subroutine was for this component
	 */
	public boolean setDeviceAccess(String subroutine, final boolean enable)
	{
		if(this.result.equals(subroutine))
		{
			final String type= this.type;
			
			m_bDeviceAccess= enable;
			DisplayAdapter.syncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					m_oComponent.setEnabled(enable);
					if(	type.equals("text")
						&&
						!enable						)
					{
						for (textFormat obj : m_aResult)
						{
							if(obj.result != null)
								obj.result= 0.0;
						}
						((Text)m_oComponent).setText(calculateInputValue());
					}
				}
			
			}, "setDeviceAccess");
			return true;
		}
		return false;
	}
	
	/**
	 * set constant permission defined in layout file getting from server
	 * 
	 * @param perm permission layout from layout file
	 */
	public void setLayoutPermission(layout perm)
	{
		actLayout= perm;
		normal= perm;
	}
	/**
	 * setter routine to set actual permission inside of an tag
	 * 
	 * @param formatObj set permission into textFormat object when parameter not null, otherwise to member variables of Component object
	 * @return actual permission
	 * 
	 * @param perm permission to set
	 */
	protected void setPermission(permission perm, textFormat formatObj)
	{
		if(HtmTags.debug)
		{
			System.out.print("set permission '" + perm + "' for ");
			if(formatObj != null)
				System.out.println("result: " + formatObj.resultStr);
			else
				System.out.println(" no result");
		}
		if(formatObj == null)
			super.setPermission(perm);
		else
			formatObj.actPermission= perm;
		switch (perm)
		{
		case writeable:			
			if(	normal == layout.readonly ||
				normal == layout.disabled	)
			{
				if(formatObj == null)
					actLayout= normal;
				else
					formatObj.actLayout= normal;
			}else
			{
				if(formatObj == null)
					actLayout= layout.normal;
				else
					formatObj.actLayout= layout.normal;
			}
			break;
			
		case readable:
			if(normal == layout.disabled)
			{
				if(formatObj == null)
					this.actLayout= normal;
				else
					formatObj.actLayout= normal;
			}else
			{
				if(formatObj == null)
					actLayout= layout.readonly;
				else
					formatObj.actLayout= layout.readonly;
			}
			break;
			
		case None:
		default:
			if(formatObj == null)
				actLayout= layout.disabled;
			else
				formatObj.actLayout= layout.disabled;
			break;
		}
	}
	
	/**
	 * check permission on server for this component
	 * 
	 * @param formatObj set permission into textFormat object when parameter not null, otherwise to member variables of Component object
	 * @return actual permission
	 * 
	 * @throws IOException
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public permission askPermission(textFormat formatObj) throws IOException
	{
		Double res;
		String result;
		permission eRv;
		MsgClientConnector client;
		
		if(	(	formatObj == null &&
				permissionSet == true	) ||
			(	formatObj != null &&
				formatObj.permissionSet == true	)	)
		{
			if(formatObj != null)
				return formatObj.actPermission;
			return super.getPermission();
		}
		if(	(	formatObj != null &&
				formatObj.resultStr.equals("") ) ||
			(	formatObj == null &&
				this.result.equals("")	)			)
		{
			setPermission(permission.None, formatObj);
			if(formatObj != null)
				formatObj.permissionSet= true;
			else
				permissionSet= true;
			return permission.None;
		}
		if(formatObj != null)
			result= formatObj.resultStr;
		else
			result= this.result;
		client= MsgClientConnector.instance();
		res= client.getValue(result, /*bthrow*/true);
		if(!client.hasError())
		{
			if(	normal == layout.readonly ||
				!client.setValue(result, res, /*bthrow*/true)	)
			{
				eRv= permission.readable;
				setPermission(eRv, formatObj);
				
			}else
			{
				eRv= permission.writeable;
				setPermission(eRv, formatObj);
			}
			if(formatObj != null)
				formatObj.permissionSet= true;
			else
				permissionSet= true;
				
		}else
		{	
			eRv= permission.None;
			setPermission(eRv, formatObj);
			System.out.println(client.getErrorMessage());
			if(client.getErrorCode().equals("PORTSERVERERROR016"))
			{
				if(formatObj != null)
					formatObj.deviceAccess= false;
				else
					m_bDeviceAccess= false;
			}else
			{
				if(formatObj != null)
					formatObj.permissionSet= true;
				else
					permissionSet= true;
			}
		}
		return eRv;
	}
	
	/**
	 * whether result is an soft button
	 * 
	 * @param classes all class definition for any tags
	 * @return whether result is an soft button
	 */
	private boolean isOnlySoftButton(HashMap<String, HtmTags> classes)
	{
		String sBrowserClass= "";
		
		m_nSoftButton= false;
		if(	result.length() > 15 &&
			(	result.substring(0, 15).equals("::browser_home@") ||
				result.substring(0, 15).equals("::browser_stop@") ||
				result.substring(0, 15).equals("::browser_back@") ||
				result.substring(0, 15).equals("::browser_info@") ||
				result.substring(0, 15).equals("::browser_load@")	)	)
		{
			m_sSoftButtonName= result.substring(2, 14);
			sBrowserClass= result.substring(15);
			if(	!m_sSoftButtonName.equals("browser_info") &&
				!m_sSoftButtonName.equals("browser_load") &&
				sBrowserClass.contains("-")					)
			{
				String spl[];
				
				spl= sBrowserClass.split("-");
				result= sBrowserClass.substring(spl[0].length() + 1);
				sBrowserClass= spl[0];
			}else
				result= "";
			
		}else if(	result.length() > 18 &&
					(	result.substring(0, 18).equals("::browser_refresh@") ||
						result.substring(0, 18).equals("::browser_forward@")	)	)
		{
			m_sSoftButtonName= result.substring(2, 17);
			sBrowserClass= result.substring(18);
			if(sBrowserClass.contains("-"))
			{
				String spl[];
				
				spl= sBrowserClass.split("-");
				result= sBrowserClass.substring(spl[0].length() + 1);
				sBrowserClass= spl[0];
			}else
				result= "";
			
		}else if(	result.length() > 14 &&
					result.substring(0, 14).equals("::browser_url@")	)
		{
			String[] resUrl;
			
			m_sSoftButtonName= result.substring(2, 13);
			sBrowserClass= result.substring(14);
			resUrl= sBrowserClass.split("@");	
			if(resUrl.length == 2)
			{
				m_sSoftButtonUrl= resUrl[0];
				sBrowserClass= resUrl[1];
			}
			if(sBrowserClass.contains("-"))
			{
				String spl[];
				
				spl= sBrowserClass.split("-");
				result= sBrowserClass.substring(spl[0].length() + 1);
				sBrowserClass= spl[0];
			}else
				result= "";
			
		}else if(	result.length() > 2 &&
					result.substring(0, 2).equals("::")	)
		{
			System.out.println("### ERROR: unknown soft button '" + result + "' be set");
			result= "";
		}
		if(!sBrowserClass.equals(""))
		{
			if(	(	(	m_sSoftButtonName.equals("browser_home") ||
						m_sSoftButtonName.equals("browser_stop") ||
						m_sSoftButtonName.equals("browser_back") ||
						m_sSoftButtonName.equals("browser_refresh") ||
						m_sSoftButtonName.equals("browser_forward") ||
						(	m_sSoftButtonName.equals("browser_url") &&
							!m_sSoftButtonUrl.equals("")				)	) &&
					(	type.equals("button") ||		
						type.equals("upbutton") ||
						type.equals("leftbutton") ||
						type.equals("rightbutton") ||
						type.equals("downbutton") ||
						type.equals("togglebutton") ||		
						type.equals("hidden")			)							) ||
						
				(	m_sSoftButtonName.equals("browser_load") &&
					(	type.equals("checkbox") ||
						type.equals("radio") ||
						type.equals("togglebutton")	)				) ||
							
				(	(	m_sSoftButtonName.equals("browser_info") ||
						(	m_sSoftButtonName.equals("browser_url") &&
							m_sSoftButtonUrl.equals("")					)	) &&
					type.equals("text")												)		)
/*			if(	(	type.equals("text") &&
					m_sSoftButtonUrl.equals("") &&
					(	m_sSoftButtonName.equals("browser_url") ||
						m_sSoftButtonName.equals("browser_info")	)	) ||
				(	m_sSoftButtonName.equals("browser_url") &&
					!m_sSoftButtonUrl.equals("") &&
					!type.equals("text")						) ||
				(	m_sSoftButtonName.equals("browser_load") &&
					(	type.equals("checkbox") ||
						type.equals("radio") ||
						type.equals("togglebutton")	)			) ||
				(	!m_sSoftButtonName.equals("browser_url") &&	// there is also allowed
					!m_sSoftButtonName.equals("browser_info") &&	// an URL with new address
					!m_sSoftButtonName.equals("browser_load") &&
					(	type.equals("button") ||		
						type.equals("upbutton") ||
						type.equals("leftbutton") ||
						type.equals("rightbutton") ||
						type.equals("downbutton")		)		)				)*/
			{
				m_oSoftButtonTag= null;
				m_oSoftButtonTag= classes.get(sBrowserClass);
				if(m_oSoftButtonTag != null)
				{
					if(	m_sSoftButtonName.equals("browser_info") ||
						m_sSoftButtonName.equals("browser_load")	)
						setPermission(permission.readable);
					else
						setPermission(permission.writeable);
					m_nSoftButton= true;
					
				}else
				{
					System.out.println("### ERROR: cannot find td-tag with class \"" + sBrowserClass + "\" for soft button " + m_sSoftButtonName);
					System.out.println("           so do not define button as soft button");
					result= "";
				}
			}else
			{
				System.out.print("### ERROR: an soft button like '" + m_sSoftButtonName + "@" + sBrowserClass + " can only be defined in ");
				if(	m_sSoftButtonName.equals("browser_url") ||
					m_sSoftButtonName.equals("browser_info")	)
				{
					System.out.println("an text input-tag");
					
				}else if(m_sSoftButtonName.equals("browser_load"))
					System.out.println("an button-tag with type togglebutton, checkbox or radio");
				else
					System.out.println("any button-tag (button, upbutton, ...) but no togglebutton");
				result= "";
			}
		}
		if(result.equals(""))
			return m_nSoftButton;
		return false;
	}
	
	/**
	 * make result for defined soft button
	 */
	private void doSoftButton()
	{
		if(m_sSoftButtonName.equals("browser_home"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
			
			if(HtmTags.debug)
				System.out.println("set browser to HOME URL " + tdTag.href);
			tdTag.getBrowser().setUrl(tdTag.href);
			
		}else if(m_sSoftButtonName.equals("browser_refresh"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;

			if(HtmTags.debug)
				System.out.println("refresh browser with aktual URL " + tdTag.getBrowser().getUrl());
			tdTag.getBrowser().refresh();
			
		}else if(m_sSoftButtonName.equals("browser_stop"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;

			if(HtmTags.debug)
				System.out.println("stop browser loading by URL " + tdTag.getBrowser().getUrl());
			tdTag.getBrowser().stop();
			
		}else if(m_sSoftButtonName.equals("browser_forward"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;

			m_bActiveButton= true;
			if(HtmTags.debug)
				System.out.println("forward browser from URL " + tdTag.getBrowser().getUrl());
			tdTag.getBrowser().forward();
			if(HtmTags.debug)
				System.out.println("                  to URL " + tdTag.getBrowser().getUrl());
			
		}else if(m_sSoftButtonName.equals("browser_back"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;

			m_bActiveButton= true;
			tdTag.getBrowser().back();
			
		}else if(m_sSoftButtonName.equals("browser_url"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
			
			if(m_sSoftButtonUrl.equals(""))
			{
				value= m_oText.getText();
				if(value.equals(""))
				{
					if(tdTag.m_sActRef.equals(""))
						value= tdTag.href;
					else
						value= tdTag.m_sActRef;		
				}
				if(HtmTags.debug)
					System.out.println("set browser to URL " + value);
				tdTag.getBrowser().setUrl(value);
			}else
			{
				if(HtmTags.debug)
					System.out.println("set browser to URL " + m_sSoftButtonUrl);
				tdTag.getBrowser().setUrl(m_sSoftButtonUrl);
			}
			
		}
	}
	
	/**
	 * add listeners if the component have an correct result attribute.<br />
	 * This method is to listen on activity if the component is in an {@link Composite}
	 * witch is be on the top of the {@link StackLayout}
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void addListeners() throws IOException
	{
		boolean bAlsoOK= false;
		final String result= this.result;
		final MsgClientConnector client= MsgClientConnector.instance();

		
		//System.out.println("add listener for type:" + type + " soft button:" + m_sSoftButtonName + " and result " + result);
		haveListener= false;
		if(	m_bCorrectName &&
			!result.equals("") &&
			//getPermission().compareTo(permission.readable) >= 0 &&
			client.haveSecondConnection()							)
		{
			if(type.equals("text"))
			{
				for (textFormat obj : m_aResult)
				{
					if(!obj.resultStr.equals(""))
					{
						if(HtmTags.debug)
							System.out.println("client hear on '" + obj.resultStr + "'");
						client.hear(obj.resultStr, /*throw*/true);
					}
				}
			}else
			{
				if(HtmTags.debug)
					System.out.println("client hear on '" + result + "'");
				client.hear(result, /*bthrow*/true);
			}
		}
		if(this.type.equals("hidden"))
			return;
		if(	HtmTags.moveMouseX >= 0 ||
			HtmTags.moveMouseY >= 0		)
		{
			m_oComponent.addMouseListener(m_eMouseUpListener= new MouseAdapter() 
			{
				public void mouseUp(MouseEvent event)
				{
					int x, y;
					PopupMenu popup;
	
					popup= PopupMenu.instance();
					if(popup != null)
						popup.destroy();
					if(HtmTags.moveMouseX >= 0)
						x= HtmTags.moveMouseX;
					else
						x= event.x;
					if(HtmTags.moveMouseY >= 0)
						y= HtmTags.moveMouseY;
					else
						y= event.y;
					try{
						Thread.sleep(HtmTags.moveMouseDelay);
					}catch(InterruptedException ex)
					{
						System.out.println("cannot wait " + HtmTags.moveMouseDelay + " milliseconds before moving");
						ex.printStackTrace();
					}
					Display.getCurrent().setCursorLocation(x, y);
					/*try{
					 *
					 * throw AWTException if the XTEST 2.2 standard extension is not supported (or not enabled) by the X server. *
					 * 
						robot= new Robot();
						if(HtmTags.moveMouseDelay > 0)
						{
							if(HtmTags.debug)
								System.out.println("wait " + HtmTags.moveMouseDelay + " milliseconds before moving");
							robot.delay(HtmTags.moveMouseDelay);
						}
						if(HtmTags.debug)
							System.out.println("move mouse to position " + HtmTags.moveMouseX + "/" + HtmTags.moveMouseY + "(x/y)");
						robot.mouseMove(x, y);
						
					}catch(AWTException ex)
					{
						System.out.println("cannot define AWT Robot to move mouse");
					System.out.print("Graphics environment is ");
					if(!GraphicsEnvironment.isHeadless())
						System.out.print("not ");
					System.out.println("Headless");
						ex.printStackTrace();
					}*/
				}
			});
		}
		if(	this.type.equals("combo") ||
			(	getPermission().equals(permission.readable) &&
				(	this.type.equals("text") &&
					m_sSoftButtonName.equals("browser_info")	) ||
				(	m_sSoftButtonName.equals("browser_load") &&
					(	this.type.equals("checkbox") ||
						this.type.equals("radio") ||
						this.type.equals("togglebutton")	)	)	)	)
		{// if Component has no permission but type is an combo
		 // set listener to return back to the old value
			bAlsoOK= true;
		}
		
		if(	!m_bCorrectName ||
			(	!getPermission().equals(permission.writeable) &&
				!bAlsoOK										)	)
		{
			return;
		}
		
		if(	this.type.equals("checkbox") ||
			this.type.equals("radio") ||
			this.type.equals("togglebutton")	)
		{
			if(m_nSoftButton)
			{
				ContentFields tdTag= (ContentFields)m_oSoftButtonTag;

				haveListener= true;
				tdTag.getBrowser().addProgressListener(m_eProgressListener= new ProgressListener()
				{
					@Override
					public void changed(ProgressEvent arg0)
					{
						//System.out.println("load total:" + arg0.total + " current:" + arg0.current);
						if (arg0.total == arg0.current) return;
						((Button)m_oComponent).setSelection(true);
					}
					
					@Override
					public void completed(ProgressEvent arg0)
					{
						//System.out.println("loading was completed");
						((Button)m_oComponent).setSelection(false);
					}
					
				});
				
			}
			if(!result.equals(""))
			{
				String sValue= "0";
				if(this.type.equals("radio"))
					sValue= this.value;
				final String gType= this.type;
				final short gValue= Short.parseShort(sValue);

				haveListener= true;
				((Button)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
				{
					public void widgetSelected(SelectionEvent ev)
					{
						short nValue;
						Button button= (Button)m_oComponent;
						boolean bSet= button.getSelection();
						
						if(gType.equals("radio"))
							nValue= gValue;
						else
						{
							if(bSet)
								nValue= 1;
							else
								nValue= 0;
						}
						try{
							client.setValue(result, nValue, /*bthrow*/false);
						}catch(IOException ex)
						{}
				    	if(	HtmTags.debug &&
				    		client.hasError()	)
				    	{
				    		System.out.println("ERROR: by setValue() inside SelectionListener");
				    		System.out.println("       " + client.getErrorMessage());
				    	}
					}
				});
			}
			
		}else if(	this.type.equals("button")
					||
					this.type.equals("upbutton")
					||
					this.type.equals("leftbutton")
					||
					this.type.equals("rightbutton")
					||
					this.type.equals("downbutton")	)
		{
			haveListener= true;
			m_oComponent.addListener(SWT.MouseDown, m_eListener1= new Listener()
			{
			    public void handleEvent(final Event event)
			    {
			    	if(!result.equals(""))
				    {
				    	try{
				    		client.setValue(result, 1, /*bthrow*/false);
						}catch(IOException ex)
						{}
				    	if(	HtmTags.debug &&
				    		client.hasError()	)
				    	{
				    		System.out.println("ERROR: by setValue() inside Listener");
				    		System.out.println("       " + client.getErrorMessage());
				    	}
				    }
			    	if(m_nSoftButton)
				    	doSoftButton();
			    }
			});
			m_oComponent.addListener(SWT.MouseUp, m_eListener2= new Listener()
			{
			    public void handleEvent(final Event event)
			    {
			    	if(!result.equals(""))
			    	{
				    	try{
				    		client.setValue(result, 0, /*bthrow*/false);
						}catch(IOException ex)
						{}
				    	if(	HtmTags.debug &&
				    		client.hasError()	)
				    	{
				    		System.out.println("ERROR: by setValue() inside Listener");
				    		System.out.println("       " + client.getErrorMessage());
				    	}
			    	}
			    }
			}); 
			if(	m_nSoftButton &&
				(	m_sSoftButtonName.equals("browser_forward") ||
					m_sSoftButtonName.equals("browser_back")		)	)
			{
				final ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
				
				//doSoftButton();
				tdTag.getBrowser().addLocationListener(m_eLocationListener= new LocationListener() {
					
					@Override
					public void changing(LocationEvent arg0) {					
						// nothing to do
					}
					
					@Override
					public void changed(LocationEvent arg0) 
					{
						String url;

						url= tdTag.getBrowser().getUrl();
						if(HtmTags.debug)
							System.out.println("change browser to '" + url + "' actualice " + result);
						if(m_sSoftButtonName.equals("browser_forward"))
						{
							if(m_bActiveButton)
							{
								if(HtmTags.debug)
									System.out.println("browser will be set to new url '" + url +"'");
								if(	tdTag.getBrowser().isForwardEnabled() &&
									(	url.equals("") ||
										url.equals("about:blank") ||
										(	m_bWasBlank &&
											url.equals(tdTag.m_sActRef)	)	)	)
								{
									if(	url.equals("") ||
										url.equals("about:blank")	)
									{
										m_bWasBlank= true;
									}else
										m_bWasBlank= false;
									tdTag.getBrowser().forward();
								}else
								{
									tdTag.m_sActRef= url;
									m_bWasBlank= false;
									m_bActiveButton= false;
								}
							}
							if(tdTag.getBrowser().isForwardEnabled())
								m_oButton.setEnabled(true);
							else
								m_oButton.setEnabled(false);
						}else
						{
							if(m_bActiveButton)
							{
								if(HtmTags.debug)
									System.out.println("browser will be set to new url '" + url +"'");
								if(	tdTag.getBrowser().isBackEnabled() &&
									(	url.equals("") ||
										url.equals("about:blank") ||
										(	m_bWasBlank &&
											url.equals(tdTag.m_sActRef)	)	)	)
								{
									if(	url.equals("") ||
										url.equals("about:blank")	)
									{
										m_bWasBlank= true;
									}else
										m_bWasBlank= false;
									tdTag.getBrowser().back();
								}else
								{
									tdTag.m_sActRef= url;
									m_bWasBlank= false;
									m_bActiveButton= false;
								}
							}
							if(tdTag.getBrowser().isBackEnabled())
								m_oButton.setEnabled(true);
							else
								m_oButton.setEnabled(false);
						}
					}
				});
			}
			
		}else if(this.type.equals("text"))
		{
			haveListener= true;
			if(m_nSoftButton)
			{
				if(m_sSoftButtonName.equals("browser_info"))
				{
					ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
					
					tdTag.getBrowser().addStatusTextListener(m_eStatusListener= new StatusTextListener() {
						
						@Override
						public void changed(StatusTextEvent arg0) 
						{
							((Text)m_oComponent).setText(arg0.text);
						}
					});
					
				}else
				{
					((Text)m_oComponent).addMouseListener(m_eMouseListener= new MouseListener() {
						
						@Override
						public void mouseUp(MouseEvent arg0)
						{
							if(m_bTextFocus == false)
							{
								((Text)m_oComponent).selectAll();
								m_bTextFocus= true;
							}
						}
						
						@Override
						public void mouseDown(MouseEvent arg0) {
							// nothing to do					
						}
						
						@Override
						public void mouseDoubleClick(MouseEvent arg0) {
							// nothing to do					
						}
					});
					((Text)m_oComponent).addFocusListener(m_eFocusListener= new FocusListener() {
						
						@Override
						public void focusLost(FocusEvent arg0) 
						{
							m_bTextFocus= false;
						}
						
						@Override
						public void focusGained(FocusEvent arg0)
						{
							// nothing to do
							
						}
					});
				}
			}
			((Text)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{
				@Override
				public void widgetDefaultSelected(SelectionEvent ev)
				{
					String string= ((Text)m_oComponent).getText();
					String strRes= string;
					double value;
					
					if(!m_nSoftButton)
					{
						for (textFormat formatObj : m_aResult)
						{
							strRes= string;
							if(formatObj.changePattern.match(string))
							{
								if(HtmTags.debug)
									System.out.println("user change text field " + name + " to " + string);
								strRes= formatObj.changePattern.getParen(2);
								string= string.substring(formatObj.changePattern.getParenEnd(2));
								if(!strRes.equals(""))
								{
									if(HtmTags.debug)
										System.out.println(" generate to number '" + strRes + "'");
									try{
										value= Double.parseDouble(strRes);
										strRes= formatObj.changePattern.getParen(1);
										if(	strRes.length() > 0 &&
											strRes.substring(strRes.length()-1).equals("-")	)
										{
											value*= -1;
										}
										
									}catch(NumberFormatException ex)
									{
										if(HtmTags.debug)
										{
											System.out.println("NumberFormatException for textfield " + name);
											System.out.println(" cannot convert value " + strRes);
											System.out.println();
										}
										value= 0;
									}
								}else
								{
									if(HtmTags.debug)
									{
										System.out.println("found no correct value for new string '" + strRes + "'");
										System.out.println("set value to 0");
									}
									value= 0;
								}
							}else
							{
								if(HtmTags.debug)
								{
									System.out.println("found no correct value for new string '" + strRes + "'");
									System.out.println("set value to 0");
								}
								value= 0;
							}
							try{
								client.setValue(formatObj.resultStr, value, /*bthrow*/false);
							}catch(IOException ex)
							{}
					    	if(	HtmTags.debug &&
					    		client.hasError()	)
					    	{
					    		System.out.println("ERROR: by setValue() inside SelectionListener");
					    		System.out.println("       " + client.getErrorMessage());
					    	}
						}
					}else
						doSoftButton();
				}
			
			});
			if(m_nSoftButton)
			{
				final ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
				
				doSoftButton();
				tdTag.getBrowser().addLocationListener(m_eLocationListener= new LocationListener() {
					
					@Override
					public void changing(LocationEvent arg0)
					{
						// nothing to do					
					}
					
					@Override
					public void changed(LocationEvent arg0) 
					{
						m_oText.setText(tdTag.getBrowser().getUrl());	
					}
				});
			}
			
		}else if(this.type.equals("slider"))
		{
			haveListener= true;
			((Slider)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					double value= ((Slider)m_oComponent).getSelection();
					
					try{
						client.setValue(result, value, /*bthrow*/false);
					}catch(IOException ex)
					{}
			    	if(	HtmTags.debug &&
			    		client.hasError()	)
			    	{
			    		System.out.println("ERROR: by setValue() inside SelectionListener");
			    		System.out.println("       " + client.getErrorMessage());
			    	}
					super.widgetSelected(e);
				}
			
			});
			
		}else if(this.type.equals("range"))
		{
			haveListener= true;
			((Scale)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					double value= ((Scale)m_oComponent).getSelection();
					
					try{
						client.setValue(result, value, /*bthrow*/false);
					}catch(IOException ex)
					{}
			    	if(	HtmTags.debug &&
			    		client.hasError()	)
			    	{
			    		System.out.println("ERROR: by setValue() inside SelectionListener");
			    		System.out.println("       " + client.getErrorMessage());
			    	}
					super.widgetSelected(e);
				}
			
			});
			
		}else if(	this.type.equals("combo")
					&&
					this.size == 1				)
		{
			final permission perm= getPermission();

			haveListener= true;
			((Combo)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter() 
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					if(HtmTags.syncSWTExec)
						System.out.println("Combo is selected");
					String key= ((Combo)m_oComponent).getText();
					Double value= m_asComboValueEntrys.get(key);
					//MsgClientConnector client= MsgClientConnector.instance();
					
					if(HtmTags.syncSWTExec)
						System.out.println("    with text " + key);
					if(perm.equals(permission.readable))
					{
						((Combo)m_oComponent).setText(m_asComboNameEntrys.get(m_nAktValue));
					}else
					{
						try{
							client.setValue(result, value, /*bthrow*/false);
						}catch(IOException ex)
						{}
				    	if(	HtmTags.debug &&
				    		client.hasError()	)
				    	{
				    		System.out.println("ERROR: by setValue() inside SelectionListener");
				    		System.out.println("       " + client.getErrorMessage());
				    	}
					}
					super.widgetSelected(e);
					if(HtmTags.syncSWTExec)
						System.out.println("Combo was selected");
				}			
			});
			
		}else if(	this.type.equals("combo")
				&&
				this.size > 1					)
		{
			final permission perm= getPermission();

			haveListener= true;
			((List)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter() 
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{					
					String key[]= ((List)m_oComponent).getSelection();
					Double value= m_asComboValueEntrys.get(key[0]);

					if(perm.equals(permission.readable))
					{
						((List)m_oComponent).setSelection((int)m_nAktValue);
					}else
					{
						try{
							client.setValue(result, value, /*bthrow*/false);
						}catch(IOException ex)
						{}
				    	if(	HtmTags.debug &&
				    		client.hasError()	)
				    	{
				    		System.out.println("ERROR: by setValue() inside SelectionListener");
				    		System.out.println("       " + client.getErrorMessage());
				    	}
					}
					super.widgetSelected(e);
				}
			
			});
			
		}else if(this.type.equals("spinner"))
		{
			haveListener= true;
			((Spinner)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter()
			{		
				@Override
				public void widgetSelected(SelectionEvent e)
				{						
					int value= ((Spinner)m_oComponent).getSelection();
					
					try{
						client.setValue(result, value, /*bthrow*/false);
					}catch(IOException ex)
					{}
			    	if(	HtmTags.debug &&
			    		client.hasError()	)
			    	{
			    		System.out.println("ERROR: by setValue() inside SelectionListener");
			    		System.out.println("       " + client.getErrorMessage());
			    	}
					super.widgetSelected(e);
				}			
			});
		}
	}
	
	/**
	 * calculate the string value for an component with type text
	 * 
	 * @return value as string with observance the format attribute and add the suffix (attribute value)
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	protected String calculateInputValue()
	{
		boolean bInt= false;
		String stringValue;
		int strPos= 0;
		char stringChars[];
		String Rv;
		String result= "";
		Double sh;

		if(!m_bDeviceAccess)
			return "ERROR";
		for (textFormat obj : m_aResult)
		{
			if(obj.actPermission.equals(permission.None))
				return " --- ";
			Rv= "";
			bInt= false;
			if(obj.result != null)
				stringValue= "" + obj.result;
			else
				stringValue= "";
			stringChars= stringValue.toCharArray();
			strPos= 0;
			while(	stringChars.length > strPos &&
					stringChars[strPos] != '.'		)
			{
				Rv+= stringChars[strPos];
				++strPos;
			}
			try{
				sh= new Double(stringValue.substring(0, strPos));
			}catch(NumberFormatException ex)
			{
				sh= new Double(0);
			}
			if(	obj.result != null &&
				obj.result.equals(sh))
			{
				bInt= true;
			}
			for(int c= strPos; c<obj.numBefore; ++c)
				Rv= "0" + Rv;
			if(	obj.numBehind > 0 ||
				obj.numBehind == -1	)
			{
				int behind= obj.numBehind;
	
				if(	obj.numBehind >0 ||
					!bInt				)
				{
					++strPos;
					if(obj.numBehind == -1)
						behind= stringValue.length() - strPos;
					Rv+= ".";
					for(int c= 0; c<behind; ++c)
					{
						if(stringChars.length > strPos)
						{
							Rv+= stringChars[strPos];
							++strPos;
						}else
							Rv+= "0";
					}
				}
			}
			result+= obj.beforeValue + Rv;
		}
		return result + this.value;
	}
	
	/**
	 * remove listeners if the component have an correct result attribute.<br />
	 * This method is to remove all listeners which set before with addListeners()
	 * if the component is in an {@link Composite} witch is not on the top of the {@link StackLayout}
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void removeListeners()
	{
		//System.out.println("remove listener for type:" + type + " soft button:" + m_sSoftButtonName + " and result " + result);
		if(m_eMouseUpListener != null)
		{
			m_oComponent.removeMouseListener(m_eMouseUpListener);
			m_eMouseUpListener= null;
		}
		if(haveListener == false)
			return;
		haveListener= false;
		if(	this.type.equals("checkbox")
			||
			this.type.equals("radio")
			||
			this.type.equals("togglebutton")	)
		{
			if(m_nSoftButton)
			{
				ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
				
				tdTag.getBrowser().removeProgressListener(m_eProgressListener);
				
			}else
			{
				((Button)m_oComponent).removeSelectionListener(m_eSelectionListener);
				m_eSelectionListener= null;
			}
			
		}else if(	this.type.equals("button")
					||
					this.type.equals("upbutton")
					||
					this.type.equals("leftbutton")
					||
					this.type.equals("rightbutton")
					||
					this.type.equals("downbutton")	)
		{
			m_oComponent.removeListener(SWT.MouseDown, m_eListener1);
			m_oComponent.removeListener(SWT.MouseUp, m_eListener2); 
			m_eListener1= null;
			m_eListener2= null;
			
		}else if(this.type.equals("text"))
		{	
			((Text)m_oComponent).removeSelectionListener(m_eSelectionListener);
			if(m_nSoftButton)
			{
				ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
				
				if(m_eStatusListener != null)
					tdTag.getBrowser().removeStatusTextListener(m_eStatusListener);
				else
				{
					tdTag.getBrowser().removeLocationListener(m_eLocationListener);
					m_oComponent.removeFocusListener(m_eFocusListener);
					m_oComponent.removeMouseListener(m_eMouseListener);
				}
			}
			
		}else if(this.type.equals("slider"))
		{
			((Slider)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(this.type.equals("range"))
		{
			((Scale)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(	this.type.equals("combo")
					&&
					this.size == 1				)
		{
			((Combo)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(	this.type.equals("combo")
				&&
				this.size > 1					)
		{
			((List)m_oComponent).removeSelectionListener(m_eSelectionListener);
			
		}else if(this.type.equals("spinner"))
		{
			((Spinner)m_oComponent).removeSelectionListener(m_eSelectionListener);
		}
	}
	
	/**
	 * gets the component as SWT-widget.<br />
	 * This method is for an faster access to the components.
	 * 
	 * @return component as SWT-widget
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public Control getComponentWidget()
	{
		return m_oComponent;
	}

	/**
	 * overridden insert for inherit tags
	 * 
	 * @param newTag {@link HtmTags} which should be inherit of tr tag
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 06.12.2007
	 * @since JDK 1.6
	 */
	protected void insert(HtmTags newTag)
	{
		m_lContent.add(newTag);
		newTag.m_oParent= this;
	}
	
	/**
	 * method look on server whether value of component is changed.
	 * 
	 * @param results if attribute result not in this map ask value from server and method save the result in the map
	 * @return false if the value is not changed or the result attribute not reachable, otherwise true
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	private boolean setNewValue(final Map<String, Double> results) throws IOException
	{
		MsgClientConnector client;
		Double get= results.get(this.result);
		
		if(get == null)
		{
			client= MsgClientConnector.instance();
			get= client.getValue(this.result, /*bthrow*/true);
			if(get == null)
			{
				System.out.println(client.getErrorMessage());
				System.out.println("set component " + this.tagName + " with type " + this.type + " to false");
				m_bCorrectName= false;
				return false;
			}
			results.put(this.result, get);
		}
		if(m_nAktValue == get)
			return false;
		m_nAktValue= get;
		return true;
	}
	
	/**
	 * method listen on server whether value of component is changed
	 * 
	 * @param results map of result attributes with actual values
	 * @param cont container with new value on which result of folder and subroutine
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void serverListener(final Map<String, Double> results, NodeContainer cont) throws IOException
	{
		boolean bValue= false;
		textFormat formatObj= null;
		String foldersub= cont.getFolderName() + ":" + cont.getSubroutineName();

		if(!m_bCorrectName)
			return;
		if(	this.type.equals("text") ||
			this.type.equals("label")	)
		{	
			for (textFormat obj : m_aResult)
			{
				if(obj.resultStr.equals(foldersub))
				{
					formatObj= obj;
					break;
				}
			}
			if(formatObj == null)
				return;
			
		}else if(!foldersub.equals(this.result))
		{
			if(	this.type.equals("slider") ||
				this.type.equals("range")		)
			{
				if(	(	m_smin.equals("") ||
						!foldersub.equals(m_smin)	) &&
					(	m_smax.equals("") ||
						!foldersub.equals(m_smax)	)	)
				{
					return;
				}
			}else
				return;
		}
		if(	cont != null &&
			cont.hasStringValue() &&
			cont.getSValue().equals("access")	)
		{
			askPermission(formatObj);
		}
		if(	(	formatObj == null &&
				getPermission().equals(permission.None)	) ||
			(	formatObj != null &&
				formatObj.actPermission.equals(permission.None)	)	)
		{
			return;
		}
		if(	cont != null &&
			cont.hasValue()	)
		{
			if(cont.hasDoubleValue())
			{
				m_nAktValue= cont.getDValue();
				if(formatObj != null)
				{
					formatObj.result= m_nAktValue;
					formatObj.errorCode= "";
					formatObj.errorMessage= "";
					formatObj.correctResultString= true;
				}
				bValue= true;
				
			}else if(cont.hasStringValue())
			{
				boolean bAccess= true;
				String value= cont.getSValue();
				
				if(value.equals("access"))
				{
					bAccess= true;
					bValue= true;
				}else if(value.equals("noaccess"))
				{
					bAccess= false;
					bValue= true;
				}
				if(bValue)
					setDeviceAccess(this.result, bAccess);
			}
		}
		if(!bValue)
		{
			if(!setNewValue(results))
				return;
		}
		
		if(	this.type.equals("checkbox") ||
			this.type.equals("button") ||
			this.type.equals("togglebutton")	)
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					String text;
					Button button= (Button)m_oComponent;
					boolean bSet= m_nAktValue > 0 ? true : false;
					
					if(!type.equals("button"))
						button.setSelection(bSet);
					if(!m_asComboNameEntrys.isEmpty())
					{
						text= m_asComboNameEntrys.get(m_nAktValue);
						if(text == null)
							text= "";
						button.setText(text);
					}
					if(	bSet &&
						m_nSoftButton )
					{
						doSoftButton();
					}
				}
			
			}, this.type.toString());
			
		}else if(this.type.equals("hidden"))
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					MsgClientConnector client= MsgClientConnector.instance();
				
					if(	m_nAktValue != 0 &&
						m_nSoftButton 		)
					{
						doSoftButton();
					}
				}
			
			}, this.type.toString());
			
		}else if(this.type.equals("radio"))
		{
			Double nValue= null;
			final double fnValue;
			
			try{
				nValue= Double.parseDouble(this.value);
				
			}catch(NumberFormatException ex)
			{
				MsgTranslator msg= MsgTranslator.instance();
				
				msg.errorPool("FAULT_double_value", "radio button", this.value);
				ex.printStackTrace();
				
			}
			finally
			{
				if(nValue == null)
				{
					nValue= 1D;
				}
			}
			fnValue= nValue;
			
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Button button= (Button)m_oComponent;
					boolean bSet= m_nAktValue == fnValue ? true : false;
					
					button.setSelection(bSet);
					if(	bSet &&
						m_nSoftButton )
					{
						doSoftButton();
					}
				}
			
			}, this.type.toString());
			
		//}else if(this.type.equals("text"))
		}else if(m_oComponent instanceof Text)
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					//String string= new String(m_nAktValue + " " + suffix);
					Text text= (Text)m_oComponent;
					
					text.setText(calculateInputValue());
				}
			
			}, this.type.toString());
			
		}else if(	m_oComponent instanceof Label &&
					m_lContent.isEmpty()			)
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Label label= (Label)m_oComponent;
					
					label.setText(calculateInputValue());
					label.pack(true);
				}
			
			}, this.type.toString());
			
		}else if(this.type.equals("slider"))
		{
			final short nCurType;

			if(	!m_smin.equals("") &&
				foldersub.equals(m_smin)	)
			{
				nCurType= 1;
				
			}else if(	!m_smax.equals("") &&
						foldersub.equals(m_smax)	)
			{
				nCurType= 2;
				
			}else
				nCurType= 0;
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Slider slider= (Slider)m_oComponent;

					switch(nCurType)
					{
					case 0:
						slider.setSelection((int)m_nAktValue);
						break;
						
					case 1:
						slider.setMinimum((int)m_nAktValue);
						break;
						
					case 2:
						slider.setMaximum((int)m_nAktValue);
					}
				}
			
			}, this.type.toString());
			
		}else if(this.type.equals("range"))
		{
			final short nCurType;

			if(	!m_smin.equals("") &&
				foldersub.equals(m_smin)	)
			{
				nCurType= 1;
				
			}else if(	!m_smax.equals("") &&
						foldersub.equals(m_smax)	)
			{
				nCurType= 2;
				
			}else
				nCurType= 0;
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Scale scale= (Scale)m_oComponent;
					
					switch(nCurType)
					{
					case 0:
						scale.setSelection((int)m_nAktValue);
						break;
						
					case 1:
						scale.setMinimum((int)m_nAktValue);
						break;
						
					case 2:
						scale.setMaximum((int)m_nAktValue);
					}
				}
			
			}, this.type.toString());
			
		}else if(	m_oComponent instanceof Combo ||
					m_oComponent instanceof Label	)
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					boolean bfound= false, bLabel= type.equals("label");
					Set<String> entrys;
					Combo combo= null;
					Label label= null;
					
					if(bLabel)
						label= (Label)m_oComponent;
					else
						combo= (Combo)m_oComponent;					
					entrys= m_asComboValueEntrys.keySet();
					for(String entry : entrys)
					{
						Double akt= m_asComboValueEntrys.get(entry);
						if(akt.equals(m_nAktValue))
						{
							if(bLabel)
							{
								bfound= true;
								label.setText(entry);//calculateInputValue());
								label.pack(true);
								
							}else
							{
								bfound= true;
								combo.setText(entry);
							}
							break;
						}
					}
					if(!bfound)
					{
						if(bLabel)
						{
							label.setText("");
							label.pack(true);
							
						}else
							combo.deselectAll();
					}
				}
			
			}, this.type.toString());
			
		}else if(m_oComponent instanceof List)
		{
			final int min= this.min;
			final int max= this.max;
			
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					int value;
					List list= (List)m_oComponent;
					
					value= (int)m_nAktValue;
					if(	value < min ||
						value > max		)
					{
						list.deselectAll();
						
					}else
						list.select(value);
				}
			
			}, this.type.toString());
			
		}else if(m_oComponent instanceof Spinner)
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Spinner spinner= (Spinner)m_oComponent;
					
					spinner.setSelection((int)m_nAktValue);
				}
			
			}, this.type.toString());
		}
	}
}
