/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FILENAMEEDIT_H_
#define _FILENAMEEDIT_H_

#include <qwidget.h>

class KLineEdit;
class KCompletion;


class FileNameEdit : public QWidget
{
    Q_OBJECT
    
public:
    FileNameEdit( const QString &title, QWidget *parent=0, const char *name=0 );
    ~FileNameEdit();

    void setText(const QString &text);
    QString text() const;
    
signals:
    void finished(const QString &fileName);

private slots:
    void returnPressed();
    void selfDestroy();

private:
    virtual void keyPressEvent(QKeyEvent *e);
    
    KLineEdit *filename_edit;
    KCompletion *comp;
};

#endif
