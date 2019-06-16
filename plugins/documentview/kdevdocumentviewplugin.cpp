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

#include "kdevdocumentviewplugin.h"
#include "kdevdocumentview.h"
#include "kdevdocumentmodel.h"
#include "kdevdocumentselection.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevDocumentViewFactory, "kdevdocumentview.json", registerPlugin<KDevDocumentViewPlugin>();)

class KDevDocumentViewPluginFactory: public KDevelop::IToolViewFactory
{
    public:
        explicit KDevDocumentViewPluginFactory( KDevDocumentViewPlugin *plugin ): m_plugin( plugin )
        {}
        QWidget* create( QWidget *parent = nullptr ) override
        {
            auto* view = new KDevDocumentView( m_plugin, parent );
            KDevelop::IDocumentController* docController = m_plugin->core()->documentController();
            const auto openDocuments = docController->openDocuments();
            for (KDevelop::IDocument* doc : openDocuments) {
                view->opened( doc );
            }
            QObject::connect( docController, &IDocumentController::documentActivated,
                    view, &KDevDocumentView::activated );
            QObject::connect( docController, &IDocumentController::documentSaved,
                    view, &KDevDocumentView::saved );
            QObject::connect( docController, &IDocumentController::documentOpened,
                    view, &KDevDocumentView::opened );
            QObject::connect( docController, &IDocumentController::documentClosed,
                    view, &KDevDocumentView::closed );
            QObject::connect( docController,
                    &IDocumentController::documentContentChanged,
                    view, &KDevDocumentView::contentChanged );
            QObject::connect( docController,
                    &IDocumentController::documentStateChanged,
                    view, &KDevDocumentView::stateChanged );
            QObject::connect( docController,
                    &IDocumentController::documentUrlChanged,
                    view, &KDevDocumentView::documentUrlChanged );
            return view;
        }
        Qt::DockWidgetArea defaultPosition() override
        {
            return Qt::LeftDockWidgetArea;
        }

        QString id() const override
        {
            return QStringLiteral("org.kdevelop.DocumentsView");
        }

    private:
        KDevDocumentViewPlugin* m_plugin;
};


KDevDocumentViewPlugin::KDevDocumentViewPlugin( QObject *parent, const QVariantList& args )
        : KDevelop::IPlugin( QStringLiteral( "kdevdocumentview" ), parent )
{
    Q_UNUSED( args );

    factory = new KDevDocumentViewPluginFactory( this );

    core()->uiController()->addToolView( i18n("Documents"), factory );

    setXMLFile( QStringLiteral( "kdevdocumentview.rc" ) );
}

KDevDocumentViewPlugin::~KDevDocumentViewPlugin()
{
}

void KDevDocumentViewPlugin::unload()
{
    core()->uiController()->removeToolView( factory );
}

#include "kdevdocumentviewplugin.moc"

