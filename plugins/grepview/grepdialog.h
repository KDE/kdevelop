/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 1999-2001 the KDevelop Team
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GREPDIALOG_H
#define KDEVPLATFORM_PLUGIN_GREPDIALOG_H

#include <QDialog>
#include <QUrl>

#include "grepjob.h"
#include "ui_grepwidget.h"

class GrepOutputView;
class GrepViewPlugin;

class GrepDialog : public QDialog, private Ui::GrepWidget
{
    Q_OBJECT

public:
    /// Search results are displayed in: @p toolView if it is not null,
    /// or in a possibly created and raised GrepOutputView in the current area.
    explicit GrepDialog(GrepViewPlugin* plugin, GrepOutputView* toolView = nullptr, QWidget* parent = nullptr,
                        bool show = true);
    ~GrepDialog() override;

    /// Read last used settings from config.
    /// @pre @c show == @c false has been passed to the constructor.
    void setLastUsedSettings();

    void setPattern(const QString& pattern);

    void search(GrepJobSettings&& settings);

    /**
     * Rerun all grep jobs from a given list of settings and close the dialog.
     *
     * @param settingsHistory a nonempty list of settings to re-search
     */
    void historySearch(QList<GrepJobSettings>&& settingsHistory);

public Q_SLOTS:
    ///Start a new search
    void startSearch();

    ///Sets directory(ies)/files to search in. Also it can be semicolon separated list of directories/files or one of special strings: allOpenFilesString, allOpenProjectsString
    void setSearchLocations(const QString &dir);

private Q_SLOTS:
    void templateTypeComboActivated(int);
    void patternComboEditTextChanged( const QString& );
    QMenu* createSyncButtonMenu();
    void addUrlToMenu(QMenu* ret, const QUrl& url);
    void addStringToMenu(QMenu* ret, const QString& string);
    void synchronizeDirActionTriggered(bool);

    ///Opens the dialog to select a directory to search in, and inserts it into Location(s) field.
    void selectDirectoryDialog();

protected:
    ///Prevent showing the dialog if m_show is false
    void setVisible(bool visible) override;
    void closeEvent(QCloseEvent* closeEvent) override;

private:
    ///Returns whether the given url is a subfile/subdirectory of one of the chosen directories/files
    ///
    ///This is slow, so don't call it too often
    bool isPartOfChoice(const QUrl& url) const;

    /**
     * Save searched-in documents.
     *
     * @return whether the saving succeeded
     */
    bool saveSearchedDocuments() const;

    ///Checks what a user has entered into the dialog and saves the data in m_settings
    void updateSettings();
    /// Enables/disables limit-to-project UI depending on current search paths.
    void updateLimitToProjectEnabled();

    GrepViewPlugin * m_plugin;
    GrepOutputView* const m_toolView;
    ///Allow to show a dialog
    const bool m_show;
    ///Current setting
    GrepJobSettings m_settings;
};


#endif

