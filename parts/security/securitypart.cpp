/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#include "securitypart.h"

#include <klocale.h>
#include <kaction.h>

#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>

#include <kdevpartcontroller.h>
#include <kdevplugininfo.h>
#include <kdevgenericfactory.h>
#include <kdevmainwindow.h>
#include <urlutil.h>

#include "securitywidget.h"
#include "securitychecker.h"

typedef KDevGenericFactory<SecurityPart> SecurityFactory;
KDevPluginInfo data("kdevsecurity");
K_EXPORT_COMPONENT_FACTORY( libkdevsecurity, SecurityFactory( data ) )

SecurityPart::SecurityPart(QObject *parent, const char *name, const QStringList &/*args*/)
    :KDevPlugin(&data, parent, name ? name : "SecurityPart"), 
    m_activeEditor(0), m_activeViewCursor(0)
{    
    m_widget = new SecurityWidget(this, "Security Widget");
    m_widget->setCaption(i18n("Security Widget"));
    m_widget->setIcon(info()->icon());
    
    m_checker = new SecurityChecker(this);
        
//    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)),
//        this, SLOT(activePartChanged(KParts::Part*)));
    
    mainWindow()->embedOutputView(m_widget, "Security Problems", i18n("Potential security problems") );
}

SecurityPart::~SecurityPart()
{
	if ( m_widget )
	{
		mainWindow()->removeView( m_widget );
	}

    delete m_widget;
}

void SecurityPart::activePartChanged(KParts::Part *part)
{
    if (m_activeEditor)
    {
        disconnect(m_activeEditor, 
            SIGNAL(charactersInteractivelyInserted (int,int,const QString &)), 
            m_checker, SLOT(refresh( int, int, const QString& )));
    }
    
    kdDebug() << "SecurityPart::activePartChanged()" << endl;

    m_activeEditor = part;
    m_activeFileName = QString::null;

    KTextEditor::Document *m_activeDocument = dynamic_cast<KTextEditor::Document*>(part);
    if (m_activeDocument)
        m_activeFileName = URLUtil::canonicalPath(m_activeDocument->url().path());
    KTextEditor::View* m_activeView = part ? dynamic_cast<KTextEditor::View*>( part->widget() ) : 0;
    m_activeViewCursor = part ? dynamic_cast<KTextEditor::ViewCursorInterface*>( m_activeView ) : 0;
    m_activeEditInterface = dynamic_cast<KTextEditor::EditInterface*>(part);

    if (dynamic_cast<KTextEditor::EditInterface*>(m_activeEditor))
    {
        m_checker->check();
        connect(partController(), SIGNAL(savedFile( const KURL& ) ),
            m_checker, SLOT(check()));
    }
    else
        m_activeEditor = 0;
}

#include "securitypart.moc"
