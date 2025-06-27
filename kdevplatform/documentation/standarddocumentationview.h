/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2016 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H
#define KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H

#include <QWidget>
#include "documentationexport.h"
#include "documentationfindwidget.h"
#include <interfaces/idocumentation.h>

class QMenu;
class QWebEngineUrlSchemeHandler;

namespace KDevelop
{
class StandardDocumentationViewPrivate;

/**
 * A standard documentation view, based on QtWebEngine
 */
class KDEVPLATFORMDOCUMENTATION_EXPORT StandardDocumentationView : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(StandardDocumentationView)
public:
    static void registerCustomUrlSchemes();

    explicit StandardDocumentationView(DocumentationFindWidget* findWidget, QWidget* parent = nullptr );
    ~StandardDocumentationView() override;

    /**
     * @brief Enables zoom functionality
     *
     * @param configSubGroup KConfigGroup nested group name used to store zoom factor.
     *        Should uniquely describe current documentation provider.
     *
     * @warning Call this function at most once
     */
    void initZoom(const QString& configSubGroup);

    void setDocumentation(const IDocumentation::Ptr& doc);

    // NOTE: prefer overriding CSS by embedding a <style> element into HTML code directly instead of calling
    //       setOverrideCss*(). These functions, in case of Qt WebEngine, inject CSS code via JavaScript, which
    //       causes reloading and flickering of large pages. See for example kdevmanpage's class StyleSheetFixer.

    /**
     * Specifies the location of a user stylesheet to load with every web page
     *
     * @note each call to this function or setOverrideCssCode() overwrites any previously specified style
     */
    void setOverrideCssFile(const QString& cssFilePath);
    /**
     * Inject the specified UTF-8-encoded CSS code into each web page
     *
     * @note each call to this function or setOverrideCssFile() overwrites any previously specified style
     */
    void setOverrideCssCode(const QByteArray& cssCode);

    void load(const QUrl &url);
    void setHtml(const QString &html);

    /**
     * Install a @p handler for the @p scheme. The scheme must be a known custom URL scheme.
     *
     * @sa registerCustomUrlSchemes
     */
    void installUrlSchemeHandler(const QByteArray& scheme, QWebEngineUrlSchemeHandler* handler);

    /**
     *
     */
    void setDelegateLinks(bool delegate);

    /**
     * @return the standard context menu for this documentation view or @c nullptr if there are no actions to show
     */
    QMenu* createStandardContextMenu();

Q_SIGNALS:
    void linkClicked(const QUrl &link);
    void browseForward();
    void browseBack();

public Q_SLOTS:
    /**
     * Search for @p text in the documentation view.
     */
    void search(const QString& text, KDevelop::DocumentationFindWidget::FindOptions options);
    void searchIncremental(const QString& text, KDevelop::DocumentationFindWidget::FindOptions options);
    void finishSearch();

    /**
     * Updates the contents, in case it was initialized with a documentation instance,
     * doesn't change anything otherwise
     *
     * @sa setDocumentation(IDocumentation::Ptr)
     */
    void update();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;

private Q_SLOTS:
    void updateZoomFactor(double zoomFactor);

private:
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    const QScopedPointer<class StandardDocumentationViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(StandardDocumentationView)
};

}
#endif // KDEVPLATFORM_STANDARDDOCUMENTATIONVIEW_H
