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

#ifndef _BUFFERACTION_H_
#define _BUFFERACTION_H_

#include <kaction.h>
#include <kurl.h>

class TextEditorDocument;


class BufferAction : public KAction
{
    Q_OBJECT

public:
    BufferAction(TextEditorDocument *doc, QObject *parent=0, const char *name=0);
    BufferAction(const KURL &url, QObject *parent=0, const char *name=0);

signals:
    void activated(TextEditorDocument *doc);
    void activated(const KURL &url);

private:
    virtual void slotActivated();

    TextEditorDocument *m_doc;
    KURL m_url;
};

#endif
