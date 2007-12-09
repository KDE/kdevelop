/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "variableitem.h"

#include <QRegExp>
#include <QApplication>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "gdbparser.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"
#include "gdbglobal.h"
#include "variablecollection.h"

using namespace GDBMI;
using namespace GDBDebugger;

int VariableItem::varobjIndex = 0;

VariableItem::VariableItem(AbstractVariableItem* parent)
    : AbstractVariableItem(parent),
      highlight_(false),
      oldSpecialRepresentationSet_(false),
      format_(natural),
      numChildren_(0),
      updateUnconditionally_(false),
      frozen_(false),
      initialCreation_(true),
      baseClassMember_(false),
      alive_(true)
{
}

void VariableItem::setExpression(const QString & expression)
{
    expression_ = expression;
}

void VariableItem::setFrozen(bool frozen)
{
    if (frozen_ != frozen) {
        frozen_ = frozen;

        if (isRegisteredWithGdb())
            controller()->addCommand(new GDBCommand(VarSetFrozen, QString("%1 %2").arg(frozen_ ? "1" : "0").arg(expression_)));
    }
}

void VariableItem::setVariableObject(const GDBMI::Value & varobj, FormatTypes format, bool baseClassMember)
{
    expression_ = varobj["exp"].literal();
    varobjName_ = varobj["name"].literal();
    format_ = format;
    baseClassMember_ = baseClassMember;

    // Set type and children.
    originalValueType_ = varobj["type"].literal();
    numChildren_ = varobj["numchild"].literal().toInt();

    updateValue();
}

void VariableItem::registerWithGdb()
{
    AbstractVariableItem::registerWithGdb();

    if (varobjName_.isEmpty()) {
        // User might have entered format together with expression: like
        //   /x i1+i2
        // If we do nothing, it will be impossible to watch the variable in
        // different format, as we'll just add extra format specifier.
        // So:
        //   - detect initial value of format_
        //   - remove the format specifier from the string.
        static QRegExp explicit_format("^\\s*/(.)\\s*(.*)");
        if (explicit_format.indexIn(expression_) == 0)
        {
            format_ = formatFromGdbModifier(explicit_format.cap(1)[0].toLatin1());
            expression_ = explicit_format.cap(2);
        }

        createVarobj();

    } else {
        collection()->addVariableObject(varobjName_, this);
    }
}

void VariableItem::createVarobj()
{
    if (!varobjName_.isEmpty())
        collection()->removeVariableObject(varobjName_);

    varobjName_ = QString("KDEV%1").arg(varobjIndex++);

    collection()->addVariableObject(varobjName_, this);

    if (frozen_)
    {
        // MI had no way to freeze a variable object. So, we
        // issue print command that returns $NN convenience
        // variable and we create variable object from that.
        // TODO check version this was introduced into
        /*controller()->addCommand(
            new CliCommand(NonMI,
                QString("print %1").arg(expression_),
                this,
                &VariableItem::handleCliPrint));*/

        controller()->addCommand(new GDBCommand(VarSetFrozen, QString("1 %1").arg(expression_)));
    }
    else
    {
        controller()->addCommand(
            new CliCommand(NonMI,
                QString("print /x &%1").arg(expression_),
                this,
                &VariableItem::handleCurrentAddress,
                true));

        controller()->addCommand(
            // Need to quote expression, otherwise gdb won't like
            // spaces inside it.
            new GDBCommand(VarCreate, QString("%1 * \"%2\"")
                           .arg(varobjName_)
                           .arg(expression_),
                           this,
                           &VariableItem::varobjCreated,
                           initialCreation_ ? false : true));
    }
}

void VariableItem::varobjCreated(const GDBMI::ResultRecord& r)
{
    // If we've tried to recreate varobj (for example for watched expression)
    // after step, and it's no longer valid, it's fine.
    if (r.reason == "error")
    {
        varobjName_ = "";
        return;
    }
    setAliveRecursively(true);

    QString oldType = originalValueType_;
    originalValueType_ = r["type"].literal();
    if (!oldType.isEmpty() && oldType != originalValueType_)
    {
        deleteAllChildren();
    }

    if (r.hasField("exp"))
        expression_ = r["exp"].literal();
    numChildren_ = r["numchild"].literal().toInt();
    currentAddress_ = lastObtainedAddress_;

    setVarobjName(varobjName_);
}

