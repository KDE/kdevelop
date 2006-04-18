
#include "guibuilder_part.h"

#include <QObject>
#include <QAction>
#include <QFile>
#include <QTextStream>
#include <QtDesigner/QtDesigner>
#include <QtDesigner/QDesignerComponents>

#include <kaboutdata.h>
#include <kaction.h>
#include <kmainwindow.h>
#include <kparts/genericfactory.h>
#include <ksavefile.h>
#include <kstdaction.h>

#include <kdevmainwindow.h>
#include "internals/qdesigner_integration_p.h"

typedef KParts::GenericFactory<GuiBuilderPart> GuiBuilderPartFactory;
K_EXPORT_COMPONENT_FACTORY(kdevguibuilder, GuiBuilderPartFactory)

GuiBuilderPart::GuiBuilderPart(QWidget* parentWidget, const char* widgetName,
                               QObject *parent, const char *name,
                               const QStringList &args)
  : KDevReadWritePart(parent)
{
  setObjectName(QString::fromUtf8(name));
  QDesignerComponents::initializeResources();
  m_designer = 0;
  m_window = 0;
}

GuiBuilderPart::~GuiBuilderPart()
{
  mainWindow()->removeView( m_designer->widgetBox() );
  mainWindow()->removeView( m_designer->propertyEditor() );
}

 KAboutData* GuiBuilderPart::createAboutData()
{
  KAboutData* aboutData = new KAboutData( "KDevGuiBuilderPart",
                                          I18N_NOOP( "KDevelop GUI Builder" ),
                                          "4.0", 0,
                                          KAboutData::License_GPL_V2 );

  aboutData->addAuthor( "Matt Rogers", 0, "mattr@kde.org" );
  aboutData->addCredit( "Roberto Raggi", 0, "roberto@kdevelop.org" );

  return aboutData;
}

void GuiBuilderPart::setApiInstance( KDevApi* api )
{
  KDevReadWritePart::setApiInstance( api );
  setXMLFile( "guibuilderpart.rc" );

  m_designer = QDesignerComponents::createFormEditor(this);
  m_designer->setTopLevel(mainWindow()->main());

  m_designer->setWidgetBox(QDesignerComponents::createWidgetBox(m_designer, 0));
  Q_ASSERT(m_designer->widgetBox() != 0);

  // load the standard widgets
  m_designer->widgetBox()->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
  m_designer->widgetBox()->load();

  m_designer->setPropertyEditor(QDesignerComponents::createPropertyEditor(m_designer, 0));
  Q_ASSERT(m_designer->propertyEditor() != 0);

  (void) new qdesigner_internal::QDesignerIntegration(m_designer, this);

  mainWindow()->embedSelectView(m_designer->widgetBox(), i18n("Widget Box"), i18n("Widget Box"));
  mainWindow()->embedSelectViewRight(m_designer->propertyEditor(), i18n("Property Editor"), i18n("Property Editor"));

  setupActions();
}

QDesignerFormEditorInterface *GuiBuilderPart::designer() const
{
  return m_designer;
}

void GuiBuilderPart::setupActions()
{
    KStdAction::save( this, SLOT( save() ), actionCollection(), "designer_save" );
    QDesignerFormWindowManagerInterface* manager = designer()->formWindowManager();

    QAction* designerAction = 0;
    designerAction = manager->actionAdjustSize();
    wrapDesignerAction( designerAction, actionCollection(), "adjust_size" );

    designerAction = manager->actionBreakLayout();
    wrapDesignerAction( designerAction, actionCollection(), "break_layout" );

    designerAction = manager->actionCut();
    KStdAction::cut( designerAction, SIGNAL( triggered( bool  ) ),
                     actionCollection(), "designer_cut" );

    designerAction = manager->actionCopy();
    KStdAction::copy( designerAction, SIGNAL( triggered( bool ) ),
                      actionCollection(), "designer_copy" );

    designerAction = manager->actionPaste();
    KStdAction::paste( designerAction, SIGNAL( triggered( bool ) ),
                       actionCollection(), "designer_paste" );

    designerAction = manager->actionDelete();
    wrapDesignerAction( designerAction, actionCollection(), "designer_delete" );

    designerAction = manager->actionGridLayout();
    wrapDesignerAction( designerAction, actionCollection(), "layout_grid" );

    designerAction = manager->actionHorizontalLayout();
    wrapDesignerAction( designerAction, actionCollection(), "layout_horiz" );

    designerAction = manager->actionVerticalLayout();
    wrapDesignerAction( designerAction, actionCollection(), "layout_vertical" );

    designerAction = manager->actionUndo();
    KStdAction::undo( designerAction, SIGNAL( triggered( bool ) ),
                      actionCollection(), "designer_undo" );

    designerAction = manager->actionRedo();
    KStdAction::redo( designerAction, SIGNAL( triggered( bool ) ),
                      actionCollection(), "designer_redo" );

    designerAction = manager->actionSelectAll();
    KStdAction::selectAll( designerAction, SIGNAL( triggered( bool ) ),
                           actionCollection(), "designer_select_all" );
}

bool GuiBuilderPart::openFile()
{
  QFile uiFile(m_file);
  QDesignerFormWindowManagerInterface* manager = designer()->formWindowManager();
  QDesignerFormWindowInterface* widget = manager->createFormWindow();
  widget->setFileName(m_file);
  widget->setContents(&uiFile);
  manager->setActiveFormWindow(widget);
  setWidget( widget );
  m_window = widget;
  connect(m_window, SIGNAL(changed()), this, SLOT(setModified()));
  return true;
}

bool GuiBuilderPart::saveFile()
{
    KSaveFile uiFile( m_file );
    //FIXME: find a way to return an error. KSaveFile
    //doesn't use the KIO error codes
    if ( uiFile.status() )
        return false;

    QTextStream* stream = uiFile.textStream();
    QByteArray windowXml = m_window->contents().toUtf8();
    *stream << windowXml;

    if ( !uiFile.close() )
        return false;

    m_window->setDirty(false);
    setModified(false);
    return true;
}

void GuiBuilderPart::wrapDesignerAction( QAction* dAction,
                                         KActionCollection* parent,
                                         const char* name )
{
    KAction* a = new KAction( KIcon( dAction->icon() ), dAction->text(),
                              parent, name );
    a->setActionGroup( dAction->actionGroup() );
    a->setCheckable( dAction->isCheckable() );
    a->setChecked( dAction->isChecked() );
    a->setEnabled( dAction->isEnabled() );
    a->setData( dAction->data() );
    a->setFont( dAction->font() );
    a->setIconText( dAction->iconText() );
    a->setSeparator( dAction->isSeparator() );
    a->setShortcut( dAction->shortcut() );
    a->setShortcutContext( dAction->shortcutContext() );
    a->setStatusTip( dAction->statusTip() );
    a->setText( dAction->text() );
    a->setToolTip( dAction->toolTip() );
    a->setWhatsThis( dAction->whatsThis() );
}

#include "guibuilder_part.moc"
//kate: space-indent on; indent-width 4; replace-tabs on; indent-mode cstyle;
