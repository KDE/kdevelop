/***************************************************************************
 *   Copyright 1999-2001 by Bernd Gehrmann                                 *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GREPVIEWPART_H_
#define GREPVIEWPART_H_

#include <interfaces/iplugin.h>
#include <interfaces/contextmenuextension.h>

#include <QtCore/QVariant>

class KJob;
class GrepJob;

class GrepViewPlugin : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    GrepViewPlugin( QObject *parent, const QVariantList & = QVariantList() );
    ~GrepViewPlugin();

    void rememberSearchDirectory(QString const & directory);
    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    void showDialog(bool setLastUsed = false);
    
    /**
     * Returns a new instance of GrepJob. Since the plugin supports only one job at the same time,
     * previous job, if any, is killed before creating a new job.
     */
    GrepJob *grepJob();

private Q_SLOTS:
    void showDialogFromMenu();
    void showDialogFromProject();
    void jobFinished(KJob *job);

private:
    GrepJob *m_currentJob;
    QString m_directory;
    QString m_contextMenuDirectory;
};

#endif
