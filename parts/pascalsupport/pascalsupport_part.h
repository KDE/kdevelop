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

class PascalSupportPart : public KDevLanguageSupport
{
    Q_OBJECT
public:
    PascalSupportPart(QObject *parent, const char *name, const QStringList &);
    ~PascalSupportPart();

    virtual Features features();
    virtual QStringList fileFilters();

private slots:
    void projectOpened();
    void projectClosed();
    void savedFile(const QString &fileName);
    void configWidget(KDialogBase *dlg);
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(QPopupMenu *popup, const Context *context);

    void addedFilesToProject(const QStringList &fileList);
    void removedFilesFromProject(const QStringList &fileList);
    void slotProjectCompiled();

private:
    QGuardedPtr<PascalSupportWidget> m_widget;

    bool m_projectClosed;
    QStringList m_projectFileList;

};

#endif
