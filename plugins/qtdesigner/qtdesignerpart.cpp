
#include "qtdesignerpart.h"

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
#include <kmainwindow.h>
#include <kparts/genericfactory.h>
#include <kparts/partmanager.h>
#include <ksavefile.h>
#include <kstandardaction.h>
#include <kicon.h>

#include "kdevcore.h"
#include <kdevmainwindow.h>
#include "internals/qdesigner_integration_p.h"

typedef KParts::GenericFactory<QtDesignerPart> QtDesignerPartFactory;
K_EXPORT_COMPONENT_FACTORY(kdevqtdesigner, QtDesignerPartFactory)

QtDesignerPart::QtDesignerPart(QWidget* parentWidget,
                               QObject *parent,
                               const QStringList &args)
    : KParts::ReadWritePart(parent)
{
  QDesignerComponents::initializeResources();

  m_workspace = new QWorkspace(parentWidget);
  m_workspace->setScrollBarsEnabled(true);
  setWidget( m_workspace );

  setXMLFile( "kdevqtdesigner.rc" );

  m_designer = QDesignerComponents::createFormEditor(this);
  m_designer->setTopLevel( KDevCore::mainWindow());

  m_designer->setWidgetBox(QDesignerComponents::createWidgetBox(m_designer, 0));
  Q_ASSERT(m_designer->widgetBox() != 0);

  // load the standard widgets
  m_designer->widgetBox()->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
  m_designer->widgetBox()->load();

  m_designer->setPropertyEditor(QDesignerComponents::createPropertyEditor(m_designer, 0));
  Q_ASSERT(m_designer->propertyEditor() != 0);

  (void) new qdesigner_internal::QDesignerIntegration(m_designer, this);

  m_designer->widgetBox()->setObjectName( i18n("Widget Box") );
  m_designer->propertyEditor()->setObjectName( i18n("Property Editor") );

  setupActions();

  connect( KDevCore::documentController(), SIGNAL( documentActivated( KDevDocument* ) ),
           this, SLOT( activated( KDevDocument* ) ) );
}

QtDesignerPart::~QtDesignerPart()
{
    if (m_designer)
    {
        if (m_window)
        {
            m_designer->formWindowManager()->removeFormWindow( m_window );
            delete m_window;
        }

        delete m_designer;
    }
    if (m_workspace)
        m_workspace->deleteLater();
}

void QtDesignerPart::activated( KDevDocument *document )
{
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
}

KAboutData* QtDesignerPart::createAboutData()
{
  KAboutData* aboutData = new KAboutData( "KDevQtDesignerPart",
                                          I18N_NOOP( "KDevelop GUI Builder" ),
                                          "4.0", 0,
                                          KAboutData::License_GPL_V2 );

  aboutData->addAuthor( "Matt Rogers", 0, "mattr@kde.org" );
  aboutData->addCredit( "Roberto Raggi", 0, "roberto@kdevelop.org" );

  return aboutData;
}

QDesignerFormEditorInterface *QtDesignerPart::designer() const
{
  return m_designer;
}

void QtDesignerPart::setupActions()
{
    KStandardAction::save( this, SLOT( save() ), actionCollection(), "file_save" );
    QDesignerFormWindowManagerInterface* manager = designer()->formWindowManager();

    QAction* designerAction = 0;
    KAction* designerKAction = 0;
    designerAction = manager->actionAdjustSize();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "adjust_size" );

    designerAction = manager->actionBreakLayout();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "break_layout" );
    designerKAction->setShortcut( Qt::CTRL + Qt::Key_B );

    designerAction = manager->actionCut();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "designer_cut" );

    designerAction = manager->actionCopy();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "designer_copy" );

    designerAction = manager->actionPaste();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "designer_paste" );

    designerAction = manager->actionDelete();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "designer_delete" );

    designerAction = manager->actionGridLayout();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "layout_grid" );
    designerKAction->setShortcut( Qt::CTRL + Qt::Key_G );

    designerAction = manager->actionHorizontalLayout();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "layout_horiz" );
    designerKAction->setShortcut( Qt::CTRL + Qt::Key_H );

    designerAction = manager->actionVerticalLayout();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "layout_vertical" );
    designerKAction->setShortcut( Qt::CTRL + Qt::Key_V );

    designerAction = manager->actionSplitHorizontal();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "layout_split_horiz" );
    designerKAction->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_H );

    designerAction = manager->actionSplitVertical();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "layout_split_vert" );
    designerKAction->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_V );

    designerAction = manager->actionUndo();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "designer_undo" );
    designerKAction->setIcon( KIcon( "edit_undo" ) );

    designerAction = manager->actionRedo();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "designer_redo" );
    designerKAction->setIcon( KIcon( "edit_redo" ) );

    designerAction = manager->actionSelectAll();
    designerKAction = wrapDesignerAction( designerAction, actionCollection(),
                                          "designer_select_all" );
}

