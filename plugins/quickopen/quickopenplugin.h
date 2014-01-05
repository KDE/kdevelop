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

#ifndef KDEVPLATFORM_PLUGIN_QUICKOPENPLUGIN_H
#define KDEVPLATFORM_PLUGIN_QUICKOPENPLUGIN_H

#include <QtCore/QVariant>
#include <QtCore/QTimer>
#include <QMenu>

#include <interfaces/iplugin.h>

#include <language/interfaces/iquickopen.h>
#include <language/interfaces/quickopendataprovider.h>

#include "ui_quickopen.h"

class KAction;

namespace KDevelop {
  class SimpleCursor;
}

class QuickOpenModel;
class QuickOpenWidget;
class QuickOpenLineEdit;

class QuickOpenPlugin : public KDevelop::IPlugin, public KDevelop::IQuickOpen
{
    Q_OBJECT
    Q_INTERFACES( KDevelop::IQuickOpen )

public:
    QuickOpenPlugin( QObject *parent, const QVariantList & = QVariantList() );
    virtual ~QuickOpenPlugin();

    static QuickOpenPlugin* self();
    
    // KDevelop::Plugin methods
    virtual void unload();

    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);

    enum ModelTypes {
      Files = 1,
      Functions = 2,
      Classes = 4,
      OpenFiles = 8,
      All = Files + Functions + Classes + OpenFiles
    };

    /**
     * Shows the quickopen dialog with the specified Model-types
     * @param modes A combination of ModelTypes
     * */
    void showQuickOpen( ModelTypes modes = All );
    void showQuickOpen( const QStringList &items );

    virtual void registerProvider( const QStringList& scope, const QStringList& type, KDevelop::QuickOpenDataProviderBase* provider );

    virtual bool removeProvider( KDevelop::QuickOpenDataProviderBase* provider );

    virtual QSet<KDevelop::IndexedString> fileSet() const;

    //Frees the model by closing active quickopen dialoags, and retuns whether successful.
    bool freeModel();
    
    virtual void createActionsForMainWindow( Sublime::MainWindow* window, QString& xmlFile, KActionCollection& actions );

    QuickOpenLineEdit* createQuickOpenLineWidget();
    
    virtual KDevelop::IQuickOpenLine* createQuickOpenLine(const QStringList& scopes, const QStringList& type, QuickOpenType kind);
public slots:
    void quickOpen();
    void quickOpenFile();
    void quickOpenFunction();
    void quickOpenClass();
    void quickOpenDeclaration();
    void quickOpenOpenFile();
    void quickOpenDefinition();
    void quickOpenNavigate();
    void quickOpenNavigateFunctions();
    void quickOpenDocumentation();

    void previousFunction();
    void nextFunction();

private slots:
    void storeScopes( const QStringList& );
    void storeItems( const QStringList& );

private:
    friend class QuickOpenLineEdit;
    friend class StandardQuickOpenWidgetCreator;
    QuickOpenLineEdit* quickOpenLine(QString name = "Quickopen");

    enum FunctionJumpDirection { NextFunction, PreviousFunction };
    void jumpToNearestFunction(FunctionJumpDirection direction);

    QPair<KUrl, KDevelop::SimpleCursor> specialObjectJumpPosition() const;
    QWidget* specialObjectNavigationWidget() const;
    bool jumpToSpecialObject();
    void showQuickOpenWidget(const QStringList &items, const QStringList &scopes, bool preselectText);
    
    QuickOpenModel* m_model;
    class ProjectFileDataProvider* m_projectFileData;
    class ProjectItemDataProvider* m_projectItemData;
    class OpenFilesDataProvider* m_openFilesData;
    class DocumentationQuickOpenProvider* m_documentationItemData;
    QStringList lastUsedScopes;
    QStringList lastUsedItems;
  
    //We can only have one widget at a time, because we manipulate the model.
    QPointer<QObject> m_currentWidgetHandler;
    KAction* m_quickOpenDeclaration;
    KAction* m_quickOpenDefinition;
};

///Will delete itself once the dialog is closed, so use QPointer when referencing it permanently
class QuickOpenWidget : public QMenu {
  Q_OBJECT
  public:
  /**
   * @param initialItems List of items that should initially be enabled in the quickopen-list. If empty, all are enabled.
   * @param initialScopes List of scopes that should initially be enabled in the quickopen-list. If empty, all are enabled.
   * @param listOnly when this is true, the given items will be listed, but all filtering using checkboxes is disabled.
   * @param noSearchFied when this is true, no search-line is shown.
   * */
  QuickOpenWidget( QString title, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes, bool listOnly = false, bool noSearchField = false );
  ~QuickOpenWidget();
  void setPreselectedText(const QString &text);
    void prepareShow();

  void setAlternativeSearchField(KLineEdit* alterantiveSearchField);
    
  //Shows OK + Cancel. By default they are hidden  
  void showStandardButtons(bool show);
    void showSearchField(bool show);
    
  signals:
  void scopesChanged( const QStringList& scopes );
  void itemsChanged( const QStringList& scopes );
  void ready();

  private slots:
  void currentChanged( const QModelIndex& current, const QModelIndex& previous );
  void currentChanged( const QItemSelection& current, const QItemSelection& previous );
  
  void updateTimerInterval( bool cheapFilterChange );

  void accept();
  void textChanged( const QString& str );
  void updateProviders();
  void doubleClicked ( const QModelIndex & index );

  void applyFilter();

  private:
  virtual void showEvent(QShowEvent *);
  void callRowSelected();
  
  virtual bool eventFilter ( QObject * watched, QEvent * event );

  QuickOpenModel* m_model;
  bool m_expandedTemporary, m_hadNoCommandSinceAlt;
  QTime m_altDownTime;
  QString m_preselectedText;
  QTimer m_filterTimer;
  QString m_filter;
  public:
  Ui::QuickOpen o;
  
  friend class QuickOpenWidgetDialog;
  friend class QuickOpenPlugin;
  friend class QuickOpenLineEdit;
};

class QuickOpenWidgetDialog : public QObject {
  Q_OBJECT
  public:
    QuickOpenWidgetDialog( QString title, QuickOpenModel* model, const QStringList& initialItems, const QStringList& initialScopes, bool listOnly = false, bool noSearchField = false );
    ~QuickOpenWidgetDialog();
    ///Shows the dialog
    void run();
    QuickOpenWidget* widget() const {
      return m_widget;
    }
  private:
  QDialog* m_dialog; //Warning: m_dialog is also the parent
  QuickOpenWidget* m_widget;
};

class QuickOpenWidgetCreator;

class QuickOpenLineEdit : public KDevelop::IQuickOpenLine {
  Q_OBJECT
  public:
    QuickOpenLineEdit(QuickOpenWidgetCreator* creator) ;
    ~QuickOpenLineEdit() ;
    
    bool insideThis(QObject* object);
    void showWithWidget(QuickOpenWidget* widget);
    
    virtual void setDefaultText(const QString& text) {
      m_defaultText = text;
      setClickMessage(m_defaultText);
    }
  private slots:
    void activate() ;
    void deactivate() ;
    void checkFocus();
    void widgetDestroyed(QObject*);
  private:
    virtual void focusInEvent(QFocusEvent* ev) ;
    virtual bool eventFilter(QObject* obj, QEvent* e) ;
    virtual void hideEvent(QHideEvent* );
    
    QPointer<QuickOpenWidget> m_widget;
    bool m_forceUpdate;
    QString m_defaultText;
    QuickOpenWidgetCreator* m_widgetCreator;
};

#endif // KDEVPLATFORM_PLUGIN_QUICKOPENPLUGIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
