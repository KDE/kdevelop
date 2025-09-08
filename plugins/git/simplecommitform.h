/*
    SPDX-FileCopyrightText: 2020 Jonathan L. Verner <jonathan.verner@matfyz.cz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SIMPLE_COMMIT_FORM_H
#define KDEVPLATFORM_PLUGIN_SIMPLE_COMMIT_FORM_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTextEdit;
class QToolButton;
class KMessageWidget;

/**
 * A widget for preparing the commit message. It has
 *
 *  - an (initially hidden) KMessageWidget to show potential errors
 *  - a lineedit for editing the commit message summary
 *    (the first line of the commit message);
 *  - a textedit for editing the rest of the commit message
 *  - a button for committing
 *
 * Also, the lineedit shows the number of characters in the summary
 * on the right and, if it is >= 80, indicates this with a red background.
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
     *
     * @note: Also hides the inline error message if shown.
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

    /**
     * Shows an error message using the inline KMessageWidget
     *
     * (Use e.g. when git commit fails)
     */
    void showError(const QString& error);

    /**
     * Hides the inline error message if shown.
     */
    void clearError();


Q_SIGNALS:

    /**
     * Emitted when the user presses the commit button
     */
    void committed();

private:
    QPushButton* m_commitBtn;
    QLineEdit* m_summaryEdit;
    QTextEdit* m_messageEdit;
    KMessageWidget* m_inlineError;

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
