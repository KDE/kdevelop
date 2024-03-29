/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_VARIABLECOLLECTION_H
#define KDEVPLATFORM_VARIABLECOLLECTION_H

#include <QPointer>

#include <KLocalizedString>
#include <KTextEditor/TextHintInterface>

#include <debugger/debuggerexport.h>
#include "../util/treemodel.h"
#include "../util/treeitem.h"
#include "../../interfaces/idocument.h"
#include "../../interfaces/idebugcontroller.h"

namespace KDevMI { namespace GDB {
    class GdbTest;
}
}

namespace KDevelop
{
class VariableToolTip;
class IDebugSession;

class KDEVPLATFORMDEBUGGER_EXPORT Variable : public TreeItem
{
    Q_OBJECT
    friend class KDevMI::GDB::GdbTest;
public:
protected:
    Variable(TreeModel* model, TreeItem* parent,
             const QString& expression,
             const QString& display = {});

public:
    enum format_t { Natural, Binary, Octal, Decimal, Hexadecimal };
    static format_t str2format(const QString& str);
    static QString format2str(format_t format);

    QString expression() const;
    bool inScope() const;
    void setInScope(bool v);
    void setValue(const QString &v);
    QString value() const;
    void setType(const QString& type);
    QString type() const;
    void setTopLevel(bool v);
    void setShowError(bool v);
    bool showError();

    using TreeItem::setHasMore;
    using TreeItem::setHasMoreInitial;
    using TreeItem::appendChild;
    using TreeItem::deleteChildren;
    using TreeItem::isExpanded;
    using TreeItem::parent;

    using TreeItem::model;

    ~Variable() override;

    /* Connects this variable to debugger, fetching the current value and
       otherwise preparing this variable to be displayed everywhere.  
       The attempt may fail, for example if the expression is invalid.
       Calls slot 'callbackMethod' in 'callback' to notify of the result.
       The slot should be taking 'bool ok' parameter.  */
    virtual void attachMaybe(QObject *callback = nullptr, const char *callbackMethod = nullptr) = 0;

    virtual bool canSetFormat() const { return false; }

    void setFormat(format_t format);
    format_t format() const { return m_format; }
    virtual void formatChanged();

    // get/set 'changed' state, if the variable changed it will be highlighted
    bool isChanged() const { return m_changed; }
    void setChanged(bool c);
    void resetChanged();

public Q_SLOTS:
    void die();

protected:
    bool topLevel() const { return m_topLevel; }

private: // TreeItem overrides
    QVariant data(int column, int role) const override;

private:
    bool isPotentialProblematicValue() const;

    QString m_expression;
    bool m_inScope;
    bool m_topLevel;
    bool m_changed;
    bool m_showError;

    format_t m_format;
};

class KDEVPLATFORMDEBUGGER_EXPORT TooltipRoot : public TreeItem
{
    Q_OBJECT
public:
    explicit TooltipRoot(TreeModel* model)
    : TreeItem(model)
    {}

    void init(Variable *var)
    {
        appendChild(var);
    }

    void fetchMoreChildren() override {}
};

class KDEVPLATFORMDEBUGGER_EXPORT Watches : public TreeItem
{
    Q_OBJECT
    friend class KDevMI::GDB::GdbTest;
public:
    Watches(TreeModel* model, TreeItem* parent);
    Variable* add(const QString& expression);

    void reinstall();

    Variable *addFinishResult(const QString& convenienceVarible);
    void removeFinishResult();
    void resetChanged();

    using TreeItem::childCount;
    friend class VariableCollection;
    friend class IVariableController;
private:

    QVariant data(int column, int role) const override;

    void fetchMoreChildren() override {}

    Variable* finishResult_;
};

class KDEVPLATFORMDEBUGGER_EXPORT Locals : public TreeItem
{
    Q_OBJECT
public:
    Locals(TreeModel* model, TreeItem* parent, const QString &name);
    QList<Variable*> updateLocals(const QStringList& locals);
    void resetChanged();

    using TreeItem::deleteChildren;
    using TreeItem::setHasMore;

    friend class VariableCollection;
    friend class IVariableController;
    
private:
    void fetchMoreChildren() override {}
};

class KDEVPLATFORMDEBUGGER_EXPORT VariablesRoot : public TreeItem
{
    Q_OBJECT
public:
    explicit VariablesRoot(TreeModel* model);

    Watches *watches() const { return m_watches; }
    Locals *locals(const QString &name = QStringLiteral("Locals"));
    QHash<QString, Locals*> allLocals() const;

    void fetchMoreChildren() override {}

    void resetChanged();

private:
    Watches *m_watches;
    QHash<QString, Locals*> m_locals;
};

class VariableProvider : public KTextEditor::TextHintProvider
{
public:
    explicit VariableProvider(VariableCollection* collection);
    QString textHint(KTextEditor::View* view, const KTextEditor::Cursor& position) override;

private:
    VariableCollection* m_collection;
};

class KDEVPLATFORMDEBUGGER_EXPORT VariableCollection : public TreeModel
{
    Q_OBJECT

public:
    enum Column {
        NameColumn,
        ValueColumn,
        TypeColumn
    };

    explicit VariableCollection(IDebugController* parent);
    ~VariableCollection() override;

    VariablesRoot* root() const { return m_universe; }
    Watches* watches() const { return m_universe->watches(); }
    Locals* locals(const QString &name = QString()) const;
    QHash<QString, Locals*> allLocals() const { return m_universe->allLocals(); }

public Q_SLOTS:
    void variableWidgetShown();
    void variableWidgetHidden();

private Q_SLOTS:
    void updateAutoUpdate(KDevelop::IDebugSession* session = nullptr);

    void textDocumentCreated( KDevelop::IDocument*);
    void viewCreated(KTextEditor::Document*, KTextEditor::View*);

private:
    VariablesRoot* m_universe;
    QPointer<VariableToolTip> m_activeTooltip;
    bool m_widgetVisible;

    friend class VariableProvider;
    VariableProvider m_textHintProvider;

    QVector<KTextEditor::View*> m_textHintProvidedViews;
};

}

#endif
