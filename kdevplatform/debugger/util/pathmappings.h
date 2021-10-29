/*
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PATHMAPPINGSWIDGET_H
#define KDEVPLATFORM_PATHMAPPINGSWIDGET_H

#include <QWidget>

#include <QUrl>
#include <KConfigGroup>

#include <debugger/debuggerexport.h>

class QTableView;

namespace KDevelop {

class KDEVPLATFORMDEBUGGER_EXPORT PathMappings
{
public:
    static const QString pathMappingsEntry;
    static const QString pathMappingRemoteEntry;
    static const QString pathMappingLocalEntry;

    static QUrl convertToLocalUrl(const KConfigGroup &config, const QUrl& remoteUrl);
    static QUrl convertToRemoteUrl(const KConfigGroup &config, const QUrl& localUrl);
};

class KDEVPLATFORMDEBUGGER_EXPORT PathMappingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PathMappingsWidget(QWidget* parent = nullptr);

    void loadFromConfiguration(const KConfigGroup& cfg);
    void saveToConfiguration(const KConfigGroup& cfg) const;
Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void deletePath();
private:
    QTableView *m_pathMappingTable;
};

}

#endif
