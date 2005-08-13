/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "shellinsertdlg.h"

#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kconfig.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <klineedit.h>
#include <kstdguiitem.h>
#include <kdeversion.h>

#include "kdevplugin.h"
#include "domutil.h"
#include "filterpart.h"


ShellInsertDialog::ShellInsertDialog()
    : QDialog(0, "shell filter dialog", true)
{
    QVBoxLayout *layout = new QVBoxLayout(this, 10, 4);

    combo = new QComboBox(true, this);
    combo->setDuplicatesEnabled(false);
    layout->addWidget(combo);

    KButtonBox *buttonbox = new KButtonBox(this);
    start_button = buttonbox->addButton(i18n("&Start"));
    start_button->setDefault(true);
#if KDE_IS_VERSION( 3, 2, 90 )
    cancel_button = buttonbox->addButton(KStdGuiItem::cancel());
#else
    cancel_button = buttonbox->addButton(i18n("Cancel"));
#endif
    buttonbox->layout();
    layout->addWidget(buttonbox);

    connect( start_button, SIGNAL(clicked()),
             this, SLOT(slotStartClicked()) );
    connect( cancel_button, SIGNAL(clicked()),
             this, SLOT(reject()) );
    connect( combo->lineEdit(), SIGNAL(textChanged( const QString &)), this, SLOT(executeTextChanged( const QString &)));
    m_proc = 0;

    KConfig *config = FilterFactory::instance()->config();
    config->setGroup("General");
    QStringList items = config->readListEntry("InsertItems");
    combo->insertStringList(items);
    executeTextChanged( combo->lineEdit()->text());

}


ShellInsertDialog::~ShellInsertDialog()
{
    kdDebug(9029) << "~ShellInsertDialog" << endl;
    delete m_proc;

    // QComboBox API is a bit incomplete :-(
    QStringList list;
    for (int i=0; i < combo->count(); ++i)
        list << combo->text(i);

    KConfig *config = FilterFactory::instance()->config();
    config->setGroup("General");
    config->writeEntry("InsertItems", list);
}


void ShellInsertDialog::executeTextChanged( const QString &text)
{
    start_button->setEnabled(!text.isEmpty());
}

int ShellInsertDialog::exec()
{
    start_button->setEnabled(true);
    return QDialog::exec();
}


void ShellInsertDialog::slotStartClicked()
{
    start_button->setEnabled(false);
    m_str = QCString();

    delete m_proc;
    m_proc = new KShellProcess("/bin/sh");
    (*m_proc) << combo->currentText();
    connect( m_proc, SIGNAL(receivedStdout(KProcess*, char *, int)),
             this, SLOT(slotReceivedStdout(KProcess*, char *, int)) );
    connect( m_proc, SIGNAL(processExited(KProcess*)),
             this, SLOT(slotProcessExited(KProcess*)) );
    m_proc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}


void ShellInsertDialog::slotReceivedStdout(KProcess *, char *text, int len)
{
    m_str += QCString(text, len+1);
}


void ShellInsertDialog::slotProcessExited(KProcess *)
{
    if (m_proc->normalExit()) {
        accept();
    } else {
        KMessageBox::error(this, i18n("Process exited with status %1")
                           .arg(m_proc->exitStatus()));
        reject();
    }
}

#include "shellinsertdlg.moc"
