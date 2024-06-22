/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "launchconfigurationdialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>

#include <KComboBox>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMessageBox_KDevCompat>

#include <interfaces/launchconfigurationpage.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "core.h"
#include "runcontroller.h"
#include "launchconfiguration.h"
#include "debug.h"

#include <interfaces/ilauncher.h>
#include <interfaces/ilaunchmode.h>
#include <interfaces/launchconfigurationtype.h>

namespace KDevelop
{

bool launchConfigGreaterThan(KDevelop::LaunchConfigurationType* a, KDevelop::LaunchConfigurationType* b)
{
    return a->name()>b->name();
}

//TODO: Maybe use KPageDialog instead, might make the model stuff easier and the default-size stuff as well
LaunchConfigurationDialog::LaunchConfigurationDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle( i18nc("@title:window", "Launch Configurations" ) );

    auto* mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mainWidget);

    setupUi(mainWidget);
    splitter->setSizes(QList<int>{260, 620});
    splitter->setCollapsible(0, false);

    addConfig->setToolTip(i18nc("@info:tooltip", "Add a new launch configuration."));
    deleteConfig->setEnabled( false );
    deleteConfig->setToolTip(i18nc("@info:tooltip", "Delete selected launch configuration."));

    model = new LaunchConfigurationsModel( tree );
    tree->setModel( model );
    tree->setExpandsOnDoubleClick( true );
    tree->setSelectionBehavior( QAbstractItemView::SelectRows );
    tree->setSelectionMode( QAbstractItemView::SingleSelection );
    tree->setUniformRowHeights( true );
    tree->setItemDelegate( new LaunchConfigurationModelDelegate(this) );
    tree->setColumnHidden(1, true);
    for(int row=0; row<model->rowCount(); row++) {
        tree->setExpanded(model->index(row, 0), true);
    }

    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( tree, &QTreeView::customContextMenuRequested, this, &LaunchConfigurationDialog::doTreeContextMenu );
    connect( deleteConfig, &QPushButton::clicked, this, &LaunchConfigurationDialog::deleteConfiguration);
    connect( model, &LaunchConfigurationsModel::dataChanged, this, &LaunchConfigurationDialog::modelChanged );
    connect( tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &LaunchConfigurationDialog::selectionChanged);
    QModelIndex idx = model->indexForConfig( Core::self()->runControllerInternal()->defaultLaunch() );
    qCDebug(SHELL) << "selecting index:" << idx;
    if( !idx.isValid() )
    {
        for( int i = 0; i < model->rowCount(); i++ )
        {
            if( model->rowCount( model->index( i, 0, QModelIndex() ) ) > 0 )
            {
                idx = model->index( 1, 0, model->index( i, 0, QModelIndex() ) );
                break;
            }
        }
        if( !idx.isValid() )
        {
            idx = model->index( 0, 0, QModelIndex() );
        }
    }
    tree->selectionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    tree->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

    // Unfortunately tree->resizeColumnToContents() only looks at the top-level
    // items, instead of all open ones. Hence we're calculating it ourselves like
    // this:
    // Take the selected index, check if it has childs, if so take the first child
    // Then count the level by going up, then let the tree calculate the width
    // for the selected or its first child index and add indentation*level
    //
    // If Qt Software ever fixes resizeColumnToContents, the following line
    // can be enabled and the rest be removed
    // tree->resizeColumnToContents( 0 );
    int level = 0;
    QModelIndex widthidx = idx;
    if( model->rowCount( idx ) > 0 )
    {
        widthidx = model->index( 0, 0, idx );
    }
    QModelIndex parentidx = widthidx.parent();
    while( parentidx.isValid() )
    {
        level++;
        parentidx = parentidx.parent();
    }
    // make sure the base column width is honored, e.g. when no launch configs exist
    tree->resizeColumnToContents(0);
    int width = tree->columnWidth( 0 );
    while ( widthidx.isValid() )
    {
        width = qMax( width, level*tree->indentation() + tree->indentation() + tree->sizeHintForIndex( widthidx ).width() );
        widthidx = widthidx.parent();
    }
    tree->setColumnWidth( 0, width );

    auto* m = new QMenu(this);
    QList<LaunchConfigurationType*> types = Core::self()->runController()->launchConfigurationTypes();
    std::sort(types.begin(), types.end(), launchConfigGreaterThan); //we want it in reverse order
    for (LaunchConfigurationType* type : std::as_const(types)) {
        connect(type, &LaunchConfigurationType::signalAddLaunchConfiguration, this, &LaunchConfigurationDialog::addConfiguration);
        QMenu* suggestionsMenu = type->launcherSuggestions();

        if(suggestionsMenu) {
            // take ownership
            suggestionsMenu->setParent(m, suggestionsMenu->windowFlags());
            m->addMenu(suggestionsMenu);
        }
    }
    // Simplify menu structure to get rid of 1-entry levels
    while (m->actions().count() == 1) {
        QMenu* subMenu = m->actions().at(0)->menu();
        if (subMenu && subMenu->isEnabled() && subMenu->actions().count()<5) {
            m = subMenu;
        } else {
            break;
        }
    }
    if(!m->isEmpty()) {
        auto* separator = new QAction(m);
        separator->setSeparator(true);
        m->insertAction(m->actions().at(0), separator);
    }

    for (LaunchConfigurationType* type : std::as_const(types)) {
        auto* action = new QAction(type->icon(), type->name(), m);
        action->setProperty("configtype", QVariant::fromValue<QObject*>(type));
        connect(action, &QAction::triggered, this, &LaunchConfigurationDialog::createEmptyLauncher);

        if(!m->actions().isEmpty())
            m->insertAction(m->actions().at(0), action);
        else
            m->addAction(action);
    }
    addConfig->setMenu(m);
    addConfig->setEnabled( !m->isEmpty() );

    messageWidget->setCloseButtonVisible( false );
    messageWidget->setMessageType( KMessageWidget::Warning );
    messageWidget->setText( i18n("No launch configurations available. (Is any of the Execute plugins loaded?)") );
    messageWidget->setVisible( m->isEmpty() );

    connect(debugger, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LaunchConfigurationDialog::launchModeChanged);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &LaunchConfigurationDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &LaunchConfigurationDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, QOverload<>::of(&LaunchConfigurationDialog::saveConfig));
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, QOverload<>::of(&LaunchConfigurationDialog::saveConfig));
    mainLayout->addWidget(buttonBox);

    resize( QSize(qMax(1200, sizeHint().width()), qMax(500, sizeHint().height())) );
}

