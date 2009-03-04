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
#include <QPointer>
#include <ktexteditor/cursor.h>

namespace KTextEditor {
class View;
class Cursor;
}
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
    
        ///Returns the stored list of actions, or can be overridden to return an own set
        virtual QList<IAssistantAction::Ptr> actions() const;
        
        ///Adds the given action to the list of actions
        ///Does not emit actionsChanged(), you have to do that when you're ready
        virtual void addAction(IAssistantAction::Ptr action);
        
        ///Clears the stored list of actions
        ///Does not emit actionsChanged(), you have to do that when you're ready
        virtual void clearActions();
        
        ///May return an icon for this assistant
        virtual QIcon icon() const;
        
        ///May return the title of this assistant
        virtual QString title() const;
    public slots:
        ///Emits hide(), which causes this assistant to be hidden
        virtual void doHide();
    signals:
        ///Can be emitted by the assistant when it should be hidden
        void hide();
        ///Can be emitted by the assistant when it's actions have changed and should be re-read
        void actionsChanged();
    private:
        QList<IAssistantAction::Ptr> m_actions;
};

///A helper assistant base class that binds itself to a view, and hides itself as soon as
///the cursor was moved too far away from the invocation position, or a newline was inserted.
class KDEVPLATFORMINTERFACES_EXPORT ITextAssistant : public IAssistant
{
  Q_OBJECT
public:
  ITextAssistant(KTextEditor::View* view);
  
  ///@return The view this text-assistant was created with. May be zero if it was deleted already.
  KTextEditor::View* view() const;
  ///Position where the cursor was when this assistant was created
  KTextEditor::Cursor invocationCursor() const;
private slots:
  //This function checks whether the cursor was moved away by more than 2 lines from the initial position, and hides the assistant if so.
  //Override it to change this behavior.
  virtual void cursorPositionChanged(KTextEditor::View*,KTextEditor::Cursor);
  //This function checks whether a newline was inserted, and hides the assistant if so.
  //Override it to change this behavior.
  virtual void textInserted(KTextEditor::Document*,KTextEditor::Range);
  
private:
  QPointer<KTextEditor::View> m_view;
  KTextEditor::Cursor m_invocationCursor;
};

}

#endif // KDEVELOP_IASSISTANT_H
