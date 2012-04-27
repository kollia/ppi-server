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
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Scale;
import org.eclipse.swt.widgets.Slider;
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;

import org.apache.regexp.RE;

import at.kolli.automation.client.MsgClientConnector;
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
	 * variable is an valid text object
	 */
	private Text m_oText= null;
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
	 * minimal-value for component slider and scale
	 */
	public int min= 0;
	/**
	 * max attribute of component.<br />
	 * maximal-value for component slider and scale
	 */
	public int max= 1500000;
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
	 * @param classes all class definition for any tags
	 * @override
	 * @author Alexander Kolli
	 * @version 1.00.00, 04.12.2007
	 * @since JDK 1.6
	 */
	public void execute(Composite composite, HashMap<String, HtmTags> classes) throws IOException
	{
		boolean disabled= false;
		boolean readonly= false;
		MsgClientConnector client= MsgClientConnector.instance();

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
			if(!isSoftButton(classes))
				askPermission(null);
			if(	actLayout.compareTo(layout.disabled) == 0 ||
				!m_bDeviceAccess								)
			{
				disabled= true;
			}
			if(actLayout.compareTo(layout.readonly) == 0)
				readonly= true;
		}
		
		if(	this.type.equals("checkbox")
			||
			this.type.equals("radio")
			||
			this.type.equals("button")
			||
			this.type.equals("togglebutton")
			||
			this.type.equals("upbutton")
			||
			this.type.equals("leftbutton")
			||
			this.type.equals("rightbutton")
			||
			this.type.equals("downbutton")	)
		{
			int type= 0;
			GridData data= null;
			Double akt= null;
			String firstButtonValue= "";
			
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
				new Composite(composite, SWT.NONE);
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
			{
				data= new GridData();
				data.widthHint= width;
			}
			if(height != -1)
			{
				if(data == null)
					data= new GridData();
				data.heightHint= height;
			}
			if(data != null)
				m_oButton.setLayoutData(data);
			m_oComponent= m_oButton;
			if(	type == SWT.PUSH ||
				type == SWT.TOGGLE	)
			{
				m_oButton.setText(this.value);
			}//else
				//button.setText(this.text);
			if(type == SWT.RADIO
				&&
				this.value.equals("0") 	)
			{
				m_oButton.setSelection(true);
			}
			
		}else if(this.type.equals("text"))
		{
			boolean bread= false;
			int style= readonly ? SWT.SINGLE | SWT.READ_ONLY : SWT.SINGLE;
			m_oText= new Text(composite, style);
			GridData data= new GridData();
			//RE floatStr= new RE("([ +-/]|\\*|\\(|\\)|^)#([0-9])+(\\.([0-9])*)?([ +-/]|\\*|\\(|\\)|$)");
			//RE floatStr= new RE("(.*)(\\*)(#([0-9])+(.[0-9]*)?)?(.*)");
			//RE floatStr= new RE("(.*)((#[0-9]+)(.[0-9]*)?)?(.*)");
			RE floatStr= new RE("([\\\\]*)#([0-9]+)(.([0-9]*))?");
			textFormat formatObj;
			String[] spl;
			
			m_aResult= new ArrayList<Component.textFormat>();
			spl= this.result.split(",");
			for (String res : spl)
			{
				formatObj= new textFormat();
				formatObj.resultStr= res.trim();
				m_aResult.add(formatObj);
			}
			if(!isSoftButton(classes))
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
						
						bread= true;
						if((count+1) > m_aResult.size())
						{
							formatObj= new textFormat();
							formatObj.correctResultString= false;
							m_aResult.add(formatObj);
							
						}else
							formatObj= m_aResult.get(count);
						formatObj.beforeValue+= this.value.substring(0, floatStr.getParenStart(0));
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
								
							}else if(	b != null &&
										b.equals(".")	)
							{
								formatObj.numBehind= -1;
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
						formatObj.beforeValue= formatObj.beforeValue.replaceAll("\\\\\\\\", "\\\\");
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
			if(height != -1)
				data.heightHint= height;
			m_oComponent= m_oText;
			data.widthHint= width;
			m_oText.setLayoutData(data);
			if(!m_nSoftButton)
				m_oText.setText(calculateInputValue());
			m_oText.setEnabled(!disabled);
			
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
			slider.setMinimum(min);
			slider.setSize(width, height);
			min= slider.getMinimum();
			// toDo:	search reason why maximum need 
			//			10 values more than minimum
			slider.setMaximum(max+10);
			max= slider.getMaximum();
			// Pfeiltaste
			slider.setIncrement(arrowkey);
			// klick auf Schieberegler
			slider.setPageIncrement(rollbarfield);
			// Aktuaelle Position
			slider.setSelection(value);
			slider.setEnabled(!disabled);
			m_oComponent= slider;
			
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
			scale.setMinimum(min);
			scale.setMaximum(max);
			scale.setSize(width, height);
			// Pfeiltaste
			scale.setIncrement(arrowkey);
			// klick auf Schieberegler
			scale.setPageIncrement(rollbarfield);
			// Aktuaelle Position
			scale.setSelection(value);
			scale.setEnabled(!disabled);			
			m_oComponent= scale;
			m_nAktValue= (int)min;
			
		}else if(	this.type.equals("combo")
					&&
					this.size == 1				)
		{
			Combo combo= new Combo(composite, SWT.READ_ONLY | SWT.DROP_DOWN);
			GridData data= null;
			double value= -1;
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
			m_oComponent= combo;
			if(width != -1)
			{
				data= new GridData();
				data.widthHint= width;
			}
			if(height != -1)
			{
				if(data == null)
					data= new GridData();
				data.heightHint= height;
			}
			if(data != null)
				combo.setLayoutData(data);
			combo.setEnabled(!disabled);
			if(	!m_lContent.isEmpty())
			{
				boolean bfirst= true;
				
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
						combo.add(entry);
						if(	(	akt == null &&
								bfirst == true	) ||
							(	akt != null &&
								akt.equals(m_asComboValueEntrys.get(entry))	)
																)
						{
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
			List list= new List(composite, m_lContent.size() > this.size ? SWT.SINGLE | SWT.V_SCROLL : SWT.SINGLE);
			GridData data= new GridData();
			int item= 0;
			double value= -1;
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
			{
				data= new GridData();
				data.widthHint= width;
			}
			data.heightHint= list.getItemHeight()  * this.size;
			//data.heightHint= (list.computeSize(SWT.DEFAULT, SWT.DEFAULT, true).x/4) * this.size;
			if(data != null)
				list.setLayoutData(data);
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
						//list.setBackground(SWT.COLOR_WHITE);
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
			final Spinner spinner= new Spinner(composite, style);
			GridData data= new GridData();
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
			{
				data= new GridData();
				data.widthHint= width;
			}
			if(height != -1)
			{
				data.heightHint= height;
			}
			if(data != null)
				spinner.setLayoutData(data);
			spinner.setEnabled(!disabled);
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
		if(formatObj == null)
			super.setPermission(perm);
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
					actLayout= normal;
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
		
		if(	(	formatObj != null &&
				formatObj.resultStr.equals("") ) ||
			(	formatObj == null &&
				this.result.equals("")	)			)
		{
			setPermission(permission.None, formatObj);
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
				
		}else
		{	
			eRv= permission.None;
			setPermission(eRv, formatObj);
			System.out.println(client.getErrorMessage());
			if(client.getErrorCode().equals("ERROR 016"))
			{
				if(formatObj != null)
					formatObj.deviceAccess= false;
				else
					m_bDeviceAccess= false;
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
	private boolean isSoftButton(HashMap<String, HtmTags> classes)
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
			sBrowserClass= result.substring(15);
			result= result.substring(2, 14);
			
		}else if(	result.length() > 18 &&
					(	result.substring(0, 18).equals("::browser_refresh@") ||
						result.substring(0, 18).equals("::browser_forward@")	)	)
		{
			sBrowserClass= result.substring(18);
			result= result.substring(2, 17);
			
		}else if(	result.length() > 14 &&
					result.substring(0, 14).equals("::browser_url@"))
		{
			String[] resUrl;
			
			sBrowserClass= result.substring(14);
			result= result.substring(2, 13);
			resUrl= sBrowserClass.split("@");	
			if(resUrl.length == 2)
			{
				sBrowserClass= resUrl[0];
				result+= "@" + resUrl[1];
			}
			
		}else if(	result.length() > 2 &&
					result.substring(0, 2).equals("::")	)
		{
			System.out.println("### ERROR: unknown soft button '" + result + "' be set");
			result= "";
		}
		if(!sBrowserClass.equals(""))
		{
			if(	(	type.equals("text") &&
					(	result.equals("browser_url") ||
						result.equals("browser_info")	)	) ||
				(	result.equals("browser_load") &&
					(	type.equals("checkbox") ||
						type.equals("radio") ||
						type.equals("togglebutton")	)	) ||
				(	!result.equals("browser_url") &&	// there is also allowed
					!result.equals("browser_info") &&	// an URL with new address
					(	type.equals("button") ||		
						type.equals("upbutton") ||
						type.equals("leftbutton") ||
						type.equals("rightbutton") ||
						type.equals("downbutton")		)	)	)
			{
				m_oSoftButtonTag= null;
				m_oSoftButtonTag= classes.get(sBrowserClass);
				if(m_oSoftButtonTag != null)
				{
					if(	result.equals("browser_info") ||
						result.equals("browser_load")	)
						setPermission(permission.readable);
					else
						setPermission(permission.writeable);
					m_nSoftButton= true;
					
				}else
				{
					System.out.println("### ERROR: cannot find td-tag with class \"" + sBrowserClass + "\"");
					System.out.println("           so do not define button as soft button");
					result= "";
				}
			}else
			{
				System.out.print("### ERROR: an soft button like '" + result + "@" + sBrowserClass + " can only be defined in ");
				if(	result.equals("browser_url") ||
					result.equals("browser_info")	)
				{
					System.out.println("an text input-tag");
					
				}else if(result.equals("browser_load"))
					System.out.println("an button-tag with type togglebutton, checkbox or radio");
				else
					System.out.println("any button-tag (button, upbutton, ...) but no togglebutton");
				result= "";
			}
		}
		return m_nSoftButton;
	}
	
	/**
	 * make result for defined soft button
	 */
	private void doSoftButton()
	{
		if(result.equals("browser_home"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
			
			tdTag.getBrowser().setUrl(tdTag.href);
			
		}else if(result.equals("browser_refresh"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
			
			tdTag.getBrowser().refresh();
			
		}else if(result.equals("browser_stop"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
			
			tdTag.getBrowser().stop();
			
		}else if(result.equals("browser_forward"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;

			m_bActiveButton= true;
			tdTag.getBrowser().forward();
			
		}else if(result.equals("browser_back"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;

			m_bActiveButton= true;
			tdTag.getBrowser().back();
			
		}else if(result.equals("browser_url"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
			
			value= m_oText.getText();
			if(value.equals(""))
			{
				if(tdTag.m_sActRef.equals(""))
					value= tdTag.href;
				else
					value= tdTag.m_sActRef;		
			}
			tdTag.getBrowser().setUrl(value);
			
		}else if(	result.length() > 12 &&
					result.substring(0, 12).equals("browser_url@"))
		{
			ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
			
			tdTag.getBrowser().setUrl(result.substring(12));	
			
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

		//System.out.println(type + " " + result);
		if(	m_bCorrectName &&
			!result.equals("") &&
			!m_nSoftButton &&
			getPermission().compareTo(permission.readable) >= 0 &&
			client.haveSecondConnection()							)
		{
			if(type.equals("text"))
			{
				for (textFormat obj : m_aResult)
				{
					if(!obj.resultStr.equals(""))
						client.hear(obj.resultStr, /*throw*/true);
				}
			}else
				client.hear(result, /*bthrow*/true);
		}
		if(	this.type.equals("combo") ||
			(	getPermission().equals(permission.readable) &&
				(	this.type.equals("text") &&
					result.equals("browser_info")	) ||
				(	result.equals("browser_load") &&
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
		
		haveListener= true;
		if(	this.type.equals("checkbox")
			||
			this.type.equals("radio")
			||
			this.type.equals("togglebutton")	)
		{
			if(m_nSoftButton)
			{
				ContentFields tdTag= (ContentFields)m_oSoftButtonTag;
				
				tdTag.getBrowser().addProgressListener(m_eProgressListener= new ProgressListener()
				{
					@Override
					public void changed(ProgressEvent arg0)
					{
						if (arg0.total == arg0.current) return;
						((Button)m_oComponent).setSelection(true);
					}
					
					@Override
					public void completed(ProgressEvent arg0)
					{
						((Button)m_oComponent).setSelection(false);
					}
					
				});
				
			}else
			{
				String sValue= "0";
				if(this.type.equals("radio"))
					sValue= this.value;
				final String gType= this.type;
				final short gValue= Short.parseShort(sValue);
				
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
			m_oComponent.addListener(SWT.MouseDown, m_eListener1= new Listener()
			{
			    public void handleEvent(final Event event)
			    {
			    	if(!m_nSoftButton)
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
				    }else
				    	doSoftButton();
			    }
			});
			m_oComponent.addListener(SWT.MouseUp, m_eListener2= new Listener()
			{
			    public void handleEvent(final Event event)
			    {
			    	if(!m_nSoftButton)
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
				(	result.equals("browser_forward") ||
					result.equals("browser_back")		)	)
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
						if(result.equals("browser_forward"))
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
			RE floatStr= new RE("([ +-/]|\\*|\\(|\\)|^)#([0-9])+\\.([0-9])*([ +-/]|\\*|\\(|\\)|$)");
			String patternString= "[0-9]*";
			
			if(floatStr.match(this.format))
				patternString+= "(\\.[0-9]+)?";
			//patternString+= ").*";
			
			final RE pattern= new RE(patternString);
			
			if(m_nSoftButton)
			{
				if(result.equals("browser_info"))
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
					double value;
					
					if(!m_nSoftButton)
					{
						if(pattern.match(string))
						{
							if(HtmTags.debug)
								System.out.println("user change text field " + name + " to " + string);
							string= pattern.getParen(0);
							if(!string.equals(""))
							{
								if(HtmTags.debug)
									System.out.println(" generate to number '" + string + "'");
								try{
									value= Double.parseDouble(string);
									
								}catch(NumberFormatException ex)
								{
									if(HtmTags.debug)
									{
										System.out.println("NumberFormatException for textfield " + name);
										System.out.println(" cannot convert value " + string);
										System.out.println();
									}
									value= 0;
								}
							}else
							{
								if(HtmTags.debug)
								{
									System.out.println("found no correct value for new string '" + string + "'");
									System.out.println("set value to 0");
								}
								value= 0;
							}
						}else
						{
							if(HtmTags.debug)
							{
								System.out.println("found no correct value for new string '" + string + "'");
								System.out.println("set value to 0");
							}
							value= 0;
						}
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
			
			((Combo)m_oComponent).addSelectionListener(m_eSelectionListener= new SelectionAdapter() 
			{			
				@Override
				public void widgetSelected(SelectionEvent e)
				{
					String key= ((Combo)m_oComponent).getText();
					Double value= m_asComboValueEntrys.get(key);
					
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
				}			
			});
			
		}else if(	this.type.equals("combo")
				&&
				this.size > 1					)
		{
			final permission perm= getPermission();
			
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

		if(!m_bDeviceAccess)
			return "ERROR";
		for (textFormat obj : m_aResult)
		{
			if(getPermission().equals(permission.None))
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
			Double sh= new Double(stringValue.substring(0, strPos));
			if(obj.result.equals(sh))
				bInt= true;
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
	 * This method is to remove all listeners which set bevore with addListeners()
	 * if the component is in an {@link Composite} witch is not on the top of the {@link StackLayout}
	 * 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public void removeListeners()
	{
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

		if(	!m_bCorrectName
			||
			getPermission().equals(permission.None) 	)
		{
			return;
		}
		if(	cont != null
			&&
			cont.hasValue()	)
		{
			if(this.type.equals("text"))
			{
				String folder= cont.getFolderName() + ":" + cont.getSubroutineName();
				
				if(folder.equals("time_routines:minutes"))
					System.out.print("");
				for (textFormat obj : m_aResult)
				{
					if(obj.resultStr.equals(folder))
					{
						formatObj= obj;
						break;
					}
				}
				if(formatObj == null)
					return;
				
			}else
			{
				if(!(cont.getFolderName() + ":" + cont.getSubroutineName()).equals(this.result))
					return;
			}
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
				}
			
			});
			
		}else if(this.type.equals("radio"))
		{
			final double nValue= Double.parseDouble(this.value);
			
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Button button= (Button)m_oComponent;
					boolean bSet= m_nAktValue == nValue ? true : false;
					
					button.setSelection(bSet);
				}
			
			});
			
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
			
			});
			
		}else if(this.type.equals("slider"))
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Slider slider= (Slider)m_oComponent;
					
					slider.setSelection((int)m_nAktValue);
				}
			
			});
			
		}else if(this.type.equals("range"))
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					Scale scale= (Scale)m_oComponent;
					
					scale.setSelection((int)m_nAktValue);
				}
			
			});
		}else if(m_oComponent instanceof Combo)
		{
			DisplayAdapter.asyncExec(new Runnable()
			{				
				//@Override
				public void run() 
				{
					boolean bfound= false;
					Set<String> entrys;
					Combo combo= (Combo)m_oComponent;
					
					entrys= m_asComboValueEntrys.keySet();
					for(String entry : entrys)
					{
						Double akt= m_asComboValueEntrys.get(entry);
						if(akt.equals(m_nAktValue))
						{
							bfound= true;
							combo.setText(entry);
							break;
						}
					}
					if(!bfound)
						combo.deselectAll();
				}
			
			});
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
			
			});
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
			
			});
		}
	}
}
