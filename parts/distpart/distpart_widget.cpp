/***************************************************************************
 *   Copyright (C) 2004 by ian reinhart geiser                             *
 *   geiseri@kde.org                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "distpart_widget.h"

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include "kdevproject.h"
#include "domutil.h"
#include <ktar.h>
#include <kmessagebox.h>

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
#include <kprogress.h>

#include <qerrormessage.h>
#include "specsupport.h"
#include "lsmsupport.h"

DistpartDialog::DistpartDialog(DistpartPart *part, QWidget *parent)
        :distpart_ui(parent, "dist_widget"), m_part(part) {
    m_spec = new SpecSupport(m_part);

    connect( customProjectCheckBox, SIGNAL(toggled(bool) ),
            this, SLOT(slotcustomProjectCheckBoxChanged()));
    connect( uploadCustomCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(slotuploadCustomCheckBoxChanged()));

//    connect( okayPushButton, SIGNAL(clicked()),
//            this, SLOT(slotokayPushButtonPressed()));
//    connect( cancelPushButton, SIGNAL(clicked()),
//            this, SLOT(slotcancelPushButtonPressed()));
/*    connect(help_PushButton, SIGNAL(clicked()),
            this, SLOT(slothelp_PushButtonPressed()));*/
    connect( createSrcArchPushButton, SIGNAL(clicked()),
            this, SLOT(slotcreateSrcArchPushButtonPressed()));
    connect( resetSrcPushButton, SIGNAL(clicked()),
            this, SLOT(slotresetSrcPushButtonPressed()));

    connect( genHTMLPushButton, SIGNAL(clicked()),
            this, SLOT(slotgenHTMLPushButtonPressed()));
    connect( resetHTMLPushButton, SIGNAL(clicked()),
            this, SLOT(slotresetHTMLPushButtonPressed()));
    connect( uploadSubmitPushButton, SIGNAL(clicked()),
            this, SLOT(slotuploadSubmitPushButtonPressed()));
    connect( uploadResetPushButton, SIGNAL(clicked()),
            this, SLOT(slotuploadResetPushButtonPressed()));
    connect( uploadAddFileToolButton, SIGNAL(clicked()),
            this, SLOT(slotuploadAddFileToolButtonPressed()));
    connect( uploadRemoveToolButton, SIGNAL(clicked()),
            this, SLOT(slotuploadRemoveToolButtonPressed()));
//	connect(  buildAllPushButton, SIGNAL( clicked() ), this, SLOT( slotbuildAllPushButtonPressed() ));
//	connect(  exportSPECPushButton, SIGNAL( clicked() ), this, SLOT( slotexportSPECPushButtonPressed( ) ) );
//	connect(  srcPackagePushButton, SIGNAL( clicked() ), this, SLOT( slotsrcPackagePushButtonPressed() ) );
//	connect(  importSPECPushButton, SIGNAL( clicked() ), this, SLOT( slotimportSPECPushButtonPressed() ) );
    loadSettings();


     localOptionsGroupBox->setEnabled(false);
     devPackageCheckBox->setEnabled(false);
     docsPackageCheckBox->setEnabled(false);
     appIconCheckBox->setEnabled(false);
     uploadftpkdeorgCheckBox->setEnabled(false);
     uploadAppsKDEcomCheckBox->setEnabled(false);
}


DistpartDialog::~DistpartDialog() {
}

//    QPushButton* okayPushButton;

void DistpartDialog::slotokayPushButtonPressed() {
    storeSettings();
//    accept();
}
//    QPushButton* cancelPushButton;

void DistpartDialog::slotcancelPushButtonPressed() {
//    reject();
}
//    QPushButton* help_PushButton;

/*void DistpartDialog::slothelp_PushButtonPressed() {
    QMessageBox::about(this,i18n("Distpart Help"),i18n("In order to build a RPM package :\n\t1) Create a source archive\n\t2) Generate a SPEC File\n\t3) Build the package"));
}*/

