
#include "kdevdesigner_part.h"

#include "kdevplugininfo.h"
#include "kdevgenericfactory.h"

#include <kinstance.h>
#include <klocale.h>

typedef KDevGenericFactory<KDevDesignerPart> KDevDesignerFactory;
static const KDevPluginInfo data( "kdevdesignerpart" );
K_EXPORT_COMPONENT_FACTORY( kdevdesignerpart,
                            KDevDesignerFactory( data ) );

KDevDesignerPart::KDevDesignerPart(QWidget *parentWidget,
                                   const char* widgetName,
                                   QObject *parent,
                                   const char *name, const QStringList&)
  : KParts::ReadWritePart(parent),
    m_parentWidget(parentWidget)
{
  setObjectName(name);
  setInstance(KDevDesignerFactory::instance());
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
