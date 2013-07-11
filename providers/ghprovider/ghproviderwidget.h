/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef GH_PROVIDERWIDGET_H
#define GH_PROVIDERWIDGET_H


#include <interfaces/iprojectprovider.h>


class QLabel;
class QComboBox;
class QListView;
class QModelIndex;


namespace gh
{

class Resource;
class Account;
class LineEdit;

/**
 * @class ProviderWidget
 *
 * This class will be the widget displayed in the "Fetch Project" dialog
 * when the user selects this plugin to fetch a project.
 */
class ProviderWidget : public KDevelop::IProjectProviderWidget
{
    Q_OBJECT

public:
    /// Constructor.
    ProviderWidget(QWidget *parent = NULL);

    /// Re-implemented from KDevelop::IProjectProviderWidget.
    virtual KDevelop::VcsJob * createWorkingCopy(const KUrl &dest);

    /// Re-implemented from KDevelop::IProjectProviderWidget.
    virtual bool isCorrect() const;

private slots:
    /// Fill the m_combo member.
    void fillCombo();

    /// The user has selected a project, change the working directory.
    void projectIndexChanged(const QModelIndex &currentIndex);

    /// Show the settings dialog.
    void showSettings();

    /// Search the repo specified by the user.
    void searchRepo();

private:
    QListView *m_projects;
    Resource *m_resource;
    LineEdit *m_edit;
    Account *m_account;
    QComboBox *m_combo;
    QLabel *m_waiting;
};

} // End of namespace gh


#endif // GH_PROVIDERWIDGET_H
