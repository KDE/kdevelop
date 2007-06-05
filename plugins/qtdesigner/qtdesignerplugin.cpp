/***************************************************************************
 *   Copyright (C) 2005 by Roberto Raggi <roberto@kdevelop.org>            *
 *   Copyright (C) 2005 by Harald Fernengel <harry@kdevelop.org>           *
 *   Copyright (C) 2006 by Matt Rogers <mattr@kde.org>                     *
 *   Copyright (C) 2007 by Andreas Pakulat <apaku@gmx.de>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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
#include "qtdesignerplugin.h"

#include <QObject>
#include <QAction>
#include <QFile>
#include <QTextStream>
#include <QtDesigner/QtDesigner>
#include <QtDesigner/QDesignerComponents>
#include <QPluginLoader>
#include <QWorkspace>

#include <kaboutdata.h>
#include <kaction.h>
#include <kmimetype.h>
#include <kxmlguiwindow.h>
#include <kparts/genericfactory.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <ksavefile.h>
#include <kstandardaction.h>
#include <kicon.h>
#include <kactioncollection.h>

#include <icore.h>
#include <idocumentcontroller.h>
#include <iuicontroller.h>
#include "qtdesignerdocument.h"
#include "internals/qdesigner_integration_p.h"

typedef KGenericFactory<QtDesignerPlugin> QtDesignerPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kdevqtdesigner, QtDesignerPluginFactory("kdevqtdesigner"))

class QtDesignerDocumentFactory : public KDevelop::IDocumentFactory
{
public:
    QtDesignerDocumentFactory(QtDesignerPlugin* plugin)
        : KDevelop::IDocumentFactory(), m_plugin(plugin)
    {
    }

    KDevelop::IDocument* create( const KUrl& url, KDevelop::ICore* core)
    {
        kDebug(9000) << "creating doc for designer?" << endl;
        KMimeType::Ptr mimetype = KMimeType::findByUrl(url);
        kDebug(9000) << "mimetype for " << url << " is " << mimetype->name() << endl;
        if( mimetype->name() == "application/x-designer" )
        {
            QtDesignerDocument* d = new QtDesignerDocument(url, core);
            d->setDesignerPlugin(m_plugin);
            m_plugin->activateDocument(d);
            return d;
        }
        return 0;
    }
    private:
        QtDesignerPlugin* m_plugin;
};

class QtDesignerToolViewFactory : public KDevelop::IToolViewFactory
{
public:
    enum Type
    {
        WidgetBox,
        PropertyEditor,
        ActionEditor,
        ObjectInspector
    };
    QtDesignerToolViewFactory( QtDesignerPlugin* plugin, Type typ )
        : IToolViewFactory(), m_plugin(plugin), m_type(typ)
    {
    }

    virtual QWidget* create(QWidget *parent = 0)
    {
        if( m_type == WidgetBox )
            return m_plugin->designer()->widgetBox();
        else if( m_type == PropertyEditor )
            return m_plugin->designer()->propertyEditor();
        else if( m_type == ActionEditor )
            return m_plugin->designer()->actionEditor();
        else if( m_type == ObjectInspector )
            return m_plugin->designer()->objectInspector();
        kDebug(9039) << "Type not found: " << m_type << endl;
        return 0;
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        if( m_type == WidgetBox )
            return Qt::LeftDockWidgetArea;
        else if( m_type == PropertyEditor )
            return Qt::RightDockWidgetArea;
        else if( m_type == ActionEditor )
            return Qt::RightDockWidgetArea;
        else if( m_type == ObjectInspector )
            return Qt::RightDockWidgetArea;
        kDebug(9039) << "Type not found: " << m_type << endl;
        return Qt::TopDockWidgetArea;
    }

private:
    QtDesignerPlugin* m_plugin;
    Type m_type;
};

QtDesignerPlugin::QtDesignerPlugin(QObject *parent, const QStringList &args)
    : KDevelop::IPlugin(QtDesignerPluginFactory::componentData(),parent),
      m_activeDoc(0), m_docFactory(new QtDesignerDocumentFactory(this)),
      m_widgetBoxFactory(0), m_propertyEditorFactory(0),
      m_objectInspectorFactory(0), m_actionEditorFactory(0)
{
    Q_UNUSED(args)
    QDesignerComponents::initializeResources();
    KDevelop::IDocumentController* idc = core()->documentController();
    idc->registerDocumentForMimetype("application/x-designer", m_docFactory);

    connect( idc, SIGNAL( documentActivated( KDevelop::IDocument* ) ),
             this, SLOT( activateDocument( KDevelop::IDocument* ) ) );

    setXMLFile( "kdevqtdesigner.rc" );

    QDesignerFormEditorInterface* formeditor = QDesignerComponents::createFormEditor(this);
    QDesignerComponents::initializePlugins( formeditor );

    kDebug(9039) << "integration: " << formeditor->integration() << endl;

    //TODO apaku: if multiple mainwindows exist, this needs to be changed on mainwindow-change
    formeditor->setTopLevel(core()->uiController()->activeMainWindow());

    formeditor->setWidgetBox(QDesignerComponents::createWidgetBox(formeditor, 0));

//    load the standard widgets
    formeditor->widgetBox()->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
    formeditor->widgetBox()->load();

    formeditor->setPropertyEditor(QDesignerComponents::createPropertyEditor(formeditor, 0));
    formeditor->setActionEditor(QDesignerComponents::createActionEditor(formeditor, 0));
    formeditor->setObjectInspector(QDesignerComponents::createObjectInspector(formeditor, 0));

    m_designer = new qdesigner_internal::QDesignerIntegration(formeditor, this);
    qdesigner_internal::QDesignerIntegration::initializePlugins( formeditor );

    m_designer->core()->widgetBox()->setObjectName( i18n("Widget Box") );
    m_designer->core()->propertyEditor()->setObjectName( i18n("Property Editor") );
    m_designer->core()->actionEditor()->setObjectName( i18n("Action Editor") );
    m_designer->core()->objectInspector()->setObjectName( i18n("Object Inspector") );

    QList<QObject*> plugins = QPluginLoader::staticInstances();
    kDebug(9039) << "pluginlist from designer: " << plugins << endl;
    foreach (QObject *plugin, plugins)
    {
        QDesignerFormEditorPluginInterface *fep;

        if ( (fep = qobject_cast<QDesignerFormEditorPluginInterface*>(plugin)) )
        {
                if ( !fep->isInitialized() )
                        fep->initialize(formeditor);

                fep->action()->setCheckable(true);
                if( fep->action()->text() == "Edit Signals/Slots" )
                    actionCollection()->addAction("signaleditor", fep->action());
                if( fep->action()->text() == "Edit Buddies" )
                    actionCollection()->addAction("buddyeditor", fep->action());
                if( fep->action()->text() == "Edit Tab Order" )
                    actionCollection()->addAction("tabordereditor", fep->action());

                kDebug(9039) << "Added action: " << fep->action()->objectName() << "|" << fep->action()->text()<< endl;
        }
    }


    setupActions();

    m_widgetBoxFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::WidgetBox );
    m_propertyEditorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::PropertyEditor);
    m_actionEditorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::ActionEditor);
    m_objectInspectorFactory = new QtDesignerToolViewFactory( this,
            QtDesignerToolViewFactory::ObjectInspector);
    core()->uiController()->addToolView("Widget Box", m_widgetBoxFactory );
    core()->uiController()->addToolView("Property Editor", m_propertyEditorFactory );
    core()->uiController()->addToolView("Action Editor", m_actionEditorFactory );
    core()->uiController()->addToolView("Object Inspector", m_objectInspectorFactory );
}

QtDesignerPlugin::~QtDesignerPlugin()
{
    delete m_designer;
    delete m_docFactory;
}

QDesignerFormEditorInterface *QtDesignerPlugin::designer() const
{
    return m_designer->core();
}

void QtDesignerPlugin::setupActions()
{
    QDesignerFormWindowManagerInterface* manager = designer()->formWindowManager();
    KActionCollection* ac = actionCollection();

    ac->addAction( "file_save", KStandardAction::save( this, SLOT( saveActiveDocument() ), ac) );
    ac->addAction( "adjust_size", manager->actionAdjustSize() );
    ac->addAction( "break_layout", manager->actionBreakLayout() );
    ac->addAction( "designer_cut", manager->actionCut() );
    ac->addAction( "designer_copy", manager->actionCopy() );
    ac->addAction( "designer_paste", manager->actionPaste() );
    ac->addAction( "designer_delete", manager->actionDelete() );
    ac->addAction( "layout_grid", manager->actionGridLayout() );
    ac->addAction( "layout_horiz", manager->actionHorizontalLayout() );
    ac->addAction( "layout_vertical", manager->actionVerticalLayout() );
    ac->addAction( "layout_split_horiz", manager->actionSplitHorizontal() );
    ac->addAction( "layout_split_vert", manager->actionSplitVertical() );
    ac->addAction( "designer_undo", manager->actionUndo() );
    ac->addAction( "designer_redo", manager->actionRedo() );
    ac->addAction( "designer_select_all", manager->actionSelectAll() );
}


// bool QtDesignerPlugin::saveFile()
// {
//     KSaveFile uiFile( localFilePath() );
//     //FIXME: find a way to return an error. KSaveFile
//     //doesn't use the KIO error codes
//     if ( !uiFile.open() )
//         return false;
//
//     QTextStream stream ( &uiFile );
//     QByteArray windowXml = m_window->contents().toUtf8();
//     stream << windowXml;
//
//     if ( !uiFile.finalize() )
//         return false;
//
//     m_window->setDirty(false);
//     setModified(false);
//     return true;
// }

void QtDesignerPlugin::saveActiveDocument()
{
    kDebug(9039) << "going to save: " << m_activeDoc << endl;
    if( m_activeDoc )
    {
        m_activeDoc->save( KDevelop::IDocument::Default );
    }
}

void QtDesignerPlugin::activateDocument( KDevelop::IDocument* doc )
{
    if( doc->mimeType()->is( "application/x-designer" ) )
    {
        kDebug(9039) << "Doc activated: " << doc << endl;
        m_activeDoc = doc;
    }
}

#include "qtdesignerplugin.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; indent-mode cstyle;