//    QPushButton* createSrcArchPushButton;
void DistpartDialog::slotcreateSrcArchPushButtonPressed() {
    kdDebug() << "Starting archive..." << endl;
    QString dist = (getcustomProjectCheckBoxState() && getbzipCheckBoxState()) ? "application/x-bzip2" : "application/x-gzip";

    QString filename = m_part->project()->projectDirectory() + "/" + getappNameFormatLineEditText() +
		       "-" +
		       getversionLineEditText() +
		       ((getcustomProjectCheckBoxState() && getbzipCheckBoxState()) ? ".tar.bz2" : ".tar.gz");

    KTar tar(filename, dist);
    if ( tar.open(IO_WriteOnly) )
    {
    	QStringList files = m_part->project()->distFiles();
	KProgressDialog *prog = new KProgressDialog( m_part->widget(), "dialog", i18n("Building Package"), "", true );
	prog->show();
	for( uint idx = 0; idx < files.count(); ++idx)
	{
		if ( !tar.addLocalFile( m_part->project()->projectDirectory() + "/" + files[idx], getappNameFormatLineEditText() + "/" + files[idx]) )
		{
			kdDebug() << "Failed to write file " << files[idx] << endl;
		}
		else
		{
			prog->setLabel(i18n("Adding file: ") + files[idx]);
			prog->progressBar()->setValue( (idx*100)/files.count() );
		}
	}
    	tar.close( );
	prog->hide();
	delete prog;
	KMessageBox::information( this, i18n("Archive made at: ") + filename, i18n("Archive Done") );

    }
    else
    {
    	kdDebug() << "Failed to open archive..." << endl;
    }

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
    for(unsigned int i=0; i<  uploadFileListBox->count(); i++)
	if ( uploadFileListBox->isSelected(i))  uploadFileListBox->removeItem(i);
}

