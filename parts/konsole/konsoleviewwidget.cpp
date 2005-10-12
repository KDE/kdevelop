/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   kdevelop-devel@kde.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlabel.h>
#include <q3frame.h>
#include <qdir.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "konsoleviewpart.h"
#include "kdevpartcontroller.h"

#include "konsoleviewwidget.h"

#include <QtCore/qdebug.h>

KonsoleViewWidget::KonsoleViewWidget(KonsoleViewPart *part)
 : QWidget(0), part(0), owner( part )
{
    setObjectName(QString::fromUtf8("konsole widget"));
    connect(part->partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(activePartChanged(KParts::Part*)));
    vbox = new QVBoxLayout(this);
}

KonsoleViewWidget::~KonsoleViewWidget()
{
}

void KonsoleViewWidget::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  activate();
}

void KonsoleViewWidget::activate()
{
    kdDebug(9035) << k_funcinfo << endl;

    static bool initialized = false;

    if (initialized)
        return;

    Q_ASSERT(part == 0);
    initialized = true;

    if (KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart"))
      {
        if (0 != (part = qobject_cast<KParts::ReadOnlyPart*>(factory->create(this))))
          {
            part->widget()->setFocusPolicy(Qt::WheelFocus);
            setFocusProxy(part->widget());
            part->widget()->setFocus();

            if (Q3Frame *frame = qobject_cast<Q3Frame*>(part->widget()))
              frame->setFrameStyle(Q3Frame::Panel|Q3Frame::Sunken);

            vbox->addWidget(part->widget());
            part->widget()->show();

            connect(part, SIGNAL(destroyed()), this, SLOT(partDestroyed()));
          }
      }
    else
      {
        vbox->addWidget(new QLabel(i18n("Part not available"), this));
      }
}


void KonsoleViewWidget::activePartChanged(KParts::Part *activatedPart)
{
    kdDebug(9035) << k_funcinfo << endl;
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(activatedPart);

    if (ro_part && !ro_part->url().isLocalFile())
    {
        kdDebug(9035) << k_funcinfo << "part is null or not local" << endl;
        return;
    }
    QString dir;
    if (ro_part)
        dir = ro_part->url().directory();
    else if (owner->project())
        dir = owner->project()->projectDirectory();

    kdDebug(9035) << k_funcinfo "Changing dir to " << dir << endl;
    if (dir.isEmpty())
        return;
    setDirectory( KURL(dir) );
}

void KonsoleViewWidget::setDirectory(const KURL &dirUrl)
{
    kdDebug(9035) << k_funcinfo << "part is " << (long)part << endl;

    if (part && dirUrl != part->url())
    {
        kdDebug(9035) << k_funcinfo << "Changing dirUrl.path() == " << dirUrl.path() << endl;
        kdDebug(9035) << k_funcinfo << "Changing part->url.path() == " << part->url().path() << endl;

        part->openURL( dirUrl );
    }
}

void KonsoleViewWidget::partDestroyed()
{
    part = 0;
    activate();
}


#include "konsoleviewwidget.moc"
