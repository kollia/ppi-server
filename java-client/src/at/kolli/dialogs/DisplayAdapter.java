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
package at.kolli.dialogs;

import org.eclipse.swt.widgets.Display;

import at.kolli.layout.HtmTags;

/**
 * adapter pattern for synchronize SWT syncExec and asyncExec
 * for debugging messages
 * 
 * @author Alexander Kolli
 * @version 1.0
 */
public class DisplayAdapter {

	
	public static void syncExec(Runnable runnable)
	{
		syncExec(runnable, "");
	}
	public static void syncExec(Runnable runnable, String def)
	{
		Thread t= null;
		
		if(	HtmTags.debug &&
			HtmTags.syncSWTExec	)
		{
			t= Thread.currentThread();
			System.out.println(t.getName()+" synchronize display for SWT by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}
		Display.getDefault().syncExec(runnable);
		if(	HtmTags.debug &&
			HtmTags.syncSWTExec	)
		{
			System.out.println(t.getName()+" has synchronized by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}
	}
	public static void asyncExec(Runnable runnable)
	{
		asyncExec(runnable, "");
	}
	public static void asyncExec(Runnable runnable, String def)
	{
		Thread t= null;
		
		if(	HtmTags.debug &&
			HtmTags.syncSWTExec	)
		{
			t= Thread.currentThread();
			System.out.println(t.getName()+" set asynchron display for SWT by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}
		Display.getDefault().asyncExec(runnable);
		if(	HtmTags.debug &&
			HtmTags.syncSWTExec	)
		{
			System.out.println(t.getName()+" has synchronized by " + runnable);
			if(!def.equals(""))
				System.out.println("   by defined position of " + def);
		}
	}
}
