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

#ifndef KOFIND_H
#define KOFIND_H

#include <kdialogbase.h>
//Added by qt3to4:
#include <QLabel>
#include <QGridLayout>
#include <Q3PopupMenu>

class KHistoryCombo;
class QCheckBox;
class Q3GroupBox;
class QLabel;
class Q3PopupMenu;
class QPushButton;
class QRect;
class QGridLayout;

/**
 * @short A generic "find" widget.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 *
 * This widget inherits from @ref KDialogBase and implements
 * the following additional functionalities:  a find string
 * object and an area for a user-defined widget to extend the dialog.
 *
 * Example
 *
 * To use the basic find dialog:
 *
 * <pre>
 *  KoFindDialog dlg(....)
 *  if ( dlg.exec() != QDialog::Accepted )
 *      return;
 *
 *  // proceed with KoFind from here
 * </pre>
 *
 * To use your own extensions:
 *
 * <pre>
 * </pre>
 */

class KoFindDialog:
    public KDialogBase
{
    Q_OBJECT

public:

    // Options.

    typedef enum
    {
        WholeWordsOnly = 1,     // Match whole words only.
        FromCursor = 2,         // Start from current cursor position.
        SelectedText = 4,       // Only search selected area.
        Qt::CaseSensitive = 8,      // Consider case when matching.
        FindBackwards = 16,     // Go backwards.
        RegularExpression = 32, // Interpret the pattern as a regular expression.
        // User extensions can use boolean options above this value.
        MinimumUserOption = 65536
    } Options;

    /**
     * Construct a find dialog with a parent object and a name.
     *
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     * @param options A bitfield of the @ref Options to be enabled.
     * @param findStrings The find history, @see findHistory()
     * @param hasSelection Whether a selection exists
     */
    KoFindDialog( QWidget *parent = 0, const char *name = 0, long options = 0,
                  const QStringList &findStrings = QStringList(), bool hasSelection = true );

    /**
     * Destructor.
     */
    virtual ~KoFindDialog();

    /**
     * Provide the list of @p strings to be displayed as the history
     * of find strings. @p strings might get truncated if it is
     * too long.
     *
     * @param strings The find history.
     * @see #findHistory
     */
    void setFindHistory( const QStringList &strings );

    /**
     * Returns the list of history items.
     *
     * @see #setFindHistory
     */
    QStringList findHistory() const;

    /**
     * Enable/disable the 'search in selection' option, depending
     * on whether there actually is a selection.
     *
     * @param hasSelection true if a selection exists
     */
    void setHasSelection( bool hasSelection );

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
     * Returns the pattern to find.
     */
    QString pattern() const;

    /**
     * Returns an empty widget which the user may fill with additional UI
     * elements as required. The widget occupies the width of the dialog,
     * and is positioned immediately below the regular expression support
     * widgets for the pattern string.
     */
    QWidget *findExtension();

signals:

    /**
     * The OK button was pressed, the pattern was not empty, and if it is
     * supposed to be a regular expression, it is known to be valid.
     */
    void okClicked();

protected slots:

    void slotOk();
    void slotSelectedTextToggled(bool);
    void showPatterns();
    void showPlaceholders();
    void textSearchChanged( const QString &);
  
public:        
    KHistoryCombo *m_find;
    KHistoryCombo *m_replace;

private:
    Q3GroupBox *m_findGrp;
    QLabel *m_findLabel;
    QCheckBox *m_regExp;
    QPushButton *m_regExpItem;
    QGridLayout *m_findLayout;
    QWidget *m_findExtension;

    Q3GroupBox *m_optionGrp;
    QCheckBox *m_wholeWordsOnly;
    QCheckBox *m_fromCursor;
    QCheckBox *m_selectedText;
    QCheckBox *m_caseSensitive;
    QCheckBox *m_findBackwards;

    Q3PopupMenu *m_patterns;

    // Our dirty little secret is that we also implement the "replace" dialog. But we
    // keep that fact hidden from all but our friends.

    friend class KoReplaceDialog;

    /**
     * Construct a find dialog with a parent object and a name. This version of the
     * constructor is for use by friends only!
     *
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     * @param forReplace Is this a replace dialog?
     */
    KoFindDialog( QWidget *parent, const char *name, bool forReplace );
    void init( bool forReplace, const QStringList &findStrings, bool hasSelection );

    Q3GroupBox *m_replaceGrp;
    QLabel *m_replaceLabel;
    QCheckBox* m_backRef;
    QPushButton* m_backRefItem;
    QGridLayout *m_replaceLayout;
    QWidget *m_replaceExtension;

    QCheckBox* m_promptOnReplace;

    Q3PopupMenu *m_placeholders;

    // Binary compatible extensibility.
    class KoFindDialogPrivate;
    KoFindDialogPrivate *d;
};

