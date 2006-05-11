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

#include <q3ptrstack.h>
#include <QRegExp>
#include <q3textedit.h>
#include <q3valuevector.h>
#include <q3intdict.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <Q3PopupMenu>

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
class KUrl;

namespace KTextEditor { class Document; }

class MakeWidget : public Q3TextEdit
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
    virtual Q3PopupMenu *createPopupMenu( const QPoint& pos );
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
    void slotDocumentOpened(KDevDocument* document);
    void slotDocumentAboutToClose(KTextEditor::Document* doc);

private:
    void checkIfDocumentLoaded(ErrorItem* e);
    void specialCheck( const QString& file, QString& fName ) const;
    virtual void keyPressEvent(QKeyEvent *e);
    void searchItem(int parag);
    QString guessFileName( const QString& fName, int parag ) const;
    QString directory(int parag) const;
    bool brightBg();
    void refill();

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
    QString currentCommand;
    KProcess *childproc;
    ProcessLineMaker* procLineMaker;
    Q3PtrStack<QString> dirstack;
    MakeItem* m_pendingItem;
    Q3ValueVector<MakeItem*> m_items;
    Q3IntDict<MakeItem> m_paragraphToItem;
    long m_paragraphs;
    int m_lastErrorSelected;

    MakeViewPart *m_part;

    bool m_vertScrolling, m_horizScrolling;
    bool m_bCompiling;

    bool m_bLineWrapping;
    EOutputLevel m_compilerOutputLevel;
};

#endif
