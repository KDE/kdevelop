/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MAKEWIDGET_H_
#define _MAKEWIDGET_H_

#include <qptrstack.h>
#include <qregexp.h>
#include <qtextedit.h>
#include <qvaluevector.h>
#include <qintdict.h>

#include "outputfilter.h"
#include "directorystatusmessagefilter.h"
#include "compileerrorfilter.h"
#include "commandcontinuationfilter.h"
#include "makeactionfilter.h"
#include "otherfilter.h"
#include "makeitem.h"

class MakeViewPart;
class MakeItem;
class KProcess;
class ProcessLineMaker;
class KURL;

namespace KTextEditor { class Document; }

class MakeWidget : public QTextEdit
{
    Q_OBJECT

public:
    MakeWidget(MakeViewPart *part);
    ~MakeWidget();

    void queueJob(const QString &dir, const QString &command);
    bool isRunning();
    void updateSettingsFromConfig();
    void processLine( const QString& line );

public slots:
    void startNextJob();
    void killJob();
    void nextError();
    void prevError();
    void copy();

protected:
    virtual QPopupMenu *createPopupMenu( const QPoint& pos );
    virtual void contentsMouseReleaseEvent( QMouseEvent* e );

private slots:
    void insertStdoutLine(const QString& line);
    void insertStderrLine(const QString& line);
    void slotProcessExited(KProcess *);
    void verticScrollingOn() { m_vertScrolling = true; }
    void verticScrollingOff() { m_vertScrolling = false; }
    void horizScrollingOn() { m_horizScrolling = true; }
    void horizScrollingOff() { m_horizScrolling = false; }
    void toggleLineWrapping();
    void slotVeryShortCompilerOutput();
    void slotShortCompilerOutput();
    void slotFullCompilerOutput();
    void toggleShowDirNavigMessages();
    void slotEnteredDirectory( EnteringDirectoryItem* );
    void slotExitedDirectory( ExitingDirectoryItem* );
    void insertItem( MakeItem* );
    void slotDocumentOpened(const KURL& filename);
    void slotDocumentClosed(QObject* doc);

private:
    void specialCheck( const QString& file, QString& fName ) const;
    virtual void keyPressEvent(QKeyEvent *e);
    void searchItem(int parag);
    QString guessFileName( const QString& fName, int parag ) const;
    QString directory(int parag) const;
    bool brightBg();
    void refill();
    void createCursor(ErrorItem* e, KTextEditor::Document* doc);

    bool scanErrorForward(int parag);
    bool scanErrorBackward(int parag);

    DirectoryStatusMessageFilter  m_directoryStatusFilter;
    CompileErrorFilter            m_errorFilter;
    CommandContinuationFilter     m_continuationFilter;
    MakeActionFilter              m_actionFilter;
    OtherFilter                   m_otherFilter;

    void setPendingItem(MakeItem* new_item);
    void displayPendingItem();
	bool appendToLastLine( const QString& text );
	void setCompilerOutputLevel(EOutputLevel level);

    QStringList commandList;
    QStringList dirList;
    QString m_lastBuildDir; //stores directory of last build, for the case we don't have proper path information
    QString currentCommand;
    KProcess *childproc;
    ProcessLineMaker* procLineMaker;
    QPtrStack<QString> dirstack;
    MakeItem* m_pendingItem;
    QValueVector<MakeItem*> m_items;
    QIntDict<MakeItem> m_paragraphToItem;
    long m_paragraphs;
    int m_lastErrorSelected;

    MakeViewPart *m_part;

    bool m_vertScrolling, m_horizScrolling;
    bool m_bCompiling;

    bool m_bLineWrapping;
    EOutputLevel m_compilerOutputLevel;
};

#endif
