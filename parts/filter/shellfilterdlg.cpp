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

#include "shellfilterdlg.h"

#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstdguiitem.h>
#include <kdeversion.h>

#include "kdevplugin.h"
#include "domutil.h"
#include "filterpart.h"


ShellFilterDialog::ShellFilterDialog()
    : QDialog(0, "shell filter dialog", true)
{
    QVBoxLayout *layout = new QVBoxLayout(this, 10, 4);

    combo = new QComboBox(true, this);
    combo->setDuplicatesEnabled(false);
    layout->addWidget(combo);

    KButtonBox *buttonbox = new KButtonBox(this);
    start_button = buttonbox->addButton(i18n("&Start"));
    start_button->setDefault(true);
    cancel_button = buttonbox->addButton(KStdGuiItem::cancel());
    buttonbox->layout();
    layout->addWidget(buttonbox);

    connect( start_button, SIGNAL(clicked()),
             this, SLOT(slotStartClicked()) );
    connect( cancel_button, SIGNAL(clicked()),
             this, SLOT(reject()) );

    m_proc = 0;

    KConfig *config = FilterFactory::instance()->config();
    config->setGroup("General");
    combo->insertStringList(config->readListEntry("filteritems"));
}


ShellFilterDialog::~ShellFilterDialog()
{
    kdDebug(9029) << "~ShellFilterDialog" << endl;
    delete m_proc;

    // QComboBox API is a bit incomplete :-(
    QStringList list;
    for (int i=0; i < combo->count(); ++i)
        list << combo->text(i);

    KConfig *config = FilterFactory::instance()->config();
    config->setGroup("General");
    config->writeEntry("filteritems", list);
}


void ShellFilterDialog::slotStartClicked()
{
    start_button->setEnabled(false);
    m_outstr = QCString();

    delete m_proc;
    m_proc = new KShellProcess("/bin/sh");
    (*m_proc) << combo->currentText();
    connect( m_proc, SIGNAL(receivedStdout(KProcess*, char *, int)),
             this, SLOT(slotReceivedStdout(KProcess*, char *, int)) );
    connect( m_proc, SIGNAL(wroteStdin(KProcess*)),
             this, SLOT(slotWroteStdin(KProcess*)) );
    connect( m_proc, SIGNAL(processExited(KProcess*)),
             this, SLOT(slotProcessExited(KProcess*)) );
    m_proc->start(KProcess::NotifyOnExit, KProcess::All);
    m_proc->writeStdin(m_instr, m_instr.length());
}


int ShellFilterDialog::exec()
{
    start_button->setEnabled(true);
    return QDialog::exec();
}


void ShellFilterDialog::slotReceivedStdout(KProcess *, char *text, int len)
{
    m_outstr += QString::fromLocal8Bit(text, len+1);
    kdDebug(9029) << "outstr " << m_outstr << endl;
}


void ShellFilterDialog::slotWroteStdin(KProcess *)
{
    m_proc->closeStdin();
    kdDebug(9029) << "close stdin " << m_outstr << endl;
}


void ShellFilterDialog::slotProcessExited(KProcess *)
{
    kdDebug(9029) << "process exit " << m_proc->normalExit() << endl;
    if (m_proc->normalExit()) {
        accept();
    } else {
        KMessageBox::error(this, i18n("Process exited with status %1")
                           .arg(m_proc->exitStatus()));
        reject();
    }
}

#include "shellfilterdlg.moc"