void LaunchConfigurationDialog::doTreeContextMenu(const QPoint& point)
{
    if ( ! tree->selectionModel()->selectedRows().isEmpty() ) {
        QModelIndex selected = tree->selectionModel()->selectedRows().first();
        if ( selected.parent().isValid() && ! selected.parent().parent().isValid() ) {
            // only display the menu if a launch config is clicked
            QMenu menu(tree);
            auto* rename = new QAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18nc("@action:inmenu", "Rename Configuration"), &menu);
            auto* delete_ = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18nc("@action:inmenu", "Delete Configuration"), &menu);
            connect(rename, &QAction::triggered, this, &LaunchConfigurationDialog::renameSelected);
            connect(delete_, &QAction::triggered, this, &LaunchConfigurationDialog::deleteConfiguration);
            menu.addAction(rename);
            menu.addAction(delete_);
            menu.exec(tree->viewport()->mapToGlobal(point));
        }
    }
}

void LaunchConfigurationDialog::renameSelected()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        QModelIndex parent = tree->selectionModel()->selectedRows().first();
        if( parent.parent().isValid() )
        {
            parent = parent.parent();
        }
        QModelIndex index = model->index(tree->selectionModel()->selectedRows().first().row(), 0, parent);
        tree->edit( index );
    }
}

QSize LaunchConfigurationDialog::sizeHint() const
{
    QSize s = QDialog::sizeHint();
    return s.expandedTo(QSize(880, 520));
}

void LaunchConfigurationDialog::createEmptyLauncher()
{
    auto* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);

    auto* type = qobject_cast<LaunchConfigurationType*>(action->property("configtype").value<QObject*>());
    Q_ASSERT(type);

    IProject* p = model->projectForIndex(tree->currentIndex());
    QPair< QString, QString > launcher( type->launchers().at( 0 )->supportedModes().at(0), type->launchers().at( 0 )->id() );
    ILaunchConfiguration* l = ICore::self()->runController()->createLaunchConfiguration(type, launcher, p);
    addConfiguration(l);
}

