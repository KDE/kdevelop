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

class CParsedMethod;
class CParsedAttribute;
class CClassStore;
class CClassParser;


class CppSupport : public KDevLanguageSupport
{
    Q_OBJECT

public:
    CppSupport( QObject *parent=0, const char *name=0 );
    ~CppSupport();

protected:
    virtual void projectOpened(CProject *prj);
    virtual void projectClosed();
    virtual void classStoreOpened(CClassStore *store);
    virtual void classStoreClosed();
    virtual void addedFileToProject(const QString &fileName);
    virtual void removedFileFromProject(const QString &fileName);
    virtual void savedFile(const QString &fileName);

    virtual bool hasFeature(Features feature);
    virtual void newClassRequested();
    virtual void addMethodRequested(const QString &className);
    virtual void addAttributeRequested(const QString &className);

private:
    QString asHeaderCode(CParsedMethod *pm);
    QString asCppCode(CParsedMethod *pm);
    QString asHeaderCode(CParsedAttribute *pa);

    CClassStore *m_store;
    CClassParser *m_parser;
};

#endif
