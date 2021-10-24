/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sourceformatterselectionedit.h"
#include "ui_sourceformatterselectionedit.h"

#include "sourceformattercontroller.h"
#include "settings/editstyledialog.h"
#include "debug.h"
#include "core.h"
#include "plugincontroller.h"

#include <util/scopeddialog.h>

#include <KMessageBox>
#include <KTextEditor/Editor>
#include <KTextEditor/ConfigInterface>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KLocalizedString>
#include <KConfig>

#include <QMimeDatabase>

#include <algorithm>
#include <array>

#define STYLE_ROLE (Qt::UserRole+1)

using namespace KDevelop;

namespace {
namespace Strings {
QString userStylePrefix() { return QStringLiteral("User"); }
}
}

struct LanguageSettings {
    QList<QMimeType> mimetypes;
    QSet<KDevelop::SourceFormatter*> formatters;
    // weak pointers to selected formatter and style, no ownership
    KDevelop::SourceFormatter* selectedFormatter = nullptr;     // Should never be zero
    KDevelop::SourceFormatterStyle* selectedStyle = nullptr;  // TODO: can this be zero? Assume that not
};


using LanguageMap = QMap<QString, LanguageSettings>;
using FormatterMap = QMap<QString, SourceFormatter*>;


class KDevelop::SourceFormatterSelectionEditPrivate
{
public:
    Ui::SourceFormatterSelectionEdit ui;
    // Language name -> language settings
    LanguageMap languages;
    // formatter name -> formatter. Formatters owned by this
    FormatterMap formatters;
    KTextEditor::Document* document;
    KTextEditor::View* view;

    /**
     * Add the keys of @a languages to @a ui.cbLanguages.
     */
    void fillLanguageCombobox();
};

void SourceFormatterSelectionEditPrivate::fillLanguageCombobox()
{
    // Move the languages not supported by KDevelop to the bottom of the combobox.
    // Use std::array to avoid extra memory allocations.

    constexpr std::array unsupportedLanguages{
        QLatin1String("C#", 2),
        QLatin1String("Java", 4),
    };
    Q_ASSERT(std::is_sorted(unsupportedLanguages.cbegin(), unsupportedLanguages.cend()));
    std::array<QString, unsupportedLanguages.size()> skippedLanguages{};

    for (auto langIt = languages.keyBegin(); langIt != languages.keyEnd(); ++langIt) {
        const auto& name = *langIt;
        const auto unsupportedIt = std::find(unsupportedLanguages.cbegin(), unsupportedLanguages.cend(), name);
        if (unsupportedIt == unsupportedLanguages.cend()) {
            ui.cbLanguages->addItem(name);
        } else {
            skippedLanguages[unsupportedIt - unsupportedLanguages.cbegin()] = name;
        }
    }

    for (const auto& name : skippedLanguages) {
        if (!name.isEmpty()) {
            ui.cbLanguages->addItem(name);
        }
    }
}

