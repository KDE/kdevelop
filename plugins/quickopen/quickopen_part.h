/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
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

#ifndef QUICKOPEN_PART_H
#define QUICKOPEN_PART_H

#include <iplugin.h>
#include <iquickopen.h>
#include <QtCore/QVariant>

#include <quickopendataprovider.h>
#include "ui_quickopen.h"

class QuickOpenModel;

class QuickOpenPart : public KDevelop::IPlugin, public KDevelop::IQuickOpen
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IQuickOpen )

public:
    QuickOpenPart( QObject *parent, const QVariantList & = QVariantList() );
    virtual ~QuickOpenPart();

    // KDevelop::Plugin methods
    virtual void unload();
    
    enum ModelTypes {
      Files = 1,
      Functions = 2,
      Classes = 4,
      All = Files + Functions + Classes
    };

    /**
     * Shows the quickopen dialog with the specified Model-types
     * @param modes A combination of ModelTypes
     * */
    void showQuickOpen( ModelTypes modes = All );

    virtual void registerProvider( const QStringList& scope, const QString& type, KDevelop::QuickOpenDataProviderBase* provider );

    virtual bool removeProvider( KDevelop::QuickOpenDataProviderBase* provider );
    
public slots:
    void quickOpen();
    void quickOpenFile();
    void quickOpenFunction();
    void quickOpenClass();
    
  private:

    QuickOpenModel* m_model;
    class ProjectFileDataProvider* m_projectFileData;
};

class QuickOpenWidgetHandler : public QObject {
  Q_OBJECT
  public:
  /**
   * @param initialItems List of items that should initially be enabled in the quickopen-list. If empty, all are enabled.
   * @param initialScopes List of scopes that should initially be enabled in the quickopen-list. If empty, all are enabled.
   * */
  QuickOpenWidgetHandler( QDialog* d, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes );
  ~QuickOpenWidgetHandler();
  
  private slots:
  void currentChanged( const QModelIndex& current, const QModelIndex& previous );
  void accept();
  void textChanged( const QString& str );
  void updateProviders();
  void doubleClicked ( const QModelIndex & index );
  
  private:
  void callRowSelected();
  
  virtual bool eventFilter ( QObject * watched, QEvent * event );
  QDialog* m_dialog; //Warning: m_dialog is also the parent
  Ui::QuickOpen o;
  QuickOpenModel* m_model;
};

#endif // DUCHAINVIEW_PART_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
