/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Dukju Ahn <dukjuahn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef ENVWIDGET_H
#define ENVWIDGET_H

#include <QWidget>
#include "utilexport.h"
class QTableWidgetItem;
class KConfig;

template <typename T1, typename T2> class QMap;

/**
 * @short Environment variable setting widget.
 * Two entry are set by this widget -- "Default Env Group" and "Environment Variables".
 *
 * The collection of environment variable is grouped, and among the groups,
 * only one group is active by user. By default, the groupname "default" is the
 * default group. "Default Env Group" entry indicates which group should be set active.
 *
 * "Environment Variables" entry stores the actual list of
 * <groupname_varname=varvalue>. The groupname can't contain '=' or '_'.
 * For example, suppose that two configuration, say "release" and "debug" exist.
 * Then the actual contents of .kdev4 project file will be
 *
 * \code
 * [Plugin Environment Settings] // The content of this line is set by setConfig()
 * Default Env Group=debug
 * Environment Variables=debug_PATH=/home/kde-devel/usr/bin,release_PATH=/usr/bin
 * \endcode
 *
 * This widget is not compatible with KConfigXT since it deals with two
 * config entries.
 *
 * How to Use :
 * At the constructor of KCModule derived classes
   \code
    EnvWidget *d->preferencesDialog = new EnvWidget( this );
    l->addWidget( d->preferencesDialog );

    connect( d->preferencesDialog, SIGNAL( changed() ),
             this, SLOT( settingsChanged() ) );

    // init kconfigskeleton
    if( you don't use xml description file )
        d->m_skel = new ProjectConfigSkeleton( args.first() );
        d->m_skel->setDeveloperTempFile( args.at(1) );
        d->m_skel->setProjectFileUrl( args.at(2) );
        d->m_skel->setDeveloperFileUrl( args.at(3) );
        d->m_config = d->m_skel->config();
        addConfig( d->m_skel, d->preferencesDialog );
    else if( you use xml file)
        d->m_config = EnvSettings::self()->config();
        addConfig( EnvSettings:;self(), d->preferencesDialog );

        d->preferencesDialog->setConfig( d->m_config, "Plugin Environment Settings" );
   \endcode
   and define additional slot like this
   \code
   void KCModuleDerived::settingsChanged()
   {
        unmanagedWidgetChangeState( true );
   }
   \endcode
 * and call saveSettings(), loadSettings(), defaults(), in
 * KCModule's overloaded virtuals
 *
 * @sa EnvPreferences
 */
class KDEVPLATFORMUTIL_EXPORT EnvWidget: public QWidget
{
    Q_OBJECT
//     Q_PROPERTY( QStringList items READ items WRITE setItems USER true )

public:
    explicit EnvWidget( QWidget *parent = 0 );
    virtual ~EnvWidget();

    void setConfig( KConfig *config, const QString &group );

    void setVariables(const QStringList& items);
    QStringList variables() const;

    void loadSettings();
    void saveSettings();
    void defaults();

Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void newButtonClicked();
    void deleteButtonClicked();
    void processDefaultButtonClicked();
    void settingsChanged( int row, int column );
    void focusChanged( int row, int, int, int );
    void addGroupClicked();
    void removeGroupClicked();
    void activeGroupChanged(int);

private:
//     void load( bool defaults );
    bool isOverride( QTableWidgetItem *item ) const;
    bool isProcessDefault( QTableWidgetItem *item ) const;
    void setOverride( QTableWidgetItem *item );
    void setProcessDefault( QTableWidgetItem *item );
    void generateCurrentMaps();
    bool diff();

private:
    class Private;
    Private *const d;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
