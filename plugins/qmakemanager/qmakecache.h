/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QMAKECACHE_H
#define QMAKECACHE_H

#include "qmakefile.h"

class QMakeMkSpecs;

class QMakeCache : public QMakeFile
{
public:
    explicit QMakeCache( const QString& cachefile );
    void setMkSpecs( QMakeMkSpecs* specs );
    bool read() override;
private:
    QMakeMkSpecs* m_mkspecs;
};

#endif

