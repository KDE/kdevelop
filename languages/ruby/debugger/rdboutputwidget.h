// *************************************************************************
//                          gdboutputwidget.cpp  -  description
//                             -------------------
//    begin                : 10th April 2003
//    copyright            : (C) 2003 by John Birch
//    email                : jbb@kdevelop.org
//	
//                          Adapted for ruby debugging
//                          --------------------------
//    begin                : Mon Nov 1 2004
//    copyright            : (C) 2004 by Richard Dale
//    email                : Richard_Dale@tipitina.demon.co.uk
// **************************************************************************
//
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#ifndef _RDBOUTPUTWIDGET_H_
#define _RDBOUTPUTWIDGET_H_

#include <qwidget.h>
//Added by qt3to4:
#include <QFocusEvent>

class KHistoryCombo;

class Q3TextEdit;
class QToolButton;

namespace RDBDebugger
{

class RDBOutputWidget : public QWidget
{
    Q_OBJECT

public:
    RDBOutputWidget( QWidget *parent=0, const char *name=0 );
    ~RDBOutputWidget();

    void clear();

public slots:
    void slotReceivedStdout(const char* line);
    void slotReceivedStderr(const char* line);
    void slotDbgStatus     (const QString &status, int statusFlag);

    void slotRDBCmd();

protected:
    virtual void focusInEvent(QFocusEvent *e);

signals:
    void userRDBCmd(const QString &cmd);
    void breakInto();

private:
    KHistoryCombo*  m_userRDBCmdEditor;
    QToolButton*    m_Interrupt;
    Q3TextEdit*      m_rdbView;
};

}

#endif
