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

#include "documentationview.h"
#include <QAction>
#include <QVBoxLayout>
#include <QLabel>
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <QComboBox>
#include <interfaces/idocumentationprovider.h>
#include <QLineEdit>
#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <QTextEdit>
#include <QCompleter>

class ProvidersModel : public QAbstractListModel
{
    public:
        ProvidersModel(QObject* parent = 0) : QAbstractListModel(parent) {}
        
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
        {
            QVariant ret;
            switch(role)
            {
                case Qt::DisplayRole:
                    ret=provider(index.row())->name();
                    break;
                case Qt::DecorationRole:
                    ret=provider(index.row())->icon();
                    break;
            }
            return ret;
        }
        
        virtual int rowCount(const QModelIndex&) const
        {
            return KDevelop::ICore::self()->documentationController()->documentationProviders().count();
        }
        
        KDevelop::IDocumentationProvider* provider(int pos) const
        {
            return KDevelop::ICore::self()->documentationController()->documentationProviders()[pos];
        }
        
        int rowForProvider(KDevelop::IDocumentationProvider* provider)
        {
            return KDevelop::ICore::self()->documentationController()->documentationProviders().indexOf(provider);
        }
};

DocumentationView::DocumentationView(QWidget* parent)
    : QWidget(parent)
{
    setWindowIcon(KIcon("documentation"));
    setLayout(new QVBoxLayout(this));
    mActions=new KToolBar(this);
    layout()->addWidget(mActions);
    mBack=mActions->addAction(KIcon("go-previous"), i18n("Back"));
    mForward=mActions->addAction(KIcon("go-next"), i18n("Forward"));
    mActions->addSeparator();
    mProviders=new QComboBox(mActions);
    mProvidersModel=new ProvidersModel(mProviders);
    mProviders->setModel(mProvidersModel);
    connect(mProviders, SIGNAL(activated(int)), SLOT(changedProvider(int)));
    mIdentifiers=new QLineEdit(mActions);
    mIdentifiers->setCompleter(new QCompleter(mIdentifiers));
    mIdentifiers->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mIdentifiers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mIdentifiers, SIGNAL(editingFinished()), SLOT(changedSelection()));
    connect(mIdentifiers->completer(), SIGNAL(activated(QModelIndex)), SLOT(changedSelection(QModelIndex)));
    
    mActions->addWidget(mProviders);
    mActions->addWidget(mIdentifiers);
    
    mBack->setEnabled(false);
    mForward->setEnabled(false);
    connect(mBack, SIGNAL(triggered()), this, SLOT(browseBack()));
    connect(mForward, SIGNAL(triggered()), this, SLOT(browseForward()));
    mCurrent=mHistory.begin();
    
    layout()->addWidget(new QLabel(i18n("There is no documentation selected yet"), this));
    
    if(mProvidersModel->rowCount(QModelIndex())>0)
        changedProvider(0);
}

void DocumentationView::browseBack()
{
    mCurrent--;
    mBack->setEnabled(mCurrent!=mHistory.begin());
    mForward->setEnabled(true);
    
    updateView();
}

void DocumentationView::browseForward()
{
    mCurrent++;
    mForward->setEnabled(mCurrent+1!=mHistory.end());
    mBack->setEnabled(true);
    
    updateView();
}

void DocumentationView::changedSelection()
{
    changedSelection(mIdentifiers->completer()->currentIndex());
}

void DocumentationView::changedSelection(const QModelIndex& idx)
{
    if(idx.isValid())
    {
        KSharedPtr<KDevelop::IDocumentation> doc=mProvidersModel->provider(mProviders->currentIndex())->documentationForIndex(idx);
        if(doc)
            showDocumentation(doc);
    }
}

void DocumentationView::showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc)
{
    kDebug(9529) << "showing" << doc->name();
    
    mBack->setEnabled( !mHistory.isEmpty() );
    mForward->setEnabled(false);
    mHistory.append(doc);
    mCurrent=mHistory.end()-1;
    updateView();
}

void DocumentationView::updateView()
{
    mProviders->setCurrentIndex(mProvidersModel->rowForProvider((*mCurrent)->provider()));
    mIdentifiers->completer()->setModel((*mCurrent)->provider()->indexModel());
    mIdentifiers->setText((*mCurrent)->name());
    
    delete layout()->takeAt(1);
    
    QWidget* w;
    if((*mCurrent)->providesWidget())
        w=(*mCurrent)->documentationWidget(this);
    else {
        QTextEdit* widget=new QTextEdit(this);
        widget->setText((*mCurrent)->description());
        w=widget;
    }
    Q_ASSERT(w);
    layout()->addWidget(w);
}

void DocumentationView::changedProvider(int row)
{
    mIdentifiers->completer()->setModel(mProvidersModel->provider(row)->indexModel());
    mIdentifiers->clear();
}
