#include "distpart_widget.h"

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include <kdevcore.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include "distpart_part.h"


DistpartDialog::DistpartDialog(DistpartPart *part)
	: distpart_ui(0, "distpart widget")
{
    connect(customProjectCheckBox, SIGNAL(toggled(bool) ),
           this, SLOT(slotcustomProjectCheckBoxChanged()));
    connect(uploadCustomCheckBox, SIGNAL(toggled(bool)),
           this, SLOT(slotuploadCustomCheckBoxChanged()));

}


DistpartDialog::~DistpartDialog()
{
}

//    QPushButton* okayPushButton;
void DistpartDialog::slotokayPushButtonPressed()
{
   emit okay();
}
//    QPushButton* cancelPushButton;
void DistpartDialog::slotcancelPushButtonPressed()
{
   emit cancel();
}
//    QPushButton* help_PushButton;
void DistpartDialog::slothelp_PushButtonPressed()
{
   emit help();
}
//    QPushButton* createSrcArchPushButton;
void DistpartDialog::slotcreateSrcArchPushButtonPressed()
{
}
//    QPushButton* resetSrcPushButton;
void DistpartDialog::slotresetSrcPushButtonPressed()
{
}
//    QPushButton* buildAllPushButton;
void DistpartDialog::slotbuildAllPushButtonPressed()
{
}
//    QPushButton* exportSPECPushButton;
void DistpartDialog::slotexportSPECPushButtonPressed()
{
}
//    QPushButton* importSPECPushButton;
void DistpartDialog::slotimportPushButtonPressed()
{
}
//    QPushButton* srcPackagePushButton;
void DistpartDialog::slotsrcPackagePushButtonPressed()
{
}
//    QPushButton* genHTMLPushButton;
void DistpartDialog::slotgenHTMLPushButtonPressed()
{

}
//    QPushButton* resetHTMLPushButton;
void DistpartDialog::slotresetHTMLPushButtonPressed()
{

}
//    QToolButton* uploadAddFileToolButton;
void DistpartDialog::slotuploadAddFileToolButtonPressed()
{

}
//    QToolButton* uploadRemoveToolButton;
void DistpartDialog::slotuploadRemoveToolButtonPressed()
{

}
//    QPushButton* uploadSubmitPushButton;
void DistpartDialog::slotuploadSubmitPushButtonPressed()
{

}
//    QPushButton* uploadResetPushButton;
void DistpartDialog::slotuploadResetPushButtonPressed()
{

}

// Connect Slots to the following widgets and add
// accessors and mutators
//    QCheckBox* customProjectCheckBox;
void DistpartDialog::slotcustomProjectCheckBoxChanged()
{
  kdDebug () << "New State" << customProjectCheckBox->isChecked() << endl;
  sourceOptionsGroupBox->setEnabled(customProjectCheckBox->isChecked());
}
bool DistpartDialog::getcustomProjectCheckBoxState()
{
    return customProjectCheckBox->isChecked();
}
void DistpartDialog::setcustomProjectCheckBoxState(bool state)
{
    customProjectCheckBox->setChecked(state);
}
//  QCheckBox* uploadCustomCheckBox;
void DistpartDialog::slotuploadCustomCheckBoxChanged()
{
    uploadURLLineEdit->setEnabled(uploadCustomCheckBox->isChecked());
}
bool DistpartDialog::getuploadCustomCheckBoxState()
{
    return uploadCustomCheckBox->isChecked();
}
void DistpartDialog::setuploadCustomCheckBoxState(bool state)
{
    uploadCustomCheckBox->setChecked(state);
}

// Add accessors and mutators for the following

