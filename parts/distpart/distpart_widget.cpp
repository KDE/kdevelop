#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>


#include <kdevcore.h>


#include "distpart_part.h"
#include "distpart_widget.h"


distpartWidget::distpartWidget(distpartPart *part)
	: distpart_ui(0, "distpart widget")
{
}


distpartWidget::~distpartWidget()
{
}

//    QPushButton* okayPushButton;
void distpartWidget::slotokayPushButtonPressed() { }
//    QPushButton* cancelPushButton;
void distpartWidget::slotcancelPushButtonPressed() { }
//    QPushButton* help_PushButton;
void distpartWidget::slothelp_PushButtonPressed() { }
//    QPushButton* createSrcArchPushButton;
void distpartWidget::slotcreateSrcArchPushButtonPressed() { }
//    QPushButton* resetSrcPushButton;
void distpartWidget::slotresetSrcPushButtonPressed() { }
//    QPushButton* buildAllPushButton;
void distpartWidget::slotbuildAllPushButtonPressed() { }
//    QPushButton* exportSPECPushButton;
void distpartWidget::slotexportSPECPushButtonPressed() { }
//    QPushButton* importSPECPushButton;
void distpartWidget::slotimportPushButtonPressed() { }
//    QPushButton* srcPackagePushButton;
void distpartWidget::slotsrcPackagePushButtonPressed() { }
//    QPushButton* genHTMLPushButton;
void distpartWidget::slotgenHTMLPushButtonPressed() { }
//    QPushButton* resetHTMLPushButton;
void distpartWidget::slotresetHTMLPushButtonPressed() { }
//    QToolButton* uploadAddFileToolButton;
void distpartWidget::slotuploadAddFileToolButtonPressed() { }
//    QToolButton* uploadRemoveToolButton;
void distpartWidget::slotuploadRemoveToolButtonPressed() { }
//    QPushButton* uploadSubmitPushButton;
void distpartWidget::slotuploadSubmitPushButtonPressed() { }
//    QPushButton* uploadResetPushButton;
void distpartWidget::slotuploadResetPushButtonPressed() { }

// Connect Slots to the following widgets and add
// accessors and mutators
//    QCheckBox* customProjectCheckBox;
void distpartWidget::slotcustomProjectCheckBoxChanged() { }
//  QCheckBox* uploadCustomCheckBox;
void distpartWidget::slotuploadCustomCheckBoxChanged() { }

bool distpartWidget::getcustomProjectCheckBoxState() { }
void distpartWidget::setcustomProjectCheckBoxState(bool state) { }
bool distpartWidget::getuploadCustomCheckBoxState() { }
void distpartWidget::setuploadCustomCheckBoxState(bool state) { }

// Add accessors and mutators for the following

//   QCheckBox* bzipCheckBox;
bool distpartWidget::getbzipCheckBoxState() { }
void distpartWidget::setbzipCheckBoxState(boot state) { }
//    QCheckBox* appIconCheckBox;
bool distpartWidget::getappIconCheckBoxState() { }
void distpartWidget::setappIconCheckBoxState(boot state) { }
//    QCheckBox* genHTMLCheckBox;
bool distpartWidget::getgenHTMLCheckBoxState() { }
void distpartWidget::setgenHTMLCheckBoxState(boot state) { }
//    QCheckBox* useRPMInfoCheckBox;
bool distpartWidget::getuseRPMInfoCheckBoxState() { }
void distpartWidget::setuseRPMInfoCheckBoxState(boot state) { }
//    QCheckBox* uploadAppsKDEcomCheckBox;
bool distpartWidget::getuploadAppsKDEcomCheckBoxState() { }
void distpartWidget::setuploadAppsKDEcomCheckBoxState(boot state) { }
//    QCheckBox* uploadftpkdeorgCheckBox;
bool distpartWidget::getuploadftpkdeorgCheckBoxState() { }
void distpartWidget::setuploadftpkdeorgCheckBoxState(boot state) { }
//    QCheckBox* devPackageCheckBox;
bool distpartWidget::getdevPackageCheckBoxState() { }
void distpartWidget::setdevPackageCheckBoxState(boot state) { }
//    QCheckBox* docsPackageCheckBox;
bool distpartWidget::getdocsPackageCheckBoxState() { }
void distpartWidget::setdocsPackageCheckBoxState(boot state) { }
//    QProgressBar* uploadProgressBar;
int distpartWidget::getuploadProgressBarProgress() { }
void distpartWidget::setuploadProgressBarProgress(int progress) { }
//    QLineEdit* archNameFormatLineEdit;
QString distpartWidget::getarchNameFormatLineEditText() { }
void distpartWidget::setarchNameFormatLineEditText(QString text) { }
//    QLineEdit* appNameLineEdit;
QString distpartWidget::getappNameFormatLineEditText() { }
void distpartWidget::setappNameFormatLineEditText(QString text) { }
//    QLineEdit* summaryLineEdit;
QString distpartWidget::getsummaryLineEditText() { }
void distpartWidget::setsummaryLineEditText(QString text) { }
//    QLineEdit* authorLineEdit;
QString distpartWidget::getauthorLineEditText() { }
void distpartWidget::setauthorLineEditText(QString text) { }
//    QLineEdit* groupLineEdit;
QString distpartWidget::getgroupLineEditText() { }
void distpartWidget::setgroupLineEditText(QString text) { }
//    QLineEdit* releaseLineEdit;
QString distpartWidget::getreleaseLineEditText() { }
void distpartWidget::setreleaseLineEditText(QString text) { }
//    QLineEdit* VersionLineEdit;
QString distpartWidget::getversionLineEditText() { }
void distpartWidget::setversionLineEditText(QString text) { }
//    QLineEdit* VendorLineEdit;
QString distpartWidget::getvendorLineEditText() { }
void distpartWidget::setvendorLineEditText(QString text) { }
//    QLineEdit* LicenseLineEdit;
QString distpartWidget::getlicenseLineEditText() { }
void distpartWidget::setlicenseLineEditText(QString text) { }
//    QLineEdit* uploadURLLineEdit;
QString distpartWidget::getuploadURLLineEditText() { }
void distpartWidget::setuploadURLLineEditText(QString text) { }
//    QLineEdit* PackagerLineEdit;
QString distpartWidget::getpackagerLineEditText() { }
void distpartWidget::setpackagerLineEditText(QString text) { }
//    QComboBox* archComboBox;
QString distpartWidget::getarchComboBoxText() { }
int distpartWidget::getarchComboBoxItem() { }
void distpartWidget::setarchComboBoxItem(int item) { }
//    QListBox* uploadFileListBox;
QString distpartWidget::getuploadFileListBoxText() { }
int distpartWidget::getuploadFileListBoxItem() { }
void distpartWidget::setuploadFileListBoxItem(int item) { }
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
    srcDistFileListBoxItem->setCurrentItem( item );
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