SourceFormatterSelectionEdit::SourceFormatterSelectionEdit(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new SourceFormatterSelectionEditPrivate)
{
    Q_D(SourceFormatterSelectionEdit);

    d->ui.setupUi(this);

    connect(d->ui.cbLanguages, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, &SourceFormatterSelectionEdit::selectLanguage);
    connect(d->ui.cbFormatters, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, &SourceFormatterSelectionEdit::selectFormatter);
    connect(d->ui.styleList, &QListWidget::currentRowChanged, this, &SourceFormatterSelectionEdit::selectStyle);
    connect(d->ui.btnDelStyle, &QPushButton::clicked, this, &SourceFormatterSelectionEdit::deleteStyle);
    connect(d->ui.btnNewStyle, &QPushButton::clicked, this, &SourceFormatterSelectionEdit::newStyle);
    connect(d->ui.btnEditStyle, &QPushButton::clicked, this, &SourceFormatterSelectionEdit::editStyle);
    connect(d->ui.styleList, &QListWidget::itemChanged, this, &SourceFormatterSelectionEdit::styleNameChanged);

    d->document = KTextEditor::Editor::instance()->createDocument(this);
    d->document->setReadWrite(false);

    d->view = d->document->createView(d->ui.textEditor);
    d->view->setStatusBarEnabled(false);

    auto *layout2 = new QVBoxLayout(d->ui.textEditor);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(d->view);
    d->ui.textEditor->setLayout(layout2);
    d->view->show();

    KTextEditor::ConfigInterface *iface =
    qobject_cast<KTextEditor::ConfigInterface*>(d->view);
    if (iface) {
        iface->setConfigValue(QStringLiteral("dynamic-word-wrap"), false);
        iface->setConfigValue(QStringLiteral("icon-bar"), false);
        iface->setConfigValue(QStringLiteral("scrollbar-minimap"), false);
    }

    SourceFormatterController* controller = Core::self()->sourceFormatterControllerInternal();
    connect(controller, &SourceFormatterController::formatterLoaded,
            this, &SourceFormatterSelectionEdit::addSourceFormatter);
    connect(controller, &SourceFormatterController::formatterUnloading,
            this, &SourceFormatterSelectionEdit::removeSourceFormatter);
    const auto& formatters = controller->formatters();
    for (auto* formatter : formatters) {
        addSourceFormatterNoUi(formatter); // loadSettings() calls resetUi() once later
    }
}

SourceFormatterSelectionEdit::~SourceFormatterSelectionEdit()
{
    Q_D(SourceFormatterSelectionEdit);

    qDeleteAll(d->formatters);
}

static void selectAvailableStyle(LanguageSettings& lang)
{
    Q_ASSERT(!lang.selectedFormatter->styles.empty());
    lang.selectedStyle = *lang.selectedFormatter->styles.begin();
}

void SourceFormatterSelectionEdit::addSourceFormatterNoUi(ISourceFormatter* ifmt)
{
    Q_D(SourceFormatterSelectionEdit);

    qCDebug(SHELL) << "Adding source formatter:" << ifmt->name();

    SourceFormatter* formatter;
    FormatterMap::const_iterator iter = d->formatters.constFind(ifmt->name());
    if (iter == d->formatters.constEnd()) {
        formatter = Core::self()->sourceFormatterControllerInternal()->createFormatterForPlugin(ifmt);
        d->formatters[ifmt->name()] = formatter;
    } else {
        qCWarning(SHELL) << "formatter plugin" << ifmt->name() << "loading which was already seen before by SourceFormatterSelectionEdit";
        return;
    }

    for (const SourceFormatterStyle* style : qAsConst(formatter->styles)) {
        const auto mimeTypes = style->mimeTypes();
        for ( const SourceFormatterStyle::MimeHighlightPair& item : mimeTypes) {
            QMimeType mime = QMimeDatabase().mimeTypeForName(item.mimeType);
            if (!mime.isValid()) {
                qCWarning(SHELL) << "formatter plugin" << ifmt->name() << "supports unknown mimetype entry" << item.mimeType;
                continue;
            }
            QString languageName = item.highlightMode;
            LanguageSettings& l = d->languages[languageName];
            l.mimetypes.append(mime);
            l.formatters.insert( formatter );
            // init selection if needed
            if (!l.selectedFormatter) {
                l.selectedFormatter = formatter;
                selectAvailableStyle(l);
            }
        }
    }
}

void SourceFormatterSelectionEdit::addSourceFormatter(ISourceFormatter* ifmt)
{
    addSourceFormatterNoUi(ifmt);
    resetUi();
}

void SourceFormatterSelectionEdit::removeSourceFormatter(ISourceFormatter* ifmt)
{
    Q_D(SourceFormatterSelectionEdit);

    qCDebug(SHELL) << "Removing source formatter:" << ifmt->name();

    auto iter = d->formatters.find(ifmt->name());
    if (iter == d->formatters.end()) {
        qCWarning(SHELL) << "formatter plugin" << ifmt->name() << "unloading which was not seen before by SourceFormatterSelectionEdit";
        return;
    }
    d->formatters.erase(iter);
    auto formatter = iter.value();

    auto languageIter = d->languages.begin();
    while (languageIter != d->languages.end()) {
        LanguageSettings& l = languageIter.value();

        l.formatters.remove(formatter);
        if (l.formatters.isEmpty()) {
            languageIter = d->languages.erase(languageIter);
        } else {
            // reset selected formatter if needed
            if (l.selectedFormatter == formatter) {
                l.selectedFormatter = *l.formatters.begin();
                selectAvailableStyle(l);
            }
            ++languageIter;
        }
    }
    delete formatter;

    resetUi();
}

