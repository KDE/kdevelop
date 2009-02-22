/***************************************************************************
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>                    *
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
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef DOCUMENTSWITCHERPLUGIN_H
#define DOCUMENTSWITCHERPLUGIN_H

#include <interfaces/iplugin.h>
#include <QtCore/QVariant>

namespace Sublime 
{
    class View;
    class MainWindow;
    class AreaIndex;
    class Area;
}

class QSignalMapper;

class DocumentSwitcherPlugin: public KDevelop::IPlugin {
    Q_OBJECT
public:
    DocumentSwitcherPlugin( QObject *parent, const QVariantList &args = QVariantList() );
    ~DocumentSwitcherPlugin();
    
    virtual void unload();
    
private slots:
    void changeView( Sublime::View* );
    void addMainWindow( Sublime::MainWindow* );
    void changeArea( Sublime::Area* );
    void removeView( Sublime::View* );
    void removeMainWindow(QObject*);
private:
    void storeAreaViewList( Sublime::MainWindow* mainwindow, Sublime::Area* area );
    QMap<Sublime::MainWindow*, QMap<Sublime::Area*, QList<Sublime::View*> > > documentLists;
};

#endif

