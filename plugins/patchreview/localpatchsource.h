/***************************************************************************
   Copyright 2006-2009 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_LOCALPATCHSOURCE_H
#define KDEVPLATFORM_PLUGIN_LOCALPATCHSOURCE_H

#include <QString>
#include <QWidget>
#include <QUrl>

#include <interfaces/ipatchsource.h>

namespace Ui { class LocalPatchWidget; }

class LocalPatchSource : public KDevelop::IPatchSource
{
    Q_OBJECT
    friend class LocalPatchWidget;
public:
    LocalPatchSource();
    ~LocalPatchSource() override;

    QString name() const override;

    QUrl baseDir() const override {
        return m_baseDir;
    }

    QUrl file() const override {
        return m_filename;
    }

    void update() override;
    QIcon icon() const override;

    void setFilename(const QUrl& filename) { m_filename = filename; }
    void setBaseDir(const QUrl& dir) { m_baseDir = dir; }
    void setCommand(const QString& cmd) { m_command = cmd; }

    QString command() const { return m_command; }

    bool isAlreadyApplied() const override { return m_applied; }

    void setAlreadyApplied( bool applied ) { m_applied = applied; }

    // the widget should be created _after_ the basic
    // values have been filled
    void createWidget();

    QWidget* customWidget() const override;

private:
    QUrl m_filename;
    QUrl m_baseDir;
    QString m_command;
    bool m_applied = false;
    uint m_depth;
    class LocalPatchWidget* m_widget = nullptr;
};

class LocalPatchWidget : public QWidget
{
    Q_OBJECT
public:
    LocalPatchWidget(LocalPatchSource* lpatch, QWidget* parent);

public Q_SLOTS:
    void updatePatchFromEdit();
    void syncPatch();

private:
    LocalPatchSource* m_lpatch;
    Ui::LocalPatchWidget* m_ui;
};

#endif // KDEVPLATFORM_PLUGIN_LOCALPATCHSOURCE_H
