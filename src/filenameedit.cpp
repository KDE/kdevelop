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

#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcompletion.h>

#include "filenameedit.h"


FileNameEdit::FileNameEdit(const QString &title, QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    QLabel *label = new QLabel(title, this);
    label->setFrameStyle(QFrame::Plain);
    filename_edit = new KLineEdit(this, "filename edit");
    comp = new KURLCompletion();
    filename_edit->setCompletionObject(comp);
    filename_edit->setCompletionMode(KGlobalSettings::CompletionAuto);
    
    QBoxLayout *layout = new QHBoxLayout(this, 0, 0);
    layout->addWidget(label, 0);
    layout->addWidget(filename_edit, 1);

    connect( filename_edit, SIGNAL(returnPressed()), this, SLOT(returnPressed()) );

    setFocusProxy(filename_edit);
}



FileNameEdit::~FileNameEdit()
{
    delete comp;
}


void FileNameEdit::setText(const QString &text)
{
    filename_edit->setText(text);
}


QString FileNameEdit::text() const
{
    return filename_edit->text();
}


void FileNameEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Key_Escape) {
        e->ignore();
        delete this;
    }
}


void FileNameEdit::returnPressed()
{
    hide();
    emit finished(filename_edit->text());

    // We don't know whether there are other slots connected
    // to the signal, so at this point of time it is not
    // safe to destroy ourselves.
    QTimer::singleShot(0, this, SLOT(selfDestroy()));
}


void FileNameEdit::selfDestroy()
{
    delete this;
}
#include "filenameedit.moc"
