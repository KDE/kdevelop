/*
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>
   Copyright (C) 2001 by smeier@kdevelop.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _PHPSUPPORTPART_H_
#define _PHPSUPPORTPART_H_

#include <qdir.h>
#include <qfile.h>
#include <qprogressbar.h>

#include <kdialogbase.h>
#include "kdevlanguagesupport.h"
#include <kio/job.h>
#include <kdevgenericfactory.h>

#include <ktexteditor/editinterface.h>

class QStringList;
class PHPHTMLView;
class KShellProcess;
class KProcess;
class PHPErrorView;
class PHPConfigData;
class PHPCodeCompletion;
class PHPParser;
class PHPFile;

class PHPSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
   PHPSupportPart( QObject *parent, const char *name, const QStringList & );
   ~PHPSupportPart();

   PHPErrorView *ErrorView();
   PHPParser *Parser( ) ;

   QString getIncludePath();
   QString getExePath();

   void emitFileParsed( PHPFile *file );
   virtual void customEvent( QCustomEvent* ev );

protected:
   virtual Features features();
   virtual KMimeType::List mimeTypes();

private slots:
   void projectOpened();
   void projectClosed();
   void savedFile(const KURL &fileName);
   void addedFilesToProject(const QStringList &fileList);
   void removedFilesFromProject(const QStringList &fileList);
   void slotRun();
   void slotNewClass();
   void projectConfigWidget(KDialogBase *dlg);
   void slotReceivedPHPExeStderr (KProcess* proc, char* buffer, int buflen);
   void slotReceivedPHPExeStdout (KProcess* proc, char* buffer, int buflen);
   void slotPHPExeExited (KProcess* proc);
   void slotWebData(KIO::Job* job,const QByteArray& data);
   void slotWebResult(KIO::Job* job);
   void slotWebJobStarted(KIO::Job* job);

   // Internal
   void initialParse();
   void slotParseFiles();
   bool parseProject();

   void slotActivePartChanged(KParts::Part *part);
   void slotTextChanged();
   /* the configuration was written, mostly after a config dialog call
      -> reorder connects
    */
   void slotConfigStored();

private:
   QString getExecuteFile();
   void executeOnWebserver();
   void executeInTerminal();
   bool validateConfig();

   PHPHTMLView* m_htmlView;
   PHPErrorView* m_phpErrorView;
   KShellProcess* phpExeProc;
   QString m_phpExeOutput;
   PHPConfigData* configData;
   PHPCodeCompletion* m_codeCompletion;
   PHPParser* m_parser;
   KTextEditor::EditInterface *m_editInterface;

   struct JobData
   {
      QDir dir;
      QGuardedPtr<QProgressBar> progressBar;
      QStringList::Iterator it;
      QStringList files;
      QMap< QString, QPair<uint, uint> > pcs;
      QDataStream stream;
      QFile file;

      ~JobData()
      {
         delete progressBar;
      }
   };

   JobData * _jd;

   ClassDom LastClass;
   FunctionDom LastMethod;
   VariableDom LastVariable;
};

typedef KDevGenericFactory<PHPSupportPart> PHPSupportFactory;

#endif
