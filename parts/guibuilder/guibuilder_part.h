#ifndef GUIBUILDER_PART_H
#define GUIBUILDER_PART_H

#include <kdevplugin.h>

class GuiBuilderPart: public KDevPlugin
{
  Q_OBJECT
public:
  GuiBuilderPart(QObject *parent, const char *name, const QStringList &args);
  virtual ~GuiBuilderPart();
};

#endif // GUIBUILDER_PART_H