void VariableItem::setVarobjName(const QString& name)
{
    if (!varobjName_.isEmpty())
        collection()->removeVariableObject(varobjName_);

    varobjName_ = name;

    collection()->addVariableObject(varobjName_, this);

    if (format_ != natural)
    {
        controller()->addCommand(
            new GDBCommand(VarSetFormat, QString("\"%1\" %2")
                           .arg(varobjName_).arg(varobjFormatName())));
    }

    // Get the initial value.
    updateValue();

//     if (isOpen())
//     {
//         // This regets children list.
//         setOpen(true);
//     }
}

void VariableItem::valueDone(const GDBMI::ResultRecord& r)
{
    QString s;

    if (r.reason == "done")
    {
        s = GDBParser::getGDBParser()->undecorateValue(
            r["value"].literal());

        if (format_ == character)
        {
            QString encoded = s;
            bool ok;
            int value = s.toInt(&ok);
            if (ok)
            {
                char c = (char)value;
                encoded += " '";
                if (std::isprint(c))
                    encoded += c;
                else {
                    // Try common escape characters.
                    static char *backslashed[] = {"a", "b", "f", "n",
                                                  "r", "t", "v", "0"};
                    static char represented[] = "\a\b\f\n\r\t\v";

                    const char* ix = strchr (represented, c);
                    if (ix) {
                        encoded += "\\";
                        encoded += backslashed[ix - represented];
                    }
                    else
                        encoded += "\\" + s;
                }
                encoded += "'";
                s = encoded;
            }
        }

        if (format_ == binary)
        {
            // For binary format, split the value at 4-bit boundaries
            static QRegExp r("^[01]+$");
            int i = r.indexIn(s);
            if (i == 0)
            {
                QString split;
                for(int i = 0; i < s.length(); ++i)
                {
                    // For string 11111, we should split it as
                    // 1 1111, not as 1111 1.

                    // 0 is past the end character
                    int distance = i - s.length();

                    if (distance % 4 == 0 && !split.isEmpty())
                        split.append(' ');
                    split.append(s[i]);
                }
                s = split;
            }
        }
    }
    else
    {
        s = r["msg"].literal();
        // Error response.
        if (s.startsWith("Cannot access memory"))
        {
            s = "(inaccessible)";
        }
    }

    if (m_value != s) {
        m_value = s;
        collection()->dataChanged(this, ColumnValue);
    }
}

void VariableItem::createChildren(const GDBMI::ResultRecord& r,
                             bool children_of_fake)
{
    if (!r.hasField("children"))
        return;

    const GDBMI::Value& children = r["children"];

    /* In order to figure out which variable objects correspond
       to base class subobject, we first must detect if *this
       is a structure type. We use present of 'public'/'private'/'protected'
       fake child as an indicator. */
    bool structureType = false;
    if (!children_of_fake && children.size() > 0)
    {
        QString exp = children[0]["exp"].literal();
        bool ok = false;
        exp.toInt(&ok);
        if (!ok || exp[0] != '*')
        {
            structureType = true;
        }
    }

    for (int i = 0; i < children.size(); ++i)
    {
        QString exp = children[i]["exp"].literal();
        // For artificial accessibility nodes,
        // fetch their children.
        if (exp == "public" || exp == "protected" || exp == "private")
        {
            QString name = children[i]["name"].literal();
            controller()->addCommand(new GDBCommand(VarListChildren,
                                        "\"" +
                                        name + "\"",
                                        this,
                                        &VariableItem::childrenOfFakesDone));
        }
        else
        {
            /* All children of structures that are not artifical
               are base subobjects. */
            bool baseObject = structureType;

            VariableItem* existing = 0;
            foreach (AbstractVariableItem* child, AbstractVariableItem::children())
            {
                VariableItem* v = qobject_cast<VariableItem*>(child);
                kDebug(9012) << "Child exp:" << v->expression_ <<
                    "new exp" << exp;

                if (v->expression_ == exp)
                {
                    existing = v;
                }
            }

            if (existing)
            {
                existing->setVarobjName(children[i]["name"].literal());
            }
            else
            {
                kDebug(9012) << "Creating new varobj" << exp << baseObject;
                // Propagate format from parent.
                VariableItem* v = 0;
                v = new VariableItem(this);
                v->setVariableObject(children[i], format_, baseObject);
                addChild(v);
            }
        }
    }
}


