#include <qwhatsthis.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kaction.h>
#include <kparts/part.h>
#include <kdialogbase.h>


#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevlanguagesupport.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"
#include "kdevappfrontend.h"

#include "rubysupport_part.h"
#include "rubyconfigwidget.h"
#include "domutil.h"

typedef KGenericFactory<RubySupportPart> RubySupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevrubysupport, RubySupportFactory( "kdevrubysupport" ) );

RubySupportPart::RubySupportPart(QObject *parent, const char *name, const QStringList& )
  : KDevLanguageSupport ("KDevPart", "kdevpart", parent, name ? name : "RubySupportPart" )
{
  setInstance(RubySupportFactory::instance());
  setXMLFile("kdevrubysupport.rc");

  KAction *action;
  action = new KAction( i18n("&Run"), "exec",Key_F9,this, SLOT(slotRun()),actionCollection(), "build_execute" );

  kdDebug() << "Creating RubySupportPart" << endl;

  connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );


}


RubySupportPart::~RubySupportPart() {
}


void RubySupportPart::projectConfigWidget(KDialogBase *dlg) {
    QVBox *vbox = dlg->addVBoxPage(i18n("Ruby"));
    RubyConfigWidget *w = new RubyConfigWidget(*projectDom(), (QWidget *)vbox, "ruby config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}


void RubySupportPart::slotRun () {
  QString file;
  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(partController()->activePart());
  if(ro_part) file = ro_part->url().path();

  QString cmd = interpreter() + " " + file;
  startApplication(cmd);
}

QString RubySupportPart::interpreter() {
    QString prog = DomUtil::readEntry(*projectDom(), "/kdevrubysupport/run/interpreter");
    if (prog.isEmpty())
        prog = "ruby";
    return prog;
}


void RubySupportPart::startApplication(const QString &program) {
    kdDebug() << "starting application" << program << endl;
    appFrontend()->startAppCommand(QString::QString(), program, TRUE);
}


KDevLanguageSupport::Features RubySupportPart::features() {
    return Features(Classes | Functions);
}


KMimeType::List RubySupportPart::mimeTypes( )
{
    KMimeType::List list;
    KMimeType::Ptr mime = KMimeType::mimeType( "text/x-ruby" );
    if( mime )
	list << mime;
    return list;
}

#include "rubysupport_part.moc"