void SourceFormatterSelectionEdit::loadSettings(const KConfigGroup& config)
{
    Q_D(SourceFormatterSelectionEdit);

    for (auto& l : d->languages) {
        // Pick the first appropriate mimetype for this language
        const QList<QMimeType> mimetypes = l.mimetypes;
        for (const QMimeType& mimetype : mimetypes) {
            const QString formatter = config.readEntry(mimetype.name(), QString());
            const int pos = formatter.indexOf(QLatin1String("||"));
            const QString formatterName = formatter.left(pos);
            FormatterMap::const_iterator formatterIter = d->formatters.constFind(formatterName);
            if (formatterIter == d->formatters.constEnd()) {
                qCDebug(SHELL) << "Reference to unknown formatter" << formatterName;
                Q_ASSERT(!l.formatters.empty());        // otherwise there should be no entry for 'name'
                l.selectedFormatter = *l.formatters.begin();
                selectAvailableStyle(l);
            } else {
                l.selectedFormatter = formatterIter.value();
                const QString styleName = formatter.mid(pos + 2);
                SourceFormatter::StyleMap::const_iterator styleIter = l.selectedFormatter->styles.constFind(styleName);
                if (styleIter == l.selectedFormatter->styles.constEnd()) {
                    qCDebug(SHELL) << "No style" << styleName << "found for formatter" << formatterName;
                    selectAvailableStyle(l);
                } else {
                    l.selectedStyle = styleIter.value();
                }
            }
        }
        if (!l.selectedFormatter) {
            Q_ASSERT(!l.formatters.empty());
            l.selectedFormatter = *l.formatters.begin();
        }
        if (!l.selectedStyle) {
            selectAvailableStyle(l);
        }
    }

    resetUi();
}

void SourceFormatterSelectionEdit::resetUi()
{
    Q_D(SourceFormatterSelectionEdit);

    qCDebug(SHELL) << "Resetting UI";

    bool b = blockSignals( true );
    d->ui.cbLanguages->blockSignals(!b);
    d->ui.cbFormatters->blockSignals(!b);
    d->ui.styleList->blockSignals(!b);
    d->ui.cbLanguages->clear();
    d->ui.cbFormatters->clear();
    d->ui.styleList->clear();

    d->fillLanguageCombobox();
    if (d->ui.cbLanguages->count() == 0) {
        d->ui.cbLanguages->setEnabled(false);
        selectLanguage( -1 );
    } else
    {
        d->ui.cbLanguages->setCurrentIndex(0);
        d->ui.cbLanguages->setEnabled(true);
        selectLanguage( 0 );
    }
    updatePreview();
    blockSignals( b );
    d->ui.cbLanguages->blockSignals(b);
    d->ui.cbFormatters->blockSignals(b);
    d->ui.styleList->blockSignals(b);
}

