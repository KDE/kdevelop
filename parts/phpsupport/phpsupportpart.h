/***************************************************************************
 *   Copyright (C) 2001 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PHPSUPPORTPART_H_
#define _PHPSUPPORTPART_H_

#include <kdialogbase.h>
#include "kdevlanguagesupport.h"
#include <kio/job.h>
#include <kgenericfactory.h>

#include <ktexteditor/editinterface.h>

class PHPHTMLView;
class KShellProcess;
class KProcess;
class PHPErrorView;
class PHPConfigData;
class PHPCodeCompletion;
class PHPParser;

class PHPSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:    PHPSupportPart( QObject *parent, const char *name, const QStringList & );
    ~PHPSupportPart();

protected:
    virtual KDevLanguageSupport::Features features();

private slots:
    void projectOpened();
    void projectClosed();
    void savedFile(const QString &fileName);
    void addedFileToProject(const QString &fileName);
    void removedFileFromProject(const QString &fileName);
    void slotRun();
    void slotNewClass();
    void projectConfigWidget(KDialogBase *dlg);
    void slotReceivedPHPExeStderr (KProcess* proc, char* buffer, int buflen);
    void slotReceivedPHPExeStdout (KProcess* proc, char* buffer, int buflen);
    void slotPHPExeExited (KProcess* proc);
    void slotErrorMessageSelected(const QString&filename,int line);
    void slotWebData(KIO::Job* job,const QByteArray& data);
    void slotWebResult(KIO::Job* job);
    void slotWebJobStarted(KIO::Job* job);

    // Internal
    void initialParse();
    void slotActivePartChanged(KParts::Part *part);
    void slotTextChanged();

private:
    void maybeParse(const QString fileName);
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
};

typedef KGenericFactory<PHPSupportPart> PHPSupportFactory;

#endif
