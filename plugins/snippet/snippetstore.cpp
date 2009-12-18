/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetstore.h"


#include "snippetrepository.h"
#include "snippetplugin.h"

#include <KMessageBox>
#include <KLocalizedString>
#include <QApplication>
#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <KDebug>

SnippetStore* SnippetStore::self_ = 0;


SnippetStore::SnippetStore(SnippetPlugin* plugin)
    : plugin_(plugin)
{
    self_ = this;
    load();
}

SnippetStore::~SnippetStore()
{
    invisibleRootItem()->removeRows( 0, invisibleRootItem()->rowCount() );
    repos_.clear();
    self_ = 0;
}

void SnippetStore::init(SnippetPlugin* plugin)
{
    Q_ASSERT(!SnippetStore::self());
    new SnippetStore(plugin);
}

SnippetStore* SnippetStore::self()
{
    return self_;
}

void SnippetStore::createNewRepository(SnippetRepository* parent, const QString& name, const QString& dir)
{
    {
        QDir location(dir);
        if ( dir.isEmpty() || (!location.exists() && !location.mkpath(dir)) ) {
            KMessageBox::error(
                QApplication::activeWindow(),
                i18n("Could not create the repository folder \"%1\".", dir)
            );
            return;
        }
    }

    if (!parent) {
        // Check if the directory is not already in the SnippetStore
        // We allow each toplevel repository only once
        foreach(SnippetRepository* repo, repos_) {
            if (repo->getLocation() == QDir::cleanPath(dir) ) {
                return;
            }
        }
    }

    //Add the given directory to the SnippetStore
    SnippetRepository *item = new SnippetRepository(name, dir);
    if (parent) {
        parent->addSubRepo( item );
    } else {
        repos_.append( item );
        appendRow( item );
        KConfigGroup config = getConfig().group("repos");
        config.writeEntry(item->text(), item->getLocation());
        config.sync();
    }
}

Qt::ItemFlags SnippetStore::flags(const QModelIndex & index) const
{
    Q_UNUSED(index)

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return flags;
}

KConfigGroup SnippetStore::getConfig()
{
    return plugin_->core()->activeSession()->config()->group("Snippets");
}

void SnippetStore::remove(SnippetRepository* repo)
{
    kDebug() << "removing repo" << repo->text() << repo->getLocation();
    int idx = repos_.indexOf( repo );

    if (idx >= 0) {
        KConfigGroup config = getConfig().group("repos");
        kDebug() << config.keyList();
        config.deleteEntry(repo->text());
        config.sync();
        kDebug() << "should be removed";
        repos_.removeAt( idx );
        // Only remove the given item if it's really a tolevel repo
        invisibleRootItem()->removeRows( repo->row(), 1 );
    }
}

void SnippetStore::load()
{
    KConfigGroup config = getConfig();
    if (config.hasGroup("repos")) {
        KConfigGroup repoGroup = config.group("repos");
        QStringList keys = repoGroup.keyList();
        foreach(QString key, keys) {
            createNewRepository(0, key, repoGroup.readEntry(key));
        }
    }
}

#include "snippetstore.moc"
