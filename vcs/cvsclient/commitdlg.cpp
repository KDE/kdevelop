/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kbuttonbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>

#include "commitdlg.h"

//#include "ksyntaxhighlighter.h"

CommitDialog::CommitDialog()
    : QDialog(0, "", true )
{
    setCaption( i18n("CVS Commit") );

    QBoxLayout *layout = new QVBoxLayout(this, 10);

    QLabel *messagelabel = new QLabel(i18n("Enter log message:"), this);
    messagelabel->setMinimumSize(messagelabel->sizeHint());
    layout->addWidget(messagelabel, 0);

    textEdit = new QTextEdit(this);
    QFontMetrics fm(textEdit->fontMetrics());
    textEdit->setMinimumSize(fm.width("0")*80, fm.lineSpacing()*3);
    layout->addWidget(textEdit, 10);

    checkAddToChangelog = new QCheckBox( i18n("&Add entry to master changelog too"), this, "checkboxaddtochangelog" );
    layout->addWidget( checkAddToChangelog, 0 );

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
#if KDE_IS_VERSION( 3, 2, 90 )
    QPushButton *ok = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel());
#else
    QPushButton *ok = buttonbox->addButton(KStdGuiItem::ok().text());
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel().text());
#endif
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    ok->setDefault(true);
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    layout->activate();
    adjustSize();

//    new KDictSpellingHighlighter( textEdit );
}

QStringList CommitDialog::logMessage() const
{
    QStringList textLines;
//    textEdit->setText( "Bla bla bla bla\n" );
    for (int i=0; i<textEdit->paragraphs(); ++i)
    {
        textLines << textEdit->text( i );
    }
    return textLines;
}

bool CommitDialog::mustAddToChangeLog() const
{
    return checkAddToChangelog->isChecked();
}

void CommitDialog::accept()
{
    if (textEdit->text().isNull() || textEdit->text().isEmpty()) {
        int s = KMessageBox::warningContinueCancel( this,
            i18n("You are committing your changes without any comment. This is not a good practice. Continue anyway?"),
            i18n("CVS Commit Warning"),
            KStdGuiItem::cont(),
            i18n("askWhenCommittingEmptyLogs") );
        if ( s != KMessageBox::Continue ) {
            return;
        }
    }
    QDialog::accept();
}

#include "commitdlg.moc"

// Local Variables:
// c-basic-offset: 4
// End:


