/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   Based on KHTML Factory from Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __qeditor_factory_h__
#define __qeditor_factory_h__

#include <kparts/factory.h>
#include <ktrader.h>
//Added by qt3to4:
#include <Q3PtrList>

class KInstance;
class KAboutData;

class QEditorPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    QEditorPartFactory( bool clone = false );
    virtual ~QEditorPartFactory();

    virtual KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args );

    static KInstance *instance();

    static const Q3PtrList<class QEditorPart>& documents();
    static void registerDocument ( class QEditorPart *doc );
    static void deregisterDocument ( class QEditorPart *doc );

    static const Q3PtrList<class QEditorView>& views();
    static void registerView ( class QEditorView *view );
    static void deregisterView ( class QEditorView *view );

    static KTrader::OfferList *plugins ();

private:
    static void ref();
    static void deref();

    static unsigned long s_refcnt;
    static QEditorPartFactory *s_self;

    static Q3PtrList<class QEditorPart> *s_documents;
    static Q3PtrList<class QEditorView> *s_views;

    static KInstance *s_instance;
    static KAboutData *s_about;

    static KTrader::OfferList *s_plugins;
};

#endif
