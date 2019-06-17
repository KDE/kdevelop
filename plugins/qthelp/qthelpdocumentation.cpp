/*  This file is part of KDevelop
    Copyright 2009 Aleix Pol <aleixpol@kde.org>
    Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "qthelpdocumentation.h"

#include <QLabel>
#include <QUrl>
#include <QTreeView>
#include <QHelpContentModel>
#include <QHeaderView>
#include <QMenu>
#include <QTemporaryFile>
#include <QRegularExpression>

#include <KLocalizedString>

#include <interfaces/icore.h>
#include <interfaces/idocumentationcontroller.h>
#include <documentation/standarddocumentationview.h>
#include "qthelpnetwork.h"
#include "qthelpproviderabstract.h"

using namespace KDevelop;

QtHelpProviderAbstract* QtHelpDocumentation::s_provider=nullptr;

QtHelpDocumentation::QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info)
    : m_provider(s_provider), m_name(name), m_info(info), m_current(info.constBegin()), lastView(nullptr)
{}

QtHelpDocumentation::QtHelpDocumentation(const QString& name, const QMap<QString, QUrl>& info, const QString& key)
    : m_provider(s_provider), m_name(name), m_info(info), m_current(m_info.find(key)), lastView(nullptr)
{ Q_ASSERT(m_current!=m_info.constEnd()); }

QtHelpDocumentation::~QtHelpDocumentation()
{
    delete m_lastStyleSheet.data();
}

QString QtHelpDocumentation::description() const
{
    const QUrl url(m_current.value());
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
            const QStringRef between = dataString.midRef(titleEnd, pos-titleEnd).trimmed();
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

    return QStringList(m_info.keys()).join(QStringLiteral(", "));
}

void QtHelpDocumentation::setUserStyleSheet(StandardDocumentationView* view, const QUrl& url)
{
    auto* file = new QTemporaryFile(view);
    file->open();

    QTextStream ts(file);
    ts << "html { background: white !important; }\n";
    if (url.scheme() == QLatin1String("qthelp") && url.host().startsWith(QLatin1String("com.trolltech.qt."))) {
       ts << ".content .toc + .title + p { clear:left; }\n"
          << "#qtdocheader .qtref { position: absolute !important; top: 5px !important; right: 0 !important; }\n";
    }
    file->close();
    view->setOverrideCss(QUrl::fromLocalFile(file->fileName()));

    delete m_lastStyleSheet.data();
    m_lastStyleSheet = file;
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

        setUserStyleSheet(view, m_current.value());
        view->load(m_current.value());
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
            const QString& name = it.key();
            auto* act=new QtHelpAlternativeLink(name, this, actionGroup);
            act->setCheckable(true);
            act->setChecked(name==m_current.key());
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
    w->header()->setVisible(false);
    w->setModel(m_provider->engine()->contentModel());

    connect(w, &QTreeView::clicked, this, &HomeDocumentation::clicked);
    return w;
}

void HomeDocumentation::clicked(const QModelIndex& idx)
{
    QHelpContentModel* model = m_provider->engine()->contentModel();
    QHelpContentItem* it=model->contentItemAt(idx);
    QMap<QString, QUrl> info;
    info.insert(it->title(), it->url());

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
