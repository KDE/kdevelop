/***************************************************************************
 *   Copyright (C) 1999 by Jonas Nordin                                    *
 *   jonas.nordin@syncom.se                                                *
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _JAVASUPPORTPART_H_
#define _JAVASUPPORTPART_H_


#include "kdevlanguagesupport.h"

class ParsedMethod;
class ParsedAttribute;
class ClassStore;
class Context;
class JavaClassParser;


class JavaSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    JavaSupportPart( bool cpp, KDevApi *api, QObject *parent=0, const char *name=0 );
    ~JavaSupportPart();

protected:
    virtual bool hasFeature(Features feature);
    virtual void newClass();
    virtual void addMethod(const QString &className);
    virtual void addAttribute(const QString &className);

private slots:
    void projectOpened();
    void projectClosed();
    void savedFile(const QString &fileName);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void addedFileToProject(const QString &fileName);
    void removedFileFromProject(const QString &fileName);

//    void slotGotoIncludeFile();
    // Internal
    void initialParse();

private:
    void maybeParse(const QString fileName);
    QString asJavaCode(ParsedMethod *pm);
    QString asJavaCode(ParsedAttribute *pa);

    JavaClassParser *m_parser;
    bool withjava;
    QString popupstr;
};

#endif
