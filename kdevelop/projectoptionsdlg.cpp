#include <qhbox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kseparator.h>
#include <klocale.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kmessagebox.h>

#include "kdevcompileroptions.h"
#include "projectoptionsdlg.h"


class ServiceComboBox : public QComboBox
{
public:
    ServiceComboBox( const QValueList<KService::Ptr> &list,
                     QWidget *parent=0, const char *name=0 );
    ~ServiceComboBox();

    QString currentText() const;
private:
    QStringList names;
};


ServiceComboBox::ServiceComboBox( const QValueList<KService::Ptr> &list,
                                  QWidget *parent, const char *name)
    : QComboBox(parent, name)
{
    QValueList<KService::Ptr>::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        insertItem((*it)->comment());
        names << (*it)->name();
    }
}


ServiceComboBox::~ServiceComboBox()
{}


QString ServiceComboBox::currentText() const
{
    if (currentItem() == -1)
        return QString::null;
    return names[currentItem()];
}



GeneralPage::GeneralPage(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setAutoAdd(true);
    
    new QLabel("Meep!", this);
    
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    layout->addStretch();
}


GeneralPage::~GeneralPage()
{}


CompilerPage::CompilerPage(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    // It is really the project management's job to decide which compiler options
    // it needs, but meanwhile... of course this is very preliminary anyways
    
    QLabel *ccompiler_label   = new QLabel(i18n("C Compiler"), this);
    QLabel *cxxcompiler_label = new QLabel(i18n("C++ Compiler"), this);
    QLabel *f77compiler_label = new QLabel(i18n("Fortran Compiler"), this);
    QLabel *cbinary_label     = new QLabel(i18n("Compiler command (CC):"), this);
    QLabel *cxxbinary_label   = new QLabel(i18n("Compiler command (CXX):"), this);
    QLabel *f77binary_label   = new QLabel(i18n("Compiler command (F77):"), this);

    KTrader::OfferList coffers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C'");
    KTrader::OfferList cxxoffers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'C++'");
    KTrader::OfferList f77offers =
        KTrader::self()->query("KDevelop/CompilerOptions", "[X-KDevelop-Language] == 'Fortran'");
    
    cservice_combo   = new ServiceComboBox(coffers, this, "c services");
    cxxservice_combo = new ServiceComboBox(cxxoffers, this, "cpp services");
    f77service_combo = new ServiceComboBox(f77offers, this, "f77 services");

    cbinary_edit   = new QLineEdit(this, "cbinary");
    cxxbinary_edit = new QLineEdit(this, "cxxbinary");
    f77binary_edit = new QLineEdit(this, "f77binary");
    
    QHBox *cflags_box   = new QHBox(this);
    QHBox *cxxflags_box = new QHBox(this);
    QHBox *f77flags_box = new QHBox(this);
    
    QLabel *cflags_label      = new QLabel(i18n("Compiler flags (CFLAGS):"), cflags_box);
    QLabel *cxxflags_label    = new QLabel(i18n("Compiler flags (CXXFLAGS):"), cxxflags_box);
    QLabel *f77flags_label    = new QLabel(i18n("Compiler flags (FFLAGS):"), f77flags_box);

    cflags_edit   = new QLineEdit(cflags_box, "cflags");
    cxxflags_edit = new QLineEdit(cxxflags_box, "cxxflags");
    f77flags_edit = new QLineEdit(f77flags_box, "f77flags");

    QPushButton *cflags_button   = new QPushButton("...", cflags_box);
    cflags_button->setFixedSize(30, 30);
    QPushButton *cxxflags_button = new QPushButton("...", cxxflags_box);
    cxxflags_button->setFixedSize(30, 30);
    QPushButton *f77flags_button = new QPushButton("...", f77flags_box);
    f77flags_button->setFixedSize(30, 30);

    if (coffers.isEmpty())
        cflags_button->setEnabled(false);
    if (cxxoffers.isEmpty())
        cxxflags_button->setEnabled(false);
    if (f77offers.isEmpty())
        f77flags_button->setEnabled(false);
    
    connect(cflags_button,   SIGNAL(clicked()), this, SLOT(cflagsClicked()));
    connect(cxxflags_button, SIGNAL(clicked()), this, SLOT(cxxflagsClicked()));
    connect(f77flags_button, SIGNAL(clicked()), this, SLOT(f77flagsClicked()));

    QGridLayout *grid = new QGridLayout(this, 12, 3,
                                        KDialog::marginHint(), KDialog::spacingHint());
    grid->setColStretch(0, 1);
    grid->setColStretch(1, 2);
    grid->setColStretch(2, 1);
    grid->setRowStretch(0, 0);
    grid->setRowStretch(1, 0);
    grid->setRowStretch(2, 0);
    grid->setRowStretch(4, 0);
    grid->setRowStretch(5, 0);
    grid->setRowStretch(6, 0);
    grid->setRowStretch(8, 0);
    grid->setRowStretch(9, 0);
    grid->setRowStretch(10, 0);

    grid->addWidget(ccompiler_label, 0, 0);
    grid->addWidget(cbinary_label, 0, 1);
    grid->addWidget(cservice_combo, 1, 0);
    grid->addWidget(cbinary_edit, 1, 1);
    grid->addMultiCellWidget(cflags_box, 2, 2, 0, 2);
    grid->addMultiCellWidget(new KSeparator(this), 3, 3, 0, 2);
    
    grid->addWidget(cxxcompiler_label, 4, 0);
    grid->addWidget(cxxbinary_label, 4, 1);
    grid->addWidget(cxxservice_combo, 5, 0);
    grid->addWidget(cxxbinary_edit, 5, 1);
    grid->addMultiCellWidget(cxxflags_box, 6, 6, 0, 2);
    grid->addMultiCellWidget(new KSeparator(this), 7, 7, 0, 2);

    grid->addWidget(f77compiler_label, 8, 0);
    grid->addWidget(f77binary_label, 8, 1);
    grid->addWidget(f77service_combo, 9, 0);
    grid->addWidget(f77binary_edit, 9, 1);
    grid->addMultiCellWidget(f77flags_box, 10, 10, 0, 2);
    grid->addMultiCellWidget(new KSeparator(this), 11, 11, 0, 2);
}


