#include "distpart_widget.h"

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include "kdevproject.h"
#include "domutil.h"

#include <kdevcore.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qnetwork.h>
#include <qurloperator.h>
#include <qmessagebox.h>
#include "distpart_part.h"
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include "kdevmakefrontend.h"

#include <qerrormessage.h>

DistpartDialog::DistpartDialog(DistpartPart *part)
        : distpart_ui(0, "distpart widget") {
    m_part = part;

    plug_box = new QTabWidget( tab_2, "plug_box" );
    tabLayout_2->addWidget( plug_box, 9, 2 );
    
    connect(customProjectCheckBox, SIGNAL(toggled(bool) ),
            this, SLOT(slotcustomProjectCheckBoxChanged()));
    connect(uploadCustomCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotuploadCustomCheckBoxChanged()));

    connect(okayPushButton, SIGNAL(clicked()),
            this, SLOT(slotokayPushButtonPressed()));
    connect(cancelPushButton, SIGNAL(clicked()),
            this, SLOT(slotcancelPushButtonPressed()));
    connect(help_PushButton, SIGNAL(clicked()),
            this, SLOT(slothelp_PushButtonPressed()));
    connect(createSrcArchPushButton, SIGNAL(clicked()),
            this, SLOT(slotcreateSrcArchPushButtonPressed()));
    connect(resetSrcPushButton, SIGNAL(clicked()),
            this, SLOT(slotresetSrcPushButtonPressed()));

    connect(genHTMLPushButton, SIGNAL(clicked()),
            this, SLOT(slotgenHTMLPushButtonPressed()));
    connect(resetHTMLPushButton, SIGNAL(clicked()),
            this, SLOT(slotresetHTMLPushButtonPressed()));
    connect(uploadSubmitPushButton, SIGNAL(clicked()),
            this, SLOT(slotuploadSubmitPushButtonPressed()));
    connect(uploadResetPushButton, SIGNAL(clicked()),
            this, SLOT(slotuploadResetPushButtonPressed()));
    connect(uploadAddFileToolButton, SIGNAL(clicked()),
            this, SLOT(slotuploadAddFileToolButtonPressed()));
    connect(uploadRemoveToolButton, SIGNAL(clicked()),
            this, SLOT(slotuploadRemoveToolButtonPressed()));

    loadSettings();

 
}


DistpartDialog::~DistpartDialog() {
    delete plug_box;
}

//    QPushButton* okayPushButton;

void DistpartDialog::slotokayPushButtonPressed() {
    storeSettings();
    accept();
}
//    QPushButton* cancelPushButton;

void DistpartDialog::slotcancelPushButtonPressed() {
    reject();
}
//    QPushButton* help_PushButton;

void DistpartDialog::slothelp_PushButtonPressed() {
    QMessageBox::about(this,"Distpart Help","In order to build a RPM package :\n\t1) Create a source archive\n\t2) Generate a SPEC File\n\t3) Build the package");
}

//    QPushButton* createSrcArchPushButton;
void DistpartDialog::slotcreateSrcArchPushButtonPressed() {
    QString dist = (getcustomProjectCheckBoxState() && getbzipCheckBoxState()) ? "make dist-bzip2" : "make dist";
    QString filename = getappNameFormatLineEditText() + 
		       "-" + 
		       getversionLineEditText() + 
		       ((getcustomProjectCheckBoxState() && getbzipCheckBoxState()) ? ".tar.bz2" : ".tar.gz");
    m_part->makeFrontend()->queueCommand(dir,"cd " + dir + " && " + dist);
}

//    QPushButton* resetSrcPushButton;
void DistpartDialog::slotresetSrcPushButtonPressed() {}



//    QPushButton* genHTMLPushButton;
void DistpartDialog::slotgenHTMLPushButtonPressed() {}

//    QPushButton* resetHTMLPushButton;
void DistpartDialog::slotresetHTMLPushButtonPressed() {}

//    QToolButton* uploadAddFileToolButton;
void DistpartDialog::slotuploadAddFileToolButtonPressed() {
    uploadFileListBox->insertStringList(KFileDialog::getOpenFileNames());
}

//    QToolButton* uploadRemoveToolButton;
void DistpartDialog::slotuploadRemoveToolButtonPressed() {
    for(unsigned int i=0; i<uploadFileListBox->count(); i++)
	if (uploadFileListBox->isSelected(i)) uploadFileListBox->removeItem(i);
}

