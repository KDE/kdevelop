#include "KDevPartControllerIface.h"


#include "kdevpartcontroller.h"


KDevPartController::KDevPartController(QWidget *parent)
  : KParts::PartManager(parent)
{
  new KDevPartControllerIface(this);
}


#include "kdevpartcontroller.moc"
