#include <qwhatsthis.h>
#include <kiconloader.h>
#include <klocale.h>
#include "domutil.h"
#include <kdevproject.h>
#include <kdevappfrontend.h>
#include <qvbox.h>
#include <kgenericfactory.h>
#include <kaction.h>


#include <kdevcore.h>

#include "visualboyadvance_part.h"
#include "vbaconfigwidget.h"

using namespace VisualBoyAdvance;

typedef KGenericFactory<VisualBoyAdvancePart> VisualBoyAdvanceFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevvisualboyadvance, VisualBoyAdvanceFactory( "kdevvisualboyadvance" ) );
  
VisualBoyAdvancePart::VisualBoyAdvancePart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name){
  setInstance(VisualBoyAdvanceFactory::instance());

  setXMLFile("kdevpart_visualboyadvance.rc");
  
  KAction *action;
  action = new KAction( i18n("Execute program"), "exec",  Key_F9,
			this, SLOT(slotExecute()),
			actionCollection(), "build_execute" );

  connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
	   this, SLOT(projectConfigWidget(KDialogBase*)) );
}


VisualBoyAdvancePart::~VisualBoyAdvancePart()
{
  
}
void VisualBoyAdvancePart::slotExecute(){
  KDevProject* prj = project();
  QDomDocument &doc = *projectDom();
  QString binary = DomUtil::readEntry(doc, "/kdevvisualadvance/binary");
  QString emulator = DomUtil::readEntry(doc, "/kdevvisualadvance/emulator");
  QString graphicFilter = DomUtil::readEntry(doc, "/kdevvisualadvance/graphicFilter");
  QString scaling = DomUtil::readEntry(doc, "/kdevvisualadvance/scaling");
  QString addOptions = DomUtil::readEntry(doc, "/kdevvisualadvance/addOptions");
  bool terminal = DomUtil::readBoolEntry(doc, "/kdevvisualadvance/terminal");
  bool fullscreen = DomUtil::readBoolEntry(doc, "/kdevvisualadvance/fullscreen");

  if(emulator.isNull()) emulator = "VisualBoyAdvance";
  QString program = emulator + " " + graphicFilter +" " + scaling + " " + addOptions +" ";
  if(fullscreen){
    program += "-F ";
  }

  program += prj->projectDirectory() + "/" + binary;

  if(terminal){
    QString terminal = "konsole -e /bin/sh -c '";
    terminal += program;
    terminal += "; echo \"\n";
    terminal += i18n("Press Enter to continue!");
    terminal += "\";read'";
    program = terminal;
  }
  appFrontend()->startAppCommand(program);
}

void VisualBoyAdvancePart::projectConfigWidget(KDialogBase *dlg){
  QVBox *vbox;
  vbox = dlg->addVBoxPage(i18n("Run Options"));
  VBAConfigWidget* w = new VBAConfigWidget(this,vbox);
  connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}
  

#include "visualboyadvance_part.moc"
