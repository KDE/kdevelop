/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
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

#include "appwizarddlgbase.h"

class ApplicationInfo {
public:
  QString templateName;
  QString name;
  QString comment;
  QString category;
  QListViewItem* pItem; // item pointer to the listview
};

class AppWizardDialog : public AppWizardDialogBase
{
    Q_OBJECT

public:
    AppWizardDialog( AppWizardPart *part, QWidget *parent=0, const char *name=0 );
    ~AppWizardDialog();
    virtual void templatesTreeViewClicked(QListViewItem*);
    virtual void destButtonClicked();
protected:
    virtual void accept();

private slots:
    void textChanged();
    void licenseChanged();
    
private:
    void insertIntoTreeView(ApplicationInfo* pInfo);
    void insertCategoryIntoTreeView(QString completeCategoryPath);
    QStringList m_templateNames;
    KTempFile *tempFile;
    QList<ApplicationInfo> m_appsInfo;
    QDict<QListViewItem> m_categoryMap; //store the category name and the pointer in the treeview

    AppWizardPart *m_part;
    ApplicationInfo* m_pCurrentAppInfo;
};

#endif
