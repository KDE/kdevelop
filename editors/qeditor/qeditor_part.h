/*
 *  Copyright (C) 2001 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef QEDITORPART_H
#define QEDITORPART_H

#include <kdeversion.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/cursorinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/searchinterface.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/configinterface.h>
#if (KDE_VERSION > 305)
# include <ktexteditor/markinterfaceextension.h>
#else
# include "kde30x_markinterfaceextension.h"
#endif
#include <qptrlist.h>

#include "qeditor_factory.h"

class QWidget;
class QPainter;
class KURL;
class QEditorView;
class QEditorIndenter;
class QEditorBrowserExtension;
class KAboutData;
class HLMode;
class KConfig;
class QSourceColorizer;
class KDialogBase;

class QEditorPart:
	public KTextEditor::Document,
	public KTextEditor::EditInterface,
	public KTextEditor::UndoInterface,
	public KTextEditor::CursorInterface,
	public KTextEditor::SelectionInterface,
	public KTextEditor::HighlightingInterface,
	public KTextEditor::SearchInterface,
	public KTextEditor::MarkInterface,
	public KTextEditor::MarkInterfaceExtension,
	public KTextEditor::ConfigInterface
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    QEditorPart(QWidget *parentWidget, const char *widgetName,
                QObject *parent, const char *name, const QStringList &args);

    /**
     * Destructor
     */
    virtual ~QEditorPart();

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    virtual void setReadWrite(bool rw);

    /**
     * Reimplemented to disable and enable Save action
     */
    virtual void setModified(bool modified);

    QSourceColorizer* colorizer() const;
    QEditorIndenter* indenter() const;

signals:
    void fileNameChanged();
    void configWidget( KDialogBase* );

// -- ConfigInterface ----------------------------------------------------------------------
public slots:
    /**
      Read/Write the config to the standard place where this editor
      part saves it config, say: read/save default values for that
      editor part
    */
    virtual void readConfig ();
    virtual void writeConfig ();

    /**
      Read/Write the config of the part to a given kconfig object
      to store the settings in a different place than the standard
    */
    virtual void readConfig (KConfig *);
    virtual void writeConfig (KConfig *);

    /**
      Read/Write session config of only this document/view/plugin
    */
    virtual void readSessionConfig (KConfig *);
    virtual void writeSessionConfig (KConfig *);

    /**
      Shows a config dialog for the part
    */
    virtual void configDialog();

// -- MarkInterface ------------------------------------------------------------------------
public:
    virtual uint mark (uint line);

    virtual void setMark (uint line, uint markType);
    virtual void clearMark (uint line);

    virtual void addMark (uint line, uint markType);
    virtual void removeMark (uint line, uint markType);

    virtual QPtrList<KTextEditor::Mark> marks ();
    virtual void clearMarks ();

signals:
    void marksChanged();

// -- MarkInterfaceExtension----------------------------------------------------------------
public:
    virtual void setPixmap(MarkInterface::MarkTypes, const QPixmap &);
    virtual void setDescription(MarkInterface::MarkTypes, const QString &);
    virtual void setMarksUserChangable(uint markMask);

signals:
    virtual void markChanged (KTextEditor::Mark mark,
                              KTextEditor::MarkInterfaceExtension::MarkChangeAction action);

// -- SearchInterface ----------------------------------------------------------------------
public:
    virtual bool searchText (unsigned int startLine, unsigned int startCol,
                             const QString &text, unsigned int *foundAtLine, unsigned int *foundAtCol,
                             unsigned int *matchLen, bool casesensitive = true, bool backwards = false);

    virtual bool searchText (unsigned int startLine, unsigned int startCol,
                             const QRegExp &regexp, unsigned int *foundAtLine,
                             unsigned int *foundAtCol, unsigned int *matchLen, bool backwards = false);

// -- Document ------------------------------------------------------------------------------
public:
    QEditorView* currentView() const { return m_currentView; }

    /**
     * Create a view that will display the document data. You can create as many
     * views as you like. When the user modifies data in one view then all other
     * views will be updated as well.
     */
    virtual KTextEditor::View *createView ( QWidget *parent, const char *name = 0 );



    /*
     * Accessor to the list of views.
     */
    virtual QPtrList<KTextEditor::View> views () const;

