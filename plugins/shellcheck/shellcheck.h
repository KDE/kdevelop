/***************************************************************************
 *   Copyright 2020  Morten Danielsen Volden                               *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#ifndef SHELLCHECK_H
#define SHELLCHECK_H

#include <interfaces/iplugin.h>
//#include <shell/problemmodel.h>


class QMimeType;

namespace KDevelop
{
class IProject;
}


class ShellCheck : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    ShellCheck(QObject* parent, const QVariantList& args);
    
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;
    
    void runShellcheck(KDevelop::IProject* project, const QString& path);
    bool isRunning() const;

private:
    bool isSupportedMimeType(const QMimeType& mimeType) const;
    void runShellcheck(bool /*checkProject*/);
    
    KDevelop::IProject* m_currentProject;
    
    QAction* m_contextActionFile;
    
    //QScopedPointer<KDevelop::ProblemModel> m_model;
};

#endif // SHELLCHECK_H
