/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PYTHONSUPPORTPART_H_
#define _PYTHONSUPPORTPART_H_


#include "kdevlanguagesupport.h"


class PythonSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    PythonSupportPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~PythonSupportPart();

protected:
    virtual KDevLanguageSupport::Features features();

private slots:
    void projectOpened();
    void projectClosed();
    void savedFile(const QString &fileName);
    void addedFileToProject(const QString &fileName);
    void removedFileFromProject(const QString &fileName);

    // Internal
    void initialParse();

private:
    void maybeParse(const QString fileName);
    void parse(const QString &fileName);
};

#endif
