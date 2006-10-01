/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *   Copyright (C) 2004-2005 by Sascha Cunz                                *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _APPWIZARDDIALOG_H_
#define _APPWIZARDDIALOG_H_

class QTextEdit;
class QHBoxLayout;

class KTemporaryFile;
class KMenu;

class ProfileSupport;

#include <QList>
#include <QLinkedList>
#include <QStringList>

#include <k3listview.h>
#include <k3iconview.h>

#include "appwizarddlgbase.h"
#include "kdevlicense.h"
#include "appwizardpart.h"
#include "kdevapptemplate.h"

struct AppWizardFileTemplate
{
    AppWizardFileTemplate() : edit( 0 ){}

    QString suffix;
    QString style;
    QTextEdit *edit;
};

class AppWizardDialog : public Q3Wizard, public Ui_AppWizardDialogBase
{
    Q_OBJECT

public:
    AppWizardDialog( AppWizardPart *appwizardpart, QWidget *parent=0, const char *name=0 );
    ~AppWizardDialog();
    QString getProjectName() { return appname_edit->text().lower(); }
    QString getProjectLocation() { return finalLoc_label->text(); }

protected:
    virtual void accept();

    const QHash<QString, KDevLicense>& licenses();
    void loadLicenses();

protected slots:
    virtual void templatesTreeViewActivated(const QModelIndex& index);
    virtual void updateNextButtons();
    virtual void licenseChanged();
    virtual void destButtonClicked(const QString&);
    virtual void projectLocationChanged();
//  virtual void favouritesIconViewClicked( Q3IconViewItem * );
//  virtual void templatesContextMenu(Q3ListViewItem*, const QPoint&, int);
//  virtual void favouritesContextMenu(Q3IconViewItem* item, const QPoint& point);
//  virtual void addTemplateToFavourites();
//  virtual void removeFavourite();
    virtual void done(int r);
    virtual void pageChanged();
    virtual void showTemplates(bool all);

private: //methods
    void loadVcs();
    void populateFavourites();
//  void addFavourite(Q3ListViewItem* item, QString favouriteName="");
//  ApplicationInfo* findFavouriteInfo(Q3IconViewItem* item);

    void openAfterGeneration();
    void checkAndHideItems(Q3ListView *view);
    bool checkAndHideItems(Q3ListViewItem *item);
    void clearTemplateSelection();

private: //data
    QLinkedList<AppWizardFileTemplate*> m_fileTemplates;
    QHBoxLayout *m_custom_options_layout;
    AppWizardPart *m_part;
    QWidget *m_lastPage;
    QList<KTemporaryFile*> m_tempFiles;
    KDevAppTemplateModel m_templateModel;
    KDevAppTemplate* m_currentTemplate;
    bool m_projectLocationWasChanged;
    bool m_pathIsValid;
    KMenu* m_favouritesMenu;
    KMenu* m_templatesMenu;
    QHash<QString, KDevLicense*> m_licenses;
    ProfileSupport *m_profileSupport;
};

#endif

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
