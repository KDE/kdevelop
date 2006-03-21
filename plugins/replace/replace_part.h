/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_REPLACE_H__
#define __KDEVPART_REPLACE_H__


#include <qpointer.h>
#include <kdevplugin.h>


class ReplaceWidget;


class ReplacePart : public KDevPlugin
{
    Q_OBJECT

public:
    ReplacePart(QObject *parent, const char *name, const QStringList &);
    ~ReplacePart();

public slots:
    void slotReplace();

private:
    QPointer<ReplaceWidget> m_widget;

};


#endif
