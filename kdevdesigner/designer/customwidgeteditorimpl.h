/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef CUSTOMWIDGETEDITORIMPL_H
#define CUSTOMWIDGETEDITORIMPL_H

#include "customwidgeteditor.h"
#include "metadatabase.h"

#include <qmap.h>
#include <qobject.h>

class QTimer;
class Q3ListBoxItem;
class MainWindow;

class CustomWidgetEditor : public CustomWidgetEditorBase
{
    Q_OBJECT

public:
    CustomWidgetEditor( QWidget *parent , MainWindow *mw);

protected slots:
    void addWidgetClicked();
    void classNameChanged( const QString & );
    void currentWidgetChanged( Q3ListBoxItem * );
    void deleteWidgetClicked();
    void headerFileChanged( const QString & );
    void heightChanged( int );
    void includePolicyChanged( int );
    void pixmapChoosen();
    void widthChanged( int );
    void chooseHeader();
    void checkWidgetName();
    void closeClicked();
    void currentSignalChanged( Q3ListBoxItem *i );
    void addSignal();
    void removeSignal();
    void signalNameChanged( const QString &s );
    void slotAccessChanged( const QString & );
    void slotNameChanged( const QString & );
    void addSlot();
    void removeSlot();
    void currentSlotChanged( Q3ListViewItem * );
    void propertyTypeChanged( const QString & );
    void propertyNameChanged( const QString & );
    void addProperty();
    void removeProperty();
    void currentPropertyChanged( Q3ListViewItem * );
    void saveDescription();
    void loadDescription();
    void horDataChanged( int );
    void verDataChanged( int );
    void widgetIsContainer( bool );
    
private:
    MetaDataBase::CustomWidget *findWidget( Q3ListBoxItem *i );
    void setupDefinition();
    void setupSignals();
    void setupSlots();
    void setupProperties();
    void updateCustomWidgetSizes();

private:
    QMap<Q3ListBoxItem*, MetaDataBase::CustomWidget*> customWidgets;
    QString oldName;
    QTimer *checkTimer;
    Q3ListBoxItem *oldItem;
    MainWindow *mainWindow;
    QObjectList cwLst;

};

#endif