//    QPushButton* uploadSubmitPushButton;
void DistpartDialog::slotuploadSubmitPushButtonPressed() {

    if(getuploadftpkdeorgCheckBoxState() || getuploadAppsKDEcomCheckBoxState())
        kdDebug() << "Implement ftp.kde.org & apps.kde.com ftp transfer" << endl;
    else {
        for(unsigned int i=0; i<uploadFileListBox->count(); i++)
	    KIO::NetAccess::copy("file:"+ uploadFileListBox->text(i),getuploadURLLineEditText() + uploadFileListBox->text(i).replace(QRegExp("[^/]*/"),""));
    }
}

//    QPushButton* uploadResetPushButton;
void DistpartDialog::slotuploadResetPushButtonPressed() {
    uploadFileListBox->clear();
}

// Connect Slots to the following widgets and add
// accessors and mutators


//    QCheckBox* customProjectCheckBox;
void DistpartDialog::slotcustomProjectCheckBoxChanged() {
    kdDebug () << "New State" << customProjectCheckBox->isChecked() << endl;
    sourceOptionsGroupBox->setEnabled(customProjectCheckBox->isChecked());
}

bool DistpartDialog::getcustomProjectCheckBoxState() {
    return customProjectCheckBox->isChecked();
}

void DistpartDialog::setcustomProjectCheckBoxState(bool state) {
    customProjectCheckBox->setChecked(state);
}

//  QCheckBox* uploadCustomCheckBox;
void DistpartDialog::slotuploadCustomCheckBoxChanged() {
    uploadURLLineEdit->setEnabled(uploadCustomCheckBox->isChecked());
}

bool DistpartDialog::getuploadCustomCheckBoxState() {
    return uploadCustomCheckBox->isChecked();
}

void DistpartDialog::setuploadCustomCheckBoxState(bool state) {
    uploadCustomCheckBox->setChecked(state);
}

// Add accessors and mutators for the following

//   QCheckBox* bzipCheckBox;
bool DistpartDialog::getbzipCheckBoxState() {
    return bzipCheckBox->isChecked();
}

void DistpartDialog::setbzipCheckBoxState(bool state) {
    bzipCheckBox->setChecked(state);
}

//    QCheckBox* appIconCheckBox;
bool DistpartDialog::getappIconCheckBoxState() {
    return appIconCheckBox->isChecked();
}

void DistpartDialog::setappIconCheckBoxState(bool state) {
    appIconCheckBox->setChecked(state);
}

//    QCheckBox* genHTMLCheckBox;
bool DistpartDialog::getgenHTMLCheckBoxState() {
    return genHTMLCheckBox->isChecked();
}

void DistpartDialog::setgenHTMLCheckBoxState(bool state) {
    genHTMLCheckBox->setChecked(state);
}

//    QCheckBox* useRPMInfoCheckBox;
bool DistpartDialog::getuseRPMInfoCheckBoxState() {
    return useRPMInfoCheckBox->isChecked();
}

void DistpartDialog::setuseRPMInfoCheckBoxState(bool state) {
    useRPMInfoCheckBox->setChecked(state);
}

//    QCheckBox* uploadAppsKDEcomCheckBox;
bool DistpartDialog::getuploadAppsKDEcomCheckBoxState() {
    return uploadAppsKDEcomCheckBox->isChecked();
}

void DistpartDialog::setuploadAppsKDEcomCheckBoxState(bool state) {
    uploadAppsKDEcomCheckBox->setChecked(state);
}

//    QCheckBox* uploadftpkdeorgCheckBox;
bool DistpartDialog::getuploadftpkdeorgCheckBoxState() {
    return uploadftpkdeorgCheckBox->isChecked();
}

void DistpartDialog::setuploadftpkdeorgCheckBoxState(bool state) {
    uploadftpkdeorgCheckBox->setChecked(state);
}

//    QCheckBox* devPackageCheckBox;
bool DistpartDialog::getdevPackageCheckBoxState() {
    return devPackageCheckBox->isChecked();
}

void DistpartDialog::setdevPackageCheckBoxState(bool state) {
    devPackageCheckBox->setChecked(state);
}

//    QCheckBox* docsPackageCheckBox;
bool DistpartDialog::getdocsPackageCheckBoxState() {
    return docsPackageCheckBox->isChecked();
}

void DistpartDialog::setdocsPackageCheckBoxState(bool state) {
    docsPackageCheckBox->setChecked(state);
}

//    QLineEdit* archNameFormatLineEdit;
QString DistpartDialog::getarchNameFormatLineEditText() {
    return archNameFormatLineEdit->text();
}

