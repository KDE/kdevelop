#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include <kdevcore.h>
#include "distpart_part.h"
#include "distpart_widget.h"
#include <qlayout.h>

distpartWidget::distpartWidget(distpartPart *part)
	: distpart_ui(0, "distpart widget")
{
    connect(customProjectCheckBox, SIGNAL(released( ) ),
           this, SLOT(slotcustomProjectCheckBoxChanged()));
    connect(uploadCustomCheckBox, SIGNAL(released()),
           this, SLOT(slotuploadCustomCheckBoxChanged()));

}


distpartWidget::~distpartWidget()
{
}

//    QPushButton* okayPushButton;
void distpartWidget::slotokayPushButtonPressed()
{
   emit okay();
}
//    QPushButton* cancelPushButton;
void distpartWidget::slotcancelPushButtonPressed()
{
   emit cancel();
}
//    QPushButton* help_PushButton;
void distpartWidget::slothelp_PushButtonPressed()
{
   emit help();
}
//    QPushButton* createSrcArchPushButton;
void distpartWidget::slotcreateSrcArchPushButtonPressed()
{
}
//    QPushButton* resetSrcPushButton;
void distpartWidget::slotresetSrcPushButtonPressed()
{
}
//    QPushButton* buildAllPushButton;
void distpartWidget::slotbuildAllPushButtonPressed()
{
}
//    QPushButton* exportSPECPushButton;
void distpartWidget::slotexportSPECPushButtonPressed()
{
}
//    QPushButton* importSPECPushButton;
void distpartWidget::slotimportPushButtonPressed()
{
}
//    QPushButton* srcPackagePushButton;
void distpartWidget::slotsrcPackagePushButtonPressed()
{
}
//    QPushButton* genHTMLPushButton;
void distpartWidget::slotgenHTMLPushButtonPressed()
{

}
//    QPushButton* resetHTMLPushButton;
void distpartWidget::slotresetHTMLPushButtonPressed()
{

}
//    QToolButton* uploadAddFileToolButton;
void distpartWidget::slotuploadAddFileToolButtonPressed()
{

}
//    QToolButton* uploadRemoveToolButton;
void distpartWidget::slotuploadRemoveToolButtonPressed()
{

}
//    QPushButton* uploadSubmitPushButton;
void distpartWidget::slotuploadSubmitPushButtonPressed()
{

}
//    QPushButton* uploadResetPushButton;
void distpartWidget::slotuploadResetPushButtonPressed()
{

}

// Connect Slots to the following widgets and add
// accessors and mutators
//    QCheckBox* customProjectCheckBox;
void distpartWidget::slotcustomProjectCheckBoxChanged()
{
  kdDebug () << "New State" << customProjectCheckBox->isChecked() << endl;
  Layout36->setEnabled(customProjectCheckBox->isChecked());
}
bool distpartWidget::getcustomProjectCheckBoxState()
{
    return customProjectCheckBox->isChecked();
}
void distpartWidget::setcustomProjectCheckBoxState(bool state)
{
    customProjectCheckBox->setChecked(state);
}
//  QCheckBox* uploadCustomCheckBox;
void distpartWidget::slotuploadCustomCheckBoxChanged()
{
    uploadURLLineEdit->setEnabled(uploadCustomCheckBox->isChecked());
}
bool distpartWidget::getuploadCustomCheckBoxState()
{
    return uploadCustomCheckBox->isChecked();
}
void distpartWidget::setuploadCustomCheckBoxState(bool state)
{
    uploadCustomCheckBox->setChecked(state);
}

// Add accessors and mutators for the following

