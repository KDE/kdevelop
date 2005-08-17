/***************************************************************************
 *   Copyright (C) 2005 by Mathieu Chouinard                               *
 *   mchoui@e-c.qc.ca                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KDEVANNOTATION_H
#define KDEVANNOTATION_H

#include <kdevplugin.h>

#include <qpointer.h>
//Added by qt3to4:
#include <Q3PopupMenu>

class Q3PopupMenu;
class KAction;
class KDialogBase;
class Context;
class ConfigWidgetProxy;
class annotationWidget;
class KAboutData;
class CodeModelItemContext;
/**
Please read the README.dox file for more info about this part
*/
class annotationPart: public KDevPlugin
{
    Q_OBJECT
public:
    annotationPart(QObject *parent, const char *name, const QStringList &args);
    ~annotationPart();
    KAboutData* aboutData();
  
private slots:
    void init();
    
    void insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo);
    void contextMenu(Q3PopupMenu *popup, const Context *context);
    void projectOpened();
    void projectClosed();
    
    void doSomething();

private:
    void setupActions();
    
    KAction *action;
    
    QString m_itemAnnotationName;
    QString m_itemAnnotationFilename;
    
    QPointer<annotationWidget> m_widget;
    ConfigWidgetProxy *m_configProxy;
public slots:
    void SlotDoAnnotate();
    void SlotDoAnnotate(QString itemname);
};

#endif