void VariableItem::childrenDone(const GDBMI::ResultRecord& r)
{
    createChildren(r, false);
}

void VariableItem::childrenOfFakesDone(const GDBMI::ResultRecord& r)
{
    createChildren(r, true);
}

void VariableItem::handleCurrentAddress(const QStringList& lines)
{
    lastObtainedAddress_ = "";
    if (lines.count() > 1)
    {
        static QRegExp r("\\$\\d+ = ([^\n]*)");
        int i = r.indexIn(lines[1]);
        if (i == 0)
        {
            lastObtainedAddress_ = r.cap(1);
            kDebug(9012) << "new address" << lastObtainedAddress_;
        }
    }
}

void VariableItem::handleType(const QStringList& lines)
{
    bool recreate = false;

    if (lastObtainedAddress_ != currentAddress_)
    {
        kDebug(9012) << "Address changed from" << currentAddress_
                      << "to" << lastObtainedAddress_;
        recreate = true;
    }
    else
    {
        // FIXME: add error diagnostic.
        if (lines.count() > 1)
        {
            static QRegExp r("type = ([^\n]*)");
            int i = r.indexIn(lines[1]);
            if (i == 0)
            {
                kDebug(9012) << "found type:" << r.cap(1);
                kDebug(9012) << "original Type:" << originalValueType_;

                if (r.cap(1) != originalValueType_)
                {
                    recreate = true;
                }
            }
        }
    }
    if (recreate)
    {
        this->recreate();
    }
}

QString VariableItem::displayName() const
{
    if (!expression_.isEmpty() && expression_[0] != '*')
        return expression_;

    if (VariableItem* p = parentItem())
    {
        return "*" + p->displayName();
    }
    else
    {
        return expression_;
    }
}

void VariableItem::setVariableName(const QString & name)
{
    expression_ = name;
}

void VariableItem::setAliveRecursively(bool enable)
{
    alive_ = true;

    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* v = qobject_cast<VariableItem*>(child))
            v->setAliveRecursively(enable);
    }
}

QString VariableItem::gdbExpression() const
{
    // The expression for this item can be either:
    //  - number, for array element
    //  - identifier, for member,
    //  - ***intentifier, for derefenreced pointer.
    const VariableItem* parent = parentItem();

    bool ok = false;
    expression_.toInt(&ok);
    if (ok)
    {
        // Array, parent always exists.
        return parent->gdbExpression() + "[" + expression_ + "]";
    }
    else if (expression_[0] == '*')
    {
        if (parent)
        {
            // For MI, expression_ can be "*0" (meaing
            // references 0-th element of some array).
            // So, we really need to get to the parent to computed the right
            // gdb expression.
            return "*" + parent->gdbExpression();
        }
        else
        {
            // Parent can be null for watched expressions. In that case,
            // expression_ should be a valid C++ expression.
            return expression_;
        }
    }
    else
    {
        if (parent)
            /* This is varitem corresponds to a base suboject,
               the expression should cast parent to the base's
               type. */
            if (baseClassMember_)
                return "((" + expression_ + ")" + parent->gdbExpression() + ")";
            else
                return parent->gdbExpression() + "." + expression_;
        else
            return expression_;
    }
}

// **************************************************************************


void VariableItem::clearHighlight()
{
    highlight_ = false;

    foreach (AbstractVariableItem* child, children())
    {
        if (VariableItem* v = qobject_cast<VariableItem*>(child))
            v->clearHighlight();
    }
}

// **************************************************************************

void VariableItem::updateValue()
{
/*    if (handleSpecialTypes())
    {
        // 1. Gdb never includes structures in output from -var-update
        // 2. Even if it did, the internal state of object can be
        //    arbitrary complex and gdb can't detect if pretty-printed
        //    value remains the same.
        // So, we need to reload value on each step.
        updateUnconditionally_ = true;
        return;
    }*/
    updateUnconditionally_ = false;

    controller()->addCommand(
        new GDBCommand(VarEvaluateExpression,
            "\"" + varobjName_ + "\"",
            this,
            &VariableItem::valueDone,
            true /* handle error */));
}

void VariableItem::setValue(const QString& new_value)
{
    controller()->addCommand(
        new GDBCommand(VarAssign, QString("\"%1\" %2").arg(varobjName_)
                       .arg(new_value)));

    // And immediately reload it from gdb,
    // so that it's display format is the one gdb uses,
    // not the one user has typed. Otherwise, on the next
    // step, the visible value might change and be highlighted
    // as changed, which is bogus.
    updateValue();
}

