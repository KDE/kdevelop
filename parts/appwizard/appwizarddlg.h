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
class Q3MultiLineEdit;
class QRadioButton;
class KTempFile;
class Q3WidgetStack;
class Q3VBox;
class KPopupMenu;
class QHBoxLayout;
class KArchiveDirectory;
class KArchiveFile;
class ProfileSupport;
#include <q3ptrlist.h>
#include <q3dict.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <QHBoxLayout>
#include <Q3ValueList>
#include <klistview.h>
#include <kiconview.h>
//#include "kdevversioncontrol.h"
//#include "kdevvcsintegrator.h"
#include "appwizarddlgbase.h"
//#include "vcs_form.h"
//#include "autoform.h"
#include "kdevlicense.h"

namespace PropertyLib {
	class PropertyList;
	class PropertyEditor;
}

struct installFile
{
	QString source;
	QString dest;
	QString option;
	bool process;
	bool isXML;
};

struct installArchive
{
	QString source;
	QString dest;
	QString option;
	bool process;
};

struct installDir
{
	QString dir;
	QString option;
	int perms;
};

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
    QString templateFile;
    QMap<QString,QString> subMap;
    QMap<QString,QString> subMapXML;
    QStringList includes;
    
    //QMap<autoKey,QVariant> subValues;
    PropertyLib::PropertyList *propValues;
	
    Q3ValueList<installFile> fileList;
    Q3ValueList<installArchive> archList;
    Q3ValueList<installDir> dirList;
    QString customUI;
    QString message;
    QString finishCmd;
    QString finishCmdDir;
    QString sourceArchive;

    //! item pointer to the listview
    Q3ListViewItem *item;
	
    //! pointer to favourite icon (NULL if there isn't one)
    Q3IconViewItem *favourite;	

    ApplicationInfo()
    : item( 0 ), favourite( 0 )
    {}
};

struct AppWizardFileTemplate
{
    QString suffix;
    QString style;
    Q3MultiLineEdit *edit;

    AppWizardFileTemplate()
    : edit( 0 )
    {}
};


class AppWizardDialog : public Q3Wizard, public Ui_AppWizardDialogBase
{
    Q_OBJECT

public:
    AppWizardDialog( AppWizardPart *part, QWidget *parent=0, const char *name=0 );
    ~AppWizardDialog();
    QString getProjectName() { return appname_edit->text().lower(); }
    QString getProjectLocation() { return finalLoc_label->text(); }

protected:
    virtual void accept();

    virtual Q3Dict<KDevLicense> licenses();
    void loadLicenses();

protected slots:
    virtual void templatesTreeViewClicked(Q3ListViewItem*);
    virtual void textChanged();
    virtual void licenseChanged();
    virtual void destButtonClicked(const QString&);
    virtual void projectNameChanged();
    virtual void projectLocationChanged();
    virtual void favouritesIconViewClicked( Q3IconViewItem * );
    virtual void templatesContextMenu(Q3ListViewItem*, const QPoint&, int);
    virtual void favouritesContextMenu(Q3IconViewItem* item, const QPoint& point);
    virtual void addTemplateToFavourites();
    virtual void done(int r);
    virtual void removeFavourite();
    virtual void pageChanged();
	
private: //methods

    ApplicationInfo *templateForItem(Q3ListViewItem *item);
    void insertCategoryIntoTreeView(const QString &completeCategoryPath);
    void loadVcs();
    void updateNextButtons();

    void populateFavourites();
    void addFavourite(Q3ListViewItem* item, QString favouriteName="");
    ApplicationInfo* findFavouriteInfo(Q3IconViewItem* item);
	
    void unpackArchive( const KArchiveDirectory *dir, const QString &dest, bool process );
    bool copyFile( const installFile& file );
    bool copyFile( const QString &source, const QString &dest, bool isXML, bool process );
    QString kdevRoot(const QString &templateName ) const;
    void openAfterGeneration();
	
    void setPermissions(const KArchiveFile *source, QString dest);
    void setPermissions(const installFile &file);
	
    void checkAndHideItems(Q3ListView *view);
    bool checkAndHideItems(Q3ListViewItem *item);
	
private: //data

    Q3PtrList<ApplicationInfo> m_appsInfo;
    Q3ValueList<AppWizardFileTemplate> m_fileTemplates;
    //! Store the category name and the pointer in the treeview
    Q3Dict<Q3ListViewItem> m_categoryMap;
    Q3ValueList<Q3ListViewItem*> m_categoryItems;
    //! A list of currently available version control systems
//    QDict<KDevVersionControl> m_availVcs;
	
    QHBoxLayout *m_custom_options_layout;
    PropertyLib::PropertyEditor *m_customOptions;
    AppWizardPart *m_part;
    QWidget *m_lastPage;
    Q3PtrList<KTempFile> m_tempFiles;
    ApplicationInfo* m_pCurrentAppInfo;
    bool m_projectLocationWasChanged;
//    VcsForm *m_vcsForm;
    bool m_pathIsValid;
    KPopupMenu* m_favouritesMenu;
    KPopupMenu* m_templatesMenu;
	
    Q3Dict<KDevLicense> m_licenses;
//    Q3Dict<KDevVCSIntegrator> m_integrators;
//    QMap<int, VCSDialog*> m_integratorDialogs;
	
    ProfileSupport *m_profileSupport;

public slots:
    virtual void showTemplates(bool all);
};

#endif

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
