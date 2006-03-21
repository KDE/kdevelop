
#include "guibuilder_part.h"
#include "internals/qdesigner_integration_p.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QDesignerComponents>

#include <kdevplugininfo.h>
#include <kdevgenericfactory.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>

#include <kmainwindow.h>

static const KDevPluginInfo data("kdevguibuilder");
K_EXPORT_COMPONENT_FACTORY(libkdevguibuilder, KDevGenericFactory<GuiBuilderPart>(data))

GuiBuilderPart::GuiBuilderPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent)
{
  setObjectName(QString::fromUtf8(name));

  QDesignerComponents::initializeResources();

  m_designer = QDesignerComponents::createFormEditor(this);
  m_designer->setTopLevel(mainWindow()->main());

  m_designer->setWidgetBox(QDesignerComponents::createWidgetBox(m_designer, 0));
  Q_ASSERT(m_designer->widgetBox() != 0);
  
  // load the standard widgets
  m_designer->widgetBox()->setFileName(QLatin1String(":/trolltech/widgetbox/widgetbox.xml"));
  m_designer->widgetBox()->load();
  
  m_designer->setPropertyEditor(QDesignerComponents::createPropertyEditor(m_designer, 0));
  Q_ASSERT(m_designer->propertyEditor() != 0);

  (void) new QDesignerIntegration(m_designer, this);

  mainWindow()->embedSelectView(m_designer->widgetBox(), i18n("Widget Box"), i18n("Widget Box"));
  mainWindow()->embedSelectViewRight(m_designer->propertyEditor(), i18n("Property Editor"), i18n("Property Editor"));
}

GuiBuilderPart::~GuiBuilderPart()
{
  delete m_designer;
  m_designer = 0;
}

QDesignerFormEditorInterface *GuiBuilderPart::designer() const
{
  return m_designer;
}

#include "guibuilder_part.moc"