// -- HighlightingInterface ---------------------------------------------------------------------
public:
    /**
     * returns the current active highlighting mode
     */
    virtual unsigned int hlMode ();

    /**
     * set the current active highlighting mode
     */
    virtual bool setHlMode (unsigned int mode);

    /**
     * returns the number of available highlightings
     */
    virtual unsigned int hlModeCount ();

    /**
     * returns the name of the highlighting with number "mode"
     */
    virtual QString hlModeName (unsigned int mode);

    /**
     * returns the sectionname of the highlighting with number "mode"
     */
    virtual QString hlModeSectionName (unsigned int mode);

signals:
    void hlChanged ();

private:
    void setupHighlighting();
    int findMode( const QString& );

private:
    QPtrList<HLMode> m_modes;
    unsigned int m_currentMode;

// -- UndoInterface -----------------------------------------------------------------------------
public:
    virtual void clearUndo ();
    virtual void clearRedo ();
    virtual unsigned int undoCount () const;
    virtual unsigned int redoCount () const;
    virtual unsigned int undoSteps () const;
    virtual void setUndoSteps ( unsigned int steps );

public slots:
    virtual void undo ();
    virtual void redo ();

signals:
    void undoChanged ();

// -- EditInterface ----------------------------------------------------------------------
public:
    /**
     * @return the complete document as a single QString
     */
    virtual QString text () const;

    /**
     * @return a QString
     */
    virtual QString text ( unsigned int startLine, unsigned int startCol, unsigned int endLine, unsigned int endCol ) const;

    /**
     * @return All the text from the requested line.
     */
    virtual QString textLine ( unsigned int line ) const;

    /**
     * @return The current number of lines in the document
     */
    virtual unsigned int numLines () const;

    /**
     * @return the number of characters in the document
     */
    virtual unsigned int length () const;

    /**
     * @return the number of characters in the line (-1 if no line "line")
     */
    virtual int lineLength ( unsigned int line ) const;

    /**
     * Set the given text into the view.
     * Warning: This will overwrite any data currently held in this view.
     */
    virtual bool setText ( const QString &text );

    /**
     *  clears the document
     * Warning: This will overwrite any data currently held in this view.
     */
    virtual bool clear ();

    /**
     *  Inserts text at line "line", column "col"
     *  returns true if success
     */
    virtual bool insertText ( unsigned int line, unsigned int col, const QString &text );

    /**
     *  remove text at line "line", column "col"
     *  returns true if success
     */
    virtual bool removeText ( unsigned int startLine, unsigned int startCol, unsigned int endLine, unsigned int endCol );

    /**
     * Insert line(s) at the given line number. If the line number is -1
     * (the default) then the line is added to end of the document
     */
    virtual bool insertLine ( unsigned int line, const QString &text );

    /**
     * Insert line(s) at the given line number. If the line number is -1
     * (the default) then the line is added to end of the document
     */
    virtual bool removeLine ( unsigned int line );

signals:
    void textChanged ();
    void newStatus ();
    void charactersInteractivelyInserted(int ,int ,const QString&);

// -- CursorInterface ------------------------------------------------------------------------
public:
    /**
     * Create a new cursor object
     */
    virtual KTextEditor::Cursor *createCursor ( );

    /*
     * Accessor to the list of views.
     */
    virtual QPtrList<KTextEditor::Cursor> cursors () const;

// -- SelectionInterface ----------------------------------------------------------------------
public:
    /**
     *  @return set the selection from line_start,col_start to line_end,col_end
     */
    virtual bool setSelection ( unsigned int startLine, unsigned int startCol, unsigned int endLine, unsigned int endCol );

    /**
     *  removes the current Selection (not Text)
     */
    virtual bool clearSelection ();

    /**
     *  @return true if there is a selection
     */
    virtual bool hasSelection () const;

    /**
     *  @return a QString for the selected text
     */
    virtual QString selection () const;

    /**
     *  removes the selected Text
     */
    virtual bool removeSelectedText ();

    /**
     *  select the whole text
     */
    virtual bool selectAll();

signals:
    void selectionChanged ();

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

    /**
     * This must be implemented by each read-write part
     */
    virtual bool saveFile();

public slots:
    void fileReload();

protected slots:
    void fileOpen();
    void fileSaveAs();

private:
    QEditorView* m_currentView;
    QPtrList<KTextEditor::View> m_views;
    QPtrList<KTextEditor::Cursor> m_cursors;
    QPtrList<KTextEditor::Mark> m_marks;
    QEditorBrowserExtension* m_extension;
};

#endif // QEDITORPART_H
