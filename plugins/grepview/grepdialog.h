/***************************************************************************
*   Copyright 1999-2001 by Bernd Gehrmann and the KDevelop Team           *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_GREPDIALOG_H
#define KDEVPLATFORM_PLUGIN_GREPDIALOG_H

#include <QDialog>
#include <QUrl>

#include "ui_grepwidget.h"

class KConfig;
class KUrlRequester;
class QPushButton;
class GrepViewPlugin;
class KLineEdit;

class GrepDialog : public QDialog, private Ui::GrepWidget
{
    Q_OBJECT

public:
    explicit GrepDialog( GrepViewPlugin * plugin, QWidget *parent=0 );
    ~GrepDialog();

    void setPattern(const QString &pattern);
    void setEnableProjectBox(bool enable);

    QString patternString() const;
    QString templateString() const;
    QString replacementTemplateString() const;
    QString filesString() const;
    QString excludeString() const;

    bool useProjectFilesFlag() const;
    bool regexpFlag() const;
    bool caseSensitiveFlag() const;

    int depthValue() const;

public Q_SLOTS:
    void startSearch();

    ///Sets directory(ies)/files to search in. Also it can be semicolon separated list of directories/files or one of special strings: allOpenFilesString, allOpenProjectsString
    void setSearchLocations(const QString &dir);

private Q_SLOTS:
    void templateTypeComboActivated(int);
    void patternComboEditTextChanged( const QString& );
    void directoryChanged(const QString &dir);
    QMenu* createSyncButtonMenu();
    void addUrlToMenu(QMenu* ret, const QUrl& url);
    void addStringToMenu(QMenu* ret, QString string);
    void synchronizeDirActionTriggered(bool);

    ///Opens the dialog to select a directory to search in, and inserts it into Location(s) field.
    void selectDirectoryDialog();

private:
    // Returns the chosen directories or files (only the top directories, not subfiles)
    QList< QUrl > getDirectoryChoice() const;
    // Returns whether the given url is a subfile/subdirectory of one of the chosen directories/files
    // This is slow, so don't call it too often
    bool isPartOfChoice(QUrl url) const;

    GrepViewPlugin * m_plugin;
};


#endif

