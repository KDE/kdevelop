/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef __KDEVPART_PASCALSUPPORT_H__
#define __KDEVPART_PASCALSUPPORT_H__

#include <qguardedptr.h>
#include "kdevlanguagesupport.h"

class PascalSupportWidget;
class KDialogBase;
class QPopupMenu;
class Context;
class PascalSupportPartData;

class PascalSupportPart : public KDevLanguageSupport
{
    Q_OBJECT
public:
    PascalSupportPart(QObject *parent, const char *name, const QStringList &);
    ~PascalSupportPart();

    virtual Features features();
    virtual KMimeType::List mimeTypes();

    virtual QString formatTag( const Tag& tag );
    virtual QString formatModelItem( const CodeModelItem *item, bool shortDescription=false );
    
private slots:
    void projectOpened();
    void projectClosed();
    void savedFile(const KURL &fileName);
    void configWidget(KDialogBase *dlg);
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(QPopupMenu *popup, const Context *context);

    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void slotProjectCompiled();

    void initialParse();

private:
    void maybeParse(const QString &fileName);
    void parse(const QString &fileName);

    QGuardedPtr<PascalSupportWidget> m_widget;

    bool m_projectClosed;
    QStringList m_projectFileList;

    PascalSupportPartData* d;

};

#endif
