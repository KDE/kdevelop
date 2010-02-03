/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *   Copyright 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "snippetstore.h"

#include "snippetplugin.h"
#include "snippetrepository.h"

#include <interfaces/icore.h>
#include <interfaces/isession.h>

#include <KStandardDirs>
#include <KDebug>

SnippetStore* SnippetStore::self_ = 0;

SnippetStore::SnippetStore(SnippetPlugin* plugin)
    : plugin_(plugin)
{
    self_ = this;

    const QStringList list = KGlobal::dirs()->findAllResources("data",
        "kate/plugins/katesnippets_tng/data/*.xml", KStandardDirs::NoDuplicates)
                        << KGlobal::dirs()->findAllResources("data",
        "kate/plugins/katesnippets_tng/data/ghns/*.xml", KStandardDirs::NoDuplicates);

    foreach(const QString& file, list ) {
        SnippetRepository* repo = new SnippetRepository(file);
        appendRow(repo);
    }
}

SnippetStore::~SnippetStore()
{
    invisibleRootItem()->removeRows( 0, invisibleRootItem()->rowCount() );
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

#include "snippetstore.moc"
