/*
 * This file is part of KDevelop
 * Copyright 2020 Jonathan L. Verner <jonathan.verner@matfyz.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_PLUGIN_SIMPLE_COMMIT_FORM_H
#define KDEVPLATFORM_PLUGIN_SIMPLE_COMMIT_FORM_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTextEdit;
class QToolButton;

/**
 * A widget for preparing the commit message. It has a
 *  - lineedit for editing the commit message summary
 *    (the first line of the commit message);
 *  - textedit for editing the rest of the commit message
 *  - a button for committing
 *
 * Also, the lineedit shows the number of characters in
 * the summary on the right and, if it is >= 80, indicates
 * this with a red background.
 */
class SimpleCommitForm : public QWidget
{
    Q_OBJECT

public:
    SimpleCommitForm(QWidget* parent = nullptr);

    /**
     * @returns the current value of the commit message summary.
     */
    QString summary() const;

    /**
     * Sets the current value of the commit message summary.
     */
    void setSummary(const QString& txt);

    /**
     * @param wrapAtColumn if >0, the returned message is wrapped so that
     * it fits into the specified number of columns.
     *
     * @returns the current value of the extended commit message.
     *
     * @note: The wrapping is done very simplistically. Whenever a line
     * is encountered which is longer then @ref:wrapAtColumn, the first
     * space character at position > @ref:wrapAtColumn is replaced by
     * a new-line character '\n'.
     */
    QString extendedDescription(int wrapAtColumn = -1) const;

    /**
     * Sets the current value of the commit extended commit message.
     */
    void setExtendedDescription(const QString& txt);

public Q_SLOTS:

    /**
     * Disables the commit button.
     */
    void disableCommitButton();

    /**
     * Enables the commit button.
     */
    void enableCommitButton();

    /**
     * Enable input in the commit form.
     */
    void enable();

    /**
     * Disable input in the commit form
     * (used e.g. while a commit job is in progress)
     */
    void disable();

    /**
     * Clear the commit form (summary & description)
     */
    void clear();

    /**
     * Sets the project name where changes will be
     * committed, so that it can be displayed in the
     * form (currently in some tooltips).
     */
    void setProjectName(const QString& projName);

    /**
     * Sets the branch name, where changes will be
     * committed, so that it can be displayed in the
     * form (currently in some tooltips).
     */
    void setBranchName(const QString& branchName);

Q_SIGNALS:

    /**
     * Emmitted when the user presses the commit button
     */
    void committed();

private:
    QPushButton* m_commitBtn;
    QLineEdit* m_summaryEdit;
    QTextEdit* m_messageEdit;

    /**
     * true if the commit button is disabled by calling disable();
     *
     * We use this to track disable/enable requests; We can't use the
     * button state for this, since we also internally disable the button
     * if the commit message is empty
     */
    bool m_disabled;

    QString m_projectName;     /**< The project to which changes will be committed */
    QString m_branchName;      /**< The branch on which changes will be committed */
};

#endif