void LaunchConfigurationDialog::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if( !deselected.indexes().isEmpty() )
    {
        LaunchConfiguration* l = model->configForIndex( deselected.indexes().first() );
        if( l )
        {
            disconnect(l, &LaunchConfiguration::nameChanged, this,  &LaunchConfigurationDialog::updateNameLabel);
            if( currentPageChanged )
            {
                if (KMessageBox::questionTwoActions(
                        this, i18n("Selected Launch Configuration has unsaved changes. Do you want to save it?"),
                        i18nc("@title:window", "Unsaved Changes"), KStandardGuiItem::save(),
                        KStandardGuiItem::discard())
                    == KMessageBox::PrimaryAction) {
                    saveConfig( deselected.indexes().first() );
                } else {
                    auto* tab = qobject_cast<LaunchConfigPagesContainer*>( stack->currentWidget() );
                    tab->setLaunchConfiguration( l );
                    buttonBox->button(QDialogButtonBox::Apply)->setEnabled( false );
                    currentPageChanged = false;
                }
            }
        }
    }
    updateNameLabel(nullptr);

    for( int i = 1; i < stack->count(); i++ )
    {
        QWidget* w = stack->widget(i);
        stack->removeWidget(w);
        delete w;
    }

    if( !selected.indexes().isEmpty() )
    {
        QModelIndex idx = selected.indexes().first();
        LaunchConfiguration* l = model->configForIndex( idx );
        ILaunchMode* lm = model->modeForIndex( idx );

        if( l )
        {
            updateNameLabel( l );
            tree->expand( model->indexForConfig( l ) );
            connect( l, &LaunchConfiguration::nameChanged, this, &LaunchConfigurationDialog::updateNameLabel );
            if( lm )
            {
                QVariant currentLaunchMode = idx.sibling(idx.row(), 1).data(Qt::EditRole);
                {
                    QSignalBlocker blocker(debugger);
                    const QList<ILauncher*> launchers = l->type()->launchers();

                    debugger->clear();
                    for (ILauncher* launcher : launchers) {
                        if (launcher->supportedModes().contains(lm->id())) {
                            debugger->addItem(launcher->name(), launcher->id());
                        }
                    }

                    debugger->setCurrentIndex(debugger->findData(currentLaunchMode));
                }

                debugger->setVisible(debugger->count()>0);
                debugLabel->setVisible(debugger->count()>0);

                ILauncher* launcher = l->type()->launcherForId( currentLaunchMode.toString() );
                if( launcher )
                {
                    LaunchConfigPagesContainer* tab = launcherWidgets.value( launcher );
                    if(!tab)
                    {
                        QList<KDevelop::LaunchConfigurationPageFactory*> pages = launcher->configPages();
                        if(!pages.isEmpty()) {
                            tab = new LaunchConfigPagesContainer( launcher->configPages(), stack );
                            connect( tab, &LaunchConfigPagesContainer::changed, this, &LaunchConfigurationDialog::pageChanged );
                            stack->addWidget( tab );
                        }
                    }

                    if(tab) {
                        tab->setLaunchConfiguration( l );
                        stack->setCurrentWidget( tab );
                    } else {
                        auto* label = new QLabel(i18nc("%1 is a launcher name",
                                                       "No configuration is needed for '%1'",
                                                       launcher->name()), stack);
                        label->setAlignment(Qt::AlignCenter);
                        QFont font = label->font();
                        font.setItalic(true);
                        label->setFont(font);
                        stack->addWidget(label);
                        stack->setCurrentWidget(label);
                    }

                    updateNameLabel( l );
                    addConfig->setEnabled( false );
                    deleteConfig->setEnabled( false );
                } else
                {
                    addConfig->setEnabled( false );
                    deleteConfig->setEnabled( false );
                    stack->setCurrentIndex( 0 );
                }
            } else
            {
                //TODO: enable removal button
                LaunchConfigurationType* type = l->type();
                LaunchConfigPagesContainer* tab = typeWidgets.value( type );
                if( !tab )
                {
                    tab = new LaunchConfigPagesContainer( type->configPages(), stack );
                    connect( tab, &LaunchConfigPagesContainer::changed, this, &LaunchConfigurationDialog::pageChanged );
                    stack->addWidget( tab );
                }
                qCDebug(SHELL) << "created pages, setting config up";
                tab->setLaunchConfiguration( l );
                stack->setCurrentWidget( tab );

                addConfig->setEnabled( addConfig->menu() && !addConfig->menu()->isEmpty() );
                deleteConfig->setEnabled( true );
                debugger->setVisible( false );
                debugLabel->setVisible( false );
            }
        } else
        {
            addConfig->setEnabled( addConfig->menu() && !addConfig->menu()->isEmpty() );
            deleteConfig->setEnabled( false );
            stack->setCurrentIndex( 0 );
            auto* l = new QLabel(i18n("<i>Select a configuration to edit from the left,<br>"
                                      "or click the \"Add\" button to add a new one.</i>"), stack);
            l->setAlignment(Qt::AlignCenter);
            stack->addWidget(l);
            stack->setCurrentWidget(l);
            debugger->setVisible( false );
            debugLabel->setVisible( false );
        }
    } else
    {
        debugger->setVisible( false );
        debugLabel->setVisible( false );
        addConfig->setEnabled( false );
        deleteConfig->setEnabled( false );
        stack->setCurrentIndex( 0 );
    }
}

