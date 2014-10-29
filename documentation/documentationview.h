/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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

#ifndef KDEVPLATFORM_DOCUMENTATIONVIEW_H
#define KDEVPLATFORM_DOCUMENTATIONVIEW_H

#include <QWidget>
#include <QAbstractListModel>
#include <KToolBar>
#include <interfaces/idocumentation.h>
#include "documentationexport.h"

namespace KDevelop {
    class IPlugin;
    class DocumentationFindWidget;
}

class QModelIndex;
class QLineEdit;
class ProvidersModel;
class QComboBox;

class KDEVPLATFORMDOCUMENTATION_EXPORT DocumentationView : public QWidget
{
    Q_OBJECT
    public:
        DocumentationView(QWidget* parent, ProvidersModel* m);

        void showDocumentation(QExplicitlySharedDataPointer< KDevelop::IDocumentation > doc);

    public slots:
        void initialize();

        void addHistory(QExplicitlySharedDataPointer< KDevelop::IDocumentation > doc);
        void emptyHistory();

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
        QAction* mFind;
        QLineEdit* mIdentifiers;
        QList< QExplicitlySharedDataPointer< KDevelop::IDocumentation > > mHistory;
        QList< QExplicitlySharedDataPointer< KDevelop::IDocumentation > >::iterator mCurrent;
        QComboBox* mProviders;
        ProvidersModel* mProvidersModel;
        KDevelop::DocumentationFindWidget* mFindDoc;
};

class KDEVPLATFORMDOCUMENTATION_EXPORT ProvidersModel : public QAbstractListModel
{
    Q_OBJECT
    public:

        ProvidersModel(QObject* parent = 0);

        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual int rowCount(const QModelIndex& idx = QModelIndex()) const;
        QList<KDevelop::IDocumentationProvider*> providers();
        KDevelop::IDocumentationProvider* provider(int pos) const;
        int rowForProvider(KDevelop::IDocumentationProvider* provider);

    public slots:
        void unloaded(KDevelop::IPlugin* p);
        void loaded(KDevelop::IPlugin* p);
        void reloadProviders();

    private:
        QList<KDevelop::IDocumentationProvider*> mProviders;
signals:
        void providersChanged();
};

#endif // KDEVPLATFORM_DOCUMENTATIONVIEW_H
