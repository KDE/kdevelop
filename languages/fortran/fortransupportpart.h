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

#ifndef _FORTRANSUPPORTPART_H_
#define _FORTRANSUPPORTPART_H_

class FixedFormParser;
class KDialogBase;
class QStringList;
class KDevMakeFrontend;

#include "kdevlanguagesupport.h"


class FortranSupportPart : public KDevLanguageSupport
{
    Q_OBJECT

public:
    FortranSupportPart( QObject *parent, const char *name, const QStringList & );
    ~FortranSupportPart();

protected:
    virtual KDevLanguageSupport::Features features();

private slots:
    void slotFtnchek();
    void projectConfigWidget(KDialogBase *dlg);
    void projectOpened();
    void projectClosed();
    void savedFile(const KURL &fileName);
    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);

    // Internal
    void initialParse();

private:
    void maybeParse(const QString fileName);
    KDevMakeFrontend *makeFrontend();

    FixedFormParser *parser;
};

#endif