void VariableItem::updateSpecialRepresentation(const QString& xs)
{
    QString s(xs);
    if (s[0] == '$')
    {
        int i = s.indexOf('=');
        if (i != -1)
            s = s.mid(i+2);
    }

    // A hack to nicely display QStrings. The content of QString is unicode
    // for for ASCII only strings we get ascii character mixed with \000.
    // Remove those \000 now.

    // This is not very nice, becuse we're doing this unconditionally
    // and this method can be called twice: first with data that gdb sends
    // for a variable, and second after we request the string data. In theory
    // the data sent by gdb might contain \000 that should not be translated.
    //
    // What's even worse, ideally we should convert the string data from
    // gdb into a QString again, handling all other escapes and composing
    // one QChar from two characters from gdb. But to do that, we *should*
    // now if the data if generic gdb value, and result of request for string
    // data. Fixing is is for later.
    s.replace( QRegExp("\\\\000|\\\\0"), "" );

    // FIXME: for now, assume that all special representations are
    // just strings.

    m_value = GDBParser::getGDBParser()->undecorateValue(s);

    // On the first stop, when VariableItem was just created,
    // don't show it in red.
    if (oldSpecialRepresentationSet_)
        highlight_ = (oldSpecialRepresentation_ != s);
    else
        highlight_ = false;

    oldSpecialRepresentationSet_ = true;
    oldSpecialRepresentation_ = s;
}

void VariableItem::recreateLocallyMaybe()
{
    controller()->addCommand(
        new CliCommand(NonMI,
            QString("print /x &%1").arg(expression_),
            this,
            &VariableItem::handleCurrentAddress,
            true));

    controller()->addCommand(
        new CliCommand(NonMI,
            QString("whatis %1").arg(expression_),
            this,
            &VariableItem::handleType));
}

void VariableItem::recreate()
{
    deregisterWithGdb();

    initialCreation_ = false;

    registerWithGdb();
}


// **************************************************************************

void VariableItem::refresh()
{
    if (isDirty())
    {
        setDirty(false);

        controller()->addCommand(new GDBCommand(VarListChildren,
                                    "\"" + varobjName_ + "\"",
                                    this,
                                    &VariableItem::childrenDone));
    }
}

/*bool VariableItem::handleSpecialTypes()
{
    kDebug(9012) << "handleSpecialTypes:" << originalValueType_;
    if (originalValueType_.isEmpty())
        return false;

    static QRegExp qstring("^(const)?[ ]*QString[ ]*&?$");

    if (qstring.exactMatch(originalValueType_)) {

        VariableTree* varTree = static_cast<VariableTree*>(listView());

        varTree->controller()->addCommand(
            new ResultlessCommand(NonMI, QString("print $kdev_d=%1.d")
                                  .arg(gdbExpression()),
                                  true / ignore error /));

        if (varTree->controller()->qtVersion() >= 4)
            varTree->controller()->addCommand(
                new ResultlessCommand(NonMI, QString("print $kdev_s=$kdev_d.size"),
                                      true));
        else
            varTree->controller()->addCommand(
                new ResultlessCommand(NonMI, QString("print $kdev_s=$kdev_d.len"),
                                      true));

        varTree->controller()->addCommand(
            new ResultlessCommand(NonMI,
                QString("print $kdev_s= ($kdev_s > 0)? ($kdev_s > 100 ? 200 : 2*$kdev_s) : 0"),
                true));

        if (varTree->controller()->qtVersion() >= 4)
            varTree->controller()->addCommand(
                new ValueSpecialRepresentationCommand(NonMI,
                    this, "print ($kdev_s>0) ? (*((char*)&$kdev_d.data[0])@$kdev_s) : \"\""));
        else
            varTree->controller()->addCommand(
                new ValueSpecialRepresentationCommand(NonMI,
                    this, "print ($kdev_s>0) ? (*((char*)&$kdev_d.unicode[0])@$kdev_s) : \"\""));

        return true;
    }

    return false;
}*/

// **************************************************************************

VariableItem::FormatTypes VariableItem::format() const
{
    return format_;
}