void LaunchConfigurationDialog::saveConfig( const QModelIndex& idx )
{
    Q_UNUSED( idx );
    auto* tab = qobject_cast<LaunchConfigPagesContainer*>( stack->currentWidget() );
    if( tab )
    {
        tab->save();
        buttonBox->button(QDialogButtonBox::Apply)->setEnabled( false );
        currentPageChanged = false;
    }
}

void LaunchConfigurationDialog::saveConfig()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        saveConfig( tree->selectionModel()->selectedRows().first() );
    }
}


void LaunchConfigurationDialog::pageChanged()
{
    currentPageChanged = true;
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled( true );
}

void LaunchConfigurationDialog::modelChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if (tree->selectionModel())
    {
        QModelIndex index = tree->selectionModel()->selectedRows().first();
        if (index.row() >= topLeft.row() && index.row() <= bottomRight.row() && bottomRight.column() == 1)
            selectionChanged(tree->selectionModel()->selection(), tree->selectionModel()->selection());
    }
}

void LaunchConfigurationDialog::deleteConfiguration()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        model->deleteConfiguration( tree->selectionModel()->selectedRows().first() );
        tree->resizeColumnToContents( 0 );
    }
}

void LaunchConfigurationDialog::updateNameLabel( LaunchConfiguration* l )
{
    if( l )
    {
        configName->setText( i18n("Editing %2: <b>%1</b>", l->name(), l->type()->name() ) );
    } else
    {
        configName->clear();
    }
}

