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
    void savedFile(const QString &fileName);
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
/*
    bool    m_inpackage;
    bool    m_inscript;
    bool    m_inclass;
    
    QString m_lastsub;
    QString m_lastparentclass;
    QString m_lastattr;
    QString m_lastscript;
    QString m_lastpackagename;

    NamespaceDom m_lastpackage;
    ClassDom m_lastclass;
*/
    //Perl Parser
    perlparser* m_parser;
    
    //CodeModel
/*
    CodeModel* m_model;
    FileDom m_file;
    
    //this willhav a list of INC paths
    QStringList m_INClist;
    //this will get a list off all files "use" in the perl files
    //and need additional parsing to include the classes in the classview
    QStringList m_usefiles;
*/
};

#endif
