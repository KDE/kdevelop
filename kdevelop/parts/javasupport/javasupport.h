/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _JAVASUPPORT_H_
#define _JAVASUPPORT_H_


#include "kdevlanguagesupport.h"

class ParsedMethod;
class ParsedAttribute;
class ClassStore;
class JavaClassParser;
class KDevFileNode;
class ProjectSpace;
class KDevEditorManager;

class JavaSupport : public KDevLanguageSupport
{
    Q_OBJECT

public:
    JavaSupport( QObject *parent=0, const char *name=0 );
    ~JavaSupport();
		
		ClassStore* classStore();
		ProjectSpace* projectSpace();
		KDevEditorManager* editorManager();

public slots:	
	void slotProjectSpaceOpened();

protected:
    virtual bool hasFeature(Features feature);
    virtual void newClassRequested();
    virtual void addMethodRequested(const QString &className);
    virtual void addAttributeRequested(const QString &className);
    virtual QStringList fileFilters();

private slots:
    void initialParse();
    void addedFileToProject(KDevFileNode* pNode);
    void removedFileFromProject(KDevFileNode* pNode);
    void savedFile(const QString &fileName);

private:
    QString asJavaCode(ParsedMethod *pm);
    QString asJavaCode(ParsedAttribute *pm);

    JavaClassParser *m_parser;
};

#endif