void LaunchConfigurationDialog::createConfiguration()
{
    if( !tree->selectionModel()->selectedRows().isEmpty() )
    {
        QModelIndex idx = tree->selectionModel()->selectedRows().first();
        if( idx.parent().isValid() )
        {
            idx = idx.parent();
        }
        model->createConfiguration( idx );
        QModelIndex newindex = model->index( model->rowCount( idx ) - 1, 0, idx );
        tree->selectionModel()->select( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
        tree->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
        tree->edit( newindex );
        tree->resizeColumnToContents( 0 );
    }
}

void LaunchConfigurationDialog::addConfiguration(ILaunchConfiguration* _launch)
{
    auto* launch = dynamic_cast<LaunchConfiguration*>(_launch);
    Q_ASSERT(launch);
    int row = launch->project() ? model->findItemForProject(launch->project())->row : 0;
    QModelIndex idx  = model->index(row, 0);

    model->addConfiguration(launch, idx);

    QModelIndex newindex = model->index( model->rowCount( idx ) - 1, 0, idx );
    tree->selectionModel()->select( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    tree->selectionModel()->setCurrentIndex( newindex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    tree->edit( newindex );
    tree->resizeColumnToContents( 0 );
}

LaunchConfigurationsModel::LaunchConfigurationsModel(QObject* parent): QAbstractItemModel(parent)
{
    auto* global = new GenericPageItem;
    global->text = i18n("Global");
    global->row = 0;
    const auto projects = Core::self()->projectController()->projects();
    topItems.reserve(1 + projects.size());
    topItems << global;
    for (IProject* p :  projects) {
        auto* t = new ProjectItem;
        t->project = p;
        t->row = topItems.count();
        topItems << t;
    }
    const auto launchConfigurations = Core::self()->runControllerInternal()->launchConfigurationsInternal();
    for (LaunchConfiguration* l : launchConfigurations) {
        addItemForLaunchConfig( l );
    }
}

void LaunchConfigurationsModel::addItemForLaunchConfig( LaunchConfiguration* l )
{
    auto* t = new LaunchItem;
    t->launch = l;
    TreeItem* parent;
    if( l->project() ) {
        parent = findItemForProject( l->project() );
    } else {
        parent = topItems.at(0);
    }
    t->parent = parent;
    t->row = parent->children.count();
    parent->children.append( t );
    addLaunchModeItemsForLaunchConfig ( t );
}

void LaunchConfigurationsModel::addLaunchModeItemsForLaunchConfig ( LaunchItem* t )
{
    QList<TreeItem*> items;
    QSet<QString> modes;
    const auto launchers = t->launch->type()->launchers();
    for (ILauncher* launcher : launchers) {
        const auto supportedModes = launcher->supportedModes();
        for (const QString& mode : supportedModes) {
            if( !modes.contains( mode ) && launcher->configPages().count() > 0 )
            {
                modes.insert( mode );
                auto* lmi = new LaunchModeItem;
                lmi->mode = Core::self()->runController()->launchModeForId( mode );
                lmi->parent = t;
                lmi->row = t->children.count();
                items.append( lmi );
            }
        }
    }
    if( !items.isEmpty() )
    {
        QModelIndex p = indexForConfig( t->launch );
        beginInsertRows( p, t->children.count(), t->children.count() + items.count() - 1  );
        t->children.append( items );
        endInsertRows();
    }
}

LaunchConfigurationsModel::ProjectItem* LaunchConfigurationsModel::findItemForProject(IProject* p) const
{
    for (TreeItem* t : topItems) {
        auto* pi = dynamic_cast<ProjectItem*>( t );
        if( pi && pi->project == p )
        {
            return pi;
        }
    }
    Q_ASSERT(false);
    return nullptr;
}

int LaunchConfigurationsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED( parent );
    return 2;
}

QVariant LaunchConfigurationsModel::data(const QModelIndex& index, int role) const
{
    if( index.isValid() && index.column() >= 0 && index.column() < 2 )
    {
        auto* t = static_cast<TreeItem*>( index.internalPointer() );
        switch( role )
        {
            case Qt::DisplayRole:
            {
                auto* li = dynamic_cast<LaunchItem*>( t );
                if( li )
                {
                    if( index.column() == 0 )
                    {
                        return li->launch->name();
                    } else if( index.column() == 1 )
                    {
                        return li->launch->type()->name();
                    }
                }
                auto* pi = dynamic_cast<ProjectItem*>( t );
                if( pi && index.column() == 0 )
                {
                    return pi->project->name();
                }
                auto* gpi = dynamic_cast<GenericPageItem*>( t );
                if( gpi && index.column() == 0 )
                {
                    return gpi->text;
                }
                auto* lmi = dynamic_cast<LaunchModeItem*>( t );
                if( lmi )
                {
                    if( index.column() == 0 )
                    {
                        return lmi->mode->name();
                    } else if( index.column() == 1 )
                    {
                        LaunchConfiguration* l = configForIndex( index );
                        return l->type()->launcherForId( l->launcherForMode( lmi->mode->id() ) )->name();
                    }
                }
                break;
            }
            case Qt::DecorationRole:
            {
                auto* li = dynamic_cast<LaunchItem*>( t );
                if( index.column() == 0 && li )
                {
                    return li->launch->type()->icon();
                }
                auto* lmi = dynamic_cast<LaunchModeItem*>( t );
                if( lmi && index.column() == 0 )
                {
                    return lmi->mode->icon();
                }
                if ( index.column() == 0 && !index.parent().isValid() ) {
                    if (index.row() == 0) {
                        // global item
                        return QIcon::fromTheme(QStringLiteral("folder"));
                    } else {
                        // project item
                        return QIcon::fromTheme(QStringLiteral("folder-development"));
                    }
                }
                break;
            }
            case Qt::EditRole:
            {
                auto* li = dynamic_cast<LaunchItem*>( t );
                if( li )
                {
                    if( index.column() == 0 )
                    {
                        return li->launch->name();
                    } else if ( index.column() == 1 )
                    {
                        return li->launch->type()->id();
                    }
                }
                auto* lmi = dynamic_cast<LaunchModeItem*>( t );
                if( lmi && index.column() == 1  )
                {
                    return configForIndex( index )->launcherForMode( lmi->mode->id() );
                }
                break;
            }
            default:
                break;
        }
    }
    return QVariant();
}

QModelIndex LaunchConfigurationsModel::index(int row, int column, const QModelIndex& parent) const
{
    if( !hasIndex( row, column, parent ) )
        return QModelIndex();
    TreeItem* tree;

    if( !parent.isValid() )
    {
        tree = topItems.at( row );
    } else
    {
        auto* t = static_cast<TreeItem*>( parent.internalPointer() );
        tree = t->children.at( row );
    }
    if( tree )
    {
        return createIndex( row, column, tree );
    }
    return QModelIndex();
}

QModelIndex LaunchConfigurationsModel::parent(const QModelIndex& child) const
{
    if( child.isValid()  )
    {
        auto* t = static_cast<TreeItem*>( child.internalPointer() );
        if( t->parent )
        {
            return createIndex( t->parent->row, 0, t->parent );
        }
    }
    return QModelIndex();
}

int LaunchConfigurationsModel::rowCount(const QModelIndex& parent) const
{
    if( parent.column() > 0 )
        return 0;
    if( parent.isValid() )
    {
        auto* t = static_cast<TreeItem*>( parent.internalPointer() );
        return t->children.count();
    } else
    {
        return topItems.count();
    }
    return 0;
}

QVariant LaunchConfigurationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        if( section == 0 )
        {
            return i18nc("@title:column Name of the Launch Configurations", "Name");
        } else if( section == 1 )
        {
            return i18nc("@title:column Type of the Launch Configurations (i.e. Python Application, C++ Application)", "Type");
        }
    }
    return QVariant();
}

