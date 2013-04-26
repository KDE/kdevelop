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

#ifndef KDEVPLATFORM_KOMPARESUPPORT_H
#define KDEVPLATFORM_KOMPARESUPPORT_H

#include <config-kdevplatform.h>

class QWidget;
class QString;

namespace KDevelop
{
struct KompareWidgetsPrivate;
class IndexedString;

class KompareWidgets
{
  public:
#ifdef HAVE_KOMPARE
    static const bool enabled = true;
#else
    static const bool enabled = false;
#endif

    KompareWidgets();
    ~KompareWidgets();
    
    /**
     * Create a widget at index @p index. if @p index == -1 then widget will be appended
     * @return the index number of the retrieved/created widget, 
     */
    bool compare(const IndexedString & original, const QString & modified, QWidget * widget, int index = -1);
    
    ///@return true if the widget is being displayed
    bool widgetActive(int index);
    
    ///Set the widget to unused, and removes the current widget its attached to
    void hideWidget(int index);

  private:
    KompareWidgetsPrivate * const d;
};

}

#endif //KDEVPLATFORM_KOMPARESUPPORT_H
