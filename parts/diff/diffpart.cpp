/***************************************************************************
 *   Copyright (C) 2001 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sys/stat.h>

#include <qpopupmenu.h>

#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <kdebug.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"

#include "diffpart.h"
#include "diffdlg.h"
#include "diffwidget.h"

typedef KGenericFactory<DiffPart> DiffFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdiff, DiffFactory( "kdevdiff" ) );

DiffPart::DiffPart(QObject *parent, const char *name, const QStringList &)
    : KDevDiffFrontend("Diff", "diff", parent, name ? name : "DiffPart"), proc(0)
{
  setInstance(DiffFactory::instance());
  setXMLFile("kdevdiff.rc");

  diffWidget = new DiffWidget();
  QString nm( i18n( "Difference Viewer" ) );
  diffWidget->setCaption( nm );
  mainWindow()->embedOutputView( diffWidget, nm, i18n("output of the diff command") );

  core()->insertNewAction( new KAction( i18n("Difference Viewer..."), 0,
					this, SLOT(slotExecDiff()),
					actionCollection(), "tools_diff" ) );

  connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
           this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
}

static bool urlIsEqual(const KURL &a, const KURL &b)
{
  if (a.isLocalFile() && b.isLocalFile())
  {
    struct stat aStat, bStat;

    if ((::stat(QFile::encodeName(a.fileName()), &aStat) == 0)
        && (::stat(QFile::encodeName(b.fileName()), &bStat) == 0))
    {
      return (aStat.st_dev == bStat.st_dev) && (aStat.st_ino == bStat.st_ino);
    }
  }

  return a == b;
}

static KParts::ReadWritePart* partForURL(const KURL &url, KDevPartController* pc)
{
  if ( !pc )
    return 0;
  QPtrListIterator<KParts::Part> it(*(pc->parts()));
  for ( ; it.current(); ++it)
  {
    KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(it.current());
    if ( rw_part && dynamic_cast<KTextEditor::EditInterface*>(it.current()) && urlIsEqual(url, rw_part->url()) )
      return rw_part;
  }

  return 0;
}

void DiffPart::contextMenu( QPopupMenu* popup, const Context* context )
{
  if (!context->hasType("editor"))
    return;

  const EditorContext *eContext = static_cast<const EditorContext*>(context);
  popupFile = eContext->url();

  if ( !popupFile.isLocalFile() )
    return;

  KParts::ReadWritePart* rw_part = partForURL( popupFile, partController() );
  if ( !rw_part || !rw_part->isModified() )
    return;

  popup->insertItem( i18n( "Difference to saved file" ), 
                     this, SLOT(localDiff()) );
}

DiffPart::~DiffPart()
{
  delete proc;
  delete (DiffWidget*) diffWidget;
}

void DiffPart::localDiff()
{
  KParts::ReadWritePart* rw_part = partForURL( popupFile, partController() );
  if ( !rw_part )
    return;

  KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>(rw_part);
  if ( !editIface )
    return;
  buffer = editIface->text().local8Bit();
  resultBuffer = resultErr = QString::null;

  delete proc;
  proc = new KProcess();

  *proc << "diff";
  *proc << "-u3" << popupFile.path() << "-";
  proc->setWorkingDirectory( popupFile.directory() );

  connect( proc, SIGNAL(processExited( KProcess* )),
           this, SLOT(processExited( KProcess* )) );
  connect( proc, SIGNAL(receivedStdout( KProcess*, char*, int )),
           this, SLOT(receivedStdout( KProcess*, char*, int )) );
  connect( proc, SIGNAL(receivedStderr( KProcess*, char*, int )),
           this, SLOT(receivedStderr( KProcess*, char*, int )) );
  connect( proc, SIGNAL(wroteStdin( KProcess* )),
           this, SLOT(wroteStdin( KProcess* )) );

  if ( !proc->start( KProcess::NotifyOnExit, KProcess::All ) ) {
    showMessage( i18n( "Could not invoke the \"diff\" command." ) );
    delete proc;
    proc = 0;
    return;
  }
  proc->writeStdin( buffer.data(), buffer.length() );
}

void DiffPart::processExited( KProcess* p )
{
  // diff has exit status 0 and 1 for success
  if ( p->normalExit() && ( p->exitStatus() == 0 || p->exitStatus() == 1 ) ) {
    if ( resultBuffer.isEmpty() )
      showMessage( i18n("No differences found.") );
    else
      showDiff( resultBuffer );
  } else {
    showMessage( i18n("Diff command failed (%1):\n").arg( p->exitStatus() ) + resultErr );
  }
  resultBuffer = resultErr = QString::null;
  delete proc;
  proc = 0;
}

void DiffPart::receivedStdout( KProcess* /* p */, char* buf, int buflen )
{
  resultBuffer += QString::fromLocal8Bit( buf, buflen );
}

void DiffPart::receivedStderr( KProcess* /* p */, char* buf, int buflen )
{
  kdDebug(9033) << "received Stderr: " << QString::fromLocal8Bit( buf, buflen ) << endl;
  resultErr += QString::fromLocal8Bit( buf, buflen );
}

void DiffPart::wroteStdin( KProcess* p )
{
  buffer = QString::null;
  p->closeStdin();
}

void DiffPart::openURL( const KURL& url )
{
  diffWidget->slotClear();
  diffWidget->openURL( url );
  mainWindow()->raiseView( diffWidget );
/*
  DiffDlg* diffDlg = new DiffDlg( 0, "diffDlg" );

  diffDlg->openURL( url );
  diffDlg->exec();
  delete diffDlg;
*/
}

void DiffPart::showMessage( const QString& message )
{
  diffWidget->showMessage( message );
  mainWindow()->raiseView( diffWidget );
}

void DiffPart::showDiff( const QString& diff )
{
  diffWidget->slotClear();
  diffWidget->setDiff( diff );
  mainWindow()->raiseView( diffWidget );
/*
  DiffDlg* diffDlg = new DiffDlg( 0, "diffDlg" );

  diffDlg->setDiff( diff );
  diffDlg->exec();
  delete diffDlg; 
*/
}

void DiffPart::slotExecDiff()
{
  KURL url = KFileDialog::getOpenURL( QString::null, QString::null, 0, i18n("Please Select Patch File") );

  if ( url.isEmpty() )
    return;

  openURL( url );
}

#include "diffpart.moc"
