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
#include <QWorkspace>

#include <kaboutdata.h>
#include <kaction.h>
#include <kmimetype.h>
#include <kxmlguiwindow.h>
#include <kparts/genericfactory.h>
#include <kparts/partmanager.h>
#include <ksavefile.h>
#include <kstandardaction.h>
#include <kicon.h>
#include <kactioncollection.h>

#include "icore.h"
#include "idocumentcontroller.h"
#include "iuicontroller.h"
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
      m_docFactory(new QtDesignerDocumentFactory(this))
{
    Q_UNUSED(args)
    QDesignerComponents::initializeResources();

    core()->documentController()->registerDocumentForMimetype("application/x-designer", m_docFactory);

//   m_workspace = new QWorkspace(parentWidget);
//   m_workspace->setScrollBarsEnabled(true);
//   setWidget( m_workspace );

    setXMLFile( "kdevqtdesigner.rc" );

    m_designer = QDesignerComponents::createFormEditor(this);
//     m_designer->setTopLevel( parentWidget );

    m_designer->setWidgetBox(QDesignerComponents::createWidgetBox(m_designer, 0));
//   Q_ASSERT(m_designer->widgetBox() != 0);
//
//   // load the standard widgets
    m_designer->widgetBox()->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
    m_designer->widgetBox()->load();
//
    m_designer->setPropertyEditor(QDesignerComponents::createPropertyEditor(m_designer, 0));
    m_designer->setActionEditor(QDesignerComponents::createActionEditor(m_designer, 0));
    m_designer->setObjectInspector(QDesignerComponents::createObjectInspector(m_designer, 0));
//   Q_ASSERT(m_designer->propertyEditor() != 0);
//
    new qdesigner_internal::QDesignerIntegration(m_designer, this);
//
    m_designer->widgetBox()->setObjectName( i18n("Widget Box") );
    m_designer->propertyEditor()->setObjectName( i18n("Property Editor") );
//
//   setupActions();

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

//   connect( KDevelop::ICore::documentController(), SIGNAL( documentActivated( KDevelop::Document* ) ),
//            this, SLOT( activated( KDevelop::Document* ) ) );
}

QtDesignerPlugin::~QtDesignerPlugin()
{
//         if (m_window)
//         {
//             m_designer->formWindowManager()->removeFormWindow( m_window );
//             delete m_window;
//         }
//
    delete m_designer;
//     if (m_workspace)
//         m_workspace->deleteLater();
    delete m_docFactory;
}

// void QtDesignerPlugin::activated( KDevelop::IDocument *document )
// {
//     Q_UNUSED(document)
    //FIXME
//     if ( document->url() == url() )
//     {
//         KDevCore::mainWindow()->raiseView(
//                 m_designer->widgetBox(),
//                 Qt::LeftDockWidgetArea);
//         KDevCore::mainWindow()->raiseView(
//                 m_designer->propertyEditor(),
//                 Qt::RightDockWidgetArea);
//     }
//     KMimeType::Ptr mimeType = KMimeType::findByURL( document->url() );
//     if (!mimeType->is( "application/x-designer" ))
//     {
//         KDevCore::mainWindow()->lowerView( m_designer->widgetBox() );
//         KDevCore::mainWindow()->lowerView( m_designer->propertyEditor() );
//     }
// }

// KAboutData* QtDesignerPlugin::createAboutData()
// {
//   KAboutData* aboutData = new KAboutData( "KDevQtDesignerPlugin",
//                                           I18N_NOOP( "KDevelop GUI Builder" ),
//                                           "4.0", 0,
//                                           KAboutData::License_GPL_V2 );
//
//   aboutData->addAuthor( "Matt Rogers", 0, "mattr@kde.org" );
//   aboutData->addCredit( "Roberto Raggi", 0, "roberto@kdevelop.org" );
//
//   return aboutData;
// }

QDesignerFormEditorInterface *QtDesignerPlugin::designer() const
{
    return m_designer;
}

