#include "standarddocumentationview.h"
#include <QMenu>
#include <QWidget>
#include <QDebug>


namespace KDevelop {
class StandardDocumentationViewPrivate
{
public:
    StandardDocumentationViewPrivate() = default;
    ~StandardDocumentationViewPrivate() = default;
};
} 

namespace KDevelop {


StandardDocumentationView::StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new StandardDocumentationViewPrivate)
{
    qWarning() << "QtWebEngine is disabled";
    Q_UNUSED(findWidget);
}

void StandardDocumentationView::registerCustomUrlSchemes() { }

StandardDocumentationView::~StandardDocumentationView() = default;

void StandardDocumentationView::search(const QString&, DocumentationFindWidget::FindOptions) { }
void StandardDocumentationView::searchIncremental(const QString&, DocumentationFindWidget::FindOptions) { }
void StandardDocumentationView::finishSearch() { }
void StandardDocumentationView::initZoom(const QString&) { }
void StandardDocumentationView::setDocumentation(const IDocumentation::Ptr&) { }
void StandardDocumentationView::update() { }
void StandardDocumentationView::setOverrideCssFile(const QString&) { }
void StandardDocumentationView::setOverrideCssCode(const QByteArray&) { }
void StandardDocumentationView::load(const QUrl&) { }
void StandardDocumentationView::setHtml(const QString&) { }
void StandardDocumentationView::installUrlSchemeHandler(const QByteArray&, QWebEngineUrlSchemeHandler*) { }
void StandardDocumentationView::setDelegateLinks(bool) { }
QMenu* StandardDocumentationView::createStandardContextMenu() { return new QMenu(this); }
bool StandardDocumentationView::eventFilter(QObject*, QEvent*) { return false; }
void StandardDocumentationView::contextMenuEvent(QContextMenuEvent*) { }
void StandardDocumentationView::updateZoomFactor(double) { }
void StandardDocumentationView::keyReleaseEvent(QKeyEvent* event) { QWidget::keyReleaseEvent(event); }

}