void DistpartDialog::setarchNameFormatLineEditText(QString text) {
    archNameFormatLineEdit->setText(text);
}

//    QLineEdit* appNameLineEdit;
QString DistpartDialog::getappNameFormatLineEditText() {
    return appNameLineEdit->text();
}

void DistpartDialog::setappNameFormatLineEditText(QString text) {
    appNameLineEdit->setText(text);
}

//    QLineEdit* summaryLineEdit;
QString DistpartDialog::getsummaryLineEditText() {
    return summaryLineEdit->text();
}

void DistpartDialog::setsummaryLineEditText(QString text) {
    summaryLineEdit->setText(text);
}

//    QLineEdit* groupLineEdit;
QString DistpartDialog::getgroupLineEditText() {
    return groupLineEdit->text();
}

void DistpartDialog::setgroupLineEditText(QString text) {
    groupLineEdit->setText(text);
}

//    QLineEdit* releaseLineEdit;
QString DistpartDialog::getreleaseLineEditText() {
    return releaseLineEdit->text();
}

void DistpartDialog::setreleaseLineEditText(QString text) {
    releaseLineEdit->setText(text);
}

//    QLineEdit* VersionLineEdit;
QString DistpartDialog::getversionLineEditText() {
    return versionLineEdit->text();
}

void DistpartDialog::setversionLineEditText(QString text) {
    versionLineEdit->setText( text );
}

//    QLineEdit* VendorLineEdit;
QString DistpartDialog::getvendorLineEditText() {
    return vendorLineEdit->text();
}

void DistpartDialog::setvendorLineEditText(QString text) {
    vendorLineEdit->setText(text);
}

//    QLineEdit* LicenseLineEdit;
QString DistpartDialog::getlicenseLineEditText() {
    return licenseLineEdit->text();
}

void DistpartDialog::setlicenseLineEditText(QString text) {
    licenseLineEdit->setText(text);
}

//    QLineEdit* uploadURLLineEdit;
QString DistpartDialog::getuploadURLLineEditText() {
    return uploadURLLineEdit->text();
}

void DistpartDialog::setuploadURLLineEditText(QString text) {
    uploadURLLineEdit->setText(text);
}

//    QLineEdit* PackagerLineEdit;
QString DistpartDialog::getpackagerLineEditText() {
    return packagerLineEdit->text();
}

void DistpartDialog::setpackagerLineEditText(QString text) {
    packagerLineEdit->setText(text);
}

//    QComboBox* archComboBox;
QString DistpartDialog::getarchComboBoxText() {
    return archComboBox->currentText();
}

int DistpartDialog::getarchComboBoxItem() {
    return archComboBox->currentItem();
}

void DistpartDialog::setarchComboBoxItem(int item) {
    archComboBox->setCurrentItem( item );
}

//    QListBox* uploadFileListBox;
QString DistpartDialog::getuploadFileListBoxText() {
    return uploadFileListBox->currentText();
}

int DistpartDialog::getuploadFileListBoxItem() {
    return uploadFileListBox->currentItem();
}

void DistpartDialog::setuploadFileListBoxItem(int item) {
    uploadFileListBox->setCurrentItem( item);
}

//    QListBox* srcDistFileListBox;
QString DistpartDialog::getsrcDistFileListBoxText() {
    return srcDistFileListBox->currentText ();
}

int DistpartDialog::getsrcDistFileListBoxItem() {
    return srcDistFileListBox->currentItem ();
}

void DistpartDialog::setsrcDistFileListBoxItem(int item) {
    srcDistFileListBox->setCurrentItem( item );
}

//    QMultiLineEdit* projectDescriptionMultilineEdit;
QString DistpartDialog::getprojectDescriptionMultilineEditText() {
    return projectDescriptionMultilineEdit->text();
}

void DistpartDialog::setprojectDescriptionMultilineEditText(QString text) {
    projectDescriptionMultilineEdit->setText(text);
}

QString DistpartDialog::getprojectChangelogMultilineEditText() {
    return projectChangelogMultilineEdit->text();
}

void DistpartDialog::setprojectChangelogMultilineEditText(QString text) {
    projectChangelogMultilineEdit->setText(text);
}

