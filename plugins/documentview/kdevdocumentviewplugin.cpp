/*
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
        Qt::DockWidgetArea defaultPosition() const override
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

KDevDocumentViewPlugin::KDevDocumentViewPlugin(QObject* parent, const KPluginMetaData& metaData,
                                               const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevdocumentview"), parent, metaData)
{
    Q_UNUSED( args );

    factory = new KDevDocumentViewPluginFactory( this );

    core()->uiController()->addToolView(i18nc("@title:window", "Documents"), factory);

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
#include "moc_kdevdocumentviewplugin.cpp"
