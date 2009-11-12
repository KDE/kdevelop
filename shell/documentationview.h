/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   
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

#ifndef DOCUMENTATIONVIEW_H
#define DOCUMENTATIONVIEW_H

#include <QWidget>
#include <KToolBar>
#include <interfaces/idocumentation.h>

class QModelIndex;
class KLineEdit;
class ProvidersModel;
class QComboBox;

class DocumentationView : public QWidget
{
    Q_OBJECT
    public:
        DocumentationView(QWidget* parent);
        
        void showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc);
        
    public slots:
        void addHistory(KSharedPtr< KDevelop::IDocumentation > doc);
        
        void browseForward();
        void browseBack();
        void changedSelection();
        void changedProvider(int);
        void changeProvider(const QModelIndex &);
        void showHome();
        
    private:
        void updateView();
        
        KToolBar* mActions;
        QAction* mForward;
        QAction* mBack;
        KLineEdit* mIdentifiers;
        QList< KSharedPtr< KDevelop::IDocumentation > > mHistory;
        QList< KSharedPtr< KDevelop::IDocumentation > >::iterator mCurrent;
        QComboBox* mProviders;
        ProvidersModel* mProvidersModel;
};

#endif // DOCUMENTATIONVIEW_H
