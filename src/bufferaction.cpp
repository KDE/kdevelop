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

#include "bufferaction.h"
#include "texteditor.h"


BufferAction::BufferAction(TextEditorDocument *doc, QObject *parent=0, const char *name=0)
    : KAction(doc->fileName(), 0, 0, 0), m_doc(doc)
{}


BufferAction::BufferAction(const KURL &url, QObject *parent=0, const char *name=0)
    : KAction(url.url(), 0, 0, 0), m_doc(0), m_url(url)
{}


void BufferAction::slotActivated()
{
    if (m_doc)
        emit activated(m_doc);
    else
        emit activated(m_url);
}

#include "bufferaction.moc"
