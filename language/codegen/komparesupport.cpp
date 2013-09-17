/*
 * Copyright 2008 Ramón Zarazúa <killerfox512+kde@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "komparesupport.h"
#include <config-kdevplatform.h>

#ifdef HAVE_KOMPARE

#include <stddef.h>
#include <cstddef>

#include <QWidget>
#include <QList>
#include <kompare/kompareinterface.h>
#include <language/duchain/indexedstring.h>
#include <interfaces/icore.h>
#include <interfaces/ipartcontroller.h>
#include <KParts/Factory>
#include <KService>
#include <KMimeTypeTrader>
#include <KDebug>
#include <KParts/Part>
#include "coderepresentation.h"
#include <QBitArray>


namespace KDevelop
{

struct KompareWidgetsPrivate
{
    KompareWidgetsPrivate();
    KPluginFactory * factory;
    QList<KParts::Part *> m_parts;
    QBitArray m_usedWidgets;
    
    bool createWidget(int index, QWidget * widget);
    inline bool partExists(int index);
};

KompareWidgetsPrivate::KompareWidgetsPrivate()
{
    factory = ICore::self()->partController()->findPartFactory(  "text/x-patch",
                                                                  "Kompare/ViewPart",
                                                                  QString("") );
}

inline bool KompareWidgetsPrivate::partExists(int index)
{
    return m_parts.size() > index && m_parts[index];
}

KompareWidgets::KompareWidgets() : d(new KompareWidgetsPrivate)
{
}

KompareWidgets::~KompareWidgets()
{
    delete d;
}

bool KompareWidgetsPrivate::createWidget(int index, QWidget * widget)
{
    KParts::Part * part = factory->create<KomparePart>(widget, 0);
    if(!part)
    {
        kWarning() << "Kompare factory did not return a part";
        return false;
    }
    KompareInterface * ipart = qobject_cast<KompareInterface *>(part);
    
    if(ipart)
    {
        int newIndex = index == -1 ? m_parts.size() : index;
        m_parts.insert(newIndex, part);
        
        if(m_usedWidgets.size() <= index)
            m_usedWidgets.resize(index + 1);
        m_usedWidgets[index] = true;
        
        return true;
    }
    else
    {
        part->deleteLater();
        kWarning() << "KompareInterface cast failed";
        return false;
    }
}

bool KompareWidgets::compare(const IndexedString & original, const QString & modified, QWidget * widget, int index)
{
    //If there is no current part created, Create it
    if( !d->partExists(index) &&
        !d->createWidget(index, widget))
        return false;
    
    //Prepare the part
    KParts::Part * part = d->m_parts[index];
    KompareInterface * ipart = qobject_cast<KompareInterface * >(part);
    Q_ASSERT(part);
    
    part->widget()->setVisible(true);
    part->widget()->resize(part->widget()->parentWidget()->size());
    part->widget()->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    
    //Compare
    ipart->compareFileString(original.toUrl(), modified);
    
    //Set to used
    d->m_usedWidgets[index] = true;
    return true;
}

bool KompareWidgets::widgetActive(int index)
{
    return d->m_usedWidgets.size() > index ? d->m_usedWidgets[index] : false;
}

void KompareWidgets::hideWidget(int index)
{
    if(d->partExists(index))
    {
        KParts::Part * part = d->m_parts[index];
        part->widget()->setVisible(false);
        d->m_usedWidgets[index] = false;
    }
}

#else  //KOMPARE_ENABLED

namespace KDevelop
{
// ----- Empty implementations, don't support anything

struct KompareWidgetsPrivate
{
};

KompareWidgets::KompareWidgets()
{
}

KompareWidgets::~KompareWidgets()
{
}

bool KompareWidgets::compare(const IndexedString &, const QString &, QWidget *, int)
{
    return false;
}

bool KompareWidgets::widgetActive(int)
{
    return true;
}

void KompareWidgets::hideWidget(int)
{
}

#endif //KOMPARE_ENABLED
}
