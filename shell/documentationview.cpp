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
#include <QComboBox>
#include <QTextEdit>
#include <QCompleter>
#include <QLayout>
#include <KLineEdit>
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <interfaces/icore.h>
#include <interfaces/idocumentationprovider.h>
#include <interfaces/idocumentationcontroller.h>
#include <QTextBrowser>

class ProvidersModel : public QAbstractListModel
{
    public:
        
        ProvidersModel(QObject* parent = 0)
            : QAbstractListModel(parent)
            , mProviders(KDevelop::ICore::self()->documentationController()->documentationProviders())
        {}
        
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
            return mProviders.count();
        }
        
        QList<KDevelop::IDocumentationProvider*> providers() {
            return mProviders;
        }
        
        KDevelop::IDocumentationProvider* provider(int pos) const
        {
            return mProviders[pos];
        }
        
        int rowForProvider(KDevelop::IDocumentationProvider* provider)
        {
            return mProviders.indexOf(provider);
        }
    private:
        QList<KDevelop::IDocumentationProvider*> mProviders;
};

DocumentationView::DocumentationView(QWidget* parent)
    : QWidget(parent)
{
    setWindowIcon(KIcon("documentation"));
    setLayout(new QVBoxLayout(this));
    layout()->setMargin(0);
    layout()->setSpacing(0);
    mActions=new KToolBar(this);
    mActions->setToolButtonStyle(Qt::ToolButtonIconOnly);
    layout()->addWidget(mActions);
    mBack=mActions->addAction(KIcon("go-previous"), i18n("Back"));
    mForward=mActions->addAction(KIcon("go-next"), i18n("Forward"));
    mActions->addSeparator();
    mActions->addAction(KIcon("go-home"), i18n("Home"), this, SLOT(showHome()));
    mProviders=new QComboBox(mActions);
    mProviders->setFocusPolicy(Qt::NoFocus);
    mProvidersModel=new ProvidersModel(mProviders);
    mProviders->setModel(mProvidersModel);
    foreach(KDevelop::IDocumentationProvider* p, mProvidersModel->providers()) {
        connect(dynamic_cast<QObject*>(p), SIGNAL(addHistory(KSharedPtr<KDevelop::IDocumentation>)),
                SLOT(addHistory(KSharedPtr<KDevelop::IDocumentation>)));
    }
    
    connect(mProviders, SIGNAL(activated(int)), SLOT(changedProvider(int)));
    mIdentifiers=new KLineEdit(mActions);
    mIdentifiers->setClearButtonShown(true);
    mIdentifiers->setCompleter(new QCompleter(mIdentifiers));
//     mIdentifiers->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mIdentifiers->completer()->setCaseSensitivity(Qt::CaseInsensitive);

    /* vertical size policy should be left to the style. */
    mIdentifiers->setSizePolicy(QSizePolicy::Expanding, mIdentifiers->sizePolicy().verticalPolicy());
    connect(mIdentifiers, SIGNAL(returnPressed()), SLOT(changedSelection()));
    connect(mIdentifiers->completer(), SIGNAL(activated(QModelIndex)), SLOT(changeProvider(QModelIndex)));
    
    mActions->addWidget(mProviders);
    mActions->addWidget(mIdentifiers);
    
    mBack->setEnabled(false);
    mForward->setEnabled(false);
    connect(mBack, SIGNAL(triggered()), this, SLOT(browseBack()));
    connect(mForward, SIGNAL(triggered()), this, SLOT(browseForward()));
    mCurrent=mHistory.end();
    
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

void DocumentationView::showHome()
{
    KDevelop::IDocumentationProvider* prov=mProvidersModel->provider(mProviders->currentIndex());
    
    showDocumentation(prov->homePage());
}

void DocumentationView::changedSelection()
{
    changeProvider(mIdentifiers->completer()->currentIndex());
}

void DocumentationView::changeProvider(const QModelIndex& idx)
{
    if(idx.isValid())
    {
        KDevelop::IDocumentationProvider* prov=mProvidersModel->provider(mProviders->currentIndex());
        KSharedPtr<KDevelop::IDocumentation> doc=prov->documentationForIndex(idx);
        if(doc)
            showDocumentation(doc);
    }
}

void DocumentationView::showDocumentation(KSharedPtr< KDevelop::IDocumentation > doc)
{
    kDebug(9529) << "showing" << doc->name();
    
    addHistory(doc);
    updateView();
}

void DocumentationView::addHistory(KSharedPtr< KDevelop::IDocumentation > doc)
{
    mBack->setEnabled( !mHistory.isEmpty() );
    mForward->setEnabled(false);
    mHistory.append(doc);
    mCurrent=mHistory.end()-1;
}

void DocumentationView::updateView()
{
    mProviders->setCurrentIndex(mProvidersModel->rowForProvider((*mCurrent)->provider()));
    mIdentifiers->completer()->setModel((*mCurrent)->provider()->indexModel());
    mIdentifiers->setText((*mCurrent)->name());
    
    QLayoutItem* lastview=layout()->takeAt(1);
    if(lastview && lastview->widget()->parent()==this)
        lastview->widget()->deleteLater();
    
    delete lastview;
    
    QWidget* w=(*mCurrent)->documentationWidget(this);
    Q_ASSERT(w);
    layout()->addWidget(w);
}

void DocumentationView::changedProvider(int row)
{
    mIdentifiers->completer()->setModel(mProvidersModel->provider(row)->indexModel());
    mIdentifiers->clear();
    
    showHome();
}
