/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _APPWIZARDDIALOG_H_
#define _APPWIZARDDIALOG_H_

class AppWizardPart;
class QMultiLineEdit;
class QRadioButton;
class KTempFile;
class QWidgetStack;
class QVBox;

#include <qptrlist.h>
#include <qdict.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstringlist.h>

#include "kdevversioncontrol.h"
#include "appwizarddlgbase.h"
#include "vcs_form.h"

struct ApplicationInfo
{
    QString templateName;
    QString name;
    QString comment;
    QString icon;
    QString category;
    QString defaultDestDir;
    QString fileTemplates;
    QStringList openFilesAfterGeneration;

    QListViewItem *item; // item pointer to the listview

    ApplicationInfo()
    : item( 0 )
    {}
};


struct AppWizardFileTemplate
{
    QString suffix;
    QString style;
    QMultiLineEdit *edit;

    AppWizardFileTemplate()
    : edit( 0 )
    {}
};


class AppWizardDialog : public AppWizardDialogBase
{
    Q_OBJECT

public:
    AppWizardDialog( AppWizardPart *part, QWidget *parent=0, const char *name=0 );
    ~AppWizardDialog();
    QString getProjectName() { return appname_edit->text(); }
    QString getProjectLocation() { return finalLoc_label->text(); }
    QString getCommandLine() { return m_cmdline; }
    QStringList getFilesToOpenAfterGeneration();

protected:
    virtual void templatesTreeViewClicked(QListViewItem*);
    virtual void textChanged();
    virtual void licenseChanged();
    virtual void destButtonClicked(const QString&);
    virtual void projectNameChanged();
    virtual void projectLocationChanged();
    virtual void accept();

private:
    ApplicationInfo *templateForItem(QListViewItem *item);
    void insertCategoryIntoTreeView(const QString &completeCategoryPath);
    void loadVcs();

    QPtrList<ApplicationInfo> m_appsInfo;
    QValueList<AppWizardFileTemplate> m_fileTemplates;
    // Store the category name and the pointer in the treeview
    QDict<QListViewItem> m_categoryMap;
    // A list of currently available version control systems
    QDict<KDevVersionControl> m_availVcs;

    AppWizardPart *m_part;
    QWidget *m_lastPage;
    QString m_cmdline;
    QPtrList<KTempFile> m_tempFiles;
    ApplicationInfo* m_pCurrentAppInfo;
    bool m_projectLocationWasChanged;
    VcsForm *m_vcsForm;
    bool m_pathIsValid;
};

#endif
