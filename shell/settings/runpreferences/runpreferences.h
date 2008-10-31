/* KDevelop Project Settings
 *
 * Copyright 2006  Matt Rogers <mattr@kde.org>
 * Copyright 2007-2008 Hamish Rodda <rodda@kde.org>
 * Copyright 2008  Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVRUNPREFERENCES_H
#define KDEVRUNPREFERENCES_H

#include <KCModule>
#include <KUrl>
#include <KStandardDirs>
#include <KConfig>
#include <QSet>

class KConfigDialogManager;
class RunSettings;
class QComboBox;
class QStackedLayout;
class QVBoxLayout;
class QPushButton;

namespace Ui { class RunConfig;
               class RunSettings; }

namespace KDevelop
{
    
class TargetProperties : public QWidget
{
    Q_OBJECT
    public:
        TargetProperties(const QVariantList& args, const QString& targetName, QWidget* parent=0);
        ~TargetProperties();
        
        void save() const;
        void load();
        
        Ui::RunSettings* preferencesDialog;
        RunSettings* m_settings;
        KConfigDialogManager* m_manager;
    private slots:
        void slotAddCompileTarget();
        void removeCompileTarget();
        void upClicked();
        void downClicked();
    signals:
        void changed(bool);
        
    private:
        void addCompileTarget(const QString& name);
        QString args0;
        QString groupPrefix;
};


class RunPreferences : public KCModule
{
    Q_OBJECT
public:
    RunPreferences( QWidget *parent, const QVariantList &args );
    virtual ~RunPreferences();

    virtual void save();
    virtual void load();

    virtual KUrl localNonShareableFile() const
    {
        return KUrl::fromPath(
                   KStandardDirs::locate( "data", "kdevelop/data.kdev4" ) );
    }
signals:
        void changed(bool);
private Q_SLOTS:
    void newRunConfig();
    void deleteRunConfig();

private:
    void addTarget(const QString& name);
    void removeTarget(int index);
    
    Ui::RunConfig* m_configUi;
    QStackedLayout* stacked;
    QList<TargetProperties*> m_targetWidgets;
    QSet<QString> commitDeleteGroups;
    
    QVariantList m_args;
    
    QString m_projectFile;
    KSharedConfig::Ptr m_config;
};

}
#endif
