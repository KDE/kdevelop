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
    virtual ~LocalPatchSource();

    virtual QString name() const override;

    virtual QUrl baseDir() const override {
        return m_baseDir;
    }

    virtual QUrl file() const override {
        return m_filename;
    }

    virtual uint depth() const override {
        return m_depth;
    }

    virtual void update() override;
    virtual QIcon icon() const override;

    void setFilename(const QUrl& filename) { m_filename = filename; }
    void setBaseDir(const QUrl& dir) { m_baseDir = dir; }
    void setCommand(const QString& cmd) { m_command = cmd; }

    QString command() const { return m_command; }

    virtual bool isAlreadyApplied() const override { return m_applied; }

    void setAlreadyApplied( bool applied ) { m_applied = applied; }

    virtual QWidget* customWidget() const override;

private:
    QUrl m_filename;
    QUrl m_baseDir;
    QString m_command;
    bool m_applied;
    uint m_depth;
    class LocalPatchWidget* m_widget;
};

class LocalPatchWidget : public QWidget
{
    Q_OBJECT
public:
    LocalPatchWidget(LocalPatchSource* lpatch, QWidget* parent);

public slots:
    void updatePatchFromEdit();
    void syncPatch();

private:
    LocalPatchSource* m_lpatch;
    Ui::LocalPatchWidget* m_ui;
};

#endif // KDEVPLATFORM_PLUGIN_LOCALPATCHSOURCE_H
