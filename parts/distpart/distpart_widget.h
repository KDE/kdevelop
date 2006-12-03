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
#ifndef __DISTPART_WIDGET_H__
#define __DISTPART_WIDGET_H__


#include <qwidget.h>
#include <qstring.h>
#include "distpart_ui.h"
#include <klistbox.h>
#include <qcheckbox.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <qcombobox.h>
#include <qprogressbar.h>

class KDevProject;
class DistpartPart;
class QUrlOperator;
class KProcess;
class QTabWidget;
class SpecSupport;

namespace KParts {
class ReadOnlyPart;
}


class DistpartDialog : public distpart_ui {
    Q_OBJECT

public:

    DistpartDialog(DistpartPart *part, QWidget *parent);
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
//    void slothelp_PushButtonPressed();
    //    QPushButton* createSrcArchPushButton;
    void slotcreateSrcArchPushButtonPressed();
    //    QPushButton* resetSrcPushButton;
    void slotresetSrcPushButtonPressed();

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

    void slotbuildAllPushButtonPressed();
    void slotexportSPECPushButtonPressed();
    void slotimportSPECPushButtonPressed();
    void slotsrcPackagePushButtonPressed();
	void slotAddFileButtonPressed();
	void slotRemoveFile(QListBoxItem *item);

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

    QString getSourceName();

private:
    void loadSettings();
    void storeSettings();
    void setPackageData();
    void getPackageData();

    SpecSupport *m_spec;
    DistpartPart *m_part;
    QString dir;
};


#endif