// void QtDesignerPlugin::setupActions()
// {
//     QAction *a = KStandardAction::save( this, SLOT( save() ), actionCollection());
//     actionCollection()->addAction( "file_save", a );
//     QDesignerFormWindowManagerInterface* manager = designer()->formWindowManager();
//
//     QAction* designerAction = 0;
//     QAction* designerKAction = 0;
//     designerAction = manager->actionAdjustSize();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "adjust_size" );
//
//     designerAction = manager->actionBreakLayout();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "break_layout" );
//     designerKAction->setShortcut( Qt::CTRL + Qt::Key_B );
//
//     designerAction = manager->actionCut();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "designer_cut" );
//
//     designerAction = manager->actionCopy();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "designer_copy" );
//
//     designerAction = manager->actionPaste();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "designer_paste" );
//
//     designerAction = manager->actionDelete();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "designer_delete" );
//
//     designerAction = manager->actionGridLayout();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "layout_grid" );
//     designerKAction->setShortcut( Qt::CTRL + Qt::Key_G );
//
//     designerAction = manager->actionHorizontalLayout();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "layout_horiz" );
//     designerKAction->setShortcut( Qt::CTRL + Qt::Key_H );
//
//     designerAction = manager->actionVerticalLayout();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "layout_vertical" );
//     designerKAction->setShortcut( Qt::CTRL + Qt::Key_V );
//
//     designerAction = manager->actionSplitHorizontal();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "layout_split_horiz" );
//     designerKAction->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_H );
//
//     designerAction = manager->actionSplitVertical();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "layout_split_vert" );
//     designerKAction->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_V );
//
//     designerAction = manager->actionUndo();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "designer_undo" );
//     designerKAction->setIcon( KIcon( "edit-undo" ) );
//
//     designerAction = manager->actionRedo();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "designer_redo" );
//     designerKAction->setIcon( KIcon( "edit-redo" ) );
//
//     designerAction = manager->actionSelectAll();
//     designerKAction = wrapDesignerAction( designerAction, actionCollection(),
//                                           "designer_select_all" );
// }

// bool QtDesignerPlugin::openFile()
// {
//   QFile uiFile(localFilePath());
//   QDesignerFormWindowManagerInterface* manager = m_designer->formWindowManager();
//   QDesignerFormWindowInterface* widget = manager->createFormWindow();
//   widget->setFileName(localFilePath());
//   widget->setContents(&uiFile);
//   manager->setActiveFormWindow(widget);
//   m_workspace->addWindow(widget);
//   m_window = widget;
//   m_window->installEventFilter( this ); //be able to catch the close event
//
//   connect( m_window, SIGNAL( changed() ), this, SLOT(setModified()));
//   connect( m_window, SIGNAL( changed() ), this, SLOT( updateDesignerActions() ) );
//   connect( m_window, SIGNAL( selectionChanged() ), this, SLOT( updateDesignerActions() ) );
//   connect( m_window, SIGNAL( toolChanged( int ) ), this, SLOT( updateDesignerActions() ) );
//
//   return true;
// }
//
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

// bool QtDesignerPlugin::eventFilter( QObject* obj, QEvent* event )
// {
//     if ( event->type() == QEvent::Close && obj == m_window )
//     {
//         designer()->formWindowManager()->removeFormWindow( m_window );
//         widget()->deleteLater();
//     }
//     return false;
// }

// QAction* QtDesignerPlugin::wrapDesignerAction( QAction* dAction,
//                                          KActionCollection* parent,
//                                          const char* name )
// {
//     QAction *a = parent->addAction( name );
//     a->setText( dAction->text() );
//     a->setIcon(  KIcon( dAction->icon() ) );
//     a->setShortcut( dAction->shortcut() );
//     a->setShortcutContext( dAction->shortcutContext() );
//     connect( a, SIGNAL( triggered() ), dAction, SIGNAL( triggered() ) );
//
//     m_designerActions[a] = dAction;
//     updateDesignerAction( a, dAction );
//     return a;
// }
//
// void QtDesignerPlugin::updateDesignerAction( QAction* a, QAction* dAction )
// {
//     a->setActionGroup( dAction->actionGroup() );
//     a->setCheckable( dAction->isCheckable() );
//     a->setChecked( dAction->isChecked() );
//     a->setEnabled( dAction->isEnabled() );
//     a->setData( dAction->data() );
//     a->setFont( dAction->font() );
//     a->setIconText( dAction->iconText() );
//     a->setSeparator( dAction->isSeparator() );
//     a->setStatusTip( dAction->statusTip() );
//     a->setText( dAction->text() );
//     a->setToolTip( dAction->toolTip() );
//     a->setWhatsThis( dAction->whatsThis() );
// }
//
// void QtDesignerPlugin::updateDesignerActions()
// {
//     DesignerActionHash::ConstIterator it, itEnd = m_designerActions.constEnd();
//     for ( it = m_designerActions.constBegin(); it != itEnd; ++it )
//     {
//         updateDesignerAction( it.key(), it.value() );
//     }
// }

#include "qtdesignerplugin.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; indent-mode cstyle;
