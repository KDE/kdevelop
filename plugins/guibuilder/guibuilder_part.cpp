
#include "guibuilder_part.h"

#include <QObject>
#include <QFile>
#include <QtDesigner/QtDesigner>
#include <QtDesigner/QDesignerComponents>

#include <kaboutdata.h>
#include <kmainwindow.h>
#include <kparts/genericfactory.h>
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
}

GuiBuilderPart::~GuiBuilderPart()
{
  mainWindow()->removeView( m_designer->widgetBox() );
  mainWindow()->removeView( m_designer->propertyEditor() );
  delete m_designer;
  m_designer = 0;
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
}

QDesignerFormEditorInterface *GuiBuilderPart::designer() const
{
  return m_designer;
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
  return true;
}

bool GuiBuilderPart::saveFile()
{
  return false;
}

#include "guibuilder_part.moc"
//kate: space-indent on; indent-width 2; replace-tabs on; indent-mode cstyle;
