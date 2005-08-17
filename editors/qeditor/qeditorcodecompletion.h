/***************************************************************************
                          qeditorcodecompletion.h  -  description
                             -------------------
  begin      : Sun Nov 18 20:00 CET 2001
  copyright  : (C) 2001 Joseph Wenninger <jowenn@kde.org>
               (C) 2002 John Firebaugh <jfirebaugh@kde.org>
               (C) 2002 Roberto Raggi <roberto@kdevelop.org>

  taken from KDEVELOP:
  begin   : Sam Jul 14 18:20:00 CEST 2001
  copyright : (C) 2001 by Victor Röder <Victor_Roeder@GMX.de>
 ***************************************************************************/

/******** Partly based on the ArgHintWidget of Qt3 by Trolltech AS *********/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __QEditorCodeCompletion_H__
#define __QEditorCodeCompletion_H__

#include <q3valuelist.h>
#include <qstringlist.h>
#include <q3vbox.h>
#include <q3listbox.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QEvent>
#include <Q3Frame>
#include <QResizeEvent>

#include <ktexteditor/codecompletioninterface.h>

class QEditorArgHint;
class QEditorView;

class QEditorCodeCompletionCommentLabel : public QLabel
{
    Q_OBJECT
public:
    QEditorCodeCompletionCommentLabel( QWidget* parent, const QString& text) : QLabel( parent, "toolTipTip",
             Qt::WStyle_StaysOnTop | Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool | Qt::WX11BypassWM )
    {
        setMargin(1);
        setIndent(0);
        setAutoMask( FALSE );
        setFrameStyle( Q3Frame::Plain | Q3Frame::Box );
        setLineWidth( 1 );
        setAlignment( Qt::AlignLeft | Qt::AlignTop );
        polish();
        setText(text);
        adjustSize();
    }
};

/**
 *This class is used for providing a codecompletionbox with the same size in all editorwindows.
 *Therefor the size is stored statically and provided over sizeHint().
 *@short Codecompletion-Listbox
 */
class CCListBox : public Q3ListBox{
public:
    CCListBox(QWidget* parent = 0, const char* name = 0, Qt::WFlags f = 0):Q3ListBox(parent, name, f){
    	resize(m_size);
	//resize the frame containing the listbox (bad "style" but i don't know a better way)
	if (parent)
		parent->resize(m_size +  QSize(2,2));
    };

    QSize sizeHint()  const {
	return m_size;
    };

protected:
   void resizeEvent(QResizeEvent* rev){
	m_size = rev->size();
	Q3ListBox::resizeEvent(rev);
    };

private:
    static QSize m_size;
};

class QEditorCodeCompletion : public QObject
{
  Q_OBJECT

public:
  QEditorCodeCompletion(QEditorView *view);

  void showArgHint(
      QStringList functionList, const QString& strWrapping, const QString& strDelimiter );
  void showCompletionBox(
      Q3ValueList<KTextEditor::CompletionEntry> entries, int offset = 0, bool casesensitive = true );
  bool eventFilter( QObject* o, QEvent* e );

public slots:
  void slotCursorPosChanged();
  void showComment();

signals:
  void completionAborted();
  void completionDone();
  void argHintHidden();
  void completionDone(KTextEditor::CompletionEntry);
  void filterInsertString(KTextEditor::CompletionEntry*,QString *);

private:
  void abortCompletion();
  void complete( KTextEditor::CompletionEntry );
  void updateBox( bool newCoordinate = false );

  QEditorArgHint*    m_pArgHint;
  QEditorView*       m_view;
  Q3VBox*          m_completionPopup;
  CCListBox*      m_completionListBox;
  Q3ValueList<KTextEditor::CompletionEntry> m_complList;
  uint            m_lineCursor;
  uint            m_colCursor;
  int             m_offset;
  bool            m_caseSensitive;
  QEditorCodeCompletionCommentLabel* m_commentLabel;
};

#endif
