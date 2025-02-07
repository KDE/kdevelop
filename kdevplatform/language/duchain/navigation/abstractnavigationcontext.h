/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ABSTRACTNAVIGATIONCONTEXT_H
#define KDEVPLATFORM_ABSTRACTNAVIGATIONCONTEXT_H

#include "../indexeddeclaration.h"
#include "navigationaction.h"

#include <language/languageexport.h>
#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QSharedData>

namespace KDevelop {
class AbstractNavigationContextPrivate;

class AbstractNavigationContext;
using NavigationContextPointer = QExplicitlySharedDataPointer<AbstractNavigationContext>;

class KDEVPLATFORMLANGUAGE_EXPORT AbstractNavigationContext
    : public QObject
    , public QSharedData
{
    Q_OBJECT

public:
    explicit AbstractNavigationContext(const TopDUContextPointer& topContext = TopDUContextPointer(),
                                       AbstractNavigationContext* previousContext = nullptr);
    ~AbstractNavigationContext() override;

    int linkCount() const;
    bool nextLink();
    bool previousLink();
    bool up();
    bool down();
    void resetNavigation();

    NavigationContextPointer accept();
    NavigationContextPointer back();
    NavigationContextPointer accept(IndexedDeclaration decl);
    NavigationContextPointer acceptLink(const QString& link);
    NavigationAction currentAction() const;

    virtual QString name() const = 0;
    ///Here the context can return html to be displayed.
    ///NOTE: The DUChain must be locked while this is called.
    virtual QString html(bool shorten = false);
    ///Here the context can return a widget to be displayed.
    ///The widget stays owned by this navigation-context.
    ///The widget may have a signal "navigateDeclaration(KDevelop::IndexedDeclaration)".
    ///If that signal is emitted, the new declaration is navigated in the navigation-wdiget.
    virtual QWidget* widget() const;

    ///Whether the widget returned by widget() should take the maximum possible space.
    ///The default implementation returns true.
    virtual bool isWidgetMaximized() const;

    ///Returns whether this context's string has already been computed, and is up to date.
    ///After clear() was called, this returns false again.
    bool alreadyComputed() const;

    TopDUContextPointer topContext() const;
    void setTopContext(const TopDUContextPointer& context);

    void executeLink(const QString& link);
    NavigationContextPointer execute(const NavigationAction& action);

Q_SIGNALS:
    void contentsChanged();

protected:
    AbstractNavigationContext* previousContext() const;
    virtual void setPreviousContext(AbstractNavigationContext* previousContext);

    struct TextHandler
    {
        explicit TextHandler(AbstractNavigationContext* c) : context(c)
        {
        }
        void operator+=(const QString& str) const
        {
            context->addHtml(str);
        }
        AbstractNavigationContext* context;
    };

    ///Override this to execute own key-actions using NavigationAction
    virtual NavigationContextPointer executeKeyAction(const QString& key);

    ///Adds given the text to currentHtml()
    void addHtml(const QString& html);
    ///Returns the html text being built in its current state
    QString currentHtml() const;
    ///Returns a convenience object that allows writing "modifyHtml() += "Hallo";"
    TextHandler modifyHtml()
    {
        return TextHandler(this);
    }

    //Clears the computed html and links
    void clear();

    ///Creates and registers a link to the given declaration, labeled by the given name
    virtual void makeLink(const QString& name, const DeclarationPointer& declaration,
                          NavigationAction::Type actionType);

    ///Creates a link that executes the given action and adds it to the current context
    void makeLink(const QString& name, const QString& targetId, const NavigationAction& action);

    ///Creates a link that executes the given action and returns it
    QString createLink(const QString& name, const QString& targetId, const NavigationAction& action);

    NavigationContextPointer registerChild(const DeclarationPointer& /*declaration*/);
    NavigationContextPointer registerChild(AbstractNavigationContext* context);

    virtual QString declarationKind(const DeclarationPointer& decl);

    static QString typeHighlight(const QString& str);
    static QString errorHighlight(const QString& str);
    static QString labelHighlight(const QString& str);
    static QString codeHighlight(const QString& str);
    static QString propertyHighlight(const QString& str);
    static QString navigationHighlight(const QString& str);
    static QString importantHighlight(const QString& str);
    static QString commentHighlight(const QString& str);
    static QString nameHighlight(const QString& str);

private:
    const QScopedPointer<class AbstractNavigationContextPrivate> d_ptr;
    Q_DECLARE_PRIVATE(AbstractNavigationContext)
};

}

#endif
