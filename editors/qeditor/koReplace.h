/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KOREPLACE_H
#define KOREPLACE_H

#include "koFind.h"

class KHistoryCombo;
class QCheckBox;
class QGroupBox;
class QLabel;
class QPopupMenu;
class QPushButton;
class QRect;

/**
 * @short A generic "replace" widget.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 *
 * This widget inherits from @ref KoFindDialog and implements
 * the following additional functionalities:  a replacement string
 * object and an area for a user-defined widget to extend the dialog.
 *
 * Example
 *
 * To use the basic replace dialog:
 *
 * <pre>
 * </pre>
 *
 * To use your own extensions:
 *
 * <pre>
 * </pre>
 */

class KoReplaceDialog:
    public KoFindDialog
{
    Q_OBJECT

public:

    // Options.

    typedef enum
    {
        // Should the user be prompted before the replace operation?
        PromptOnReplace = 256,
        BackReference = 512
    } Options;

    /**
     * Construct a replace dialog.read-only or rather select-only combo box with a
     * parent object and a name.
     *
     * @param parent The parent object of this widget
     * @param name The name of this widget
     * @param options A bitfield of the @ref Options to be enabled.
     * @param findStrings A list of strings to find.
     * @param replaceStrings A list of strings to replace.
     * @param hasSelection Whether a selection exists
     */
    KoReplaceDialog( QWidget *parent = 0, const char *name = 0, long options = 0,
                     const QStringList &findStrings = QStringList(), const QStringList &replaceStrings = QStringList(),
                     bool hasSelection = true );

    /**
     * Destructor.
     */
    virtual ~KoReplaceDialog();

    /**
     * Provide the list of @p strings to be displayed as the history
     * of replacement strings. @p strings might get truncated if it is
     * too long.
     *
     * @see #replacementHistory
     */
    void setReplacementHistory( const QStringList &strings );

    /**
     * Returns the list of history items.
     *
     * @see #setReplacementHistory
     */
    QStringList replacementHistory() const;

    /**
     * Set the options which are enabled.
     *
     * @param options The setting of the @ref Options.
     */
    void setOptions( long options );

    /**
     * Returns the state of the options. Disabled options may be returned in
     * an indeterminate state.
     *
     * @see #setOptions
     */
    long options() const;

    /**
     * Returns the replacement string.
     */
    QString replacement() const;

    /**
     * Returns an empty widget which the user may fill with additional UI
     * elements as required. The widget occupies the width of the dialog,
     * and is positioned immediately the regular expression support widgets
     * for the replacement string.
     */
    QWidget *replaceExtension();

protected slots:

    void slotOk();

private:

    // Binary compatible extensibility.
    class KoReplaceDialogPrivate;
    KoReplaceDialogPrivate *d;
};

/**
 * @short A generic implementation of the "replace" function.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 *
 * This class includes prompt handling etc. Also provides some
 * static functions which can be used to create custom behavior
 * instead of using the class directly.
 *
 * Example
 *
 * To use the class to implement a complete replace feature:
 *
 * <pre>
 *
 *  // This creates a replace-on-prompt dialog if needed.
 *  dialog = new KoReplace(find, replace, options);
 *
 *  // Connect signals to code which handles highlighting
 *  // of found text, and on-the-fly replacement.
 *  QObject::connect(
 *      dialog, SIGNAL( highlight( const QString &, int, int, const QRect & ) ),
 *      this, SLOT( highlight( const QString &, int, int, const QRect & ) ) );
 *  QObject::connect(
 *      dialog, SIGNAL( replace( const QString &, int, int, const QRect & ) ),
 *      this, SLOT( replace( const QString &, int, int, const QRect & ) ) );
 *
 *  for (text chosen by option SelectedText and in a direction set by FindBackwards)
 *  {
 *      dialog->replace()
 *  }
 *  delete dialog;
 *
 * </pre>
 */

class KoReplace :
    public KDialogBase
{
    Q_OBJECT

public:

    /** Will create a prompt dialog and use it as needed. */
    KoReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent = 0);
    ~KoReplace();

    /**
     * Walk the text fragment (e.g. kwrite line, kspread cell) looking for matches.
     * For each match, if prompt-on-replace is specified, emits the expose() signal
     * and displays the prompt-for-replace dialog before doing the replace.
     *
     * @param text The text fragment to modify.
     * @param expose The region to expose
     * @return False if the user elected to discontinue the replace.
     */
    bool replace(QString &text, const QRect &expose);

    /**
     * @param text  The text fragment to modify
     * @param index The starting index where there is word found.
     * @param matchedlength The length of word found
     */
    virtual bool validateMatch( const QString &text, int index, int matchedlength ); 


    /**
     * Search the given string, and returns whether a match was found. If one is,
     * the replacement string length is also returned.
     *
     * A performance optimised version of the function is provided for use
     * with regular expressions.
     *
     * @param text The string to search.
     * @param pattern The pattern to look for.
     * @param replacement The replacement string.
     * @param index The starting index into the string.
     * @param options The options to use.
     * @param replacedLength The replaced length.
     * @return The index at which a match was found, or -1 if no match was found.
     */
    static int replace( QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength );
    static int replace( QString &text, const QRegExp &pattern, const QString &replacement, int index, long options, int *replacedLength );

    /**
     * Abort the current find/replace process. Call this when the parent widget
     * is getting destroyed.
     */
    void abort();

signals:

    /**
     * Connect to this slot to implement highlighting of found text during the replace
     * operation.
     */
    void highlight(const QString &text, int matchingIndex, int matchedLength, const QRect &expose);

    /**
     * Connect to this slot to implement updating of replaced text during the replace
     * operation.
     */
    void replace(const QString &text, int replacementIndex, int replacedLength,int matchedLength, const QRect &expose);

private:
    void doReplace();

    QString m_pattern;
    QRegExp *m_regExp;
    QString m_replacement;
    long m_options;
    QWidget *m_parent;
    unsigned m_replacements;
    QString m_text;
    int m_index;
    QRect m_expose;
    int m_matchedLength;
    bool m_cancelled;

    static int replace( QString &text, const QString &replacement, int index, int length );

    // Binary compatible extensibility.
    class KoReplacePrivate;
    KoReplacePrivate *d;

private slots:

    void slotUser1();   // All
    void slotUser2();   // Skip
    void slotUser3();   // Yes
    void slotClose();
};
#endif
