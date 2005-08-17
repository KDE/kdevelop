
#include "guibuilder_part.h"

#include <kdevplugininfo.h>
#include <kdevgenericfactory.h>

static const KDevPluginInfo data("kdevguibuilder");
K_EXPORT_COMPONENT_FACTORY(libkdevguibuilder, KDevGenericFactory<GuiBuilderPart>(data))

GuiBuilderPart::GuiBuilderPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent)
{
  setObjectName(QString::fromUtf8(name));
}

GuiBuilderPart::~GuiBuilderPart()
{
}