void SourceFormatterSelectionEdit::saveSettings(KConfigGroup& config) const
{
    Q_D(const SourceFormatterSelectionEdit);

    // store formatters globally
    KConfigGroup globalConfig = Core::self()->sourceFormatterControllerInternal()->globalConfig();

    for (const SourceFormatter* fmt : qAsConst(d->formatters)) {
        KConfigGroup fmtgrp = globalConfig.group( fmt->formatter->name() );

        // delete all styles so we don't leave any behind when all user styles are deleted
        const auto oldStyleGroups = fmtgrp.groupList();
        for (const QString& subgrp : oldStyleGroups) {
            if( subgrp.startsWith( Strings::userStylePrefix() ) ) {
                fmtgrp.deleteGroup( subgrp );
            }
        }
        for (const SourceFormatterStyle* style : fmt->styles) {
            if( style->name().startsWith( Strings::userStylePrefix() ) )
            {
                KConfigGroup stylegrp = fmtgrp.group( style->name() );
                stylegrp.writeEntry( SourceFormatterController::styleCaptionKey(), style->caption() );
                stylegrp.writeEntry( SourceFormatterController::styleShowPreviewKey(), style->usePreview() );
                stylegrp.writeEntry( SourceFormatterController::styleContentKey(), style->content() );
                stylegrp.writeEntry( SourceFormatterController::styleMimeTypesKey(), style->mimeTypesVariant() );
                stylegrp.writeEntry( SourceFormatterController::styleSampleKey(), style->overrideSample() );
            }
        }
    }
    globalConfig.sync();

    // store selected formatters in given language
    for (const auto& setting : qAsConst(d->languages)) {
        for(const auto& mime : setting.mimetypes) {
            const QString formatterId = setting.selectedFormatter->formatter->name() + QLatin1String("||") + setting.selectedStyle->name();
            config.writeEntry(mime.name(), formatterId);
        }
    }
}


void SourceFormatterSelectionEdit::enableStyleButtons()
{
    Q_D(SourceFormatterSelectionEdit);

    bool userEntry = d->ui.styleList->currentItem()
                     && d->ui.styleList->currentItem()->data(STYLE_ROLE).toString().startsWith(Strings::userStylePrefix());

    QString languageName = d->ui.cbLanguages->currentText();
    QMap<QString, LanguageSettings>::const_iterator it = d->languages.constFind(languageName);
    bool hasEditWidget = false;
    if (it != d->languages.constEnd()) {
        const LanguageSettings& l = it.value();
        Q_ASSERT(l.selectedFormatter);
        ISourceFormatter* fmt = l.selectedFormatter->formatter;
        hasEditWidget = ( fmt && QScopedPointer<QObject>(fmt->editStyleWidget( l.mimetypes.first() )) );
    }
    d->ui.btnDelStyle->setEnabled(userEntry);
    d->ui.btnEditStyle->setEnabled(userEntry && hasEditWidget);
    d->ui.btnNewStyle->setEnabled(d->ui.cbFormatters->currentIndex() >= 0 && hasEditWidget);
}

void SourceFormatterSelectionEdit::selectLanguage( int idx )
{
    Q_D(SourceFormatterSelectionEdit);

    d->ui.cbFormatters->clear();
    if( idx < 0 )
    {
        d->ui.cbFormatters->setEnabled(false);
        selectFormatter( -1 );
        return;
    }
    d->ui.cbFormatters->setEnabled(true);
    {
        QSignalBlocker blocker(d->ui.cbFormatters);
        LanguageSettings& l = d->languages[d->ui.cbLanguages->itemText(idx)];
        for (const SourceFormatter* fmt : qAsConst(l.formatters)) {
            d->ui.cbFormatters->addItem(fmt->formatter->caption(), fmt->formatter->name());
        }
        d->ui.cbFormatters->setCurrentIndex(d->ui.cbFormatters->findData(l.selectedFormatter->formatter->name()));
    }
    selectFormatter(d->ui.cbFormatters->currentIndex());
    emit changed();
}

void SourceFormatterSelectionEdit::selectFormatter( int idx )
{
    Q_D(SourceFormatterSelectionEdit);

    d->ui.styleList->clear();
    if( idx < 0 )
    {
        d->ui.styleList->setEnabled(false);
        enableStyleButtons();
        return;
    }
    d->ui.styleList->setEnabled(true);
    LanguageSettings& l = d->languages[d->ui.cbLanguages->currentText()];
    Q_ASSERT( idx < l.formatters.size() );
    FormatterMap::const_iterator formatterIter = d->formatters.constFind(d->ui.cbFormatters->itemData(idx).toString());
    Q_ASSERT( formatterIter != d->formatters.constEnd() );
    Q_ASSERT( l.formatters.contains(formatterIter.value()) );
    if (l.selectedFormatter != formatterIter.value()) {
        l.selectedFormatter = formatterIter.value();
        l.selectedStyle = nullptr;    // will hold 0 until a style is picked
    }

    const auto usageHint = l.selectedFormatter->formatter->usageHint();
    if (usageHint.isEmpty()) {
        d->ui.usageHintLabel->hide(); // save UI space
    } else {
        d->ui.usageHintLabel->setText(usageHint);
        d->ui.usageHintLabel->show();
    }

    for (const SourceFormatterStyle* style : qAsConst(formatterIter.value()->styles)) {
        if (!style->supportsLanguage(d->ui.cbLanguages->currentText())) {
            // do not list items which do not support the selected language
            continue;
        }
        QListWidgetItem* item = addStyle( *style );
        if (style == l.selectedStyle) {
            d->ui.styleList->setCurrentItem(item);
        }
    }
    if (l.selectedStyle == nullptr) {
        d->ui.styleList->setCurrentRow(0);
    }
    enableStyleButtons();
    emit changed();
}

