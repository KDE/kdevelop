/***************************************************************************
                          memview.h  -  description
                             -------------------
    begin                : Tue Oct 5 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MEMVIEW_H_
#define _MEMVIEW_H_

#include <kdialog.h>

class KLineEdit;
class QMultiLineEdit;

namespace JAVADebugger
{


/**
 * @author John Birch
 */

class MemoryViewDialog : public KDialog
{
    Q_OBJECT

public:
    MemoryViewDialog( QWidget *parent=0, const char *name=0 );
    ~MemoryViewDialog();

signals:
    void disassemble(const QString &start, const QString &end);
    void memoryDump(const QString &start, const QString &end);
    void registers();
    void libraries();

public slots:
    void slotRawJDBMemoryView(char *buf);

private slots:
    void slotDisassemble();
    void slotMemoryDump();

private:
    KLineEdit *start_, *end_;
    QMultiLineEdit *output_;
};

}

#endif
