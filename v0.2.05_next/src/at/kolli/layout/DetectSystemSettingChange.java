/*******************************************************************************
 * Copyright (c) 2000, 2006 IBM Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     IBM Corporation - initial API and implementation
 *******************************************************************************/

/* 
 * example snippet: detect a system settings change
 *
 * For a list of all SWT example snippets see
 * http://www.eclipse.org/swt/snippets/
 * 
 * @since 3.2
 */
package at.kolli.layout;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.graphics.Region;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;

public class DetectSystemSettingChange {

  public static void display() {
    final Display display = new Display();
    final Shell shell = new Shell(display);
    shell.setText("The SWT.Settings Event");
    shell.setLayout(new GridLayout());
    Label label = new Label(shell, SWT.WRAP);
    label.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false));
    label.setText("Change a system setting and the table below will be updated.");
    final Table table = new Table(shell, SWT.BORDER);
    table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
    TableColumn column = new TableColumn(table, SWT.NONE);
    column = new TableColumn(table, SWT.NONE);
    column.setWidth(150);
    column = new TableColumn(table, SWT.NONE);
    for (int i = 0; i < colorIds.length; i++) {
      TableItem item = new TableItem(table, SWT.NONE);
      Color color = display.getSystemColor(colorIds[i]);
      item.setText(0, colorNames[i]);
      item.setBackground(1, color);
      item.setText(2, color.toString());
    }
    TableColumn[] columns = table.getColumns();
    columns[0].pack();
    columns[2].pack();
    display.addListener(SWT.Settings, new Listener() {
      public void handleEvent(Event event) {
        for (int i = 0; i < colorIds.length; i++) {
          Color color = display.getSystemColor(colorIds[i]);
          TableItem item = table.getItem(i);
          item.setBackground(1, color);
        }
        TableColumn[] columns = table.getColumns();
        columns[0].pack();
        columns[2].pack();
      }
    });

    shell.pack();
    shell.open();
    while (!shell.isDisposed()) {
      if (!display.readAndDispatch())
        display.sleep();
    }
    display.dispose();
  }

  static int[] colorIds = new int[] { SWT.COLOR_INFO_BACKGROUND, SWT.COLOR_INFO_FOREGROUND,
      SWT.COLOR_LIST_BACKGROUND, SWT.COLOR_LIST_FOREGROUND, SWT.COLOR_LIST_SELECTION,
      SWT.COLOR_LIST_SELECTION_TEXT, SWT.COLOR_TITLE_BACKGROUND,
      SWT.COLOR_TITLE_BACKGROUND_GRADIENT, SWT.COLOR_TITLE_FOREGROUND,
      SWT.COLOR_TITLE_INACTIVE_BACKGROUND, SWT.COLOR_TITLE_INACTIVE_BACKGROUND_GRADIENT,
      SWT.COLOR_TITLE_INACTIVE_FOREGROUND, SWT.COLOR_WIDGET_BACKGROUND, SWT.COLOR_WIDGET_BORDER,
      SWT.COLOR_WIDGET_DARK_SHADOW, SWT.COLOR_WIDGET_FOREGROUND, SWT.COLOR_WIDGET_HIGHLIGHT_SHADOW,
      SWT.COLOR_WIDGET_LIGHT_SHADOW, SWT.COLOR_WIDGET_NORMAL_SHADOW, };

  static String[] colorNames = new String[] { "COLOR_INFO_BACKGROUND", "COLOR_INFO_FOREGROUND",
      "COLOR_LIST_BACKGROUND", "COLOR_LIST_FOREGROUND", "COLOR_LIST_SELECTION",
      "COLOR_LIST_SELECTION_TEXT", "COLOR_TITLE_BACKGROUND",
      "COLOR_TITLE_BACKGROUND_GRADIENT", "COLOR_TITLE_FOREGROUND",
      "COLOR_TITLE_INACTIVE_BACKGROUND", "COLOR_TITLE_INACTIVE_BACKGROUND_GRADIENT",
      "COLOR_TITLE_INACTIVE_FOREGROUND", "COLOR_WIDGET_BACKGROUND", "COLOR_WIDGET_BORDER",
      "COLOR_WIDGET_DARK_SHADOW", "COLOR_WIDGET_FOREGROUND", "COLOR_WIDGET_HIGHLIGHT_SHADOW",
      "COLOR_WIDGET_LIGHT_SHADOW", "COLOR_WIDGET_NORMAL_SHADOW", };
 

  public static void NonRectangularTransparency() {
	  
      Display display = new Display();
      final Image image = display.getSystemImage(SWT.ICON_WARNING);
      // Shell must be created with style SWT.NO_TRIM
      final Shell shell = new Shell(display, SWT.NO_TRIM | SWT.ON_TOP);
      shell.setBackground(display.getSystemColor(SWT.COLOR_RED));
      // define a region
      Region region = new Region();
      Rectangle pixel = new Rectangle(0, 0, 1, 1);
      for (int y = 0; y < 200; y += 2) {
        for (int x = 0; x < 200; x += 2) {
          pixel.x = x;
          pixel.y = y;
          region.add(pixel);
        }
      }
      // define the shape of the shell using setRegion
      shell.setRegion(region);
      Rectangle size = region.getBounds();
      shell.setSize(size.width, size.height);
      shell.addPaintListener(new PaintListener() {
        public void paintControl(PaintEvent e) {
          Rectangle bounds = image.getBounds();
          Point size = shell.getSize();
          e.gc.drawImage(image, 0, 0, bounds.width, bounds.height, 10, 10, size.x - 20, size.y - 20);
        }
      });
      shell.open();
      while (!shell.isDisposed()) {
        if (!display.readAndDispatch())
          display.sleep();
      }
      region.dispose();
      display.dispose();
  }
}