Qt::ItemFlags LaunchConfigurationsModel::flags(const QModelIndex& index) const
{
    if( index.isValid() && index.column() >= 0
        && index.column() < columnCount( QModelIndex() ) )
    {
        auto* t = static_cast<TreeItem*>( index.internalPointer() );
        if( t && ( dynamic_cast<LaunchItem*>( t ) || ( dynamic_cast<LaunchModeItem*>( t ) && index.column() == 1 ) ) )
        {
            return Qt::ItemFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
        } else if( t )
        {
            return Qt::ItemFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
        }
    }
    return Qt::NoItemFlags;
}

bool LaunchConfigurationsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if( index.isValid() && index.parent().isValid() && role == Qt::EditRole )
    {
        if( index.row() >= 0 && index.row() < rowCount( index.parent() ) )
        {
            auto* t = dynamic_cast<LaunchItem*>( static_cast<TreeItem*>( index.internalPointer() ) );
            if( t )
            {
                if( index.column() == 0 )
                {
                    t->launch->setName( value.toString() );
                } else if( index.column() == 1 )
                {
                    if (t->launch->type()->id() != value.toString()) {
                        t->launch->setType( value.toString() );
                        QModelIndex p = indexForConfig(t->launch);
                        qCDebug(SHELL) << data(p);
                        beginRemoveRows( p, 0, t->children.count() );
                        qDeleteAll( t->children );
                        t->children.clear();
                        endRemoveRows();
                        addLaunchModeItemsForLaunchConfig( t );
                    }
                }
                emit dataChanged(index, index);
                return true;
            }
            auto* lmi = dynamic_cast<LaunchModeItem*>( static_cast<TreeItem*>( index.internalPointer() ) );
            if( lmi )
            {
                if( index.column() == 1 && index.data(Qt::EditRole)!=value)
                {
                    LaunchConfiguration* l = configForIndex( index );
                    l->setLauncherForMode( lmi->mode->id(), value.toString() );
                    emit dataChanged(index, index);
                    return true;
                }
            }
        }
    }
    return false;
}

