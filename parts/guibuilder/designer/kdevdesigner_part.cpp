
#include "kdevdesigner_part.h"

KDevDesignerPart::KDevDesignerPart(QObject *parent, const char *name, const QStringList &)
  : KParts::ReadWritePart(parent, name)
{
}

KDevDesignerPart::~KDevDesignerPart()
{
}

bool KDevDesignerPart::openFile()
{
  return false;
}

bool KDevDesignerPart::saveFile()
{
  return false;
}

  
#include "kdevdesigner_part.moc"
