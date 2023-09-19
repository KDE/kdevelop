/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qthelpdocumentation.h"

#include <QLabel>
#include <QUrl>
#include <QTreeView>
#include <QHelpContentModel>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QActionGroup>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <documentation/standarddocumentationview.h>
#include "qthelpnetwork.h"
#include "qthelpproviderabstract.h"

#include <algorithm>

using namespace KDevelop;

QtHelpProviderAbstract* QtHelpDocumentation::s_provider=nullptr;

QtHelpDocumentation::QtHelpDocumentation(const QString& name, const QList<QHelpLink>& info)
    : m_provider(s_provider)
    , m_name(name)
    , m_info(info)
    , m_current(info.constBegin())
    , lastView(nullptr)
{
}

namespace {
QList<QHelpLink>::const_iterator findTitle(const QList<QHelpLink>& links, const QString& title)
{
    return std::find_if(links.begin(), links.end(), [title](const QHelpLink& helpLink) {
        return helpLink.title == title;
    });
}
}

QtHelpDocumentation::QtHelpDocumentation(const QString& name, const QList<QHelpLink>& info, const QString& key)
    : m_provider(s_provider)
    , m_name(name)
    , m_info(info)
    , m_current(::findTitle(m_info, key))
    , lastView(nullptr)
{
    Q_ASSERT(m_current!=m_info.constEnd());
}

QString QtHelpDocumentation::description() const
{
    const QUrl url = currentUrl();
    //Extract a short description from the html data
    const QString dataString = QString::fromLatin1(m_provider->engine()->fileData(url)); ///@todo encoding

    const QString fragment = url.fragment();
    const QString p = QStringLiteral("((\\\")|(\\\'))");
    const QString optionalSpace = QStringLiteral(" *");
    const QString exp = QString(QLatin1String("< a name = ") + p + fragment + p + QLatin1String(" > < / a >")).replace(QLatin1Char(' '), optionalSpace);

    const QRegularExpression findFragment(exp);
    QRegularExpressionMatch findFragmentMatch;
    int pos = dataString.indexOf(findFragment, 0, &findFragmentMatch);

    if(fragment.isEmpty()) {
        pos = 0;
    } else {

        //Check if there is a title opening-tag right before the fragment, and if yes add it, so we have a nicely formatted caption
        const QString titleRegExp = QStringLiteral("< h\\d class = \".*\" >").replace(QLatin1Char(' '), optionalSpace);
        const QRegularExpression findTitle(titleRegExp);
        const QRegularExpressionMatch match = findTitle.match(dataString, pos);
        const int titleStart = match.capturedStart();
        const int titleEnd = titleStart + match.capturedEnd();
        if(titleStart != -1) {
            const QStringView between = QStringView(dataString).mid(titleEnd, pos-titleEnd).trimmed();
            if(between.isEmpty())
                pos = titleStart;
        }
    }

    if(pos != -1) {
        const QString exp = QString(QStringLiteral("< a name = ") + p + QStringLiteral("((\\S)*)") + p + QStringLiteral(" > < / a >")).replace(QLatin1Char(' '), optionalSpace);
        const QRegularExpression nextFragmentExpression(exp);
        int endPos = dataString.indexOf(nextFragmentExpression, pos+(fragment.size() ? findFragmentMatch.capturedLength() : 0));
        if(endPos == -1) {
            endPos = dataString.size();
        }

        {
            //Find the end of the last paragraph or newline, so we don't add prefixes of the following fragment
            const QString newLineRegExp = QStringLiteral ("< br / > | < / p >").replace(QLatin1Char(' '), optionalSpace);
            const QRegularExpression lastNewLine(newLineRegExp);
            QRegularExpressionMatch match;
            const int newEnd = dataString.lastIndexOf(lastNewLine, endPos, &match);
            if(match.isValid() && newEnd > pos)
                endPos = newEnd + match.capturedLength();
        }

        {
            //Find the title, and start from there
            const QString titleRegExp = QStringLiteral("< h\\d class = \"title\" >").replace(QLatin1Char(' '), optionalSpace);
            const QRegularExpression findTitle(titleRegExp);
            const QRegularExpressionMatch match = findTitle.match(dataString);
            if (match.isValid())
                pos = qBound(pos, match.capturedStart(), endPos);
        }


        QString thisFragment = dataString.mid(pos, endPos - pos);

        {
            //Completely remove the first large header found, since we don't need a header
            const QString headerRegExp = QStringLiteral("< h\\d.*>.*?< / h\\d >").replace(QLatin1Char(' '), optionalSpace);
            const QRegularExpression findHeader(headerRegExp);
            const QRegularExpressionMatch match = findHeader.match(thisFragment);
            if(match.isValid()) {
                thisFragment.remove(match.capturedStart(), match.capturedLength());
            }
        }

        {
            //Replace all gigantic header-font sizes with <big>
            {
                const QString sizeRegExp = QStringLiteral("< h\\d ").replace(QLatin1Char(' '), optionalSpace);
                const QRegularExpression findSize(sizeRegExp);
                thisFragment.replace(findSize, QStringLiteral("<big "));
            }
            {
                const QString sizeCloseRegExp = QStringLiteral("< / h\\d >").replace(QLatin1Char(' '), optionalSpace);
                const QRegularExpression closeSize(sizeCloseRegExp);
                thisFragment.replace(closeSize, QStringLiteral("</big><br />"));
            }
        }

        {
            //Replace paragraphs by newlines
            const QString begin = QStringLiteral("< p >").replace(QLatin1Char(' '), optionalSpace);
            const QRegularExpression findBegin(begin);
            thisFragment.replace(findBegin, {});

            const QString end = QStringLiteral("< /p >").replace(QLatin1Char(' '), optionalSpace);
            const QRegularExpression findEnd(end);
            thisFragment.replace(findEnd, QStringLiteral("<br />"));
        }

        {
            //Remove links, because they won't work
            const QString link = QString(QStringLiteral("< a href = ") + p + QStringLiteral(".*?") + p).replace(QLatin1Char(' '), optionalSpace);
            const QRegularExpression exp(link, QRegularExpression::CaseInsensitiveOption);
            thisFragment.replace(exp, QStringLiteral("<a "));
        }

        return thisFragment;
    }

    QStringList titles;
    titles.reserve(m_info.size());
    for (auto& link : qAsConst(m_info)) {
        titles.append(link.title);
    }
    return titles.join(QLatin1String(", "));
}

