#include <qvbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kmessagebox.h>

#include "kdevcompileroptions.h"
#include "projectoptionsdlg.h"


ProjectOptionsDialog::ProjectOptionsDialog(QWidget *parent, const char *name)
    : KDialogBase(TreeList, i18n("Project Options"), Ok|Cancel, Ok,
                  parent, name)
{
    // It is really the project management's job to decide which compiler options
    // it needs, but meanwhile... of course this is very preliminary anyways
    
    QVBox *vbox;

    vbox = addVBoxPage(i18n("C Compiler"));
    coptdlg = createCompilerOptions("C", vbox);
    coptdlg->setFlags("-Wall -O1");

    vbox = addVBoxPage(i18n("C++ Compiler"));
    cppoptdlg = createCompilerOptions("C++", vbox);
    cppoptdlg->setFlags("-Wall -Wbad-function-cast -O2");
}


ProjectOptionsDialog::~ProjectOptionsDialog()
{}


void ProjectOptionsDialog::accept()
{
    kdDebug(9000) << "CFLAGS=" << coptdlg->flags() << endl;
    kdDebug(9000) << "CXXFLAGS=" << cppoptdlg->flags() << endl;
    KDialogBase::accept();
}


KDevCompilerOptions *ProjectOptionsDialog::createCompilerOptions(const QString &lang, QWidget *box)
{
    QString constraint = QString("[X-KDevelop-Language] == '%1'").arg(lang);
    KTrader::OfferList offers = KTrader::self()->query("KDevelop/CompilerOptions", constraint);
    if (offers.isEmpty()) {
        KMessageBox::sorry(this,
                           i18n("No compiler options dialog for %1 found").arg(lang));
        return 0;
    }

    KService *service = *offers.begin();
    kdDebug(9000) << "Found compiler options dialog " << service->name() << endl;

    KLibFactory *factory = KLibLoader::self()->factory(service->library());

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());
    
    QObject *obj = factory->create(box, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug(9000) << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;
    
    return dlg;
}

#include "projectoptionsdlg.moc"
