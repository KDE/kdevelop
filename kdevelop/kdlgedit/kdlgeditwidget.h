/***************************************************************************
                          kdlgeditwidget.h  -  description                              
                             -------------------                                         
    begin                : Wed Mar 17 1999                                           
    copyright            : (C) 1999 by Pascal Krahmer
    email                : pascal@beast.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KDLGEDITWIDGET_H
#define KDLGEDITWIDGET_H

#include <qwidget.h>


/**
  *@author Pascal Krahmer
  */

class KDlgItemDatabase;
class KDlgItem_Widget;
class KDlgItem_Base;
class CKDevelop;
class KRuler;
class QTextStream;
//class KQuickHelpWindow;

/**
 * This is the edit widget you can see in the center of the kdevelop window.
*/
class KDlgEditWidget : public QWidget  {
  Q_OBJECT
  public:
    KDlgEditWidget(CKDevelop* parCKD, QWidget *parent=0, const char *name=0);
    ~KDlgEditWidget();

    /**
     * Adds an item to the dialog and asks where to add it if nessessary.
     * The desired type of widget is given to the first parameter as string (i.e. "QLineEdit").
    */
    bool addItem(QString);

    /**
     * Adds an item to the specified item.
     * The desired type of widget is given to the second parameter as string (i.e. "QLineEdit").
    */
    KDlgItem_Widget *addItem(KDlgItem_Base*, QString);

    /**
     * returns a pointer to the main widget.
    */
    KDlgItem_Widget *mainWidget() { return main_widget; }

    /**
     * sets the pointer to the main widget.
    */
//    void setMainWidget(KDlgItem_Widget *mw) { main_widget = mw; }

    /**
     * returns a pointer to the selected widget or 0 if none is selected.
    */
    KDlgItem_Base *selectedWidget() { return selected_widget; }

    /**
     * selects the specified item.
    */
    void selectWidget(KDlgItem_Base*);

    /**
     * deselects the selected item.
    */
    void deselectWidget();

    /**
     * returns a pointer to the CKDevelop class which has created this class.
    */
    CKDevelop *getCKDevel() { return pCKDevel; }

    KRuler *horizontalRuler() { return rulh; }
    KRuler *verticalRuler() { return rulv; }

    /**
     * saves the complete dialog to the file <i>fname</i>
    */
    bool saveToFile( QString fname );

    /**
     * saves the item <i>wid</i> to the textstream <i>t</i> with <i>deep</i>*2 spaces before every line.
    */
    void saveWidget( KDlgItem_Widget *wid, QTextStream *t, int deep = 0);

    /**
     * opens a dialog from the file <i>fname</i>
    */
    bool openFromFile( QString fname );

    int raiseSelected(bool updateMe = true);
    int lowerSelected(bool updateMe = true);
    int gridSizeX() { return grid_size_x; }
    int gridSizeY() { return grid_size_y; }
    void setGridSize(int x, int y);
    bool isModified() { return is_modified; }
    void setModified(bool b = true) { is_modified = b; }
    bool wasWidgetAdded() { return was_widgetadded; }
    void setWidgetAdded(bool b = true) { was_widgetadded = b; }
    bool wasWidgetRemoved() { return was_widgetremoved; }
    void setWidgetRemoved(bool b = true) { was_widgetremoved = b; }
    bool wasVarnameChanged() { return was_varnamechanged; }
    void setVarnameChanged(bool b = true) { was_varnamechanged = b; }
    void newDialog();
  public slots:
    int slot_raiseSelected() { return raiseSelected(); }
    int slot_lowerSelected() { return lowerSelected(); }
    void slot_raiseTopSelected();
    void slot_lowerBottomSelected();
    void slot_cutSelected();
    void slot_deleteSelected();
    void slot_copySelected();
    void slot_pasteSelected();
    void slot_helpSelected();

  protected:
    class typeCount
    {
      public:
        typeCount() { clear(); }

        void clear();
        int returnCount(QString type);
        void addType(QString type);
        void increase(QString type);
        void decrease(QString type);
        QString countString(QString type);

      protected:
        QString types[64];
        int counts[64];
    } typeCounter;


    int dlgfilelinecnt;
    int grid_size_x, grid_size_y;

    QString dlgReadLine( QTextStream *t );
    bool readGrp_Ignore( QTextStream *t );
    bool readGrp_Information( QTextStream *t );
    bool readGrp_SessionManagement( QTextStream *t );
    bool readGrp_Item( KDlgItem_Widget* par, QTextStream *t, QString ctype );
    bool readGroup( QTextStream *t );


    virtual void resizeEvent ( QResizeEvent * );


//    KQuickHelpWindow *qhw;
    KDlgItemDatabase *dbase;
    KDlgItem_Widget *main_widget;
    KDlgItem_Base *selected_widget;
    CKDevelop *pCKDevel;
    KRuler *rulh, *rulv;
    bool is_modified;
    bool was_widgetadded;
    bool was_widgetremoved;
    bool was_varnamechanged;
};

#endif