void VariableItem::setFormat(FormatTypes f)
{
    if (f == format_)
        return;

    format_ = f;

    if (!children().isEmpty())
    {
        // If variable has children, change format for children.
        // - for structures, that's clearly right
        // - for arrays, that's clearly right
        // - for pointers, this can be confusing, but nobody ever wants to
        //   see the pointer in decimal!
        foreach (AbstractVariableItem* child, children())
        {
            if (VariableItem* v = qobject_cast<VariableItem*>(child))
                v->setFormat(f);
        }
    }
    else
    {
         controller()->addCommand(
            new GDBCommand(VarSetFormat, QString("\"%1\" %2")
                           .arg(varobjName_).arg(varobjFormatName())));

        updateValue();
    }
}

VariableItem::FormatTypes VariableItem::formatFromGdbModifier(char c) const
{
    FormatTypes nf;
    switch(c)
    {
    case 'n': // Not quite gdb modifier, but used in our UI.
        nf = natural; break;
    case 'x':
        nf = hexadecimal; break;
    case 'd':
        nf = decimal; break;
    case 'c':
        nf = character; break;
    case 't':
        nf = binary; break;
    default:
        nf = natural; break;
    }
    return nf;
}

QString VariableItem::varobjFormatName() const
{
    switch(format_)
    {
    case natural:
        return "natural";
        break;

    case hexadecimal:
        return "hexadecimal";
        break;

    case decimal:
        return "decimal";
        break;

        // Note: gdb does not support 'character' natively,
        // so we'll generate appropriate representation
        // ourselfs.
    case character:
        return "decimal";
        break;

    case binary:
        return "binary";
        break;
    }
    return "<undefined>";
}


void VariableItem::deregisterWithGdb()
{
    deleteAllChildren();

    AbstractVariableItem::deregisterWithGdb();

    alive_ = false;
    setDirty(true);

    collection()->removeVariableObject(varobjName_);

    if (!controller()->stateIsOn(s_dbgNotStarted) && !varobjName_.isEmpty())
    {
        controller()->addCommand(
            new GDBCommand(VarDelete,
                QString("\"%1\"").arg(varobjName_)));
    }

    varobjName_ = "";
}

bool VariableItem::updateUnconditionally() const
{
    return updateUnconditionally_;
}

bool VariableItem::isAlive() const
{
    return alive_;
}

Qt::ItemFlags VariableItem::flags(int column) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    if (alive_)
        flags |= Qt::ItemIsEnabled;

    if (column == ColumnName && !expression_.isEmpty() && expression_[0] != '*')    // Allow to change variable name by editing.
        flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant VariableItem::data(int column, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            switch (column) {
                case ColumnName:
                    return displayName();

                case ColumnValue:
                    return m_value;

                case ColumnType:
                    return m_type;
            }

        case Qt::ToolTipRole: {
            const int maxTooltipSize = 70;
            QString tip = displayName();

            if (tip.length() > maxTooltipSize)
                tip = tip.mid(0, maxTooltipSize - 1 ) + " [...]";

            if (!tip.isEmpty())
                tip += "\n" + originalValueType_;

            return tip;
        }
    }

    return QVariant();
}

void VariableItem::handleCliPrint(const QStringList& lines)
{
    static QRegExp r("(\\$[0-9]+)");
    if (lines.size() >= 2)
    {
        int i = r.indexIn(lines[1]);
        if (i == 0)
        {
            controller()->addCommand(
                new GDBCommand(VarCreate, QString("%1 * \"%2\"")
                               .arg(varobjName_)
                               .arg(r.cap(1)),
                               this,
                               &VariableItem::varobjCreated,
                               // On initial create, errors get reported
                               // by generic code. After then, errors
                               // are swallowed by varobjCreated.
                               initialCreation_ ? false : true));
        }
        else
        {
            // FIXME: merge all output lines together.
            // FIXME: add 'debuggerError' to debuggerpart.
            KMessageBox::information(
                qApp->activeWindow(),
                i18n("<b>Debugger error</b><br>%1", lines[1]),
                i18n("Debugger error"));
        }
    }
}

VariableItem * GDBDebugger::VariableItem::parentItem() const
{
    return qobject_cast<VariableItem*>(const_cast<QObject*>(QObject::parent()));
}

const QString & GDBDebugger::VariableItem::variableName() const
{
    return expression_;
}

bool GDBDebugger::VariableItem::hasChildren() const
{
    return numChildren_ || AbstractVariableItem::hasChildren();
}

#include "variableitem.moc"