//    QPushButton* uploadSubmitPushButton;
void DistpartDialog::slotuploadSubmitPushButtonPressed() {

    if(getuploadftpkdeorgCheckBoxState() || getuploadAppsKDEcomCheckBoxState())
        kdDebug() << "Implement ftp.kde.org & apps.kde.com ftp transfer" << endl;
    else {
        for(unsigned int i=0; i< uploadFileListBox->count(); i++)
	    KIO::NetAccess::copy(KURL::fromPathOrURL( uploadFileListBox->text(i) ),
                    KURL::fromPathOrURL( getuploadURLLineEditText() +  uploadFileListBox->text(i).replace(QRegExp("[^/]*/"),"") ));
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
    kdDebug () << "New State" <<  customProjectCheckBox->isChecked() << endl;
     sourceOptionsGroupBox->setEnabled( customProjectCheckBox->isChecked());
}

bool DistpartDialog::getcustomProjectCheckBoxState() {
    return  customProjectCheckBox->isChecked();
}

void DistpartDialog::setcustomProjectCheckBoxState(bool state) {
     customProjectCheckBox->setChecked(state);
}

//  QCheckBox* uploadCustomCheckBox;
void DistpartDialog::slotuploadCustomCheckBoxChanged() {
     uploadURLLineEdit->setEnabled( uploadCustomCheckBox->isChecked());
}

bool DistpartDialog::getuploadCustomCheckBoxState() {
    return  uploadCustomCheckBox->isChecked();
}

void DistpartDialog::setuploadCustomCheckBoxState(bool state) {
     uploadCustomCheckBox->setChecked(state);
}

// Add accessors and mutators for the following

//   QCheckBox* bzipCheckBox;
bool DistpartDialog::getbzipCheckBoxState() {
    return  bzipCheckBox->isChecked();
}

void DistpartDialog::setbzipCheckBoxState(bool state) {
     bzipCheckBox->setChecked(state);
}

//    QCheckBox* appIconCheckBox;
bool DistpartDialog::getappIconCheckBoxState() {
    return  appIconCheckBox->isChecked();
}

void DistpartDialog::setappIconCheckBoxState(bool state) {
     appIconCheckBox->setChecked(state);
}

//    QCheckBox* genHTMLCheckBox;
bool DistpartDialog::getgenHTMLCheckBoxState() {
    return  genHTMLCheckBox->isChecked();
}

void DistpartDialog::setgenHTMLCheckBoxState(bool state) {
     genHTMLCheckBox->setChecked(state);
}

//    QCheckBox* useRPMInfoCheckBox;
bool DistpartDialog::getuseRPMInfoCheckBoxState() {
    return  useRPMInfoCheckBox->isChecked();
}

void DistpartDialog::setuseRPMInfoCheckBoxState(bool state) {
     useRPMInfoCheckBox->setChecked(state);
}

//    QCheckBox* uploadAppsKDEcomCheckBox;
bool DistpartDialog::getuploadAppsKDEcomCheckBoxState() {
    return  uploadAppsKDEcomCheckBox->isChecked();
}

void DistpartDialog::setuploadAppsKDEcomCheckBoxState(bool state) {
     uploadAppsKDEcomCheckBox->setChecked(state);
}

//    QCheckBox* uploadftpkdeorgCheckBox;
bool DistpartDialog::getuploadftpkdeorgCheckBoxState() {
    return  uploadftpkdeorgCheckBox->isChecked();
}

void DistpartDialog::setuploadftpkdeorgCheckBoxState(bool state) {
     uploadftpkdeorgCheckBox->setChecked(state);
}

//    QCheckBox* devPackageCheckBox;
bool DistpartDialog::getdevPackageCheckBoxState() {
    return  devPackageCheckBox->isChecked();
}

void DistpartDialog::setdevPackageCheckBoxState(bool state) {
     devPackageCheckBox->setChecked(state);
}

//    QCheckBox* docsPackageCheckBox;
bool DistpartDialog::getdocsPackageCheckBoxState() {
    return  docsPackageCheckBox->isChecked();
}

void DistpartDialog::setdocsPackageCheckBoxState(bool state) {
     docsPackageCheckBox->setChecked(state);
}

//    QLineEdit* archNameFormatLineEdit;
QString DistpartDialog::getarchNameFormatLineEditText() {
    return  archNameFormatLineEdit->text();
}

void DistpartDialog::setarchNameFormatLineEditText(QString text) {
     archNameFormatLineEdit->setText(text);
}

//    QLineEdit* appNameLineEdit;
QString DistpartDialog::getappNameFormatLineEditText() {
    return  appNameLineEdit->text();
}

void DistpartDialog::setappNameFormatLineEditText(QString text) {
     appNameLineEdit->setText(text);
}

//    QLineEdit* summaryLineEdit;
QString DistpartDialog::getsummaryLineEditText() {
    return  summaryLineEdit->text();
}

void DistpartDialog::setsummaryLineEditText(QString text) {
     summaryLineEdit->setText(text);
}

//    QLineEdit* groupLineEdit;
QString DistpartDialog::getgroupLineEditText() {
    return  groupLineEdit->text();
}

void DistpartDialog::setgroupLineEditText(QString text) {
     groupLineEdit->setText(text);
}

//    QLineEdit* releaseLineEdit;
QString DistpartDialog::getreleaseLineEditText() {
    return  releaseLineEdit->text();
}

void DistpartDialog::setreleaseLineEditText(QString text) {
     releaseLineEdit->setText(text);
}

//    QLineEdit* VersionLineEdit;
QString DistpartDialog::getversionLineEditText() {
    return  versionLineEdit->text();
}

void DistpartDialog::setversionLineEditText(QString text) {
     versionLineEdit->setText( text );
}

//    QLineEdit* VendorLineEdit;
QString DistpartDialog::getvendorLineEditText() {
    return  vendorLineEdit->text();
}

void DistpartDialog::setvendorLineEditText(QString text) {
     vendorLineEdit->setText(text);
}

//    QLineEdit* LicenseLineEdit;
QString DistpartDialog::getlicenseLineEditText() {
    return  licenseLineEdit->text();
}

void DistpartDialog::setlicenseLineEditText(QString text) {
     licenseLineEdit->setText(text);
}

//    QLineEdit* uploadURLLineEdit;
QString DistpartDialog::getuploadURLLineEditText() {
    return  uploadURLLineEdit->text();
}

void DistpartDialog::setuploadURLLineEditText(QString text) {
     uploadURLLineEdit->setText(text);
}

//    QLineEdit* PackagerLineEdit;
QString DistpartDialog::getpackagerLineEditText() {
    return  packagerLineEdit->text();
}

void DistpartDialog::setpackagerLineEditText(QString text) {
     packagerLineEdit->setText(text);
}

//    QComboBox* archComboBox;
QString DistpartDialog::getarchComboBoxText() {
    return  archComboBox->currentText();
}

int DistpartDialog::getarchComboBoxItem() {
    return  archComboBox->currentItem();
}

void DistpartDialog::setarchComboBoxItem(int item) {
     archComboBox->setCurrentItem( item );
}

//    QListBox* uploadFileListBox;
QString DistpartDialog::getuploadFileListBoxText() {
    return  uploadFileListBox->currentText();
}

int DistpartDialog::getuploadFileListBoxItem() {
    return  uploadFileListBox->currentItem();
}

void DistpartDialog::setuploadFileListBoxItem(int item) {
     uploadFileListBox->setCurrentItem( item);
}

//    QListBox* srcDistFileListBox;
QString DistpartDialog::getsrcDistFileListBoxText() {
    return  srcDistFileListBox->currentText ();
}

int DistpartDialog::getsrcDistFileListBoxItem() {
    return  srcDistFileListBox->currentItem ();
}

void DistpartDialog::setsrcDistFileListBoxItem(int item) {
     srcDistFileListBox->setCurrentItem( item );
}

//    QMultiLineEdit* projectDescriptionMultilineEdit;
QString DistpartDialog::getprojectDescriptionMultilineEditText() {
    return  projectDescriptionMultilineEdit->text();
}

void DistpartDialog::setprojectDescriptionMultilineEditText(QString text) {
     projectDescriptionMultilineEdit->setText(text);
}

QString DistpartDialog::getprojectChangelogMultilineEditText() {
    return  projectChangelogMultilineEdit->text();
}

void DistpartDialog::setprojectChangelogMultilineEditText(QString text) {
     projectChangelogMultilineEdit->setText(text);
}

QString DistpartDialog::getSourceName() {
    QString name = (getcustomProjectCheckBoxState()) ? getarchNameFormatLineEditText() : QString("%n-%v");
    name += (getcustomProjectCheckBoxState() && getbzipCheckBoxState()) ? ".tar.bz2" : ".tar.gz";
    return name.replace(QRegExp("%n"),getappNameFormatLineEditText())
	.replace(QRegExp("%v"),getversionLineEditText())
	.replace(QRegExp("%d"),QDate::currentDate().toString("yyyyMMdd"));
}

void DistpartDialog::loadSettings() {
    QDomDocument &dom = *m_part->projectDom();

     srcDistFileListBox->insertStringList( m_part->project()->distFiles() );

    // First Tab
    setcustomProjectCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/custom",false));
    slotcustomProjectCheckBoxChanged();
    setbzipCheckBoxState(DomUtil::readBoolEntry(dom,"/dist/bzip",false));
    setarchNameFormatLineEditText(DomUtil::readEntry(dom,"/dist/archname"));


    // Second Tab
    setappNameFormatLineEditText(DomUtil::readEntry(dom,"/dist/appname", m_part->project()->projectName() ));
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
// Populate Spec Data from UI;
void DistpartDialog::setPackageData()
{
	m_spec->setAppRevision(  releaseLineEdit->text());
	m_spec->setAppName(  appNameLineEdit->text());
	m_spec->setAppVendor(  vendorLineEdit->text());
	m_spec->setAppLicense(  licenseLineEdit->text());
	m_spec->setAppSummary(  summaryLineEdit->text());
	m_spec->setAppPackager(  packagerLineEdit->text());
	m_spec->setAppVersion(  versionLineEdit->text());
	m_spec->setAppGroup(  groupLineEdit->text());
	m_spec->setAppDescription(  projectDescriptionMultilineEdit->text());
	m_spec->setAppChangelog(  projectChangelogMultilineEdit->text());
}
// Populate UI from Spec data;
void DistpartDialog::getPackageData()
{
	 releaseLineEdit->setText(m_spec->getAppRevision());
	 appNameLineEdit->setText(m_spec->getAppName());
	 vendorLineEdit->setText(m_spec->getAppVendor());
	 licenseLineEdit->setText(m_spec->getAppLicense( ));
	 summaryLineEdit->setText(m_spec->getAppSummary( ));
	 packagerLineEdit->setText(m_spec->getAppPackager( ));
	 versionLineEdit->setText(m_spec->getAppVersion());
	 groupLineEdit->setText(m_spec->getAppGroup( ));
	 projectDescriptionMultilineEdit->setText(m_spec->getAppDescription( ));
	 projectChangelogMultilineEdit->setText(m_spec->getAppChangelog( ));
}
void DistpartDialog::slotbuildAllPushButtonPressed( )
{
	setPackageData();
	m_spec->slotbuildAllPushButtonPressed();
}

void DistpartDialog::slotexportSPECPushButtonPressed( )
{
	setPackageData();
	m_spec->slotexportSPECPushButtonPressed();
}

void DistpartDialog::slotimportSPECPushButtonPressed( )
{
	m_spec->slotimportSPECPushButtonPressed();
	getPackageData();
}

void DistpartDialog::slotsrcPackagePushButtonPressed( )
{
	setPackageData();
	m_spec->slotsrcPackagePushButtonPressed();
}


#include "distpart_widget.moc"