//   QCheckBox* bzipCheckBox;
bool DistpartDialog::getbzipCheckBoxState()
{
    return bzipCheckBox->isChecked();
}
void DistpartDialog::setbzipCheckBoxState(bool state)
{
    bzipCheckBox->setChecked(state);
}
//    QCheckBox* appIconCheckBox;
bool DistpartDialog::getappIconCheckBoxState()
{
    return appIconCheckBox->isChecked();
}
void DistpartDialog::setappIconCheckBoxState(bool state)
{
    appIconCheckBox->setChecked(state);
}
//    QCheckBox* genHTMLCheckBox;
bool DistpartDialog::getgenHTMLCheckBoxState()
{
    return genHTMLCheckBox->isChecked();
}
void DistpartDialog::setgenHTMLCheckBoxState(bool state)
{
    genHTMLCheckBox->setChecked(state);
}
//    QCheckBox* useRPMInfoCheckBox;
bool DistpartDialog::getuseRPMInfoCheckBoxState()
{
    return useRPMInfoCheckBox->isChecked();
}
void DistpartDialog::setuseRPMInfoCheckBoxState(bool state)
{
     useRPMInfoCheckBox->setChecked(state);
}
//    QCheckBox* uploadAppsKDEcomCheckBox;
bool DistpartDialog::getuploadAppsKDEcomCheckBoxState()
{
     return uploadAppsKDEcomCheckBox->isChecked();
}
void DistpartDialog::setuploadAppsKDEcomCheckBoxState(bool state)
{
     uploadAppsKDEcomCheckBox->setChecked(state);
}
//    QCheckBox* uploadftpkdeorgCheckBox;
bool DistpartDialog::getuploadftpkdeorgCheckBoxState()
{
    return uploadftpkdeorgCheckBox->isChecked();
}
void DistpartDialog::setuploadftpkdeorgCheckBoxState(bool state)
{
    uploadftpkdeorgCheckBox->setChecked(state);
}
//    QCheckBox* devPackageCheckBox;
bool DistpartDialog::getdevPackageCheckBoxState()
{
    return devPackageCheckBox->isChecked();
}
void DistpartDialog::setdevPackageCheckBoxState(bool state)
{
    devPackageCheckBox->setChecked(state);
}
//    QCheckBox* docsPackageCheckBox;
bool DistpartDialog::getdocsPackageCheckBoxState()
{
    return docsPackageCheckBox->isChecked();
}
void DistpartDialog::setdocsPackageCheckBoxState(bool state)
{
    docsPackageCheckBox->setChecked(state);
}
//    QProgressBar* uploadProgressBar;
int DistpartDialog::getuploadProgressBarProgress()
{
    return uploadProgressBar->progress();
}
void DistpartDialog::setuploadProgressBarProgress(int progress)
{
    uploadProgressBar->setProgress( progress );
}
//    QLineEdit* archNameFormatLineEdit;
QString DistpartDialog::getarchNameFormatLineEditText()
{
    return archNameFormatLineEdit->text();
}
void DistpartDialog::setarchNameFormatLineEditText(QString text)
{
    archNameFormatLineEdit->setText(text);
}
//    QLineEdit* appNameLineEdit;
QString DistpartDialog::getappNameFormatLineEditText()
{
    return appNameLineEdit->text();
}
void DistpartDialog::setappNameFormatLineEditText(QString text)
{
    appNameLineEdit->setText(text);
}
//    QLineEdit* summaryLineEdit;
QString DistpartDialog::getsummaryLineEditText()
{
   return summaryLineEdit->text();
}
void DistpartDialog::setsummaryLineEditText(QString text)
{
   summaryLineEdit->setText(text);
}
//    QLineEdit* authorLineEdit;
QString DistpartDialog::getauthorLineEditText()
{
   return authorLineEdit->text();
}
void DistpartDialog::setauthorLineEditText(QString text)
{
   authorLineEdit->setText(text);
}
//    QLineEdit* groupLineEdit;
QString DistpartDialog::getgroupLineEditText()
{
    return groupLineEdit->text();
}
void DistpartDialog::setgroupLineEditText(QString text)
{
    groupLineEdit->setText(text);
}
//    QLineEdit* releaseLineEdit;
QString DistpartDialog::getreleaseLineEditText()
{
   return releaseLineEdit->text();
}
void DistpartDialog::setreleaseLineEditText(QString text)
{
   releaseLineEdit->setText(text);
}
//    QLineEdit* VersionLineEdit;
QString DistpartDialog::getversionLineEditText()
{
   return versionLineEdit->text();
}
void DistpartDialog::setversionLineEditText(QString text)
{
     versionLineEdit->setText( text );
}
//    QLineEdit* VendorLineEdit;
QString DistpartDialog::getvendorLineEditText()
{
    return vendorLineEdit->text();
}
void DistpartDialog::setvendorLineEditText(QString text)
{
    vendorLineEdit->setText(text);
}
//    QLineEdit* LicenseLineEdit;
QString DistpartDialog::getlicenseLineEditText()
{
   return licenseLineEdit->text();
}
void DistpartDialog::setlicenseLineEditText(QString text)
{
    licenseLineEdit->setText(text);
}
//    QLineEdit* uploadURLLineEdit;
QString DistpartDialog::getuploadURLLineEditText()
{
    return uploadURLLineEdit->text();
}
void DistpartDialog::setuploadURLLineEditText(QString text)
{
     uploadURLLineEdit->setText(text);
}
//    QLineEdit* PackagerLineEdit;
QString DistpartDialog::getpackagerLineEditText()
{
   return packagerLineEdit->text();
}
void DistpartDialog::setpackagerLineEditText(QString text)
{
   packagerLineEdit->setText(text);
}
//    QComboBox* archComboBox;
QString DistpartDialog::getarchComboBoxText()
{
   return archComboBox->currentText();
}
int DistpartDialog::getarchComboBoxItem()
{
   return archComboBox->currentItem();
}
void DistpartDialog::setarchComboBoxItem(int item)
{
   archComboBox->setCurrentItem( item );
}
//    QListBox* uploadFileListBox;
QString DistpartDialog::getuploadFileListBoxText()
{
    return uploadFileListBox->currentText();
}

int DistpartDialog::getuploadFileListBoxItem()
{
    return uploadFileListBox->currentItem();
}
void DistpartDialog::setuploadFileListBoxItem(int item)
{
    uploadFileListBox->setCurrentItem( item);
}
//    QListBox* srcDistFileListBox;
QString DistpartDialog::getsrcDistFileListBoxText()
{
    return srcDistFileListBox->currentText ();
}
int DistpartDialog::getsrcDistFileListBoxItem()
{
    return srcDistFileListBox->currentItem ();
}
void DistpartDialog::setsrcDistFileListBoxItem(int item)
{
    srcDistFileListBox->setCurrentItem( item );
}
//    QMultiLineEdit* projectDescriptionMultilineEdit;
QString DistpartDialog::getprojectDescriptionMultilineEditText()
{
    return projectDescriptionMultilineEdit->text();
}
void DistpartDialog::setprojectDescriptionMultilineEditText(QString text)
{
    projectDescriptionMultilineEdit->setText(text);
}

#include "distpart_widget.moc"
