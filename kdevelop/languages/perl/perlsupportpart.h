/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PERLSUPPORTPART_H_
#define _PERLSUPPORTPART_H_

#include "kdevlanguagesupport.h"
#include <codemodel.h>
#include "perlparser.h"

class PerlSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    PerlSupportPart( QObject *parent, const char *name, const QStringList & );
    ~PerlSupportPart();

protected:
    virtual Features features();
    virtual KMimeType::List mimeTypes();

private slots:
    void projectOpened();
    void projectClosed();
    void savedFile(const KURL &fileName);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void slotExecute();
    void slotExecuteString();
    void slotStartInterpreter();

    // Internal
    void initialParse();
    void slotPerldocFunction();
    void slotPerldocFAQ();

private:
    QString interpreter();
    void startApplication(const QString &program);
    void maybeParse(const QString fileName);
    void parse(const QString &fileName);
    void parseLines(QStringList* lines,const QString& fileName);
    void removeWithReference( const QString & fileName );
    void parseUseFiles();
    //Perl Parser
    perlparser* m_parser;

};

#endif