void DistpartDialog::loadSettings() {
    QDomDocument &dom = *m_part->projectDom();
    dir = m_part->project()->projectDirectory();
    QDir projectdir(dir);
    srcDistFileListBox->insertStringList(projectdir.entryList());

    // First Tab
    setcustomProjectCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/custom",false));
    slotcustomProjectCheckBoxChanged();
    setbzipCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/bzip",false));
    setarchNameFormatLineEditText(DomUtil::readEntry(dom,"/dist/archname"));


    // Second Tab
    setappNameFormatLineEditText(DomUtil::readEntry(dom,"/dist/appname",DomUtil::readEntry(dom,"/general/projectname")));
    setversionLineEditText(DomUtil::readEntry(dom,"/dist/version",DomUtil::readEntry(dom,"/general/version")));
    setreleaseLineEditText(DomUtil::readEntry(dom,"/dist/release"));
    setvendorLineEditText(DomUtil::readEntry(dom,"/dist/vendor"));
    setlicenseLineEditText(DomUtil::readEntry(dom,"/dist/licence"));
    setsummaryLineEditText(DomUtil::readEntry(dom,"/dist/summary"));
    setgroupLineEditText(DomUtil::readEntry(dom,"/dist/group"));
    setpackagerLineEditText(DomUtil::readEntry(dom,"/dist/packager"));
    setprojectDescriptionMultilineEditText(DomUtil::readEntry(dom,"/dist/description",DomUtil::readEntry(dom,"/general/description")));
    setprojectChangelogMultilineEditText(DomUtil::readEntry(dom,"/dist/changelog"));
    setdevPackageCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/devpackage"));
    setdocsPackageCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/docspackage"));
    setappIconCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/appicon"));
    setarchComboBoxItem(DomUtil::readIntEntry(dom,"/dist/arch"));

    // Third Tab
    setgenHTMLCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/genHTML"));
    setuseRPMInfoCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/useRPM"));
    setuploadftpkdeorgCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/ftpkde"));
    setuploadAppsKDEcomCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/appskde"));
    setuploadCustomCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/custom"));
    slotuploadCustomCheckBoxChanged();
    setuploadURLLineEditText(DomUtil::readEntry(dom,"/dist/url"));
}

void DistpartDialog::storeSettings() {
    QDomDocument &dom = *m_part->projectDom();

    // First Tab
    DomUtil::writeBoolEntry(dom,"/dist/custom",getcustomProjectCheckBoxState());
    DomUtil::writeBoolEntry(dom,"/dist/bzip",getbzipCheckBoxState());
    DomUtil::writeEntry(dom,"/dist/archname",getarchNameFormatLineEditText());

    // Second Tab
    DomUtil::writeEntry(dom,"/dist/appname",getappNameFormatLineEditText());
    DomUtil::writeEntry(dom,"/dist/version",getversionLineEditText());
    DomUtil::writeEntry(dom,"/dist/release",getreleaseLineEditText());
    DomUtil::writeEntry(dom,"/dist/vendor",getvendorLineEditText());
    DomUtil::writeEntry(dom,"/dist/licence",getlicenseLineEditText());
    DomUtil::writeEntry(dom,"/dist/summary",getsummaryLineEditText());
    DomUtil::writeEntry(dom,"/dist/group",getgroupLineEditText());
    DomUtil::writeEntry(dom,"/dist/packager",getpackagerLineEditText());
    DomUtil::writeEntry(dom,"/dist/description",getprojectDescriptionMultilineEditText());
    DomUtil::writeEntry(dom,"/dist/changelog",getprojectChangelogMultilineEditText());    
    DomUtil::writeBoolEntry(dom,"/dist/devpackage",getdevPackageCheckBoxState());
    DomUtil::writeBoolEntry(dom,"/dist/docspackage",getdocsPackageCheckBoxState());
    DomUtil::writeBoolEntry(dom,"/dist/appicon",getappIconCheckBoxState());
    DomUtil::writeIntEntry(dom,"/dist/arch",getarchComboBoxItem());

    // Third Tab
    DomUtil::writeBoolEntry(dom,"/dist/genHTML",getgenHTMLCheckBoxState());
    DomUtil::writeBoolEntry(dom,"/dist/useRPM",getuseRPMInfoCheckBoxState());
    DomUtil::writeBoolEntry(dom,"/dist/ftpkde",getuploadftpkdeorgCheckBoxState());
    DomUtil::writeBoolEntry(dom,"/dist/appskde",getuploadAppsKDEcomCheckBoxState());
    DomUtil::writeBoolEntry(dom,"/dist/custom",getuploadCustomCheckBoxState());
    DomUtil::writeEntry(dom,"/dist/url",getuploadURLLineEditText());
}


#include "distpart_widget.moc"
