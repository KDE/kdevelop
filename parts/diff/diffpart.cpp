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

#include <qmenu.h>

#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kparts/part.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevdocumentcontroller.h"
#include "kdevplugininfo.h"

#include "diffpart.h"
#include "diffdlg.h"
#include "diffwidget.h"

static const KDevPluginInfo data("kdevdiff");

typedef KDevGenericFactory<DiffPart> DiffFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdiff, DiffFactory( data ) )

DiffPart::DiffPart(QObject *parent, const char *name, const QStringList &)
    : KDevDiffFrontend(&data, parent), proc(0)
{
  setObjectName(name ? name : "DiffPart");
  setInstance(DiffFactory::instance());
  setXMLFile("kdevdiff.rc");

  diffWidget = new DiffWidget();
  diffWidget->setIcon( SmallIcon("editcopy") );
  QString nm( i18n( "Diff" ) );
  diffWidget->setCaption( i18n( "Diff Output" ) );
  diffWidget->setWhatsThis( i18n("<b>Difference viewer</b><p>Shows output of the diff format. "
    "Can utilize every installed component that is able to show diff output. "
    "For example if you have Kompare installed, Difference Viewer can use its graphical diff view.") );
  mainWindow()->embedOutputView( diffWidget, nm, i18n("Output of the diff command") );

  KAction *action = new KAction( i18n("Difference Viewer..."), 0,
           this, SLOT(slotExecDiff()),
           actionCollection(), "tools_diff" );
  action->setToolTip(i18n("Difference viewer"));
  action->setWhatsThis(i18n("<b>Difference viewer</b><p>Shows the contents of a patch file."));

  connect( core(), SIGNAL(contextMenu(QMenu *, const Context *)),
           this, SLOT(contextMenu(QMenu *, const Context *)) );
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

static KParts::ReadWritePart* partForURL(const KURL &url, KDevDocumentController* pc)
{
  if ( !pc )
    return 0;
  Q3PtrListIterator<KParts::Part> it(*(pc->parts()));
  for ( ; it.current(); ++it)
  {
    KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(it.current());
// TODO    if ( rw_part && dynamic_cast<KTextEditor::EditInterface*>(it.current()) && urlIsEqual(url, rw_part->url()) )
// TODO      return rw_part;
  }

  return 0;
}

void DiffPart::contextMenu( QMenu* popup, const Context* context )
{
    if ( context->hasType( Context::EditorContext ) )
    {
        const EditorContext *eContext = static_cast<const EditorContext*>(context);
        popupFile = eContext->url();
    }
    else if ( context->hasType( Context::FileContext ) )
    {
        const FileContext * fContext = static_cast<const FileContext*>( context );
        popupFile.setPath( fContext->urls().first().fileName() );   //@fixme - assuming absolute path. is this correct?
    }
    else
    {
        return;
    }

    KParts::ReadWritePart* rw_part = partForURL( popupFile, documentController() );
    if ( !rw_part ) return;

    if ( documentController()->documentState( rw_part->url() ) != Clean )
    {
        int id = popup->insertItem( i18n( "Difference to Disk File" ),
                            this, SLOT(localDiff()) );
        popup->setWhatsThis(id, i18n("<b>Difference to disk file</b><p>Shows the difference between "
            "the file contents in this editor and the file contents on disk."));
    }
}

DiffPart::~DiffPart()
{
  if ( diffWidget )
    mainWindow()->removeView( diffWidget );

  delete proc;
  delete (DiffWidget*) diffWidget;
}

void DiffPart::localDiff()
{
#if 0
  KParts::ReadWritePart* rw_part = partForURL( popupFile, documentController() );
  if ( !rw_part )
    return;

 KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>(rw_part);
 if ( !editIface )
    return;
  buffer = editIface->text().local8Bit();
  resultBuffer = resultErr = QString();

  delete proc;
  proc = new KProcess();

  *proc << "diff";
  *proc << "-u" << popupFile.path() << "-";
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
    KMessageBox::error( 0, i18n( "Could not invoke the \"diff\" command." ) );
    delete proc;
    proc = 0;
    return;
  }
  proc->writeStdin( buffer.data(), buffer.length() );
#endif // harryF: TODO
}

void DiffPart::processExited( KProcess* p )
{
  // diff has exit status 0 and 1 for success
  if ( p->normalExit() && ( p->exitStatus() == 0 || p->exitStatus() == 1 ) ) {
    if ( resultBuffer.isEmpty() )
      KMessageBox::information( 0, i18n("DiffPart: No differences found.") );
    else
      showDiff( resultBuffer );
  } else {
    KMessageBox::error( 0, i18n("Diff command failed (%1):\n").arg( p->exitStatus() ) + resultErr );
  }
  resultBuffer = resultErr = QString();
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
  buffer = 0L;
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
  KURL url = KFileDialog::getOpenURL( QString(), QString(), 0, i18n("Please Select Patch File") );

  if ( url.isEmpty() )
    return;

  openURL( url );
}

#include "diffpart.moc"