CompilerPage::~CompilerPage()
{}


QString CompilerPage::cCompiler() const
{ return cservice_combo->currentText(); }
QString CompilerPage::cxxCompiler() const
{ return cxxservice_combo->currentText(); }
QString CompilerPage::f77Compiler() const
{ return f77service_combo->currentText(); }
QString CompilerPage::cFlags() const
{ return cflags_edit->text(); }
QString CompilerPage::cxxFlags() const
{ return cxxflags_edit->text(); }
QString CompilerPage::f77Flags() const
{ return f77flags_edit->text(); }
void CompilerPage::setCFlags(const QString &str)
{ return cflags_edit->setText(str); }
void CompilerPage::setCxxFlags(const QString &str)
{ return cxxflags_edit->setText(str); }
void CompilerPage::setF77Flags(const QString &str)
{ return f77flags_edit->setText(str); }


void CompilerPage::cflagsClicked()
{
    KDevCompilerOptions *dlg = createCompilerOptions(cCompiler());

    if (dlg) {
        dlg->setFlags(cFlags());
        dlg->exec();
        setCFlags(dlg->flags());
        delete dlg;
    }
}


void CompilerPage::cxxflagsClicked()
{
    KDevCompilerOptions *dlg = createCompilerOptions(cxxCompiler());

    if (dlg) {
        dlg->setFlags(cxxFlags());
        dlg->exec();
        setCxxFlags(dlg->flags());
        delete dlg;
    }
}


void CompilerPage::f77flagsClicked()
{
    KDevCompilerOptions *dlg = createCompilerOptions(f77Compiler());

    if (dlg) {
        dlg->setFlags(f77Flags());
        dlg->exec();
        setF77Flags(dlg->flags());
        delete dlg;
    }
}


KDevCompilerOptions *CompilerPage::createCompilerOptions(const QString &name)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service) {
        kdDebug(9000) << "Can't find service " << name;
        return 0;
    }
    
    KLibFactory *factory = KLibLoader::self()->factory(service->library());

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());
    
    QObject *obj = factory->create(this, service->name().latin1(),
                                   "KDevCompilerOptions", args);

    if (!obj->inherits("KDevCompilerOptions")) {
        kdDebug(9000) << "Component does not inherit KDevCompilerOptions" << endl;
        return 0;
    }
    KDevCompilerOptions *dlg = (KDevCompilerOptions*) obj;
    
    return dlg;
}


ProjectOptionsDialog::ProjectOptionsDialog(QWidget *parent, const char *name)
    : KDialogBase(TreeList, i18n("Project Options"), Ok|Cancel, Ok,
                  parent, name)
{
    QVBox *vbox;

    vbox = addVBoxPage(i18n("General"));
    generalpage = new GeneralPage(vbox, "general page");

    vbox = addVBoxPage(i18n("Compilers"));
    compilerpage = new CompilerPage(vbox, "compiler page");
}


ProjectOptionsDialog::~ProjectOptionsDialog()
{}


void ProjectOptionsDialog::accept()
{
    kdDebug(9000) << "CFLAGS=" << compilerpage->cFlags() << endl;
    kdDebug(9000) << "CXXFLAGS=" << compilerpage->cxxFlags() << endl;
    KDialogBase::accept();
}

#include "projectoptionsdlg.moc"
