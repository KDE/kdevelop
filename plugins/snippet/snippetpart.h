/***************************************************************************
 *   Copyright 2007 Rober Gruber <rgruber@users.sourceforge.net>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SNIPPETPART_H__
#define __SNIPPETPART_H__

#include <iplugin.h>
#include <QtCore/QVariant>

/**
 * This is the main class of KDevelop's snippet plugin.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class SnippetPart : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    SnippetPart(QObject *parent, const QVariantList &args = QVariantList() );
    virtual ~SnippetPart();

    /**
     * Inserts the given @p snippet into the currently active view.
     * If the current active view is not inherited from KTextEditor::View
     * nothing will happen.
     */
    void insertText(const QString& snippet);

    // KDevelop::IPlugin methods
    virtual void unload();

private:
    class SnippetViewFactory *m_factory;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; indent-mode cstyle; replace-tabs on; auto-insert-doxygen on;
