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

void VariableItem::handleCliPrint(const QStringList& lines)
{
    static QRegExp r("(\\$[0-9]+)");
    if (lines.size() >= 2)
    {
        int i = r.indexIn(lines[1]);
        if (i == 0)
        {
            m_collection->controller()->addCommand(
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


int VariableItem::varobjIndex = 0;

VariableItem::VariableItem(VariableItem* parent,
                 const QString& expression,
                 bool frozen)
    : QObject(parent),
      expression_(expression),
      highlight_(false),
      oldSpecialRepresentationSet_(false),
      format_(natural),
      numChildren_(0),
      childrenFetched_(false),
      updateUnconditionally_(false),
      frozen_(frozen),
      initialCreation_(true),
      baseClassMember_(false),
      alive_(true)
{
//     connect(this, SIGNAL(varobjNameChange(const QString&, const QString&)),
//             varTree(),
//             SLOT(slotVarobjNameChanged(const QString&, const QString&)));


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

    setText(ColumnName, expression_);

    createVarobj();
}

VariableItem::VariableItem(VariableItem* parent, const GDBMI::Value& varobj,
                 format_t format, bool baseClassMember)
: QObject(parent),
  highlight_(false),
  oldSpecialRepresentationSet_(false),
  format_(format),
  numChildren_(0),
  childrenFetched_(false),
  updateUnconditionally_(false),
  frozen_(false),
  initialCreation_(false),
  baseClassMember_(baseClassMember),
  alive_(true)
{
//     connect(this, SIGNAL(varobjNameChange(const QString&, const QString&)),
//             varTree(),
//             SLOT(slotVarobjNameChanged(const QString&, const QString&)));

    expression_ = varobj["exp"].literal();
    varobjName_ = varobj["name"].literal();

    varobjNameChange("", varobjName_);

    setText(ColumnName, displayName());

    // Set type and children.
    originalValueType_ = varobj["type"].literal();
    numChildren_ = varobj["numchild"].literal().toInt();


    // Get the initial value.
    updateValue();
}

void VariableItem::createVarobj()
{
    QString old = varobjName_;
    varobjName_ = QString("KDEV%1").arg(varobjIndex++);
    emit varobjNameChange(old, varobjName_);

    if (frozen_)
    {
        // MI has no way to freeze a variable object. So, we
        // issue print command that returns $NN convenience
        // variable and we create variable object from that.
        m_collection->controller()->addCommand(
            new CliCommand(NonMI,
                QString("print %1").arg(expression_),
                this,
                &VariableItem::handleCliPrint));
    }
    else
    {
        m_collection->controller()->addCommand(
            new CliCommand(NonMI,
                QString("print /x &%1").arg(expression_),
                this,
                &VariableItem::handleCurrentAddress,
                true));

        m_collection->controller()->addCommand(
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
        // TODO beginRemoveRows
        qDeleteAll(m_children);
        m_children.clear();
    }

    if (r.hasField("exp"))
        expression_ = r["exp"].literal();
    numChildren_ = r["numchild"].literal().toInt();
    currentAddress_ = lastObtainedAddress_;

    setVarobjName(varobjName_);
}

void VariableItem::setVarobjName(const QString& name)
{
    if (varobjName_ != name)
        emit varobjNameChange(varobjName_, name);

    varobjName_ = name;

    if (format_ != natural)
    {
        m_collection->controller()->addCommand(
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
    if (r.reason == "done")
    {
        QString s = GDBParser::getGDBParser()->undecorateValue(
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

        setText(ColumnValue, s);
    }
    else
    {
        QString s = r["msg"].literal();
        // Error response.
        if (s.startsWith("Cannot access memory"))
        {
            s = "(inaccessible)";
        }
        setText(ColumnValue, s);
    }
}

void VariableItem::createChildren(const GDBMI::ResultRecord& r,
                             bool children_of_fake)
{
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
            m_collection->controller()->addCommand(new GDBCommand(VarListChildren,
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
            foreach (VariableItem* v, m_children)
            {
                kDebug(9012) << "Child exp : " << v->expression_ <<
                    " new exp " << exp << "\n";

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
                kDebug(9012) << "Creating new varobj "
                              << exp << " " << baseObject << "\n";
                // Propagate format from parent.
                VariableItem* v = 0;
                v = new VariableItem(this, children[i], format_, baseObject);
            }
        }
    }
}


void VariableItem::childrenDone(const GDBMI::ResultRecord& r)
{
    createChildren(r, false);
    childrenFetched_ = true;
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
            kDebug(9012) << "new address " << lastObtainedAddress_ << "\n";
        }
    }
}

void VariableItem::handleType(const QStringList& lines)
{
    bool recreate = false;

    if (lastObtainedAddress_ != currentAddress_)
    {
        kDebug(9012) << "Address changed from " << currentAddress_
                      << " to " << lastObtainedAddress_ << "\n";
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
                kDebug(9012) << "found type: " << r.cap(1) << "\n";
                kDebug(9012) << "original Type: " << originalValueType_ << "\n";

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
    if (expression_[0] != '*')
        return expression_;

    if (const VariableItem* p = parent())
    {
        return "*" + p->displayName();
    }
    else
    {
        return expression_;
    }
}

void VariableItem::setAliveRecursively(bool enable)
{
    alive_ = true;

    foreach (VariableItem* child, m_children)
    {
        child->setAliveRecursively(enable);
    }
}


VariableItem::~VariableItem()
{
    unhookFromGdb();
}

QString VariableItem::gdbExpression() const
{
    // The expression for this item can be either:
    //  - number, for array element
    //  - identifier, for member,
    //  - ***intentifier, for derefenreced pointer.
    const VariableItem* parent = VariableItem::parent();

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


// FIXME: we have two method to set VariableItem: this one
// and updateValue below. That's bad, must have just one.
void VariableItem::setText(int column, const QString &data)
{
    QString strData=data;

    if (column == ColumnValue) {
        QString oldValue(m_text[column]);
        if (!oldValue.isEmpty()) // Don't highlight new items
        {
            highlight_ = (oldValue != QString(data));
        }
    }

    m_text[column] = strData;
}

void VariableItem::clearHighlight()
{
    highlight_ = false;

    foreach (VariableItem* child, m_children)
    {
        child->clearHighlight();
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

    m_collection->controller()->addCommand(
        new GDBCommand(VarEvaluateExpression,
            "\"" + varobjName_ + "\"",
            this,
            &VariableItem::valueDone,
            true /* handle error */));
}

void VariableItem::setValue(const QString& new_value)
{
    m_collection->controller()->addCommand(
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

    s = GDBParser::getGDBParser()->undecorateValue(s);

    setText(ColumnValue, s);
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
    m_collection->controller()->addCommand(
        new CliCommand(NonMI,
            QString("print /x &%1").arg(expression_),
            this,
            &VariableItem::handleCurrentAddress,
            true));

    m_collection->controller()->addCommand(
        new CliCommand(NonMI,
            QString("whatis %1").arg(expression_),
            this,
            &VariableItem::handleType));
}

void VariableItem::recreate()
{
    unhookFromGdb();

    initialCreation_ = false;
    createVarobj();
}


// **************************************************************************

void VariableItem::getChildren()
{
    if (!childrenFetched_)
    {
        m_collection->controller()->addCommand(new GDBCommand(VarListChildren,
                                    "\"" + varobjName_ + "\"",
                                    this,
                                    &VariableItem::childrenDone));
    }
}

/*bool VariableItem::handleSpecialTypes()
{
    kDebug(9012) << "handleSpecialTypes: " << originalValueType_ << "\n";
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

VariableItem::format_t VariableItem::format() const
{
    return format_;
}

void VariableItem::setFormat(format_t f)
{
    if (f == format_)
        return;

    format_ = f;

    if (numChildren_)
    {
        // If variable has children, change format for children.
        // - for structures, that's clearly right
        // - for arrays, that's clearly right
        // - for pointers, this can be confusing, but nobody ever wants to
        //   see the pointer in decimal!
        foreach (VariableItem* child, m_children)
        {
            child->setFormat(f);
        }
    }
    else
    {
         m_collection->controller()->addCommand(
            new GDBCommand(VarSetFormat, QString("\"%1\" %2")
                           .arg(varobjName_).arg(varobjFormatName())));

        updateValue();
    }
}

VariableItem::format_t VariableItem::formatFromGdbModifier(char c) const
{
    format_t nf;
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


void VariableItem::unhookFromGdb()
{
    // Unhook children first, so that child varitems are deleted
    // before parent. Strictly speaking, we can avoid calling
    // -var-delete on child varitems, but that's a bit cheesy,
    foreach (VariableItem* child, m_children)
    {
        child->unhookFromGdb();
    }

    alive_ = false;
    childrenFetched_ = false;

    emit varobjNameChange(varobjName_, "");

    if (!m_collection->controller()->stateIsOn(s_dbgNotStarted) && !varobjName_.isEmpty())
    {
        m_collection->controller()->addCommand(
            new GDBCommand(VarDelete,
                QString("\"%1\"").arg(varobjName_)));
    }

    varobjName_ = "";
}

// **************************************************************************

QString VariableItem::tipText() const
{
    const int maxTooltipSize = 70;
    QString tip = m_text[ColumnValue];

    if (tip.length() > maxTooltipSize)
        tip = tip.mid(0, maxTooltipSize - 1 ) + " [...]";

    if (!tip.isEmpty())
        tip += "\n" + originalValueType_;

    return tip;
}

bool VariableItem::updateUnconditionally() const
{
    return updateUnconditionally_;
}

bool VariableItem::isAlive() const
{
    return alive_;
}

Qt::ItemFlags GDBDebugger::VariableItem::flags(int column) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    if (alive_)
        flags |= Qt::ItemIsEnabled;

    if (column == ColumnValue)    // Allow to change variable name by editing.
        flags |= Qt::ItemIsEditable;

    return flags;
}

const VariableItem * GDBDebugger::VariableItem::parent() const
{
    return qobject_cast<const VariableItem*>(parent());
}

#include "variableitem.moc"