bool QtDesignerPart::openFile()
{
  QFile uiFile(m_file);
  QDesignerFormWindowManagerInterface* manager = m_designer->formWindowManager();
  QDesignerFormWindowInterface* widget = manager->createFormWindow();
  widget->setFileName(m_file);
  widget->setContents(&uiFile);
  manager->setActiveFormWindow(widget);
  m_workspace->addWindow(widget);
  m_window = widget;
  m_window->installEventFilter( this ); //be able to catch the close event

  connect( m_window, SIGNAL( changed() ), this, SLOT(setModified()));
  connect( m_window, SIGNAL( changed() ), this, SLOT( updateDesignerActions() ) );
  connect( m_window, SIGNAL( selectionChanged() ), this, SLOT( updateDesignerActions() ) );
  connect( m_window, SIGNAL( toolChanged( int ) ), this, SLOT( updateDesignerActions() ) );

  return true;
}

bool QtDesignerPart::saveFile()
{
    KSaveFile uiFile( m_file );
    //FIXME: find a way to return an error. KSaveFile
    //doesn't use the KIO error codes
    if ( !uiFile.open() )
        return false;

    QTextStream stream ( &uiFile );
    QByteArray windowXml = m_window->contents().toUtf8();
    stream << windowXml;

    if ( !uiFile.finalize() )
        return false;

    m_window->setDirty(false);
    setModified(false);
    return true;
}

bool QtDesignerPart::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::Close && obj == m_window )
    {
        designer()->formWindowManager()->removeFormWindow( m_window );
        widget()->deleteLater();
    }
    return false;
}

KAction* QtDesignerPart::wrapDesignerAction( QAction* dAction,
                                         KActionCollection* parent,
                                         const char* name )
{
    KAction* a = new KAction( KIcon( dAction->icon() ), dAction->text(),
                              parent, name );
    a->setShortcut( dAction->shortcut() );
    a->setShortcutContext( dAction->shortcutContext() );
    connect( a, SIGNAL( triggered() ), dAction, SIGNAL( triggered() ) );

    m_designerActions[a] = dAction;
    updateDesignerAction( a, dAction );
    return a;
}

void QtDesignerPart::updateDesignerAction( KAction* a, QAction* dAction )
{
    a->setActionGroup( dAction->actionGroup() );
    a->setCheckable( dAction->isCheckable() );
    a->setChecked( dAction->isChecked() );
    a->setEnabled( dAction->isEnabled() );
    a->setData( dAction->data() );
    a->setFont( dAction->font() );
    a->setIconText( dAction->iconText() );
    a->setSeparator( dAction->isSeparator() );
    a->setStatusTip( dAction->statusTip() );
    a->setText( dAction->text() );
    a->setToolTip( dAction->toolTip() );
    a->setWhatsThis( dAction->whatsThis() );
}

void QtDesignerPart::updateDesignerActions()
{
    DesignerActionHash::ConstIterator it, itEnd = m_designerActions.constEnd();
    for ( it = m_designerActions.constBegin(); it != itEnd; ++it )
    {
        updateDesignerAction( it.key(), it.value() );
    }
}

#include "qtdesignerpart.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; indent-mode cstyle;
