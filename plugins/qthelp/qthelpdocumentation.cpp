#include "qthelpdocumentation.h"

#include <QLabel>
#include <QTreeView>
#include <QHeaderView>
#include <QMenu>
#include <KLocalizedString>

// stubbed implementations as part of disabling webengine support


QtHelpDocumentation::QtHelpDocumentation(QtHelpProviderAbstract* provider,
                                         const QString& name,
                                         const QList<QHelpLink>& info)
    : m_provider(provider)
    , m_name(name)
    , m_info(info)
    , m_current(info.constBegin())
{
    // stub
}

QtHelpDocumentation::QtHelpDocumentation(QtHelpProviderAbstract* provider,
                                         const QString& name,
                                         const QList<QHelpLink>& info,
                                         const QString& key)
    : m_provider(provider)
    , m_name(name)
    , m_info(info)
    , m_current(info.constBegin())
{
    Q_UNUSED(key);
}


QString QtHelpDocumentation::description() const{ return m_name;}

QWidget* QtHelpDocumentation::documentationWidget(KDevelop::DocumentationFindWidget* /*findWidget*/, QWidget* parent)
{ return new QLabel(i18n("webengine disabled"), parent);}

void QtHelpDocumentation::viewContextMenuRequested(const QPoint& /*pos*/){ }
void QtHelpDocumentation::jumpedTo(const QUrl& /*newUrl*/){ }

KDevelop::IDocumentationProvider* QtHelpDocumentation::provider() const{return nullptr; }


QtHelpAlternativeLink::QtHelpAlternativeLink(const QString& name,
                                               const QtHelpDocumentation* doc,
                                               QObject* parent)
    : QAction(name, parent),
      mDoc(doc),
      mName(name){ }

void QtHelpAlternativeLink::showUrl(){}


HomeDocumentation::HomeDocumentation(QtHelpProviderAbstract* provider)
    : m_provider(provider){ }

QWidget* HomeDocumentation::documentationWidget(KDevelop::DocumentationFindWidget* /*findWidget*/, QWidget* parent)
{return new QLabel(i18n("webengine disabled"), parent);}

void HomeDocumentation::clicked(const QModelIndex& /*idx*/){ }

QString HomeDocumentation::name() const{ return i18n("webengine disabled");}

KDevelop::IDocumentationProvider* HomeDocumentation::provider() const{ return nullptr;}

bool HomeDocumentation::eventFilter(QObject* /*obj*/, QEvent* /*event*/){ return false;}

