#include "windowmenupart.h"

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmainwindow.h>
#include <kparts/part.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "kdevtoplevel.h"


typedef KGenericFactory<WindowMenuPart> WindowMenuFactory;

K_EXPORT_COMPONENT_FACTORY(libkdevwindowmenu, WindowMenuFactory("kdevwindowmenu"));


WindowMenuPart::WindowMenuPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name)
{
  setInstance(WindowMenuFactory::instance());

  setXMLFile("kdevwindowmenu.rc");
	
  connect(partController(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(updateBufferMenu()));
  connect(partController(), SIGNAL(partRemoved(KParts::Part*)), this, SLOT(updateBufferMenu()));
  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(updateBufferMenu()));
}


WindowMenuPart::~WindowMenuPart()
{
}


void WindowMenuPart::updateBufferMenu()
{
  if ( !topLevelValid() )
    // do nothing otherwise it may crash while plugging the ActionLists
    return;

  QPtrList<KAction> bufferActions;

  unplugActionList("window_list");

  QPtrListIterator<KParts::Part> it(*partController()->parts());
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (!ro_part)
      continue;

    QString name = ro_part->url().url();

    KToggleAction *action = new KToggleAction(name, 0, 0, name.latin1());
    action->setChecked(ro_part == partController()->activePart());
    connect(action, SIGNAL(activated()), this, SLOT(bufferSelected()));
    bufferActions.append(action);
  }

  plugActionList("window_list", bufferActions);
}


void WindowMenuPart::bufferSelected()
{
  QPtrListIterator<KParts::Part> it(*partController()->parts());
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (ro_part && ro_part->url() == KURL(sender()->name()))
    {
      partController()->setActivePart(ro_part);

      if (ro_part->widget())
      {
        topLevel()->raiseView(ro_part->widget());
        ro_part->widget()->setFocus();
      }

      break;
    }
  }
  
  updateBufferMenu();
}


#include "windowmenupart.moc"
