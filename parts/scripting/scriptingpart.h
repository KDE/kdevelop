/***************************************************************************
 *   Copyright (C) 2005 by ian reinhart geiser                             *
 *   ian@geiseri.com                                                       *
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

#ifndef KDEVSCRIPTING_H
#define KDEVSCRIPTING_H

#include <kdevplugin.h>

#include <qguardedptr.h>

class QPopupMenu;
class KAction;
class KDialogBase;
class Context;
class ConfigWidgetProxy;
class KScriptActionManager;
/**
Please read the README.dox file for more info about this part
*/
class scriptingPart: public KDevPlugin
{
    Q_OBJECT
public:
    scriptingPart(QObject *parent, const char *name, const QStringList &args);
    ~scriptingPart();

    public slots:
        void setupActions();
private slots:
    void init();
    void insertConfigWidget(const KDialogBase *dlg, QWidget *page, unsigned int pageNo);
    
    

private:
    ConfigWidgetProxy *m_configProxy;
    KScriptActionManager *m_scripts;
};

#endif