QWidget* QtHelpDocumentation::documentationWidget(DocumentationFindWidget* findWidget, QWidget* parent)
{
    if(m_info.isEmpty()) { //QtHelp sometimes has empty info maps. e.g. availableaudioeffects i 4.5.2
        return new QLabel(i18n("Could not find any documentation for '%1'", m_name), parent);
    } else {
        auto* view = new StandardDocumentationView(findWidget, parent);
        view->initZoom(m_provider->name());
        view->setDelegateLinks(true);
        view->setNetworkAccessManager(m_provider->networkAccess());
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(view, &StandardDocumentationView::linkClicked, this, &QtHelpDocumentation::jumpedTo);
        connect(view, &StandardDocumentationView::customContextMenuRequested, this, &QtHelpDocumentation::viewContextMenuRequested);

        view->load(currentUrl());
        lastView = view;
        return view;
    }
}

void QtHelpDocumentation::viewContextMenuRequested(const QPoint& pos)
{
    auto* view = qobject_cast<StandardDocumentationView*>(sender());
    if (!view)
        return;

    auto menu = view->createStandardContextMenu();

    if (m_info.count() > 1) {
        if (!menu->isEmpty()) {
            menu->addSeparator();
        }

        auto* actionGroup = new QActionGroup(menu);
        for (auto it = m_info.constBegin(), end = m_info.constEnd(); it != end; ++it) {
            const QString& name = it->title;
            auto* act=new QtHelpAlternativeLink(name, this, actionGroup);
            act->setCheckable(true);
            act->setChecked(name==currentTitle());
            menu->addAction(act);
        }
    }

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(view->mapToGlobal(pos));
}


void QtHelpDocumentation::jumpedTo(const QUrl& newUrl)
{
    Q_ASSERT(lastView);
    m_provider->jumpedTo(newUrl);
}

IDocumentationProvider* QtHelpDocumentation::provider() const
{
    return m_provider;
}

QtHelpAlternativeLink::QtHelpAlternativeLink(const QString& name, const QtHelpDocumentation* doc, QObject* parent)
    : QAction(name, parent), mDoc(doc), mName(name)
{
    connect(this, &QtHelpAlternativeLink::triggered, this, &QtHelpAlternativeLink::showUrl);
}

void QtHelpAlternativeLink::showUrl()
{
    IDocumentation::Ptr newDoc(new QtHelpDocumentation(mName, mDoc->info(), mName));
    ICore::self()->documentationController()->showDocumentation(newDoc);
}

HomeDocumentation::HomeDocumentation() : m_provider(QtHelpDocumentation::s_provider)
{
}

QWidget* HomeDocumentation::documentationWidget(DocumentationFindWidget*, QWidget* parent)
{
    auto* w=new QTreeView(parent);
    // install an event filter to get the mouse events out of it
    w->viewport()->installEventFilter(this);
    w->header()->setVisible(false);
    w->setModel(m_provider->engine()->contentModel());

    connect(w, &QTreeView::clicked, this, &HomeDocumentation::clicked);
    return w;
}

void HomeDocumentation::clicked(const QModelIndex& idx)
{
    QHelpContentModel* model = m_provider->engine()->contentModel();
    QHelpContentItem* it=model->contentItemAt(idx);

    const QList<QHelpLink> info{{it->url(), it->title()}};
    IDocumentation::Ptr newDoc(new QtHelpDocumentation(it->title(), info));
    ICore::self()->documentationController()->showDocumentation(newDoc);
}

QString HomeDocumentation::name() const
{
    return i18n("QtHelp Home Page");
}

IDocumentationProvider* HomeDocumentation::provider() const
{
    return m_provider;
}

bool HomeDocumentation::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        // Here we need to set accpeted to false to let it propagate up
        event->setAccepted(false);
    }
    return QObject::eventFilter(obj, event);
}

#include "moc_qthelpdocumentation.cpp"
