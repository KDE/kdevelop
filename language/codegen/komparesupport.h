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

#ifndef KOMPARESUPPORT_H
#define KOMPARESUPPORT_H

#ifdef KOMPARE_ENABLED
#include <kompare/kompareinterface.h>

#include <QList>
#endif //KOMPARE_ENABLED

#include <language/duchain/indexedstring.h>
#include <QWidget>

namespace KDevelop
{

template<bool Enable>
class KompareSupport
{
  public:
    bool CreateWidget(const IndexedString & original, const IndexedString & modified, QWidget * widget)
    {
        return true;
    }
    
  private:
    QList<KompareInterface *> m_interfaces;
};

template <>
class KompareSupport<false>
{
  public:
    bool CreateWidget(const IndexedString & original, const IndexedString & modified, QWidget * widget)
    {
        return false;
    }
};

#ifdef KOMPARE_ENABLED
typedef KompareSupport<true> KompareWidgets;
#else
typedef KompareSupport<false> KompareWidgets;
#endif //KOMPARE_ENABLED
}

#undef KOMPARE_ENABLED
#endif //KOMPARESUPPORT_H