ILaunchMode* LaunchConfigurationsModel::modeForIndex( const QModelIndex& idx ) const
{
    if( idx.isValid() )
    {
        auto* item = dynamic_cast<LaunchModeItem*>( static_cast<TreeItem*>( idx.internalPointer() ) );
        if( item )
        {
            return item->mode;
        }
    }
    return nullptr;
}

LaunchConfiguration* LaunchConfigurationsModel::configForIndex(const QModelIndex& idx ) const
{
    if( idx.isValid() )
    {
        auto* item = dynamic_cast<LaunchItem*>( static_cast<TreeItem*>( idx.internalPointer() ) );
        if( item )
        {
            return item->launch;
        }
        auto* lmitem = dynamic_cast<LaunchModeItem*>( static_cast<TreeItem*>( idx.internalPointer() ) );
        if( lmitem )
        {
            return dynamic_cast<LaunchItem*>( lmitem->parent )->launch;
        }
    }
    return nullptr;
}

QModelIndex LaunchConfigurationsModel::indexForConfig( LaunchConfiguration* l ) const
{
    if( l )
    {
        TreeItem* tparent = topItems.at( 0 );
        if( l->project() )
        {
            for (TreeItem* t : topItems) {
                auto* pi = dynamic_cast<ProjectItem*>( t );
                if( pi && pi->project == l->project() )
                {
                    tparent = t;
                    break;
                }
            }
        }

        if( tparent )
        {
            for (TreeItem* c : std::as_const(tparent->children)) {
                auto* li = dynamic_cast<LaunchItem*>( c );
                if( li->launch && li->launch == l )
                {
                    return index( c->row, 0, index( tparent->row, 0, QModelIndex() ) );
                }
            }
        }
    }
    return QModelIndex();
}


void LaunchConfigurationsModel::deleteConfiguration( const QModelIndex& index )
{
    auto* t = dynamic_cast<LaunchItem*>( static_cast<TreeItem*>( index.internalPointer() ) );
    if( !t )
        return;
    beginRemoveRows( parent( index ), index.row(), index.row() );
    t->parent->children.removeAll( t );
    Core::self()->runControllerInternal()->removeLaunchConfiguration( t->launch );
    endRemoveRows();
}

void LaunchConfigurationsModel::createConfiguration(const QModelIndex& parent )
{
    if(!Core::self()->runController()->launchConfigurationTypes().isEmpty())
    {
        auto* t = static_cast<TreeItem*>( parent.internalPointer() );
        auto* ti = dynamic_cast<ProjectItem*>( t );

        LaunchConfigurationType* type = Core::self()->runController()->launchConfigurationTypes().at(0);
        QPair<QString,QString> launcher = qMakePair( type->launchers().at( 0 )->supportedModes().at(0), type->launchers().at( 0 )->id() );
        IProject* p = ( ti ? ti->project : nullptr );
        ILaunchConfiguration* l = Core::self()->runController()->createLaunchConfiguration( type, launcher, p );

        addConfiguration(l, parent);
    }
}

void LaunchConfigurationsModel::addConfiguration(ILaunchConfiguration* l, const QModelIndex& parent)
{
    if( parent.isValid() )
    {
        beginInsertRows( parent, rowCount( parent ), rowCount( parent ) );
        addItemForLaunchConfig( dynamic_cast<LaunchConfiguration*>( l ) );
        endInsertRows();
    }
    else
    {
        delete l;
        Q_ASSERT(false && "could not add the configuration");
    }
}

IProject* LaunchConfigurationsModel::projectForIndex(const QModelIndex& idx)
{
    if(idx.parent().isValid()) {
        return projectForIndex(idx.parent());
    } else {
        const auto* item = dynamic_cast<const ProjectItem*>(topItems[idx.row()]);
        return item ? item->project : nullptr;
    }
}

