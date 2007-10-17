/*
* This file is part of KDevelop
*
* Copyright 2006 Adam Treat <treat@kde.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kdevdocumentview_part.h"
#include "kdevdocumentviewdelegate.h"
#include "kdevdocumentview.h"
#include "kdevdocumentmodel.h"
#include "kdevdocumentselection.h"

#include <QtGui/QVBoxLayout>

#include <kaction.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kaboutdata.h>
#include <kgenericfactory.h>

#include <icore.h>
#include <iuicontroller.h>

K_PLUGIN_FACTORY(KDevDocumentViewFactory, registerPlugin<KDevDocumentViewPart>(); )
K_EXPORT_PLUGIN(KDevDocumentViewFactory("kdevdocumentview"))

class KDevDocumentViewPartFactory: public KDevelop::IToolViewFactory
{
    public:
        KDevDocumentViewPartFactory( KDevDocumentViewPart *part ): m_part( part )
        {}
        virtual QWidget* create( QWidget *parent = 0 )
        {
            KDevDocumentView* view = new KDevDocumentView( m_part, parent );
            KDevelop::IDocumentController* docController = m_part->core()->documentController();
            QObject::connect( docController, SIGNAL( documentActivated( KDevelop::IDocument* ) ),
                    view, SLOT( activated( KDevelop::IDocument* ) ) );
            QObject::connect( docController, SIGNAL( documentSaved( KDevelop::IDocument* ) ),
                    view, SLOT( saved( KDevelop::IDocument* ) ) );
            QObject::connect( docController, SIGNAL( documentLoaded( KDevelop::IDocument* ) ),
                    view, SLOT( loaded( KDevelop::IDocument* ) ) );
            QObject::connect( docController, SIGNAL( documentClosed( KDevelop::IDocument* ) ),
                    view, SLOT( closed( KDevelop::IDocument* ) ) );
            QObject::connect( docController,
                    SIGNAL( documentContentChanged( KDevelop::IDocument* ) ),
                    view, SLOT( contentChanged( KDevelop::IDocument* ) ) );
            QObject::connect( docController,
                    SIGNAL( documentStateChanged( KDevelop::IDocument* ) ),
                    view, SLOT( stateChanged( KDevelop::IDocument* ) ) );
            return view;
        }
        virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
        {
            return Qt::RightDockWidgetArea;
        }
    private:
        KDevDocumentViewPart* m_part;
};


KDevDocumentViewPart::KDevDocumentViewPart( QObject *parent, const QVariantList& args )
        : KDevelop::IPlugin( KDevDocumentViewFactory::componentData(), parent )
{

    factory = new KDevDocumentViewPartFactory( this );

    core()->uiController()->addToolView( "Document View", factory );

    setXMLFile( "kdevdocumentview.rc" );
}

KDevDocumentViewPart::~KDevDocumentViewPart()
{
}

Qt::DockWidgetArea KDevDocumentViewPart::dockWidgetAreaHint() const
{
    return Qt::LeftDockWidgetArea;
}

bool KDevDocumentViewPart::isCentralPlugin() const
{
    return true;
}



#include "kdevdocumentview_part.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
