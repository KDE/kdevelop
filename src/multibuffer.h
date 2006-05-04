/*
 * KDevelop Multiple Buffer Support
 *
 * Copyright (c) 2005 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef MULTIBUFFER_H
#define MULTIBUFFER_H

#include <qpair.h>
#include <qsplitter.h>
#include <qguardedptr.h>

namespace KParts
{
class Part;
class Factory;
class ReadOnlyPart;
}

class KURL;

typedef QMap< KURL, KParts::Part* > BufferMap;
typedef QMap< KParts::Part*, QPair<int, int> > ActivationMap;

class MultiBuffer : public QSplitter
{
    Q_OBJECT
public:
    MultiBuffer( QWidget * parent = 0 );
    virtual ~MultiBuffer();

    KParts::Part *activeBuffer() const;
    bool hasURL( const KURL &url ) const;
    int numberOfBuffers() const;
    bool isActivated() const;
    void setDelayedActivation( bool delayed );

    KParts::Part* createPart( const QString &mimeType,
                              const QString &partType,
                              const QString &className,
                              const QString &preferredName = QString::null );

    KParts::Part* openURL( const KURL &url );
    bool closeURL( const KURL &url );

    void registerURL( const KURL &url, KParts::Part *part );
    void registerDelayedActivation( KParts::Part *part, int line, int col );

public slots:
    virtual void show();
    virtual void setOrientation( Qt::Orientation );
    void activePartChanged( const KURL &url );

protected:
    virtual void focusInEvent( QFocusEvent *ev );

private:
    KParts::Part* createPart( const KURL &url );

private:
    BufferMap m_buffers;
    QGuardedPtr<KParts::Factory> m_editorFactory;
    ActivationMap m_delayedActivation;
    bool m_delayActivate;
    bool m_activated;
    QGuardedPtr<KParts::Part> m_activeBuffer;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
