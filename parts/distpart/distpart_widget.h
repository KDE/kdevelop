#ifndef __DISTPART_WIDGET_H__
#define __DISTPART_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>
#include "distpart_ui.h"
#include <qlistbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qprogressbar.h>

class KDevProject;
class DistpartPart;
class QUrlOperator;
class KProcess;

namespace KParts {
class ReadOnlyPart;
}


class DistpartDialog : public distpart_ui {
    Q_OBJECT

public:

    DistpartDialog(DistpartPart *part);
    ~DistpartDialog();

signals:
    void okay();
    void cancel();
    void help();
    void createsrc();
    void resetsrc();
    void buildall();
    void savespec();
    void loadspec();
    void srcrpm();
    void genhtml();
    void resethtml();
    void upload();
    void resetupload();

    // Accessors and mutators to control GUI
    // Connect Slots to the following widgets


public slots:
    //    QPushButton* okayPushButton;

    void slotokayPushButtonPressed();
    //    QPushButton* cancelPushButton;
    void slotcancelPushButtonPressed();
    //    QPushButton* help_PushButton;
    void slothelp_PushButtonPressed();
    //    QPushButton* createSrcArchPushButton;
    void slotcreateSrcArchPushButtonPressed();
    //    QPushButton* resetSrcPushButton;
    void slotresetSrcPushButtonPressed();
    //    QPushButton* buildAllPushButton;
    void slotbuildAllPushButtonPressed();
    //    QPushButton* exportSPECPushButton;
    void slotexportSPECPushButtonPressed();
    //    QPushButton* importSPECPushButton;
    void slotimportPushButtonPressed();
    //    QPushButton* srcPackagePushButton;
    void slotsrcPackagePushButtonPressed();
    //    QPushButton* genHTMLPushButton;
    void slotgenHTMLPushButtonPressed();
    //    QPushButton* resetHTMLPushButton;
    void slotresetHTMLPushButtonPressed();
    //    QToolButton* uploadAddFileToolButton;
    void slotuploadAddFileToolButtonPressed();
    //    QToolButton* uploadRemoveToolButton;
    void slotuploadRemoveToolButtonPressed();
    //    QPushButton* uploadSubmitPushButton;
    void slotuploadSubmitPushButtonPressed();
    //    QPushButton* uploadResetPushButton;
    void slotuploadResetPushButtonPressed();

    // Connect Slots to the following widgets and add
    // accessors and mutators
    //    QCheckBox* customProjectCheckBox;
    void slotcustomProjectCheckBoxChanged();
    //  QCheckBox* uploadCustomCheckBox;
    void slotuploadCustomCheckBoxChanged();

public:
    bool getcustomProjectCheckBoxState();
    void setcustomProjectCheckBoxState(bool state);
    bool getuploadCustomCheckBoxState();
    void setuploadCustomCheckBoxState(bool state);

    // Add accessors and mutators for the following

    //   QCheckBox* bzipCheckBox;
    bool getbzipCheckBoxState();
    void setbzipCheckBoxState(bool state);
    //    QCheckBox* appIconCheckBox;
    bool getappIconCheckBoxState();
    void setappIconCheckBoxState(bool state);
    //    QCheckBox* genHTMLCheckBox;
    bool getgenHTMLCheckBoxState();
    void setgenHTMLCheckBoxState(bool state);
    //    QCheckBox* useRPMInfoCheckBox;
    bool getuseRPMInfoCheckBoxState();
    void setuseRPMInfoCheckBoxState(bool state);
    //    QCheckBox* uploadAppsKDEcomCheckBox;
    bool getuploadAppsKDEcomCheckBoxState();
    void setuploadAppsKDEcomCheckBoxState(bool state);
    //    QCheckBox* uploadftpkdeorgCheckBox;
    bool getuploadftpkdeorgCheckBoxState();
    void setuploadftpkdeorgCheckBoxState(bool state);
    //    QCheckBox* devPackageCheckBox;
    bool getdevPackageCheckBoxState();
    void setdevPackageCheckBoxState(bool state);
    //    QCheckBox* docsPackageCheckBox;
    bool getdocsPackageCheckBoxState();
    void setdocsPackageCheckBoxState(bool state);
    //    QLineEdit* archNameFormatLineEdit;
    QString getarchNameFormatLineEditText();
    void setarchNameFormatLineEditText(QString text);
    //    QLineEdit* appNameLineEdit;
    QString getappNameFormatLineEditText();
    void setappNameFormatLineEditText(QString text);
    //    QLineEdit* summaryLineEdit;
    QString getsummaryLineEditText();
    void setsummaryLineEditText(QString text);
    //    QLineEdit* groupLineEdit;
    QString getgroupLineEditText();
    void setgroupLineEditText(QString text);
    //    QLineEdit* releaseLineEdit;
    QString getreleaseLineEditText();
    void setreleaseLineEditText(QString text);
    //    QLineEdit* VersionLineEdit;
    QString getversionLineEditText();
    void setversionLineEditText(QString text);
    //    QLineEdit* VendorLineEdit;
    QString getvendorLineEditText();
    void setvendorLineEditText(QString text);
    //    QLineEdit* LicenseLineEdit;
    QString getlicenseLineEditText();
    void setlicenseLineEditText(QString text);
    //    QLineEdit* uploadURLLineEdit;
    QString getuploadURLLineEditText();
    void setuploadURLLineEditText(QString text);
    //    QLineEdit* PackagerLineEdit;
    QString getpackagerLineEditText();
    void setpackagerLineEditText(QString text);
    //    QComboBox* archComboBox;
    QString getarchComboBoxText();
    int getarchComboBoxItem();
    void setarchComboBoxItem(int item);
    //    QListBox* uploadFileListBox;
    QString getuploadFileListBoxText();
    int getuploadFileListBoxItem();
    void setuploadFileListBoxItem(int item);
    //    QListBox* srcDistFileListBox;
    QString getsrcDistFileListBoxText();
    int getsrcDistFileListBoxItem();
    void setsrcDistFileListBoxItem(int item);
    //    QMultiLineEdit* projectDescriptionMultilineEdit;
    QString getprojectDescriptionMultilineEditText();
    void setprojectDescriptionMultilineEditText(QString text);

    QString getprojectChangelogMultilineEditText();
    void setprojectChangelogMultilineEditText(QString text);
    

private:
    void loadSettings();
    void storeSettings();
    
    void parseDotRpmmacros();
    bool createRpmDirectoryFromMacro(const QString &);
    
    
    DistpartPart * m_part;
    QUrlOperator * op;
    QString dir;
    KProcess *tar_proc;
    QMap<QString,QString> map;
};


#endif
