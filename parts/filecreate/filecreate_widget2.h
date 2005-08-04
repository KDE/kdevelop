/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __FILECREATE_WIDGET2_H__
#define __FILECREATE_WIDGET2_H__

#include <kdeversion.h>
#include <q3table.h>
#include <qstring.h>
#include <qmap.h>

//#include <klistview.h>

#include "filecreate_typechooser.h"

class KIconLoader;
class KDevProject;
class FileCreatePart;

namespace FileCreate {

  class FileType;

  class FriendlyWidget : public Q3Table, public TypeChooser
    {
      Q_OBJECT

      public:

        FriendlyWidget(FileCreatePart *part);
        virtual ~FriendlyWidget();

	/**
	 * Refreshes the widget with the part's file type list.
	 */
	virtual void refresh();
	/**
	 * Sets the currently highlighted file type.
	 */
	virtual void setCurrent(const FileType * current);

#if QT_VERSION < 0x030100
        void selectRow(int row);
#endif

      protected:
	/**
	 * Sets row <i>row</i> to filetype <i>filetype</i>.
	 */
	virtual void setRow(int row, FileType * filetype);
	/**
	 * Clears the table.
	 */
	virtual void empty();
	/**
	 * Sets some default column widths.
	 */
	virtual void setDefaultColumnWidths();
	/**
	 * Scans the cells in the table and adjusts their size based
	 * on their contents.
	 */
	virtual void resizeCells();
	/**
	 * Sets the height of a row to the highest
	 * cell in the row.
	 */
	virtual void resizeRow(int row);
	/**
	 * Sets the width of a column to the widest
	 * cell in the column.
	 */
	virtual void resizeColumn(int col);

	/**
	 * The default icon loader, here for convenience.
	 */
	KIconLoader * m_iconLoader;
	/**
	 * A mapping of each row number to a file type.
	 */
	QMap<int,FileType*> typeForRow;
	/**
	 * The currently selected file type, or null if none.
	 */
	FileType * m_selected;

      protected slots:
	/**
	 * When a cell is selected by the user.
	 */
	virtual void slotCellSelected(int row, int col);
	/**
	 * Invoked as a single shot after slotCellSelected, with m_selected
	 * set to the selected file type, so that
	 * the GUI will be updated before the selection event
	 * takes place.
	 */
	virtual void slotDoSelection();


    };

}

#endif
