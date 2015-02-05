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

#include "documentationview.h"

#include <QAction>
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QCompleter>
#include <QLayout>
#include <QTextBrowser>

#include <QLineEdit>
#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentationprovider.h>
#include <interfaces/idocumentationproviderprovider.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>
#include "documentationfindwidget.h"
#include "debug.h"

Q_LOGGING_CATEGORY(DOCUMENTATION, "kdevplatform.documentation")


using namespace KDevelop;

DocumentationView::DocumentationView(QWidget* parent, ProvidersModel* m)
    : QWidget(parent), mProvidersModel(m)
{
    setWindowIcon(QIcon::fromTheme("documentation"));
    setWindowTitle(i18n("Documentation"));

    setLayout(new QVBoxLayout(this));
    layout()->setMargin(0);
    layout()->setSpacing(0);

    //TODO: clean this up, simply use addAction as that will create a toolbar automatically
    //      use custom QAction's with createWidget for mProviders and mIdentifiers
    mActions=new KToolBar(this);
    // set window title so the QAction from QToolBar::toggleViewAction gets a proper name set
    mActions->setWindowTitle(i18n("Documentation Tool Bar"));
    mActions->setToolButtonStyle(Qt::ToolButtonIconOnly);
    int iconSize=style()->pixelMetric(QStyle::PM_SmallIconSize);
    mActions->setIconSize(QSize(iconSize, iconSize));

    mFindDoc = new DocumentationFindWidget;
    mFindDoc->hide();

    mBack=mActions->addAction(QIcon::fromTheme("go-previous"), i18n("Back"));
    mForward=mActions->addAction(QIcon::fromTheme("go-next"), i18n("Forward"));
    mFind=mActions->addAction(QIcon::fromTheme("edit-find"), i18n("Find"), mFindDoc, SLOT(show()));
    mActions->addSeparator();
    mActions->addAction(QIcon::fromTheme("go-home"), i18n("Home"), this, SLOT(showHome()));
    mProviders=new QComboBox(mActions);

    mIdentifiers=new QLineEdit(mActions);
    mIdentifiers->setClearButtonEnabled(true);
    mIdentifiers->setCompleter(new QCompleter(mIdentifiers));
//     mIdentifiers->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mIdentifiers->completer()->setCaseSensitivity(Qt::CaseInsensitive);

    /* vertical size policy should be left to the style. */
    mIdentifiers->setSizePolicy(QSizePolicy::Expanding, mIdentifiers->sizePolicy().verticalPolicy());
    connect(mIdentifiers, &QLineEdit::returnPressed, this, &DocumentationView::changedSelection);
    connect(mIdentifiers->completer(), static_cast<void(QCompleter::*)(const QModelIndex&)>(&QCompleter::activated), this, &DocumentationView::changeProvider);
    QWidget::setTabOrder(mProviders, mIdentifiers);

    mActions->addWidget(mProviders);
    mActions->addWidget(mIdentifiers);

    mBack->setEnabled(false);
    mForward->setEnabled(false);
    connect(mBack, &QAction::triggered, this, &DocumentationView::browseBack);
    connect(mForward, &QAction::triggered, this, &DocumentationView::browseForward);
    mCurrent=mHistory.end();

    layout()->addWidget(mActions);
    layout()->addWidget(new QWidget(this));
    layout()->addWidget(mFindDoc);

    QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

void DocumentationView::initialize()
{
    mProviders->setModel(mProvidersModel);
    connect(mProviders, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &DocumentationView::changedProvider);
    foreach (IDocumentationProvider* p, mProvidersModel->providers()) {
        // can't use new signal/slot syntax here, IDocumentation is not a QObject
        connect(dynamic_cast<QObject*>(p), SIGNAL(addHistory(KDevelop::IDocumentation::Ptr)),
                this, SLOT(addHistory(KDevelop::IDocumentation::Ptr)));
    }
    connect(mProvidersModel, &ProvidersModel::providersChanged, this, &DocumentationView::emptyHistory);

    if(mProvidersModel->rowCount()>0)
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
    auto prov = mProvidersModel->provider(mProviders->currentIndex());

    showDocumentation(prov->homePage());
}

void DocumentationView::changedSelection()
{
    changeProvider(mIdentifiers->completer()->currentIndex());
}

void DocumentationView::changeProvider(const QModelIndex& idx)
{
    if (idx.isValid()) {
        IDocumentationProvider* prov = mProvidersModel->provider(mProviders->currentIndex());
        auto doc = prov->documentationForIndex(idx);
        if (doc) {
            showDocumentation(doc);
        }
    }
}

void DocumentationView::showDocumentation(const IDocumentation::Ptr& doc)
{
    qCDebug(DOCUMENTATION) << "showing" << doc->name();

    addHistory(doc);
    updateView();
}

void DocumentationView::addHistory(const IDocumentation::Ptr& doc)
{
    mBack->setEnabled( !mHistory.isEmpty() );
    mForward->setEnabled(false);

    // clear all history following the current item, unless we're already
    // at the end (otherwise this code crashes when history is empty, which
    // happens when addHistory is first called on startup to add the
    // homepage)
    if (mCurrent+1 < mHistory.end()) {
        mHistory.erase(mCurrent+1, mHistory.end());
    }

    mHistory.append(doc);
    mCurrent=mHistory.end()-1;

    // NOTE: we assume an existing widget was used to navigate somewhere
    //       but this history entry actually contains the new info for the
    //       title... this is ugly and should be refactored somehow
    if (mIdentifiers->completer()->model() == (*mCurrent)->provider()->indexModel()) {
        mIdentifiers->setText((*mCurrent)->name());
    }
}

void DocumentationView::emptyHistory()
{
    mHistory.clear();
    mCurrent=mHistory.end();
    mBack->setEnabled(false);
    mForward->setEnabled(false);
    if(mProviders->count() > 0) {
        mProviders->setCurrentIndex(0);
        changedProvider(0);
    }
}

void DocumentationView::updateView()
{
    mProviders->setCurrentIndex(mProvidersModel->rowForProvider((*mCurrent)->provider()));
    mIdentifiers->completer()->setModel((*mCurrent)->provider()->indexModel());
    mIdentifiers->setText((*mCurrent)->name());

    QLayoutItem* lastview=layout()->takeAt(1);
    Q_ASSERT(lastview);

    if(lastview->widget()->parent()==this)
        lastview->widget()->deleteLater();

    delete lastview;

    mFindDoc->setEnabled(false);
    QWidget* w=(*mCurrent)->documentationWidget(mFindDoc, this);
    Q_ASSERT(w);
    QWidget::setTabOrder(mIdentifiers, w);

    mFind->setEnabled(mFindDoc->isEnabled());
    if(!mFindDoc->isEnabled())
        mFindDoc->hide();

    QLayoutItem* findW=layout()->takeAt(1);
    layout()->addWidget(w);
    layout()->addItem(findW);
}

void DocumentationView::changedProvider(int row)
{
    mIdentifiers->completer()->setModel(mProvidersModel->provider(row)->indexModel());
    mIdentifiers->clear();

    showHome();
}

////////////// ProvidersModel //////////////////

ProvidersModel::ProvidersModel(QObject* parent)
    : QAbstractListModel(parent)
    , mProviders(ICore::self()->documentationController()->documentationProviders())
{
    connect(ICore::self()->pluginController(), &IPluginController::unloadingPlugin, this, &ProvidersModel::unloaded);
    connect(ICore::self()->pluginController(), &IPluginController::pluginLoaded, this, &ProvidersModel::loaded);
    connect(ICore::self()->documentationController(), &IDocumentationController::providersChanged, this, &ProvidersModel::reloadProviders);
}

void ProvidersModel::reloadProviders()
{
    beginResetModel();
    mProviders = ICore::self()->documentationController()->documentationProviders();
    endResetModel();
    emit providersChanged();
}

QVariant ProvidersModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= mProviders.count() || index.row() < 0)
        return QVariant();

    QVariant ret;
    switch (role)
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

