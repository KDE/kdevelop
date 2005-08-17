/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "integratordlg.h"

#include <qfile.h>
#include <qdir.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3CString>

#include <kapplication.h>
#include <kdialogbase.h>
#include <kurlrequester.h>
#include <kprocess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <cvsservice_stub.h>

#include "initdlg.h"

IntegratorDlg::IntegratorDlg(CVSServiceIntegrator *integrator, QWidget *parent, const char *name)
    :IntegratorDlgBase(parent, name), m_integrator(integrator)
{
    QFile cvspass(QDir::homeDirPath() + "/.cvspass");
    if (cvspass.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&cvspass);
        while (!stream.atEnd()) 
        {
            QString line = stream.readLine();
            QStringList recs = QStringList::split(" ", line, false);
            repository->insertItem(recs[1]);
        }
        cvspass.close();
    }
}

void IntegratorDlg::init_clicked()
{
    KDialogBase dlg(KDialogBase::Plain, i18n("Init CVS Repository"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok);
    dlg.plainPage()->setMargin(0);
    (new QVBoxLayout(dlg.plainPage(), 0, 0))->setAutoAdd(true);
    InitDlg *initDlg = new InitDlg(dlg.plainPage());
    initDlg->show();
    
    initDlg->location->setFocus();
    initDlg->location->setMode(KFile::Directory);
    QRegExp localrep(":local:(.*)");
    if (localrep.search(repository->currentText()) != -1)
        initDlg->location->setURL(localrep.cap(1));
    
    if (dlg.exec() == QDialog::Accepted)
    {
        QString url = initDlg->location->url();
        KProcess *proc = new KProcess();
        *proc << "cvs";
        *proc << "-d" << url << "init";
        proc->start(KProcess::Block);
        if (!proc->normalExit())
            KMessageBox::error(this, i18n("cvs init did not exit normally. Please check if cvs is installed and works correctly."), i18n("Init CVS Repository"));
        else if (proc->exitStatus() != 0)
            KMessageBox::error(this, i18n("cvs init exited with status %1. Please check if the cvs location is correct.").arg(proc->exitStatus()), i18n("Init CVS Repository"));
        else
        {
            repository->insertItem(QString(":local:%1").arg(url));
            repository->setCurrentText(QString(":local:%1").arg(url));
        }
    }
}

void IntegratorDlg::login_clicked()
{
    Q3CString appId;
    QString error;

    if (KApplication::startServiceByDesktopName("cvsservice",
        QStringList(), &error, &appId))
    {
        QString msg = i18n("Unable to find the Cervisia KPart. \n"
            "Cervisia Integration will not be available. Please check your\n"
            "Cervisia installation and re-try. Reason was:\n") + error;
        KMessageBox::error(this, msg, "DCOP Error");
    }
    else
    {
        CvsService_stub *cvsService = new CvsService_stub(appId, "CvsService");
        cvsService->login(repository->currentText());
    }
}

void IntegratorDlg::accept()
{
    if (m_projectLocation.isEmpty())
        return;
    
    if (!createModule->isChecked())
        return;

    KProcess *proc = new KProcess();
    proc->setWorkingDirectory(m_projectLocation);
    *proc << "cvs";
    *proc << "-d" << repository->currentText() << "import"
        << "-m" << QString("\"%1\"").arg(comment->text()) << module->text()
        << vendorTag->text() << releaseTag->text();
    proc->start(KProcess::Block);
    if (!proc->normalExit())
        KMessageBox::error(this, i18n("cvs import did not exit normally. Please check if cvs is installed and works correctly."), i18n("Init CVS Repository"));
    else if (proc->exitStatus() != 0)
        KMessageBox::error(this, i18n("cvs import exited with status %1. Please check if the cvs location is correct.").arg(proc->exitStatus()), i18n("Init CVS Repository"));    
    else
    {
        kdDebug() << "Project is in: " << m_projectLocation << endl;
        
        KURL url = KURL::fromPathOrURL(m_projectLocation);
        QString up = url.upURL().path();
        kdDebug() << "Up is: " << up << endl;
        
        //delete sources in project dir
        KProcess *rmproc = new KProcess();
        *rmproc << "rm";
        *rmproc << "-f" << "-r" << m_projectLocation;
        rmproc->start(KProcess::Block);
        
        //checkout sources from cvs
        KProcess *coproc = new KProcess();
        coproc->setWorkingDirectory(up);
        *coproc << "cvs";
        *coproc << "-d" << repository->currentText() << "checkout" << module->text();
        coproc->start(KProcess::Block);
    }

/*    QCString appId;
    QString error;

    if (KApplication::startServiceByDesktopName("cvsservice",
        QStringList(), &error, &appId))
    {
        QString msg = i18n("Unable to find the Cervisia KPart. \n"
            "Cervisia Integration will not be available. Please check your\n"
            "Cervisia installation and re-try. Reason was:\n") + error;
        KMessageBox::error(this, msg, "DCOP Error");
    }
    else
    {
        kdDebug() << "!!!!! IMPORT" << endl;
        CvsService_stub *cvsService = new CvsService_stub(appId, "CvsService");
        cvsService->import(m_projectLocation, repository->currentText(), module->text(),
            "", comment->text(), vendorTag->text(), releaseTag->text(), false);
    }*/
}

void IntegratorDlg::createModule_clicked()
{
}

QWidget *IntegratorDlg::self()
{
    return const_cast<IntegratorDlg*>(this);
}

void IntegratorDlg::init(const QString &projectName, const QString &projectLocation)
{
    module->setText(projectName);
    m_projectLocation = projectLocation;
}

#include "integratordlg.moc"
