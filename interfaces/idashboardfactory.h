/* This file is part of KDevelop
  Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef IDASHBOARDFACTORY_H
#define IDASHBOARDFACTORY_H

class KIcon;
class QWidget;
class QGraphicsItem;
class QString;
namespace Plasma { class Applet; }

namespace KDevelop {
class IProject;

class IDashboardFactory
{
    public:
        virtual ~IDashboardFactory() {}
        
        /** @returns an untranslatable name for the dashboard item */
        virtual QString id() const = 0;
        
        /** @returns a name to identifies the produced dashboard items */
        virtual QString name() const = 0;
        
        /** @returns a comment to explain the user what the items will offer */
        virtual QString comment() const = 0;
        
        /** @returns an icon that defines the produced dashboard items */
        virtual KIcon icon() const = 0;
        
        /** @param project is the project to be used by this factory. */
        void setProject(KDevelop::IProject* project) { m_project=project; }
        
        /** @returns the project to which this factory is going to be dedicated. */
        KDevelop::IProject* project() const { return m_project; }
        
    private:
        KDevelop::IProject* m_project;
};

/**
 * This class enables Dashboard item creation
 * 
 * Override this class if you are interested on putting some information to a
 * Project Dashboard.
 */
class IDashboardWidgetFactory : public IDashboardFactory
{
    public:
        /**
         * @returns the widget we want to add to the Dashboard.
         * It will be internally transformed to be adapted to the Dashboard implementation
         */
        virtual QWidget* widget() = 0;
};

/**
 * This class enables Dashboard item creation
 * 
 * Override this class if you're interested on having the whole control on the
 * Plasmoid (Dashboard item) behaviour.
 */
class IDashboardPlasmoidFactory : public IDashboardFactory
{
    public:
        /** @returns the plasmoid that is going to be added when the user selects it for the Dashboard */
        virtual Plasma::Applet* plasmaApplet(const QString& serviceId) = 0;
};

}

#endif // IDASHBOARDFACTORY_H
