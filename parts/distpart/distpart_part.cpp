#include "distpart_part.h"

#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>

#include "kdevcore.h"

#include "distpart_widget.h"
#include "specsupport.h"
#include "lsmsupport.h"

typedef KGenericFactory<DistpartPart> DistpartFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdistpart, DistpartFactory( "kdevdistpart" ) );

DistpartPart::DistpartPart(QObject *parent, const char *name, const QStringList &)
        : KDevPlugin(parent, name ? name : "DistpartPart") {

    kdDebug(9007) << "DistpartPart::DistpartPart()" << endl;
    setInstance(DistpartFactory::instance());

    setXMLFile("kdevpart_distpart.rc");

    m_action =  new KAction( i18n("Project Distribution and Publishing"), "package", 0,
                             this, SLOT(show()),
                             actionCollection(), "make_dist" );

    m_action->setStatusText(i18n("Make Source and Binary Distribution"));
    m_action->setWhatsThis(i18n("Distribution and Publishing:\n\n"));
    //QWhatsThis::add(m_widget, i18n("This will help users package and publish their software."));
    
    m_dialog = new DistpartDialog(this);

    // Package types
    RpmPackage = new SpecSupport(this);
    //LsmPackage = new LsmSupport(this);
}


DistpartPart::~DistpartPart() {
    kdDebug(9007) << "DistpartPart::~DistpartPart()" << endl;
    delete m_dialog;
    delete RpmPackage;
    //delete LsmPackage;
}

DistpartDialog* DistpartPart::getDlg() {
    return m_dialog;
}

void DistpartPart::show() {
    m_dialog->show();
}

void DistpartPart::hide() {
    m_dialog->hide();
}

#include "distpart_part.moc"