void SourceFormatterSelectionEdit::selectStyle( int row )
{
    Q_D(SourceFormatterSelectionEdit);

    if( row < 0 )
    {
        enableStyleButtons();
        return;
    }
    d->ui.styleList->setCurrentRow(row);
    LanguageSettings& l = d->languages[d->ui.cbLanguages->currentText()];
    l.selectedStyle = l.selectedFormatter->styles[d->ui.styleList->item(row)->data(STYLE_ROLE).toString()];
    enableStyleButtons();
    updatePreview();
    emit changed();
}

void SourceFormatterSelectionEdit::deleteStyle()
{
    Q_D(SourceFormatterSelectionEdit);

    Q_ASSERT( d->ui.styleList->currentRow() >= 0 );

    QListWidgetItem* item = d->ui.styleList->currentItem();

    LanguageSettings& l = d->languages[d->ui.cbLanguages->currentText()];
    SourceFormatter* fmt = l.selectedFormatter;
    SourceFormatter::StyleMap::iterator styleIter = fmt->styles.find(item->data( STYLE_ROLE ).toString());
    QStringList otherLanguageNames;
    QList<LanguageSettings*> otherlanguages;
    for (LanguageMap::iterator languageIter = d->languages.begin(); languageIter != d->languages.end(); ++languageIter) {
        if ( &languageIter.value() != &l && languageIter.value().selectedStyle == styleIter.value() ) {
            otherLanguageNames.append(languageIter.key());
            otherlanguages.append(&languageIter.value());
        }
    }
    if (!otherLanguageNames.empty() &&
        KMessageBox::warningContinueCancel(this,
        i18n("The style %1 is also used for the following languages:\n%2.\nAre you sure you want to delete it?",
        styleIter.value()->caption(), otherLanguageNames.join(QLatin1Char('\n'))), i18nc("@title:window", "Deleting Style")) != KMessageBox::Continue) {
        return;
    }
    d->ui.styleList->takeItem(d->ui.styleList->currentRow());
    fmt->styles.erase(styleIter);
    delete item;
    selectStyle(d->ui.styleList->count() > 0 ? 0 : -1);
    for (LanguageSettings* lang : qAsConst(otherlanguages)) {
        selectAvailableStyle(*lang);
    }
    updatePreview();
    emit changed();
}

void SourceFormatterSelectionEdit::editStyle()
{
    Q_D(SourceFormatterSelectionEdit);

    QString language = d->ui.cbLanguages->currentText();
    Q_ASSERT( d->languages.contains(language) );
    LanguageSettings& l = d->languages[language];
    SourceFormatter* fmt = l.selectedFormatter;

    QMimeType mimetype = l.mimetypes.first();
    if( QScopedPointer<QObject>(fmt->formatter->editStyleWidget( mimetype )) ) {
        KDevelop::ScopedDialog<EditStyleDialog> dlg(fmt->formatter, mimetype, *l.selectedStyle, this);
        if( dlg->exec() == QDialog::Accepted )
        {
            l.selectedStyle->setContent(dlg->content());
        }
        updatePreview();
        emit changed();
    }
}

