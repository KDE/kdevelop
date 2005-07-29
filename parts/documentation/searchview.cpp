/***************************************************************************
 *   Copyright (C) 1999-2001 by Matthias Hoelzer-Kluepfel                  *
 *   hoelzer@kde.org                                                       *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "searchview.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qregexp.h>

#include <kpushbutton.h>
#include <klistview.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdialog.h>
#include <kprocess.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <kdevpartcontroller.h>
#include <kdevdocumentationplugin.h>

#include "documentation_part.h"
#include "docutils.h"

SearchView::SearchView(DocumentationPart *part, QWidget *parent, const char *name)
    :QWidget(parent, name), m_part(part)
{
    QVBoxLayout *l = new QVBoxLayout(this, 0, KDialog::spacingHint());

    QVBoxLayout *l2 = new QVBoxLayout(l, 0);
    QLabel *editLabel = new QLabel(i18n("Wor&ds to search:"), this);
    l2->addWidget(editLabel);
    QHBoxLayout *l21 = new QHBoxLayout(l2, 0);
    m_edit = new KLineEdit(this);
    editLabel->setBuddy(m_edit);
    m_goSearchButton = new KPushButton(i18n("Se&arch"), this);
    l21->addWidget(m_edit);
    l21->addWidget(m_goSearchButton);

    QGridLayout *l3 = new QGridLayout(l, 2, 2, 0);
    m_searchMethodBox = new KComboBox(this);
    m_searchMethodBox->insertItem(i18n("and"));
    m_searchMethodBox->insertItem(i18n("or"));
    QLabel *smLabel = new QLabel(m_searchMethodBox, i18n("&Method:"), this);
    m_sortMethodBox = new KComboBox(this);
    m_sortMethodBox->insertItem(i18n("Score"));
    m_sortMethodBox->insertItem(i18n("Title"));
    m_sortMethodBox->insertItem(i18n("Date"));
    QLabel *rmLabel = new QLabel(m_sortMethodBox, i18n("S&ort by:"), this);
    l3->addWidget(smLabel, 0, 0);
    l3->addWidget(m_searchMethodBox, 0, 1);
    l3->addWidget(rmLabel, 1, 0);
    l3->addWidget(m_sortMethodBox, 1, 1);

    QVBoxLayout *l4 = new QVBoxLayout(l, 0);
    m_view = new KListView(this);
    QLabel *vLabel = new QLabel(m_view, i18n("Search &results:"), this);
    l4->addWidget(vLabel);
    l4->addWidget(m_view);

    QHBoxLayout *l5 = new QHBoxLayout(l, KDialog::spacingHint());
    m_configButton = new KPushButton(i18n("Update Config"), this);
    m_indexButton = new KPushButton(i18n("Update Index"), this);
    l5->addWidget(m_configButton);
    l5->addWidget(m_indexButton);
    l5->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

    l->addSpacing(2);

    m_view->setSorting(-1);
    m_view->addColumn(i18n("Relevance"));
    m_view->addColumn(i18n("Title"));
    m_view->setColumnWidthMode(0, QListView::Maximum);
    m_view->setColumnWidthMode(1, QListView::Maximum);
    m_view->setAllColumnsShowFocus(true);
    m_view->setResizeMode( QListView::LastColumn );

    connect(m_configButton, SIGNAL(clicked()), this, SLOT(updateConfig()));
    connect(m_indexButton, SIGNAL(clicked()), this, SLOT(updateIndex()));
    connect(m_edit, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(m_goSearchButton, SIGNAL(clicked()), this, SLOT(search()));
    connect(m_view, SIGNAL(executed(QListViewItem*)), this, SLOT(executed(QListViewItem*)));
    connect(m_view, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int )),
        this, SLOT(itemMouseButtonPressed(int, QListViewItem*, const QPoint&, int )));
}

SearchView::~SearchView()
{
}

void SearchView::updateConfig()
{
    runHtdig("-c");
}

void SearchView::updateIndex()
{
    runHtdig("-i");
    KConfig *config = m_part->config();
    config->setGroup("htdig");
    config->writeEntry("IsSetup", true);
    config->sync();
}

void SearchView::runHtdig(const QString &arg)
{
    KProcess proc;
    proc << "kdevelop-htdig" << arg;
    proc.start(KProcess::DontCare);
}

void qt_enter_modal(QWidget *widget);
void qt_leave_modal(QWidget *widget);

void SearchView::search()
{
    KConfig *config = m_part->config();
    config->setGroup("htdig");
    if (config->readBoolEntry("IsSetup", false) == false)
    {
        KMessageBox::information(this, i18n("Full text search has to be set up before usage."));
        if (!m_part->configure(1))
            return;
        KMessageBox::information(this, i18n("Now the full text search database will be created.\nWait for database creation to finish and then repeat search."));
        updateIndex();
        return;
    }
    QString exe = config->readPathEntry("htsearchbin", kapp->dirs()->findExe("htsearch"));
    if (exe.isEmpty())
    {
        KMessageBox::error(this, i18n("Cannot find the htsearch executable."));
        kdDebug() << "Can not find htsearch" << endl;
        return;
    }

    QString indexdir = kapp->dirs()->saveLocation("data", "kdevdocumentation/search");
    QDir d;
    if (indexdir.isEmpty() || !QFile::exists(indexdir + "/htdig.conf"))
    {
        if (QFile::exists("/var/lib/kdevelop3/helpindex/htdig.conf"))
            indexdir = "/var/lib/kdevelop3/helpindex";
        else if (QFile::exists("/var/lib/kdevelop/helpindex/htdig.conf"))
            indexdir = "/var/lib/kdevelop/helpindex";

        if (!QFile::exists(indexdir + "/htdig.conf"))
        {
            KMessageBox::error(this, i18n("Cannot find the htdig configuration file."));
            kdDebug() << "Cannot find the htdig configuration file" << endl;
            return;
        }
    }

    QString savedir = kapp->dirs()->saveLocation("data", "kdevdocumentation/search");
    if (!d.exists(savedir))
        d.mkdir(savedir);

    QString query = QString("words=%1;method=%2;matchesperpage=%3;format=%4;sort=%5")
        .arg(m_edit->text())
        .arg(m_searchMethodBox->currentItem()==1? "or" : "and")
        .arg(50)
        .arg("builtin-short")
        .arg(m_sortMethodBox->currentItem()==2? "date" : m_sortMethodBox->currentItem()==1? "title" : "score");

    kdDebug(9002) << "starting kprocess" << endl;
    kdDebug(9002) << "htdig line:" << exe << " -c " << (indexdir + "/htdig.conf ") << query <<  endl;
    KProcess *proc = new KProcess;
    QString picdir = kapp->dirs()->findResourceDir("data", "kdevdocumentation/pics/htdig.png");
    proc->setEnvironment("PICDIR", picdir);
    *proc << exe << "-c" << (indexdir + "/htdig.conf") << query;

    connect( proc, SIGNAL(receivedStdout(KProcess *,char*,int)),
             this, SLOT(htsearchStdout(KProcess *,char*,int)) );
    connect( proc, SIGNAL(processExited(KProcess *)),
             this, SLOT(htsearchExited(KProcess *)) );

    searchResult = "";

    if (!proc->start(KProcess::NotifyOnExit, KProcess::Stdout))
    {
        KMessageBox::error(this, i18n("Cannot start the htsearch executable."));
        kdDebug() << "process start failed" << endl;
        delete proc;
        return;
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

    if (!proc->normalExit() || proc->exitStatus() != 0)
    {
        kdDebug() << "Error running htsearch... returning now" << endl;
        delete proc;
        return;
    }

    delete proc;

    // modify the search result
    searchResult = searchResult.replace(QRegExp("http://localhost/"), "file:/");
    searchResult = searchResult.replace(QRegExp("Content-type: text/html"), "");

    // dump the search result
    QFile f(savedir + "/results.html");
    if (f.open(IO_WriteOnly))
    {
        QTextStream ts(&f);
        ts << searchResult << endl;
        f.close();
    }

    //show results
    analyseSearchResults();
//    m_part->partController()->showDocument(KURL("file://" + indexdir + "/results.html"));
}

void SearchView::htsearchStdout(KProcess *, char *buffer, int len)
{
    searchResult += QString::fromLocal8Bit(buffer, len);
}

void SearchView::htsearchExited(KProcess *)
{
    kapp->exit_loop();
}

void SearchView::analyseSearchResults()
{
    m_view->clear();
    QTextStream str(searchResult, IO_ReadOnly);
    DocumentationItem *former = 0;
    while (!str.eof())
    {
        QString line = str.readLine();

        QRegExp starsExp("alt=\"\\*\"");
        starsExp.setMinimal(true);
        int stars = line.contains(starsExp);

        QRegExp urlExp("<strong><a href=\"(.*)\">(.*)</a></strong>");
        if (urlExp.search(line)==-1)
            continue;
        QString url = urlExp.cap(1);
        QString title = urlExp.cap(2);

        QString starsStr;
        for (int i = 0; i < stars; ++i)
            starsStr += "*";

        if (former)
            former = new DocumentationItem(DocumentationItem::Document, m_view, former, starsStr);
        else
            former = new DocumentationItem(DocumentationItem::Document, m_view, starsStr);
        former->setText(1, title);
        former->setURL(KURL(url));
    }
}

void SearchView::executed(QListViewItem *item)
{
    DocumentationItem *d = dynamic_cast<DocumentationItem*>(item);
    if (!d)
        return;

    m_part->partController()->showDocument(d->url());
}

void SearchView::itemMouseButtonPressed(int button, QListViewItem *item, const QPoint &pos, int // c
                                        )
{
    if ((button != Qt::RightButton) || (!item))
        return;
    DocumentationItem *docItem = dynamic_cast<DocumentationItem*>(item);
    if (!docItem)
        return;

    DocUtils::docItemPopup(m_part, docItem, pos, true, false, 1);
}

void SearchView::setSearchTerm(const QString &term)
{
    m_edit->setText(term);
}

void SearchView::askSearchTerm()
{
    m_edit->setFocus();
}

void SearchView::focusInEvent(QFocusEvent */*e*/)
{
    m_edit->setFocus();
}

#include "searchview.moc"
