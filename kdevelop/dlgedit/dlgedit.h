/***************************************************************************
                          dlgedit.h  -  description
                             -------------------
    begin                : Thu Jan 20 2000
    copyright            : (C) 2000 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGEDIT_H
#define DLGEDIT_H

#include <qlist.h>
#include "../component.h"
#include "widgetsview.h"
#include "propertyview.h"

class MdiFrame;
class DialogWidget;

/* handler class for propview, dialogviews and widgetsview
  *@author
  */

class DlgEdit : public QObject, public Component  {
    Q_OBJECT
 public: 
    DlgEdit(WidgetsView* _widgets_view,PropertyView* _prop_view);
    virtual	~DlgEdit();
    /** sets the current focused dialogwidget */
    void   setCurrentDialogWidget(DialogWidget* dialog_widget);

    public slots:
	void slotViewGridDlg();
    void slotViewPreview();
    void slotEditCopy();
    void slotEditPaste();
    void slotEditCut();
    void slotEditSelectAll();
    void slotEditDeselectAll();
    void slotFileSave();
    void slotBuildGenerate();
    
    
 private:
    PropertyView* prop_view;
    WidgetsView* widgets_view;
    QList<DialogWidget> dlgwidgets;
};

#endif
