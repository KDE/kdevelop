/***************************************************************************
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVVCSCOMMONPLUGIN_H
#define KDEVVCSCOMMONPLUGIN_H

#include <interfaces/iplugin.h>
#include <kurl.h>
#include <QtCore/QVariant>
#include <QtCore/QHash>

class KJob;

namespace KDevelop
{
class Context;
class VcsCommitDialog;
class ProjectBaseItem;
class ContextMenuExtension;
}

class KAction;

class KDevVcsCommonPlugin: public KDevelop::IPlugin
{
    Q_OBJECT
public:
    KDevVcsCommonPlugin( QObject *parent, const QVariantList & = QVariantList() );
    virtual ~KDevVcsCommonPlugin();

    KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* );
private slots:
    void commit();
    void add();
    void remove();
    void revert();
    void history();
    void annotation();
    void diffToHead();
    void diffToBase();
    void update();
    void executeCommit( KDevelop::VcsCommitDialog* dlg );
    void cancelCommit( KDevelop::VcsCommitDialog* dlg );
    void diffJobFinished( KJob* job );
private:
    KDevelop::IPlugin* findVcsPluginForProjectItem( KDevelop::ProjectBaseItem* item ) const;
    KUrl urlForItem( KDevelop::ProjectBaseItem* item ) const;
    KDevelop::IPlugin* findVcsPluginForNonProjectUrl( const KUrl& url ) const;
    KDevelop::IPlugin* findVcsPluginForUrl( const KUrl& url ) const;
    QHash<IPlugin*, KUrl::List> m_ctxUrls;
    QHash<QString, KAction*> m_actions;
};

#endif