LaunchConfigPagesContainer::LaunchConfigPagesContainer( const QList<LaunchConfigurationPageFactory*>& factories, QWidget* parent )
    : QWidget(parent)
{
    setLayout( new QVBoxLayout( this ) );
    layout()->setContentsMargins( 0, 0, 0, 0 );
    QWidget* parentwidget = this;
    QTabWidget* tab = nullptr;
    if( factories.count() > 1 )
    {
        tab = new QTabWidget( this );
        parentwidget = tab;
        layout()->addWidget( tab );
    }
    for (LaunchConfigurationPageFactory* fac : factories) {
        LaunchConfigurationPage* page = fac->createWidget( parentwidget );
        if ( page->layout() ) {
            // remove margins for single page, reset margins for tabbed display
            const int pageMargin = tab ? -1 : 0;
            page->layout()->setContentsMargins(pageMargin, pageMargin, pageMargin, pageMargin);
        }
        pages.append( page );
        connect( page, &LaunchConfigurationPage::changed, this, &LaunchConfigPagesContainer::changed );
        if( tab ) {
            tab->addTab( page, page->icon(), page->title() );
        } else
        {
            layout()->addWidget( page );
        }
    }
}

void LaunchConfigPagesContainer::setLaunchConfiguration( KDevelop::LaunchConfiguration* l )
{
    config = l;
    for (LaunchConfigurationPage* p : std::as_const(pages)) {
        p->loadFromConfiguration( config->config(), config->project() );
    }
}

void LaunchConfigPagesContainer::save()
{
    for (LaunchConfigurationPage* p : std::as_const(pages)) {
        p->saveToConfiguration( config->config() );
    }
    config->config().sync();
}


QWidget* LaunchConfigurationModelDelegate::createEditor ( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    const auto* model = static_cast<const LaunchConfigurationsModel*>(index.model());
    ILaunchMode* mode = model->modeForIndex( index );
    LaunchConfiguration* config = model->configForIndex( index );
    if( index.column() == 1 && mode && config )
    {
        auto* box = new KComboBox( parent );
        const QList<ILauncher*> launchers = config->type()->launchers();
        for (auto* launcher : launchers) {
            if (launcher->supportedModes().contains(mode->id())) {
                box->addItem(launcher->name(), launcher->id());
            }
        }
        return box;
    } else if( !mode && config && index.column() == 1 )
    {
        auto* box = new KComboBox( parent );
        const QList<LaunchConfigurationType*> types = Core::self()->runController()->launchConfigurationTypes();
        for (auto* type : types) {
            box->addItem(type->name(), type->id());
        }
        return box;
    }
    return QStyledItemDelegate::createEditor ( parent, option, index );
}

void LaunchConfigurationModelDelegate::setEditorData ( QWidget* editor, const QModelIndex& index ) const
{
    const auto* model = static_cast<const LaunchConfigurationsModel*>(index.model());
    LaunchConfiguration* config = model->configForIndex( index );
    if( index.column() == 1 && config )
    {
        auto* box = qobject_cast<KComboBox*>( editor );
        box->setCurrentIndex( box->findData( index.data( Qt::EditRole ) ) );
    }
    else
    {
        QStyledItemDelegate::setEditorData ( editor, index );
    }
}

void LaunchConfigurationModelDelegate::setModelData ( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
    auto* lmodel = static_cast<LaunchConfigurationsModel*>(model);
    LaunchConfiguration* config = lmodel->configForIndex( index );
    if( index.column() == 1 && config )
    {
        auto* box = qobject_cast<KComboBox*>( editor );
        lmodel->setData( index, box->itemData( box->currentIndex() ) );
    }
    else
    {
        QStyledItemDelegate::setModelData ( editor, model, index );
    }
}

void LaunchConfigurationDialog::launchModeChanged(int item)
{
    QModelIndex index = tree->currentIndex();
    if(debugger->isVisible() && item>=0)
        tree->model()->setData(index.sibling(index.row(), 1), debugger->itemData(item), Qt::EditRole);
}

}

#include "moc_launchconfigurationdialog.cpp"
