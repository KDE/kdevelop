/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit ProviderWidget(QWidget *parent = nullptr);

    /// Re-implemented from KDevelop::IProjectProviderWidget.
    KDevelop::VcsJob * createWorkingCopy(const QUrl &dest) override;

    /// Re-implemented from KDevelop::IProjectProviderWidget.
    bool isCorrect() const override;

private Q_SLOTS:
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
