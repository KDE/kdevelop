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

#include <qstack.h>
#include <qregexp.h>
#include <qtextedit.h>


class MakeViewPart;
class MakeItem;
class KProcess;
class ProcessLineMaker;


class MakeWidget : public QTextEdit {
    Q_OBJECT

public:
    MakeWidget(MakeViewPart *part);
    ~MakeWidget();

    void queueJob(const QString &dir, const QString &command);
    bool isRunning();
    void updateSettingsFromConfig();

public slots:
    void startNextJob();
    void killJob();
    void nextError();
    void prevError();
    void copy();
    
protected:
    void paletteChange(const QPalette& oldPalette);
    virtual QPopupMenu *createPopupMenu( const QPoint& pos );

private slots:
    void insertStdoutLine(const QString& line);
    void insertStderrLine(const QString& line);
    void slotProcessExited(KProcess *);
    void verticScrollingOn() { m_vertScrolling = true; }
    void verticScrollingOff() { m_vertScrolling = false; }
    void horizScrollingOn() { m_horizScrolling = true; }
    void horizScrollingOff() { m_horizScrolling = false; }
    void toggleLineWrapping();
    void toggleCompilerOutput();
    void toggleShowDirNavigMessages();
    
private:
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    void searchItem(int parag);

    enum Type { Normal, Error, Diagnostic, StyledDiagnostic };
    void insertLine1(const QString &line, Type type);
    void insertLine2(const QString &line, Type type, QStringList* pStringList = 0L);
    bool matchEnterDir( const QString& line, QString& dir );
    bool matchLeaveDir( const QString& line, QString& dir );
    QString getOutputColor( Type type );
    void updateColors();

    QStringList commandList;
    QStringList dirList;
    QString currentCommand;
    KProcess *childproc;
    ProcessLineMaker* procLineMaker;
    QStack<QString> dirstack;
    QList<MakeItem> items;
    int parags;
    bool moved;
    QString normalColor, errorColor, diagnosticColor;

    MakeViewPart *m_part;
    
    bool m_vertScrolling, m_horizScrolling;
    
    bool m_bLineWrapping;
    bool m_bShortCompilerOutput;
    bool m_bShowDirNavMsg;
    
    QRegExp m_errorGccRx;
    int m_errorGccFileGroup;
    int m_errorGccRowGroup;
    int m_errorGccTextGroup;

    QRegExp m_errorFtnchekRx;
    int m_errorFtnchekFileGroup;
    int m_errorFtnchekRowGroup;
    int m_errorFrnchekTextGroup;

    QRegExp m_errorJadeRx;
    int m_errorJadeFileGroup;
    int m_errorJadeRowGroup;
    int m_errorJadeTextGroup;

    QRegExp m_compileFile1;
    QRegExp m_compileFile2;
    QRegExp m_compileFile3;
    QRegExp m_mocFile;
    QRegExp m_linkFile;
    QRegExp m_installFile;
    int m_fileNameGroup;
    
    QStringList m_shortOutput;
    QStringList m_fullOutput;
};

#endif
