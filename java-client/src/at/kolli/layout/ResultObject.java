package at.kolli.layout;

import java.util.ArrayList;

import org.apache.regexp.RE;

/**
 * class representing one or more results for an component
 * 
 * @package at.kolli.layout
 * @author Alexander Kolli
 * @version 0.2.00, 19.01.2013
 * @since JDK 1.6
 */
public class ResultObject 
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
	 * structure of result string
	 * 
	 * @author Alexander Kolli
	 * @version 0.2.00, 19.01.2013
	 * @since JDK 1.6
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
	 * create instance of all components whitch can display in the window
	 * 
	 * @param holeResult result attribute defined inside layout file 
	 * @author Alexander Kolli
	 * @version 1.00.00, 09.12.2007
	 * @since JDK 1.6
	 */
	public ResultObject(String holeResult)
	{
		String[] spl;
		textFormat formatObj;
		
		m_aResult= new ArrayList<textFormat>();
		spl= holeResult.split(",");
		for (String res : spl)
		{
			formatObj= new textFormat();
			formatObj.resultStr= res.trim();
			m_aResult.add(formatObj);
		}
	}

}