//   QCheckBox* bzipCheckBox;
bool distpartWidget::getbzipCheckBoxState()
{
    return bzipCheckBox->isChecked();
}
void distpartWidget::setbzipCheckBoxState(bool state)
{
    bzipCheckBox->setChecked(state);
}
//    QCheckBox* appIconCheckBox;
bool distpartWidget::getappIconCheckBoxState()
{
    return appIconCheckBox->isChecked();
}
void distpartWidget::setappIconCheckBoxState(bool state)
{
    appIconCheckBox->setChecked(state);
}
//    QCheckBox* genHTMLCheckBox;
bool distpartWidget::getgenHTMLCheckBoxState()
{
    return genHTMLCheckBox->isChecked();
}
void distpartWidget::setgenHTMLCheckBoxState(bool state)
{
    genHTMLCheckBox->setChecked(state);
}
//    QCheckBox* useRPMInfoCheckBox;
bool distpartWidget::getuseRPMInfoCheckBoxState()
{
    return useRPMInfoCheckBox->isChecked();
}
void distpartWidget::setuseRPMInfoCheckBoxState(bool state)
{
     useRPMInfoCheckBox->setChecked(state);
}
//    QCheckBox* uploadAppsKDEcomCheckBox;
bool distpartWidget::getuploadAppsKDEcomCheckBoxState()
{
     return uploadAppsKDEcomCheckBox->isChecked();
}
void distpartWidget::setuploadAppsKDEcomCheckBoxState(bool state)
{
     uploadAppsKDEcomCheckBox->setChecked(state);
}
//    QCheckBox* uploadftpkdeorgCheckBox;
bool distpartWidget::getuploadftpkdeorgCheckBoxState()
{
    return uploadftpkdeorgCheckBox->isChecked();
}
void distpartWidget::setuploadftpkdeorgCheckBoxState(bool state)
{
    uploadftpkdeorgCheckBox->setChecked(state);
}
//    QCheckBox* devPackageCheckBox;
bool distpartWidget::getdevPackageCheckBoxState()
{
    return devPackageCheckBox->isChecked();
}
void distpartWidget::setdevPackageCheckBoxState(bool state)
{
    devPackageCheckBox->setChecked(state);
}
//    QCheckBox* docsPackageCheckBox;
bool distpartWidget::getdocsPackageCheckBoxState()
{
    return docsPackageCheckBox->isChecked();
}
void distpartWidget::setdocsPackageCheckBoxState(bool state)
{
    docsPackageCheckBox->setChecked(state);
}
//    QProgressBar* uploadProgressBar;
int distpartWidget::getuploadProgressBarProgress()
{
    return uploadProgressBar->progress();
}
void distpartWidget::setuploadProgressBarProgress(int progress)
{
    uploadProgressBar->setProgress( progress );
}
//    QLineEdit* archNameFormatLineEdit;
QString distpartWidget::getarchNameFormatLineEditText()
{
    return archNameFormatLineEdit->text();
}
void distpartWidget::setarchNameFormatLineEditText(QString text)
{
    archNameFormatLineEdit->setText(text);
}
//    QLineEdit* appNameLineEdit;
QString distpartWidget::getappNameFormatLineEditText()
{
    return appNameLineEdit->text();
}
void distpartWidget::setappNameFormatLineEditText(QString text)
{
    appNameLineEdit->setText(text);
}
//    QLineEdit* summaryLineEdit;
QString distpartWidget::getsummaryLineEditText()
{
   return summaryLineEdit->text();
}
void distpartWidget::setsummaryLineEditText(QString text)
{
   summaryLineEdit->setText(text);
}
//    QLineEdit* authorLineEdit;
QString distpartWidget::getauthorLineEditText()
{
   return authorLineEdit->text();
}
void distpartWidget::setauthorLineEditText(QString text)
{
   authorLineEdit->setText(text);
}
//    QLineEdit* groupLineEdit;
QString distpartWidget::getgroupLineEditText()
{
    return groupLineEdit->text();
}
void distpartWidget::setgroupLineEditText(QString text)
{
    groupLineEdit->setText(text);
}
//    QLineEdit* releaseLineEdit;
QString distpartWidget::getreleaseLineEditText()
{
   return releaseLineEdit->text();
}
void distpartWidget::setreleaseLineEditText(QString text)
{
   releaseLineEdit->setText(text);
}
//    QLineEdit* VersionLineEdit;
QString distpartWidget::getversionLineEditText()
{
   return VersionLineEdit->text();
}
void distpartWidget::setversionLineEditText(QString text)
{
     VersionLineEdit->setText( text );
}
//    QLineEdit* VendorLineEdit;
QString distpartWidget::getvendorLineEditText()
{
    return VendorLineEdit->text();
}
void distpartWidget::setvendorLineEditText(QString text)
{
    VendorLineEdit->setText(text);
}
//    QLineEdit* LicenseLineEdit;
QString distpartWidget::getlicenseLineEditText()
{
   return LicenseLineEdit->text();
}
void distpartWidget::setlicenseLineEditText(QString text)
{
    LicenseLineEdit->setText(text);
}
//    QLineEdit* uploadURLLineEdit;
QString distpartWidget::getuploadURLLineEditText()
{
    return uploadURLLineEdit->text();
}
void distpartWidget::setuploadURLLineEditText(QString text)
{
     uploadURLLineEdit->setText(text);
}
//    QLineEdit* PackagerLineEdit;
QString distpartWidget::getpackagerLineEditText()
{
   return PackagerLineEdit->text();
}
void distpartWidget::setpackagerLineEditText(QString text)
{
   PackagerLineEdit->setText(text);
}
//    QComboBox* archComboBox;
QString distpartWidget::getarchComboBoxText()
{
   return archComboBox->currentText();
}
int distpartWidget::getarchComboBoxItem()
{
   return archComboBox->currentItem();
}
void distpartWidget::setarchComboBoxItem(int item)
{
   archComboBox->setCurrentItem( item );
}
//    QListBox* uploadFileListBox;
QString distpartWidget::getuploadFileListBoxText()
{
    return uploadFileListBox->currentText();
}

int distpartWidget::getuploadFileListBoxItem()
{
    return uploadFileListBox->currentItem();
}
void distpartWidget::setuploadFileListBoxItem(int item)
{
    uploadFileListBox->setCurrentItem( item);
}
//    QListBox* srcDistFileListBox;
QString distpartWidget::getsrcDistFileListBoxText()
{
    return srcDistFileListBox->currentText ();
}
int distpartWidget::getsrcDistFileListBoxItem()
{
    return srcDistFileListBox->currentItem ();
}
void distpartWidget::setsrcDistFileListBoxItem(int item)
{
    srcDistFileListBox->setCurrentItem( item );
}
//    QMultiLineEdit* projectDescriptionMultilineEdit;
QString distpartWidget::getprojectDescriptionMultilineEditText()
{
    return projectDescriptionMultilineEdit->text();
}
void distpartWidget::setprojectDescriptionMultilineEditText(QString text)
{
    projectDescriptionMultilineEdit->setText(text);
}

#include "distpart_widget.moc"
