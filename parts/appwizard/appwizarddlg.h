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

#include <qlist.h>
#include <qdict.h>
#include <qlistview.h>
#include <qlineedit.h>

#include "appwizarddlgbase.h"


struct ApplicationInfo
{
    QString templateName;
    QString name;
    QString comment;
    QString icon;
    QString category;
    QString defaultDestDir;
    QString showFileAfterGeneration;
    QString fileTemplates;
    
    QListViewItem *item; // item pointer to the listview
};


struct AppWizardFileTemplate
{
    QString suffix;
    QString style;
    QMultiLineEdit *edit;
};


class AppWizardDialog : public AppWizardDialogBase
{
    Q_OBJECT

public:
    AppWizardDialog( AppWizardPart *part, QWidget *parent=0, const char *name=0 );
    ~AppWizardDialog();
    QString getProjectName() { return appname_edit->text(); }
    QString getProjectLocation() { return dest_edit->text(); }
    QString getCommandLine() { return m_cmdline; }
    QString getShowFileAfterGeneration();

protected:
    virtual void templatesTreeViewClicked(QListViewItem*);
    virtual void textChanged();
    virtual void licenseChanged();
    virtual void destButtonClicked();
    virtual void projectNameChanged();    
    virtual void projectLocationChanged();
    virtual void accept();

private:
    ApplicationInfo *templateForItem(QListViewItem *item);
    void insertCategoryIntoTreeView(const QString &completeCategoryPath);

    QList<ApplicationInfo> m_appsInfo;
    QValueList<AppWizardFileTemplate> m_fileTemplates;
    QDict<QListViewItem> m_categoryMap; //store the category name and the pointer in the treeview

    AppWizardPart *m_part;
    QWidget *m_lastPage;
    QString m_cmdline;
    QList<KTempFile> m_tempFiles;
    ApplicationInfo* m_pCurrentAppInfo;
    bool m_projectLocationWasChanged;
};

#endif
