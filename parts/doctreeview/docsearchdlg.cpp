/***************************************************************************
 *   Copyright (C) 1999-2001 by Matthias Hoelzer-Kluepfel                  *
 *   hoelzer@kde.org                                                       *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "docsearchdlg.h"

#include <qcombobox.h>
#include <qfile.h>
#include <qdir.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>
#include <kdeversion.h>


DocSearchDialog::DocSearchDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Search in Documentation"));

    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

    QLabel *searchterm_label = new QLabel(i18n("&Words to search:"), this);
    layout->addWidget(searchterm_label);

    searchterm_edit = new QLineEdit(this);
    searchterm_edit->setFocus();
    searchterm_label->setBuddy(searchterm_edit);
    QFontMetrics fm(searchterm_edit->fontMetrics());
    searchterm_edit->setMinimumWidth(fm.width('X')*30);
    layout->addWidget(searchterm_edit);

    QHBox *method_box = new QHBox(this);
    layout->addSpacing(5);
    layout->addWidget(method_box);

    QLabel *method_label = new QLabel(i18n("&Method:"), method_box);

    method_combo = new QComboBox(method_box);
    method_label->setBuddy(method_combo);
    method_combo->insertItem(i18n("and"));
    method_combo->insertItem(i18n("or"));

    QHBox *sort_box = new QHBox(this);
    layout->addSpacing(5);
    layout->addWidget(sort_box);

    QLabel *sort_label = new QLabel(i18n("&Sort"), sort_box);

    sort_combo = new QComboBox(sort_box);
    sort_label->setBuddy(sort_combo);
    sort_combo->insertItem(i18n("Score"));
    sort_combo->insertItem(i18n("Title"));
    sort_combo->insertItem(i18n("Date"));

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
#if KDE_IS_VERSION( 3, 2, 90 )
    QPushButton *ok_button = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel_button = buttonbox->addButton(KStdGuiItem::cancel());
#else
    QPushButton *ok_button = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel_button = buttonbox->addButton(i18n("Cancel"));
#endif
    ok_button->setDefault(true);
    connect( ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);
}


DocSearchDialog::~DocSearchDialog()
{}


void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );


bool DocSearchDialog::performSearch()
{
    KConfig config("kdevdoctreeviewrc", true);
    config.setGroup("htdig");
    QString exe = config.readPathEntry("htsearchbin", kapp->dirs()->findExe("htsearch"));
    if (exe.isEmpty()) {
        // Check for htsearch in /usr/lib/cgi-bin (for Debian systems)
        exe = "/usr/lib/cgi-bin/htsearch";
        QFile f(exe);
        if (!f.exists()) {
	    KMessageBox::error(this, i18n("Cannot find the htsearch executable."));
            kdDebug() << "Can not find htsearch" << endl;
            return false;
        }
    }

    QString indexdir = kapp->dirs()->saveLocation("data", "kdevdoctreeview/helpindex");
    QDir d;
    if(indexdir.isEmpty() || !QFile::exists(indexdir + "/htdig.conf")) {
        if(QFile::exists("/var/lib/kdevelop3/helpindex/htdig.conf")) {
            indexdir = "/var/lib/kdevelop3/helpindex";
        }
	else if(QFile::exists("/var/lib/kdevelop/helpindex/htdig.conf")) {
            indexdir = "/var/lib/kdevelop/helpindex";
	} else {
            kdDebug() << "Can not find the htdig configuration file" << endl;
            return false;
        }
    }

    QString savedir = kapp->dirs()->saveLocation("data", "kdevdoctreeview/helpindex");
    if(!d.exists(savedir)) {
        d.mkdir(savedir);
    }

    QString query = QString("words=%1;method=%2;matchesperpage=%3;format=%4;sort=%5")
        .arg(searchterm_edit->text())
        .arg(method_combo->currentItem()==1? "or" : "and")
        .arg(50)
        .arg("builtin-short")
        .arg(sort_combo->currentItem()==2? "date" : sort_combo->currentItem()==1? "title" : "score");

    kdDebug(9002) << "starting kprocess" << endl;
    kdDebug(9002) << "htdig line:" << exe << " -c " << (indexdir + "/htdig.conf ") << query <<  endl;
    KProcess *proc = new KProcess;
    *proc << exe << "-c" << (indexdir + "/htdig.conf") << query;

    connect( proc, SIGNAL(receivedStdout(KProcess *,char*,int)),
             this, SLOT(htsearchStdout(KProcess *,char*,int)) );
    connect( proc, SIGNAL(processExited(KProcess *)),
             this, SLOT(htsearchExited(KProcess *)) );

    searchResult = "";

    if (!proc->start(KProcess::NotifyOnExit, KProcess::Stdout)) {
        KMessageBox::error(this, i18n("Cannot start the htsearch executable."));
        kdDebug() << "process start failed" << endl;
        delete proc;
        return false;
    }

    // While receiving data from the subprocess, we want
    // to block the user interface, but still get repaint
    // events. Hack taken from NetAccess...
    kapp->setOverrideCursor(waitCursor);
    QWidget blocker(0, 0, WType_Dialog | WShowModal);
    qt_enter_modal(&blocker);
    kapp->enter_loop();
    qt_leave_modal(&blocker);
    kapp->restoreOverrideCursor();

    if (!proc->normalExit() || proc->exitStatus() != 0) {
        kdDebug() << "Error running htsearch... returning now" << endl;
        delete proc;
        return false;
    }

    delete proc;

    // modify the search result
    searchResult = searchResult.replace(QRegExp("http://localhost/"), "file:/");
    searchResult = searchResult.replace(QRegExp("Content-type: text/html"), "");

    // dump the search result
    QFile f(savedir + "/results.html");
    if (f.open(IO_WriteOnly)) {
        QTextStream ts(&f);
        ts << searchResult << endl;
        f.close();
    }

    return true;
}


void DocSearchDialog::accept()
{
    performSearch();
    QDialog::accept();
}


void DocSearchDialog::htsearchStdout(KProcess *, char *buffer, int len)
{
    searchResult += QString::fromLocal8Bit(buffer, len);
}


void DocSearchDialog::htsearchExited(KProcess *)
{
    kapp->exit_loop();
}
#include "docsearchdlg.moc"
