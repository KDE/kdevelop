/*
 * KDevelop Debugger Support
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_VARIABLECOLLECTION_H
#define KDEVPLATFORM_VARIABLECOLLECTION_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QPointer>

#include <KLocalizedString>
#include <KTextEditor/TextHintInterface>

#include <debugger/debuggerexport.h>
#include "../util/treemodel.h"
#include "../util/treeitem.h"
#include "../../interfaces/idocument.h"
#include "../interfaces/idebugsession.h"
#include "../../interfaces/idebugcontroller.h"

namespace GDBDebugger {
    class GdbTest;
}

namespace KDevelop
{
class VariableToolTip;

class KDEVPLATFORMDEBUGGER_EXPORT Variable : public TreeItem
{
    Q_OBJECT
    friend class GDBDebugger::GdbTest;
public:
protected:
    Variable(TreeModel* model, TreeItem* parent,
             const QString& expression,
             const QString& display = "");

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

    ~Variable();

    /* Connects this variable to debugger, fetching the current value and
       otherwise preparing this variable to be displayed everywhere.  
       The attempt may fail, for example if the expression is invalid.
       Calls slot 'callbackMethod' in 'callback' to notify of the result.
       The slot should be taking 'bool ok' parameter.  */
    virtual void attachMaybe(QObject *callback = 0, const char *callbackMethod = 0) = 0;

    virtual bool canSetFormat() const { return false; }

    void setFormat(format_t format);
    format_t format() const { return m_format; }
    virtual void formatChanged();

    // get/set 'changed' state, if the variable changed it will be highlighted
    bool isChanged() const { return changed_; }
    void setChanged(bool c);
    void resetChanged();

public slots:
    void die();

protected:
    bool topLevel() const { return topLevel_; }

private: // TreeItem overrides

    QVariant data(int column, int role) const override;

private:

    QString expression_;
    bool inScope_;
    bool topLevel_;
    bool changed_;
    bool showError_;

    format_t m_format;
};

class KDEVPLATFORMDEBUGGER_EXPORT TooltipRoot : public TreeItem
{
public:
    TooltipRoot(TreeModel* model)
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
    friend class GDBDebugger::GdbTest;
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
public:
    Locals(TreeModel* model, TreeItem* parent, const QString &name);
    QList<Variable*> updateLocals(QStringList locals);
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
public:
    VariablesRoot(TreeModel* model);

    Watches *watches() const { return watches_; }
    Locals *locals(const QString &name = "Locals");
    QHash<QString, Locals*> allLocals() const;

    void fetchMoreChildren() override {}

    void resetChanged();

private:
    Watches *watches_;
    QHash<QString, Locals*> locals_;
};

class VariableProvider : public KTextEditor::TextHintProvider
{
public:
    VariableProvider(VariableCollection* collection);
    virtual QString textHint(KTextEditor::View* view, const KTextEditor::Cursor& position) Q_DECL_OVERRIDE;

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

    VariableCollection(IDebugController* parent);
    virtual ~VariableCollection();

    VariablesRoot* root() const { return universe_; }
    Watches* watches() const { return universe_->watches(); }
    Locals* locals(const QString &name = i18n("Locals")) const { return universe_->locals(name); }
    QHash<QString, Locals*> allLocals() const { return universe_->allLocals(); }

public Q_SLOTS:
    void variableWidgetShown();
    void variableWidgetHidden();

private Q_SLOTS:
    void updateAutoUpdate(KDevelop::IDebugSession* session = 0);

    void textDocumentCreated( KDevelop::IDocument*);
    void viewCreated(KTextEditor::Document*, KTextEditor::View*);

private:
    VariablesRoot* universe_;
    QPointer<VariableToolTip> activeTooltip_;
    bool m_widgetVisible;

    friend class VariableProvider;
};

}

#endif
