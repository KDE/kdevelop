// *************************************************************************
//                          gdboutputwidget.cpp  -  description
//                             -------------------
//    begin                : 10th April 2003
//    copyright            : (C) 2003 by John Birch
//    email                : jbb@kdevelop.org
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

#ifndef _GDBOUTPUTWIDGET_H_
#define _GDBOUTPUTWIDGET_H_

#include <qwidget.h>

class QTextEdit;
class KHistoryCombo;

namespace GDBDebugger
{

class GDBOutputWidget : public QWidget
{
    Q_OBJECT

public:
    GDBOutputWidget( QWidget *parent=0, const char *name=0 );
    ~GDBOutputWidget();

    void clear();

public slots:
    void slotReceivedStdout(const char* line);
    void slotReceivedStderr(const char* line);

    void slotGDBCmd();

signals:
    void userGDBCmd(const QString &cmd);

private:
    QTextEdit *gdbView_;
    KHistoryCombo *userGDBCmdEditor_;
};

}

#endif