/**
 * @short A generic implementation of the "find" function.
 *
 * @author S.R.Haque <srhaque@iee.org>
 *
 *
 * This class includes prompt handling etc. Also provides some
 * static functions which can be used to create custom behaviour
 * instead of using the class directly.
 *
 * Example
 *
 * To use the class to implement a complete find feature:
 *
 * <pre>
 *
 *  // This creates a find-next-prompt dialog if needed.
 *  dialog = new KoFind(find, options);
 *
 *  // Connect signals to code which handles highlighting
 *  // of found text.
 *  connect(dialog, SIGNAL( highlight( const QString &, int, int, const QRect & ) ),
 *          this, SLOT( highlight( const QString &, int, int, const QRect & ) ) );
 *
 *  // Loop over all the text fragments of our document or selection
 *  for (text chosen by option SelectedText and in a direction set by FindBackwards)
 *       // don't forget to honour FromCursor too
 *  {
 *      // Let KoFind inspect the text fragment, and display a dialog if a match is found
 *      if ( !dialog->find( text_fragment, region_to_expose ) )
 *          break; // if cancelled by user
 *  }
 *  delete dialog;
 *
 * </pre>
 */

class KoFind :
    public KDialogBase
{
    Q_OBJECT

public:

    // Will create a prompt dialog and use it as needed.
    KoFind(const QString &pattern, long options, QWidget *parent = 0);
    ~KoFind();

    /**
     * Walk the text fragment (e.g. kwrite line, kspread cell) looking for matches.
     * For each match, emits the expose() signal and displays the find-again dialog
     * proceeding.
     *
     * @param text The text fragment to modify.
     * @param expose The region to expose
     * @return false if the user elected to discontinue the find.
     */
    bool find(const QString &text, const QRect &expose);

    virtual bool validateMatch( const QString &/*text*/, int /*index*/, int /*matchedlength*/ ) { return true; }

    /**
     * Search the given string, and returns whether a match was found. If one is,
     * the length of the string matched is also returned.
     *
     * A performance optimised version of the function is provided for use
     * with regular expressions.
     *
     * @param text The string to search.
     * @param pattern The pattern to look for.
     * @param index The starting index into the string.
     * @param options The options to use.
     * @param matchedlength The length that matched.
     * @return The index at which a match was found, or -1 if no match was found.
     */
    static int find( const QString &text, const QString &pattern, int index, long options, int *matchedlength );
    static int find( const QString &text, const QRegExp &pattern, int index, long options, int *matchedlength );

    /**
     * Abort the current find process. Call this when the parent widget
     * is getting destroyed.
     */
    void abort();

signals:

    // Connect to this signal to implement highlighting of found text during the find
    // operation.
    void highlight(const QString &text, int matchingIndex, int matchedLength, const QRect &expose);

private:

    QString m_pattern;
    QRegExp *m_regExp;
    long m_options;
    QWidget *m_parent;
    unsigned m_matches;
    QString m_text;
    int m_index;
    QRect m_expose;
    int m_matchedLength;
    bool m_cancelled;

    static bool isInWord( QChar ch );
    static bool isWholeWords( const QString &text, int starts, int matchedLength );

    // Binary compatible extensibility.
    class KoFindPrivate;
    KoFindPrivate *d;

private slots:

    virtual void slotUser1();   // Yes
    virtual void slotClose();
};

#endif
