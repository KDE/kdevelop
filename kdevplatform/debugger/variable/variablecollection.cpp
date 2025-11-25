/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "variablecollection.h"

#include <QFont>
#include <QApplication>

#include <KColorScheme>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "../../interfaces/icore.h"
#include "../../interfaces/idocumentcontroller.h"
#include "../../interfaces/iuicontroller.h"
#include "../../sublime/controller.h"
#include "../../sublime/view.h"
#include "../../interfaces/idebugcontroller.h"
#include "../interfaces/idebugsession.h"
#include "../interfaces/ivariablecontroller.h"
#include <debug.h>
#include "util/texteditorhelpers.h"
#include "variabletooltip.h"
#include <sublime/area.h>

namespace KDevelop {

IDebugSession* currentSession()
{
    return ICore::self()->debugController()->currentSession();
}

IDebugSession::DebuggerState currentSessionState()
{
    if (!currentSession()) return IDebugSession::NotStartedState;
    return currentSession()->state();
}

bool hasStartedSession()
{
    IDebugSession::DebuggerState s = currentSessionState();
    return s != IDebugSession::NotStartedState && s != IDebugSession::EndedState;
}

Variable::Variable(TreeModel* model, TreeItem* parent,
                   const QString& expression,
                   const QString& display)
  : TreeItem(model, parent)
  , m_expression(expression)
  , m_inScope(true)
  , m_topLevel(true)
  , m_changed(false)
  , m_showError(false)
  , m_format(Natural)
{
    // FIXME: should not duplicate the data, instead overload 'data'
    // and return expression_ directly.
    if (display.isEmpty())
        setData(QVector<QVariant>{expression, QString(), QString()});
    else
        setData(QVector<QVariant>{display, QString(), QString()});
}

QString Variable::expression() const
{
    return m_expression;
}

bool Variable::inScope() const
{
    return m_inScope;
}

void Variable::setValue(const QString& v)
{
    itemData[VariableCollection::ValueColumn] = v;
    reportChange();
}

QString Variable::value() const
{
    return itemData[VariableCollection::ValueColumn].toString();
}

void Variable::setType(const QString& type)
{
    itemData[VariableCollection::TypeColumn] = type;
    reportChange();
}

QString Variable::type() const
{
    return itemData[VariableCollection::TypeColumn].toString();
}

void Variable::setTopLevel(bool v)
{
    m_topLevel = v;
}

void Variable::setInScope(bool v)
{
    m_inScope = v;
    for (int i=0; i < childCount(); ++i) {
        if (auto *var = qobject_cast<Variable*>(child(i))) {
            var->setInScope(v);
        }
    }
    reportChange();
}

void Variable::setShowError (bool v)
{
    m_showError = v;
    reportChange();
}

bool Variable::showError() const
{
    return m_showError;
}


Variable::~Variable()
{
}

void Variable::die()
{
    removeSelf();
    deleteLater();
}


void Variable::setChanged(bool c)
{
    m_changed=c;
    reportChange();
}

void Variable::resetChanged()
{
    setChanged(false);
    for (int i=0; i<childCount(); ++i) {
        TreeItem* childItem = child(i);
        if (qobject_cast<Variable*>(childItem)) {
            static_cast<Variable*>(childItem)->resetChanged();
        }
    }
}

Variable::format_t Variable::str2format(const QString& str)
{
    if(str==QLatin1String("Binary") || str==QLatin1String("binary"))          return Binary;
    if(str==QLatin1String("Octal") || str==QLatin1String("octal"))            return Octal;
    if(str==QLatin1String("Decimal") || str==QLatin1String("decimal"))        return Decimal;
    if(str==QLatin1String("Hexadecimal") || str==QLatin1String("hexadecimal"))return Hexadecimal;

    return Natural; // maybe most reasonable default
}

QString Variable::format2str(format_t format)
{
    switch(format) {
        case Natural:       return QStringLiteral("natural");
        case Binary:        return QStringLiteral("binary");
        case Octal:         return QStringLiteral("octal");
        case Decimal:       return QStringLiteral("decimal");
        case Hexadecimal:   return QStringLiteral("hexadecimal");
    }
    return QString();
}


void Variable::setFormat(Variable::format_t format)
{
    if (m_format != format) {
        m_format = format;
        formatChanged();
    }
}

void Variable::formatChanged()
{
}

bool Variable::isPotentialProblematicValue() const
{
    const auto value = data(VariableCollection::ValueColumn, Qt::DisplayRole).toString();
    return value == QLatin1String("0x0");
}

QVariant Variable::data(int column, int role) const
{
    if (m_showError) {
        if (role == Qt::FontRole) {
            QVariant ret = TreeItem::data(column, role);
            QFont font = ret.value<QFont>();
            font.setStyle(QFont::StyleItalic);
            return font;
        } else if (column == 1 && role == Qt::DisplayRole) {
            return i18n("Error");
        }
    }
    if (column == 1 && role == Qt::ForegroundRole)
    {
        KColorScheme scheme(QPalette::Active);
        if (!m_inScope) {
            return scheme.foreground(KColorScheme::InactiveText).color();
        } else if (isPotentialProblematicValue()) {
            return scheme.foreground(KColorScheme::NegativeText).color();
        } else if (m_changed) {
            return scheme.foreground(KColorScheme::NeutralText).color();
        }
    }
    if (role == Qt::ToolTipRole) {
        return TreeItem::data(column, Qt::DisplayRole);
    }

    return TreeItem::data(column, role);
}

QString Watches::sectionTitle()
{
    return i18n("Auto");
}

Watches::Watches(TreeModel* model, TreeItem* parent)
: TreeItem(model, parent), finishResult_(nullptr)
{
    setData(QVariantList{sectionTitle(), QString()});
}

Variable* Watches::add(const QString& expression)
{
    if (!hasStartedSession()) return nullptr;

    Variable* v = currentSession()->variableController()->createVariable(
        model(), this, expression);
    appendChild(v);
    v->attachMaybe();
    if (childCount() == 1 && !isExpanded()) {
        setExpanded(true);
    }
    return v;
}

Variable *Watches::addFinishResult(const QString& convenienceVarible)
{
    if( finishResult_ )
    {
        removeFinishResult();
    }
    finishResult_ = currentSession()->variableController()->createVariable(
        model(), this, convenienceVarible, QStringLiteral("$ret"));
    appendChild(finishResult_);
    finishResult_->attachMaybe();
    if (childCount() == 1 && !isExpanded()) {
        setExpanded(true);
    }
    return finishResult_;
}

void Watches::removeFinishResult()
{
    if (finishResult_)
    {
        finishResult_->die();
        finishResult_ = nullptr;
    }
}

void Watches::resetChanged()
{
    for (int i=0; i<childCount(); ++i) {
        TreeItem* childItem = child(i);
        if (qobject_cast<Variable*>(childItem)) {
            static_cast<Variable*>(childItem)->resetChanged();
        }
    }
}

QVariant Watches::data(int column, int role) const
{
#if 0
    if (column == 0 && role == Qt::FontRole)
    {
        /* FIXME: is creating font again and again efficient? */
        QFont f = font();
        f.setBold(true);
        return f;
    }
#endif
    return TreeItem::data(column, role);
}

void Watches::reinstall()
{
    for (int i = 0; i < childItems.size(); ++i)
    {
        auto* v = static_cast<Variable*>(child(i));
        v->attachMaybe();
    }
}

Locals::Locals(TreeModel* model, TreeItem* parent, const QString &name)
: TreeItem(model, parent)
{
    setData(QVector<QVariant>{name, QString()});
}

QList<Variable*> Locals::updateLocals(const QStringList& locals)
{
    QSet<QString> existing, current;
    for (int i = 0; i < childItems.size(); i++)
    {
        Q_ASSERT(qobject_cast<KDevelop::Variable*>(child(i)));
        auto* var= static_cast<KDevelop::Variable*>(child(i));
        existing << var->expression();
    }

    for (const QString& var : locals) {
        current << var;
        // If we currently don't display this local var, add it.
        if( !existing.contains( var ) ) {
            // FIXME: passing variableCollection this way is awkward.
            // In future, variableCollection probably should get a
            // method to create variable.
            Variable* v =
                currentSession()->variableController()->createVariable(
                    ICore::self()->debugController()->variableCollection(),
                    this, var );
            appendChild( v, false );
        }
    }

    for (int i = 0; i < childItems.size(); ++i) {
        auto* v = static_cast<KDevelop::Variable*>(child(i));
        if (!current.contains(v->expression())) {
            removeChild(i);
            --i;
            // FIXME: check that -var-delete is sent.
            delete v;
        }
    }


    if (hasMore()) {
        setHasMore(false);
    }

    // Variables which changed just value are updated by a call to -var-update.
    // Variables that changed type -- likewise.

    QList<Variable*> ret;
    ret.reserve(childItems.size());
    for (TreeItem* i : std::as_const(childItems)) {
        Q_ASSERT(qobject_cast<Variable*>(i));
        ret << static_cast<Variable*>(i);
    }
    return ret;
}

void Locals::resetChanged()
{
    for (int i=0; i<childCount(); ++i) {
        TreeItem* childItem = child(i);
        if (qobject_cast<Variable*>(childItem)) {
            static_cast<Variable*>(childItem)->resetChanged();
        }
    }
}

VariablesRoot::VariablesRoot(TreeModel* model)
    : TreeItem(model)
    , m_watches(new Watches(model, this))
{
    appendChild(m_watches, true);
}


Locals* VariablesRoot::locals(const QString& name)
{
    auto localsIt = m_locals.find(name);
    if (localsIt == m_locals.end()) {
        localsIt = m_locals.insert(name, new Locals(model(), this, name));
        appendChild(*localsIt);
    }
    return *localsIt;
}

QHash<QString, Locals*> VariablesRoot::allLocals() const
{
    return m_locals;
}

void VariablesRoot::resetChanged()
{
    m_watches->resetChanged();
    for (Locals* l : std::as_const(m_locals)) {
        l->resetChanged();
    }
}

QString VariableCollection::defaultLocalsSectionTitle()
{
    return i18n("Locals");
}

VariableCollection::VariableCollection(IDebugController* controller)
    : TreeModel({i18n("Name"), i18n("Value"), i18n("Type")}, controller)
    , m_widgetVisible(false)
    , m_textHintProvider(this)
{
    m_universe = new VariablesRoot(this);
    setRootItem(m_universe);

    //new ModelTest(this);

    connect (ICore::self()->documentController(),
         &IDocumentController::textDocumentCreated,
         this,
         &VariableCollection::textDocumentCreated );

    connect(controller, &IDebugController::currentSessionChanged,
             this, &VariableCollection::updateAutoUpdate);

    // Qt5 signal slot syntax does not support default arguments
    auto callUpdateAutoUpdate = [&] { updateAutoUpdate(); };

    connect(locals(), &Locals::expanded, this, callUpdateAutoUpdate);
    connect(locals(), &Locals::collapsed, this, callUpdateAutoUpdate);
    connect(watches(), &Watches::expanded, this, callUpdateAutoUpdate);
    connect(watches(), &Watches::collapsed, this, callUpdateAutoUpdate);
}

void VariableCollection::variableWidgetHidden()
{
    m_widgetVisible = false;
    updateAutoUpdate();
}

void VariableCollection::variableWidgetShown()
{
    m_widgetVisible = true;
    updateAutoUpdate();
}

void VariableCollection::updateAutoUpdate(IDebugSession* session)
{
    if (!session) session = currentSession();
    qCDebug(DEBUGGER) << session;
    if (!session) return;

    if (!m_widgetVisible) {
        session->variableController()->setAutoUpdate(IVariableController::UpdateNone);
    } else {
        QFlags<IVariableController::UpdateType> t = IVariableController::UpdateNone;
        if (locals()->isExpanded()) t |= IVariableController::UpdateLocals;
        if (watches()->isExpanded()) t |= IVariableController::UpdateWatches;
        session->variableController()->setAutoUpdate(t);
    }
}

VariableCollection::~ VariableCollection()
{
    for (auto* view : std::as_const(m_textHintProvidedViews)) {
        view->unregisterTextHintProvider(&m_textHintProvider);
    }
}

void VariableCollection::textDocumentCreated(IDocument* doc)
{
  connect( doc->textDocument(),
       &KTextEditor::Document::viewCreated,
       this, &VariableCollection::viewCreated );

  const auto views = doc->textDocument()->views();
  for (KTextEditor::View* view : views) {
    viewCreated( doc->textDocument(), view );
  }
}

void VariableCollection::viewCreated(KTextEditor::Document* doc,
                                     KTextEditor::View* view)
{
    Q_UNUSED(doc);
    Q_ASSERT(view);

    if (m_textHintProvidedViews.contains(view)) {
        return;
    }
    connect(view, &QObject::destroyed, this, [this, view](QObject* obj) {
        Q_ASSERT(obj == view);
        m_textHintProvidedViews.removeOne(view);
    });

    view->registerTextHintProvider(&m_textHintProvider);
    m_textHintProvidedViews.append(view);
}

Locals* VariableCollection::locals(const QString &name) const
{
    return m_universe->locals(name.isEmpty() ? defaultLocalsSectionTitle() : name);
}

VariableProvider::VariableProvider(VariableCollection* collection)
    : KTextEditor::TextHintProvider()
    , m_collection(collection)
{
}

QString VariableProvider::textHint(KTextEditor::View* view, const KTextEditor::Cursor& cursor)
{
    if (!hasStartedSession())
        return QString();

    if (ICore::self()->uiController()->activeArea()->objectName() != QLatin1String("debug"))
        return QString();

    //TODO: These keyboardModifiers should also hide already opened tooltip, and show another one for code area.
    if (QApplication::keyboardModifiers() == Qt::ControlModifier ||
        QApplication::keyboardModifiers() == Qt::AltModifier){
        return QString();
    }

    KTextEditor::Document* doc = view->document();

    KTextEditor::Range expressionRange = currentSession()->variableController()->expressionRangeUnderCursor(doc, cursor);

    if (!expressionRange.isValid())
        return QString();
    QString expression = doc->text(expressionRange).trimmed();

    // Don't do anything if there's already an open tooltip with matching range
    if (m_collection->m_activeTooltip && m_collection->m_activeTooltip->variable()->expression() == expression)
        return QString();
    if (expression.isEmpty())
        return QString();

    QPoint local = view->cursorToCoordinate(cursor);
    QPoint global = view->mapToGlobal(local);
    QWidget* w = view->childAt(local);
    if (!w)
        w = view;

    m_collection->m_activeTooltip = new VariableToolTip(w, global+QPoint(30,30), expression);
    m_collection->m_activeTooltip->setHandleRect(KTextEditorHelpers::itemBoundingRect(view, expressionRange));
    return QString();
}

}

#include "moc_variablecollection.cpp"
