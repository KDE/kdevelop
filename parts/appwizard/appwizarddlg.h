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
class KPopupMenu;
class QHBoxLayout;
class KArchiveDirectory;
class KArchiveFile;
#include <qptrlist.h>
#include <qdict.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <klistview.h>
#include <kiconview.h>
#include "kdevversioncontrol.h"
#include "appwizarddlgbase.h"
#include "vcs_form.h"
#include "autoform.h"

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
    
    AutoPropertyMap subValues;
    QValueList<installFile> fileList;
    QValueList<installArchive> archList;
    QValueList<installDir> dirList;
    QString customUI;
    QString message;
    QString sourceArchive;

    //! item pointer to the listview
    QListViewItem *item;
	
	//! pointer to favourite icon (NULL if there isn't one)
	QIconViewItem *favourite;	

    ApplicationInfo()
    : item( 0 ), favourite( 0 )
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

protected:
    virtual void accept();

protected slots:
    virtual void templatesTreeViewClicked(QListViewItem*);
    virtual void textChanged();
    virtual void licenseChanged();
    virtual void destButtonClicked(const QString&);
    virtual void projectNameChanged();
    virtual void projectLocationChanged();
    virtual void favouritesIconViewClicked( QIconViewItem * );
	virtual void templatesContextMenu(QListViewItem*, const QPoint&, int);
	virtual void favouritesContextMenu(QIconViewItem* item, const QPoint& point);
	virtual void addTemplateToFavourites();
	virtual void done(int r);
	virtual void removeFavourite();
	virtual void pageChanged();
	
private: //methods

    ApplicationInfo *templateForItem(QListViewItem *item);
    void insertCategoryIntoTreeView(const QString &completeCategoryPath);
    void loadVcs();
	void updateNextButtons();

	void populateFavourites();
	void addFavourite(QListViewItem* item, QString favouriteName="");
	ApplicationInfo* findFavouriteInfo(QIconViewItem* item);
	
	void unpackArchive( const KArchiveDirectory *dir, const QString &dest, bool process );
	bool copyFile( const installFile& file );
	bool copyFile( const QString &source, const QString &dest, bool isXML, bool process );
	QString kdevRoot(const QString &templateName ) const;
	void openAfterGeneration();
	
	void setPermissions(const KArchiveFile *source, QString dest);
	void setPermissions(const installFile &file);
	
private: //data

    QPtrList<ApplicationInfo> m_appsInfo;
	QValueList<AppWizardFileTemplate> m_fileTemplates;
    //! Store the category name and the pointer in the treeview
    QDict<QListViewItem> m_categoryMap;
    //! A list of currently available version control systems
    QDict<KDevVersionControl> m_availVcs;
	
    QHBoxLayout *m_custom_options_layout;
    AutoForm *m_customOptions;
    AppWizardPart *m_part;
    QWidget *m_lastPage;
    QPtrList<KTempFile> m_tempFiles;
    ApplicationInfo* m_pCurrentAppInfo;
    bool m_projectLocationWasChanged;
    VcsForm *m_vcsForm;
    bool m_pathIsValid;
	KPopupMenu* m_favouritesMenu;
	KPopupMenu* m_templatesMenu;
};

#endif

// kate: indent-width 4; replace-tabs off; tab-width 4; space-indent off;
