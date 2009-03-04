/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_IASSISTANT_H
#define KDEVELOP_IASSISTANT_H

#include <KSharedPtr>
#include <QIcon>
#include "interfacesexport.h"

namespace KDevelop {

///Represents a single assistant action.
///Subclass it to create own actions
class KDEVPLATFORMINTERFACES_EXPORT IAssistantAction : public KShared {
    public:
        typedef KSharedPtr<IAssistantAction> Ptr;

        virtual ~IAssistantAction();
        
        ///Should return a short description of the action.
        ///It may contain simple HTML formatting.
        ///Must be very short, so it nicely fits into the assistant popups.
        virtual QString description() const = 0;
        ///May return additional tooltip hover information
        ///The default-implementation returns an empty string
        virtual QString toolTip() const;
        ///May return an icon for this action
        ///The default implementation returns an invalid icon, which means that no icon is shown
        virtual QIcon icon() const;
        
        ///Execute this action
        virtual void execute() = 0;
        
        enum Flags {
            NoFlag = 0,
            OwnLineFlag //If this flag is given, the action is shown in an own line. This is useful when the description tends to be very long.
        };
        
        ///May return any or'ed combination of Flags
        ///The default-implementation returns NoFlag
        virtual uint flags() const;
};

///Represents a single assistant popup
///Subclass it to create own assistants
class KDEVPLATFORMINTERFACES_EXPORT IAssistant : public QObject, public KShared
{
    Q_OBJECT
    public:
        virtual ~IAssistant();
        
        typedef KSharedPtr<IAssistant> Ptr;
    
        ///Should return the actions for this assistant
        virtual QList<IAssistantAction::Ptr> actions() = 0;
        
        ///May return an icon for this assistant
        virtual QIcon icon();
        
        ///May return the title of this assistant
        virtual QString title();
    signals:
        ///Can be emitted by the assistant when it should be hidden
        void hide();
        ///Can be emitted by the assistant when it's actions have changed and should be re-read
        void actionsChanged();
};

///Convenience-class that allows creating a simple assistant from just a list of actions
class KDEVPLATFORMINTERFACES_EXPORT StandardAssistant : public IAssistant {
    public:
        StandardAssistant(const QList<IAssistantAction::Ptr>& actions);
        virtual QList<IAssistantAction::Ptr> actions();
    private:
        QList<IAssistantAction::Ptr> m_actions;
};

}

#endif // KDEVELOP_IASSISTANT_H
