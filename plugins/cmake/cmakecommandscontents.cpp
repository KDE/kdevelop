/*
    SPDX-FileCopyrightText: 2009 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakecommandscontents.h"
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentation.h>
#include <interfaces/idocumentationcontroller.h>
#include "cmakebuilderconfig.h"
#include "cmakedoc.h"
#include <QProcess>
#include <KLocalizedString>

#include <array>

static const std::array<QString, 6> args = {
    QStringLiteral("--help-command"),
    QStringLiteral("--help-variable"),
    QStringLiteral("--help-module"),
    QStringLiteral("--help-property"),
    QStringLiteral("--help-policy"),
    QString(),
};
static const std::array<QString, 5> modules = {
    i18nc("@item cmake", "Commands"),
    i18nc("@item cmake", "Variables"),
    i18nc("@item cmake", "Modules"),
    i18nc("@item cmake", "Properties"),
    i18nc("@item cmake", "Policies"),
};

CMakeCommandsContents::CMakeCommandsContents(QObject* parent)
     : QAbstractItemModel(parent)
     , m_namesForType(CMakeDocumentation::EOType)
{
    for (int i = 0; i <= CMakeDocumentation::Policy; ++i) {
        const QStringList params = { args[i]+QStringLiteral("-list") };

        auto* process = new QProcess(this);
        process->setProperty("type", i);
        process->setProgram(CMakeBuilderSettings::self()->cmakeExecutable().toLocalFile());
        process->setArguments(params);
        KDevelop::ICore::self()->runtimeController()->currentRuntime()->startProcess(process);

        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &CMakeCommandsContents::processOutput);
    }
}

void CMakeCommandsContents::processOutput(int code)
{
    auto* process = qobject_cast<QProcess*>(sender());
    if (code!=0) {
        qDebug() << "failed" << process;
        return;
    }

    const CMakeDocumentation::Type type = CMakeDocumentation::Type(process->property("type").toInt());

    QTextStream stream(process);
    QString line = stream.readLine(); //discard first line
    QVector<QString> names;
    while(stream.readLineInto(&line)) {
        names += line;
    }

    beginInsertRows(index(type, 0, {}), 0, names.count()-1);
    for (auto& name : std::as_const(names)) {
        m_typeForName.insert(name, type);
    }
    m_namesForType[type] = names;
    endInsertRows();
}

CMakeDocumentation::Type CMakeCommandsContents::typeFor(const QString& identifier) const
{
    //TODO can do much better
    if(m_typeForName.contains(identifier)) {
        return m_typeForName[identifier];
    } else if(m_typeForName.contains(identifier.toLower())) {
        return m_typeForName[identifier.toLower()];
    } else if(m_typeForName.contains(identifier.toUpper())) {
        return m_typeForName[identifier.toUpper()];
    }
    return CMakeDocumentation::EOType;
}

QString CMakeCommandsContents::descriptionForIdentifier(const QString& id, CMakeDocumentation::Type t) const
{
    QString desc;
    if(args[t].size() != 0) {
        desc = CMake::executeProcess(CMakeBuilderSettings::self()->cmakeExecutable().toLocalFile(), { args[t], id.simplified() });
        desc.remove(QLatin1String(":ref:"));

        const QString rst2html = QStandardPaths::findExecutable(QStringLiteral("rst2html"));
        if (rst2html.isEmpty()) {
            desc = (QLatin1String("<html><body><pre><code>") + desc.toHtmlEscaped() + QLatin1String("</code></pre>")
                + i18n("<p>For better CMake documentation rendering, install rst2html.</p>")
                + QLatin1String("</body></html>"));
        } else {
            QProcess p;
            p.start(rst2html, { QStringLiteral("--no-toc-backlinks"), QStringLiteral("--quiet") });
            p.write(desc.toUtf8());
            p.closeWriteChannel();
            p.waitForFinished();
            desc = QString::fromUtf8(p.readAllStandardOutput());
        }
    }

    return desc;
}


void CMakeCommandsContents::showItemAt(const QModelIndex& idx) const
{
    if(idx.isValid() && int(idx.internalId())>=0) {
        QString desc=CMakeDoc::s_provider->descriptionForIdentifier(idx.data().toString(),
                                                                    (ICMakeDocumentation::Type) idx.parent().row());
        CMakeDoc::Ptr doc(new CMakeDoc(idx.data().toString(), desc));

        KDevelop::ICore::self()->documentationController()->showDocumentation(doc);
    }
}

QModelIndex CMakeCommandsContents::parent(const QModelIndex& child) const
{
    if(child.isValid() && child.column()==0 && int(child.internalId())>=0)
        return createIndex(child.internalId(),0, -1);
    return QModelIndex();
}

QModelIndex CMakeCommandsContents::index(int row, int column, const QModelIndex& parent) const
{
    if(row<0 || column!=0)
        return QModelIndex();
    if(!parent.isValid() && row==ICMakeDocumentation::EOType)
        return QModelIndex();

    return createIndex(row,column, int(parent.isValid() ? parent.row() : -1));
}

int CMakeCommandsContents::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid())
        return ICMakeDocumentation::EOType;
    else if(int(parent.internalId())<0) {
        int ss=names((ICMakeDocumentation::Type) parent.row()).size();
        return ss;
    }
    return 0;
}

QVariant CMakeCommandsContents::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {
        if(role==Qt::DisplayRole) {
            int internal(index.internalId());
            if(internal>=0)
                return m_namesForType[internal].count() > index.row() ? QVariant(m_namesForType[internal].at(index.row())) : QVariant();
            else
                return modules[index.row()];
        }
    }
    return QVariant();
}

int CMakeCommandsContents::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVector<QString> CMakeCommandsContents::names(ICMakeDocumentation::Type t) const
{
    return m_namesForType[t];
}

#include "moc_cmakecommandscontents.cpp"