void ProvidersModel::removeProviders(const QList<KDevelop::IDocumentationProvider*> &prov)
{
    if (prov.isEmpty())
        return;

    int idx = mProviders.indexOf(prov.first());

    if (idx>=0) {
        beginRemoveRows(QModelIndex(), idx, idx + prov.count()-1);
        for(int i=0, c=prov.count(); i<c; ++i)
            mProviders.removeAt(idx);
        endRemoveRows();
    }

    emit providersChanged();
}

void ProvidersModel::unloaded(IPlugin* p)
{
    IDocumentationProvider* prov=p->extension<IDocumentationProvider>();
    if (prov)
        removeProviders(QList<IDocumentationProvider*>() << prov);

    IDocumentationProviderProvider* provProv=p->extension<IDocumentationProviderProvider>();
    if (provProv) {
        removeProviders(provProv->providers());
    }
}

void ProvidersModel::loaded(IPlugin* p)
{
    IDocumentationProvider* prov=p->extension<IDocumentationProvider>();

    if (prov && !mProviders.contains(prov)) {
        beginInsertRows(QModelIndex(), 0, 0);
        mProviders.append(prov);
        endInsertRows();
        emit providersChanged();
    }

    IDocumentationProviderProvider* provProv=p->extension<IDocumentationProviderProvider>();
    if (provProv) {
        beginInsertRows(QModelIndex(), 0, provProv->providers().count()-1);
        mProviders.append(provProv->providers());
        endInsertRows();
        emit providersChanged();
    }
}

int ProvidersModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : mProviders.count();
}

int ProvidersModel::rowForProvider(IDocumentationProvider* provider) { return mProviders.indexOf(provider); }
IDocumentationProvider* ProvidersModel::provider(int pos) const { return mProviders[pos]; }
QList< IDocumentationProvider* > ProvidersModel::providers() { return mProviders; }