void SourceFormatterSelectionEdit::newStyle()
{
    Q_D(SourceFormatterSelectionEdit);

    QListWidgetItem* item = d->ui.styleList->currentItem();
    LanguageSettings& l = d->languages[d->ui.cbLanguages->currentText()];
    SourceFormatter* fmt = l.selectedFormatter;
    int idx = 0;
    for (int i = 0; i < d->ui.styleList->count(); ++i) {
        QString name = d->ui.styleList->item(i)->data(STYLE_ROLE).toString();
        if( name.startsWith( Strings::userStylePrefix() ) && name.midRef( Strings::userStylePrefix().length() ).toInt() >= idx )
        {
            idx = name.midRef( Strings::userStylePrefix().length() ).toInt();
        }
    }
    // Increase number for next style
    idx++;
    auto* s = new SourceFormatterStyle(QStringLiteral("%1%2").arg(Strings::userStylePrefix()).arg(idx));
    if( item ) {
        SourceFormatterStyle* existstyle = fmt->styles[ item->data( STYLE_ROLE ).toString() ];
        s->setCaption( i18n( "New %1", existstyle->caption() ) );
        s->copyDataFrom( existstyle );
    } else {
        s->setCaption( i18n( "New Style" ) );
    }
    fmt->styles[ s->name() ] = s;
    QListWidgetItem* newitem = addStyle( *s );
    selectStyle(d->ui.styleList->row(newitem));
    d->ui.styleList->editItem(newitem);
    emit changed();
}

void SourceFormatterSelectionEdit::styleNameChanged( QListWidgetItem* item )
{
    Q_D(SourceFormatterSelectionEdit);

    if ( !item->isSelected() ) {
        return;
    }

    LanguageSettings& l = d->languages[d->ui.cbLanguages->currentText()];
    l.selectedStyle->setCaption( item->text() );
    emit changed();
}

QListWidgetItem* SourceFormatterSelectionEdit::addStyle( const SourceFormatterStyle& s )
{
    Q_D(SourceFormatterSelectionEdit);

    auto* item = new QListWidgetItem(d->ui.styleList);
    item->setText( s.caption() );
    item->setData( STYLE_ROLE, s.name() );
    if( s.name().startsWith( Strings::userStylePrefix() ) )
    {
        item->setFlags( item->flags() | Qt::ItemIsEditable );
    }
    d->ui.styleList->addItem(item);
    return item;
}

void SourceFormatterSelectionEdit::updatePreview()
{
    Q_D(SourceFormatterSelectionEdit);

    d->document->setReadWrite(true);

    QString langName = d->ui.cbLanguages->itemText(d->ui.cbLanguages->currentIndex());
    if( !langName.isEmpty() )
    {
        LanguageSettings& l = d->languages[langName];
        SourceFormatter* fmt = l.selectedFormatter;
        SourceFormatterStyle* style = l.selectedStyle;

        d->ui.descriptionLabel->setText(style->description());

        if( style->usePreview() )
        {
            ISourceFormatter* ifmt = fmt->formatter;
            QMimeType mime = l.mimetypes.first();
            d->document->setHighlightingMode(style->modeForMimetype(mime));

            //NOTE: this is ugly, but otherwise kate might remove tabs again :-/
            // see also: https://bugs.kde.org/show_bug.cgi?id=291074
            KTextEditor::ConfigInterface* iface = qobject_cast<KTextEditor::ConfigInterface*>(d->document);
            QVariant oldReplaceTabs;
            if (iface) {
                oldReplaceTabs = iface->configValue(QStringLiteral("replace-tabs"));
                iface->setConfigValue(QStringLiteral("replace-tabs"), false);
            }

            d->document->setText(ifmt->formatSourceWithStyle(*style, ifmt->previewText(*style, mime), QUrl(), mime));

            if (iface) {
                iface->setConfigValue(QStringLiteral("replace-tabs"), oldReplaceTabs);
            }

            d->ui.previewLabel->show();
            d->ui.textEditor->show();
        }else{
            d->ui.previewLabel->hide();
            d->ui.textEditor->hide();
        }
    } else
    {
        d->document->setText(i18n("No language selected"));
    }
    d->view->setCursorPosition(KTextEditor::Cursor(0, 0));
    d->document->setReadWrite(false);
}
