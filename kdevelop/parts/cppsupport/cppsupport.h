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

#ifndef _CPPSUPPORT_H_
#define _CPPSUPPORT_H_


#include "kdevlanguagesupport.h"

class ParsedMethod;
class ParsedAttribute;
class ClassStore;
class CClassParser;
class KDevFileNode;
class ProjectSpace;
class KDevEditorManager;
class ClassStore;

class CppSupport : public KDevLanguageSupport
{
    Q_OBJECT

public:
    CppSupport( QObject *parent=0, const char *name=0 );
    ~CppSupport();
		
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
    QString asHeaderCode(ParsedMethod *pm);
    QString asCppCode(ParsedMethod *pm);
    QString asHeaderCode(ParsedAttribute *pa);

    CClassParser *m_parser;
};

#endif
