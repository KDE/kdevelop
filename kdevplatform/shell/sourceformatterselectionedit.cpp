/*
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sourceformatterselectionedit.h"
#include "ui_sourceformatterselectionedit.h"

#include "sourceformatterconfig.h"
#include "sourceformattercontroller.h"
#include "settings/editstyledialog.h"
#include "debug.h"
#include "core.h"

#include <util/scopeddialog.h>

#include <KMessageBox>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <KTextEditor/Document>
#include <KLocalizedString>
#include <KConfig>

#include <QListWidget>
#include <QMetaType>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSignalBlocker>
#include <QString>
#include <QStringView>
#include <QVariant>
#include <QWhatsThis>

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

using namespace KDevelop;

namespace {
constexpr int styleItemDataRole = Qt::UserRole + 1;
constexpr QLatin1String userStyleNamePrefix("User", 4);

void updateLabel(QLabel& label, const QString& text)
{
    if (text.isEmpty()) {
        label.hide(); // save UI space
    } else {
        label.setText(text);
        label.show();
    }
}

// std::map is chosen for iterator and reference stability relied upon by code in this file.
// Besides, std::map's interface is convenient for searching by name and iterating in order.
// std::set would be more convenient but cannot be used here, because its elements are
// immutable, which prevents style modifications, even if they don't affect the
// comparison, i.e. don't modify the style's name.
using StyleMap = SourceFormatterController::StyleMap;

enum class StyleCategory { UserDefined, Predefined };

/**
 * This class encapsulates an ISourceFormatter and its styles.
 *
 * The class is non-copyable and non-movable to ensure that references to it are never invalidated.
 */
class FormatterData
{
    Q_DISABLE_COPY_MOVE(FormatterData)
public:
    explicit FormatterData(const ISourceFormatter& formatter, StyleMap&& styles)
        : m_formatter{formatter}
        , m_name{m_formatter.name()}
        , m_styles{std::move(styles)}
    {
    }

    const QString& name() const
    {
        return m_name;
    }
    const ISourceFormatter& formatter() const
    {
        return m_formatter;
    }

    SourceFormatterStyle* findStyle(QStringView styleName)
    {
        const auto it = m_styles.find(styleName);
        return it == m_styles.end() ? nullptr : &it->second;
    }

    template<typename StyleUser>
    void forEachStyle(StyleUser callback)
    {
        for (auto it = m_styles.begin(), end = m_styles.end(); it != end; ++it) {
            callback(it->second);
        }
    }

    template<typename ConstStyleUser>
    void forEachUserDefinedStyle(ConstStyleUser callback) const
    {
        for (auto it = m_userStyleRange.first(m_styles); it != m_userStyleRange.last(); ++it) {
            Q_ASSERT(it->first.startsWith(userStyleNamePrefix));
            callback(it->second);
        }
    }

    template<typename StyleAndCategoryUser>
    void forEachSupportingStyleInUiOrder(const QString& supportedLanguageName, StyleAndCategoryUser callback);

    void assertExistingStyle(const SourceFormatterStyle& style)
    {
        Q_ASSERT(findStyle(style.name()) == &style);
    }

    void assertNullOrExistingStyle(const SourceFormatterStyle* style)
    {
        Q_ASSERT(!style || findStyle(style->name()) == style);
    }

    void removeStyle(const SourceFormatterStyle& style)
    {
        assertExistingStyle(style);
        Q_ASSERT_X(style.name().startsWith(userStyleNamePrefix), Q_FUNC_INFO, "Cannot remove a predefined style.");
        const auto removedCount = m_styles.erase(style.name());
        Q_ASSERT(removedCount == 1);
    }

    SourceFormatterStyle& addNewStyle()
    {
        int maxUserStyleIndex = 0;
        forEachUserDefinedStyle([&maxUserStyleIndex](const SourceFormatterStyle& userStyle) {
            const int index = QStringView{userStyle.name()}.mid(userStyleNamePrefix.size()).toInt();
            // index == 0 if conversion to int fails. Ignore such invalid user-defined style names.
            maxUserStyleIndex = std::max(maxUserStyleIndex, index);
        });

        // Use the next available user-defined style index in the new style's name.
        const QString newStyleName = userStyleNamePrefix + QString::number(maxUserStyleIndex + 1);

        const auto oldStyleCount = m_styles.size();
        const auto newStyleIt =
            m_styles.try_emplace(std::as_const(m_userStyleRange).last(), newStyleName, newStyleName);
        Q_ASSERT(newStyleIt->second.name() == newStyleIt->first);
        Q_ASSERT(m_styles.size() == oldStyleCount + 1);

        return newStyleIt->second;
    }

private:
    class UserStyleRange
    {
    public:
        using iterator = StyleMap::iterator;
        using const_iterator = StyleMap::const_iterator;

        explicit UserStyleRange(StyleMap& styles)
        {
            const auto firstUserStyle = styles.lower_bound(userStyleNamePrefix);

            // m_lastBeforeUserStyles == styles.end() means that EITHER the first style is user-defined
            // OR there are no user-defined styles and the first predefined style name would compare
            // greater than any user-defined style name.
            m_lastBeforeUserStyles = firstUserStyle == styles.begin() ? styles.end() : std::prev(firstUserStyle);

            m_firstAfterUserStyles = std::find_if_not(firstUserStyle, styles.end(), [](const auto& pair) {
                return pair.first.startsWith(userStyleNamePrefix);
            });
        }

        iterator first(StyleMap& styles)
        {
            return m_lastBeforeUserStyles == styles.end() ? styles.begin() : std::next(m_lastBeforeUserStyles);
        }
        const_iterator first(const StyleMap& styles) const
        {
            return m_lastBeforeUserStyles == styles.cend() ? styles.cbegin() : std::next(m_lastBeforeUserStyles);
        }

        iterator last()
        {
            return m_firstAfterUserStyles;
        }
        const_iterator last() const
        {
            return m_firstAfterUserStyles;
        }

    private:
        // The stored iterators point to predefined styles or equal styles.end(). They stay valid and
        // correct because only user-defined styles are inserted and erased. Furthermore, user-defined
        // styles are always positioned between these two iterators OR from the beginning until
        // m_firstAfterUserStyles if m_lastBeforeUserStyles == styles.end().
        iterator m_lastBeforeUserStyles;
        iterator m_firstAfterUserStyles;
    };

    const ISourceFormatter& m_formatter;
    const QString m_name; ///< cached m_formatter.name()
    StyleMap m_styles;
    UserStyleRange m_userStyleRange{m_styles};
};

template<typename StyleAndCategoryUser>
void FormatterData::forEachSupportingStyleInUiOrder(const QString& supportedLanguageName, StyleAndCategoryUser callback)
{
    std::vector<SourceFormatterStyle*> filteredStyles;
    // Few if any styles are filtered out => reserve the maximum possible size.
    filteredStyles.reserve(m_styles.size());

    const auto filterStyles = [&supportedLanguageName, &filteredStyles](StyleMap::iterator first,
                                                                        StyleMap::iterator last) {
        for (; first != last; ++first) {
            auto& style = first->second;
            // Filter out styles that do not support the selected language.
            if (style.supportsLanguage(supportedLanguageName)) {
                filteredStyles.push_back(&style);
            }
        }
    };

    const auto sortAndUseFilteredStyles = [&callback, &filteredStyles](StyleCategory category) {
        const auto compareForUi = [](const SourceFormatterStyle* a, const SourceFormatterStyle* b) {
            const auto& left = a->caption();
            const auto& right = b->caption();
            const int ciResult = QString::compare(left, right, Qt::CaseInsensitive);
            if (ciResult != 0) {
                return ciResult < 0;
            }
            return left < right; // compare case-sensitively as a fallback
        };
        // Stable sort ensures that styles with equal captions are ordered predictably (by style name).
        std::stable_sort(filteredStyles.begin(), filteredStyles.end(), compareForUi);

        for (auto* style : filteredStyles) {
            callback(*style, category);
        }
    };

    const auto firstUserStyle = m_userStyleRange.first(m_styles);

    // User-defined styles are more likely to be selected => show them on top of the list.
    filterStyles(firstUserStyle, m_userStyleRange.last());
    sortAndUseFilteredStyles(StyleCategory::UserDefined);

    filteredStyles.clear();
    filterStyles(m_styles.begin(), firstUserStyle);
    filterStyles(m_userStyleRange.last(), m_styles.end());
    sortAndUseFilteredStyles(StyleCategory::Predefined);
}

class LanguageSettings
{
public:
    explicit LanguageSettings(const QString& name, FormatterData& supportingFormatter)
        : m_name{name}
        , m_supportingFormatters{&supportingFormatter}
        , m_selectedFormatter{&supportingFormatter}
    {
    }

    const QString& name() const
    {
        return m_name;
    }
    FormatterData& selectedFormatter() const
    {
        return *m_selectedFormatter;
    }
    SourceFormatterStyle* selectedStyle() const
    {
        return m_selectedStyle;
    }

    bool isFormatterSupporting(const FormatterData& formatter) const
    {
        return findSupportingFormatter(formatter) != m_supportingFormatters.cend();
    }

    const auto& supportingFormatters() const
    {
        return m_supportingFormatters;
    }

    const QMimeType& defaultMimeType() const
    {
        Q_ASSERT_X(!m_mimeTypes.empty(), Q_FUNC_INFO,
                   "A valid MIME type must be added right after constructing a language. "
                   "MIME types are never removed.");
        return m_mimeTypes.front();
    }

    void setSelectedFormatter(FormatterData& selectedFormatter)
    {
        Q_ASSERT_X(m_selectedFormatter != &selectedFormatter, Q_FUNC_INFO,
                   "Reselecting an already selected formatter is currently not supported. "
                   "If this is needed, an early return here would probably be correct.");
        Q_ASSERT(isFormatterSupporting(selectedFormatter));
        m_selectedFormatter = &selectedFormatter;
        m_selectedStyle = nullptr;
    }

    void unselectStyle(const SourceFormatterStyle& selectedStyle)
    {
        Q_ASSERT(m_selectedStyle == &selectedStyle);
        m_selectedStyle = nullptr;
    }

    void setSelectedStyle(SourceFormatterStyle* style)
    {
        m_selectedFormatter->assertNullOrExistingStyle(style);
        m_selectedStyle = style;
    }

    void addMimeType(QMimeType&& mimeType)
    {
        Q_ASSERT(mimeType.isValid());
        if (std::find(m_mimeTypes.cbegin(), m_mimeTypes.cend(), mimeType) == m_mimeTypes.cend()) {
            m_mimeTypes.push_back(std::move(mimeType));
        }
    }

    void addSupportingFormatter(FormatterData& formatter)
    {
        // A linear search by pointer is much faster than a binary search by name => fast path:
        if (isFormatterSupporting(formatter)) {
            return; // already supporting => nothing to do
        }
        const auto insertionPosition = std::lower_bound(m_supportingFormatters.cbegin(), m_supportingFormatters.cend(),
                                                        &formatter, [](const FormatterData* a, const FormatterData* b) {
                                                            return a->name() < b->name();
                                                        });
        Q_ASSERT(insertionPosition == m_supportingFormatters.cend() || formatter.name() < (*insertionPosition)->name());
        m_supportingFormatters.insert(insertionPosition, &formatter);
    }

    /**
     * Removes @p formatter from the set of formatters that support this language
     * unless it is the single supporting formatter.
     *
     * @return @c true if @p formatter was not in the set or was removed from the set;
     *         @c false if @p formatter was the single supporting formatter and was not removed.
     */
    bool removeSupportingFormatter(const FormatterData& formatter)
    {
        const auto it = findSupportingFormatter(formatter);
        if (it == m_supportingFormatters.cend()) {
            return true; // formatter is not supporting => nothing to do
        }
        if (m_supportingFormatters.size() == 1) {
            return false; // removing the last supporting formatter would break an invariant => fail
        }

        m_supportingFormatters.erase(it);
        if (m_selectedFormatter == &formatter) {
            selectFirstFormatterAndUnselectStyle();
        }
        return true;
    }

    void readSelectedFormatterAndStyle(const KConfigGroup& config)
    {
        selectFirstFormatterAndUnselectStyle(); // ensure predictable selection in case of error
        for (const auto& mimeType : m_mimeTypes) {
            SourceFormatter::ConfigForMimeType parser(config, mimeType);
            if (parser.isValid()) {
                setSelectedFormatterAndStyle(std::move(parser), mimeType);
                // A valid entry for a MIME type has been processed. We are done here. Keep the first formatter
                // selected and style unselected in case of unknown or unsupporting formatter or style name.
                break;
            }
        }
    }

    void saveSettings(KConfigGroup& config) const
    {
        for (const auto& mimeType : m_mimeTypes) {
            SourceFormatter::ConfigForMimeType::writeEntry(config, mimeType, m_selectedFormatter->name(),
                                                           m_selectedStyle);
        }
    }

private:
    std::vector<FormatterData*>::const_iterator findSupportingFormatter(const FormatterData& formatter) const
    {
        return std::find(m_supportingFormatters.cbegin(), m_supportingFormatters.cend(), &formatter);
    }

    void selectFirstFormatterAndUnselectStyle()
    {
        m_selectedFormatter = m_supportingFormatters.front();
        m_selectedStyle = nullptr;
    }

    void setSelectedFormatterAndStyle(const SourceFormatter::ConfigForMimeType& parser, const QMimeType& mimeType)
    {
        const QStringView formatterName = parser.formatterName();
        const auto formatterIt = std::find_if(m_supportingFormatters.cbegin(), m_supportingFormatters.cend(),
                                              [formatterName](const FormatterData* f) {
                                                  return f->name() == formatterName;
                                              });
        if (formatterIt == m_supportingFormatters.cend()) {
            qCWarning(SHELL) << "Unknown or unsupporting formatter" << formatterName << "is selected for MIME type"
                             << mimeType.name();
            return;
        }
        m_selectedFormatter = *formatterIt;

        m_selectedStyle = m_selectedFormatter->findStyle(parser.styleName());
        if (!m_selectedStyle) {
            qCWarning(SHELL) << "The style" << parser.styleName() << "selected for MIME type" << mimeType.name()
                             << "does not belong to the selected formatter" << formatterName;
        } else if (!m_selectedStyle->supportsLanguage(m_name)) {
            qCWarning(SHELL) << *m_selectedStyle << "selected for MIME type" << mimeType.name()
                             << "does not support the language" << m_name;
            m_selectedStyle = nullptr;
        }
    }

    QString m_name; ///< the name of this language, logically const
    /// unique MIME types that belong to this language; a sequence container to keep MIME type priority order
    std::vector<QMimeType> m_mimeTypes;

    // m_supportingFormatters is a sequence container rather than a map or a set for the following reasons:
    // 1) with only two formatter plugins linear search is faster than binary search
    //    because QString's equality comparison is faster than ordering comparison;
    // 2) map or set m_supportingFormatters is error-prone - makes it easy to accidentally search by name when
    //    more precise and efficient search by pointer is intended: m_supportingFormatters.find(formatter)
    /**
     * Unique formatters that support this language. The pointers are non-owning.
     * Ordered by FormatterData::name() to ensure UI item order stability.
     * Invariants: 1) the container is never empty; 2) no nullptr values.
     */
    std::vector<FormatterData*> m_supportingFormatters;
    /// invariant: @a m_supportingFormatters contains @a m_selectedFormatter => never nullptr
    FormatterData* m_selectedFormatter;
    /// pointer to one of @a m_selectedFormatter's styles or nullptr if unselected
    SourceFormatterStyle* m_selectedStyle = nullptr;
};

} // unnamed namespace

Q_DECLARE_METATYPE(FormatterData*)

enum class NewItemPosition { Bottom, Top };

class KDevelop::SourceFormatterSelectionEditPrivate
{
    Q_DISABLE_COPY_MOVE(SourceFormatterSelectionEditPrivate)
public:
    SourceFormatterSelectionEditPrivate() = default;

    Ui::SourceFormatterSelectionEdit ui;
    // formatters is a sequence container for the same reasons as LanguageSettings::m_supportingFormatters
    // (see the comment above that data member).
    /// All known (added) formatters; unordered; no nullptr values.
    std::vector<std::unique_ptr<FormatterData>> formatters;

    // languages is a sequence container rather than a map or a set for the following reasons:
    // 1) the number of languages is small (normally less than 10), so the linear complexity of
    //    insertion and removal is not a problem;
    // 2) iterator and reference stability does not matter, because the current language is reset
    //    to the first language after insertion or removal.
    // 3) a map is less convenient because its element is a pair;
    // 4) std::set cannot be used, because its elements are immutable;
    // 5) boost::container::flat_set adds a dependency on boost and doesn't substantially simplify the code.
    /**
     * Unique programming languages displayed in the UI to support per-language formatting configuration.
     * Ordered by LanguageSettings::name() to support UI item order stability.
     */
    std::vector<LanguageSettings> languages;
    LanguageSettings* currentLanguagePtr = nullptr; ///< cached languageSelectedInUi()
    KTextEditor::Document* document;
    KTextEditor::View* view;

    // Most member functions below have preconditions. They may only be called when the values,
    // specified in their documentations, are certain to be the same in the model and in the UI.
    // If that is not the case, obtain the needed values in some other way.

    /// @pre model-UI matches: language
    LanguageSettings& currentLanguage()
    {
        Q_ASSERT(currentLanguagePtr);
        Q_ASSERT(currentLanguagePtr == &languageSelectedInUi());
        return *currentLanguagePtr;
    }

    /// @pre model-UI matches: language, formatter
    FormatterData& currentFormatter()
    {
        auto& currentFormatter = currentLanguage().selectedFormatter();
        Q_ASSERT(&currentFormatter == &formatterSelectedInUi());
        return currentFormatter;
    }

    /// @pre model-UI matches: language, formatter, style
    /// @pre current style is valid
    SourceFormatterStyle& validCurrentStyle()
    {
        auto* const style = currentStyle();
        Q_ASSERT(style);
        return *style;
    }

    /// @pre model-UI matches: language, formatter, style
    void assertValidSelectedStyleItem(const QListWidgetItem* item)
    {
        Q_ASSERT(item);
        Q_ASSERT(&styleFromVariant(item->data(styleItemDataRole)) == &validCurrentStyle());
    }

    /// @pre !languages.empty()
    LanguageSettings& languageSelectedInUi()
    {
        Q_ASSERT(!languages.empty());
        const auto languageName = ui.cbLanguages->currentText();
        Q_ASSERT(!languageName.isEmpty());

        const auto it = languageLowerBound(languageName);
        Q_ASSERT(it != languages.end());
        Q_ASSERT(it->name() == languageName);
        return *it;
    }

    /// @pre model-UI matches: language
    FormatterData& formatterSelectedInUi()
    {
        const auto currentData = ui.cbFormatters->currentData();
        Q_ASSERT(currentData.canConvert<FormatterData*>());
        auto* const formatter = currentData.value<FormatterData*>();
        Q_ASSERT(formatter);
        Q_ASSERT(currentLanguage().isFormatterSupporting(*formatter));
        return *formatter;
    }

    /// @pre model-UI matches: language, formatter
    SourceFormatterStyle* styleSelectedInUi()
    {
        const auto selectedIndexes = ui.styleList->selectionModel()->selectedIndexes();
        if (selectedIndexes.empty()) {
            return nullptr;
        }
        Q_ASSERT_X(selectedIndexes.size() == 1, Q_FUNC_INFO, "SingleSelection is assumed.");

        auto& style = styleFromVariant(selectedIndexes.constFirst().data(styleItemDataRole));
        currentFormatter().assertExistingStyle(style);
        return &style;
    }

    /// @pre model-UI matches: language, formatter
    void updateUiForCurrentFormatter();

    /// @pre languages.empty() OR model-UI matches: language, formatter, style
    void updateUiForCurrentStyle()
    {
        updateStyleButtons();
        updatePreview();
    }

    /// @pre languages.empty() OR model-UI matches: language, formatter, style
    void updateStyleButtons();
    /// @pre languages.empty() OR model-UI matches: language, formatter, style
    void updatePreview();

    QListWidgetItem& addStyleItem(SourceFormatterStyle& style, StyleCategory category,
                                  NewItemPosition position = NewItemPosition::Bottom);

    /**
     * Add the names of @a languages to @a ui.cbLanguages.
     */
    void fillLanguageCombobox();

    void addMimeTypes(const SourceFormatterStyle::MimeList& mimeTypes, FormatterData& formatter);

private:
    static bool isUserDefinedStyle(const SourceFormatterStyle& style)
    {
        return style.name().startsWith(userStyleNamePrefix);
    }

    static SourceFormatterStyle& styleFromVariant(const QVariant& variant)
    {
        Q_ASSERT(variant.canConvert<SourceFormatterStyle*>());
        auto* const style = variant.value<SourceFormatterStyle*>();
        Q_ASSERT(style);
        return *style;
    }

    std::vector<LanguageSettings>::iterator languageLowerBound(QStringView languageName)
    {
        return std::lower_bound(languages.begin(), languages.end(), languageName,
                                [](const LanguageSettings& lang, QStringView languageName) {
                                    return lang.name() < languageName;
                                });
    }

    /// @pre model-UI matches: language, formatter, style
    SourceFormatterStyle* currentStyle()
    {
        auto* const style = currentLanguage().selectedStyle();
        Q_ASSERT(style == styleSelectedInUi());
        return style;
    }

    LanguageSettings& addSupportingFormatterToLanguage(const QString& languageName, FormatterData& formatter);
};

void SourceFormatterSelectionEditPrivate::updateUiForCurrentFormatter()
{
    ui.formatterDescriptionButton->setWhatsThis(currentFormatter().formatter().description());
    updateLabel(*ui.usageHintLabel, currentFormatter().formatter().usageHint());

    {
        const QSignalBlocker blocker(ui.styleList);
        ui.styleList->clear();

        currentFormatter().forEachSupportingStyleInUiOrder(currentLanguage().name(),
                                                           [this](SourceFormatterStyle& style, StyleCategory category) {
                                                               auto& item = addStyleItem(style, category);
                                                               if (&style == currentLanguage().selectedStyle()) {
                                                                   ui.styleList->setCurrentItem(&item);
                                                               }
                                                           });
    }
    Q_ASSERT_X(currentLanguage().selectedStyle() == styleSelectedInUi(), Q_FUNC_INFO,
               "The selected style is not among the supporting styles!");

    updateUiForCurrentStyle();
}

void SourceFormatterSelectionEditPrivate::updateStyleButtons()
{
    if (languages.empty() || !currentStyle()) {
        ui.btnDelStyle->setEnabled(false);
        ui.btnEditStyle->setEnabled(false);
        // Forbid creating a new style not based on an existing (selected) style,
        // because it would be useless with no MIME types and no way to add them.
        ui.btnNewStyle->setEnabled(false);
        return;
    }

    const bool userDefined = isUserDefinedStyle(validCurrentStyle());
    const bool hasEditWidget = currentFormatter().formatter().hasEditStyleWidget();

    ui.btnDelStyle->setEnabled(userDefined);
    ui.btnEditStyle->setEnabled(userDefined && hasEditWidget);
    ui.btnNewStyle->setEnabled(hasEditWidget);
}

void SourceFormatterSelectionEditPrivate::updatePreview()
{
    if (languages.empty() || !currentStyle()) {
        ui.descriptionLabel->hide();
        ui.previewArea->hide();
        return;
    }

    const auto& currentStyle = validCurrentStyle();

    updateLabel(*ui.descriptionLabel, currentStyle.description());

    if (!currentStyle.usePreview()) {
        ui.previewArea->hide();
        return;
    }

    document->setReadWrite(true);

    const auto& mimeType = currentLanguage().defaultMimeType();
    document->setHighlightingMode(currentStyle.modeForMimetype(mimeType));

    //NOTE: this is ugly, but otherwise kate might remove tabs again :-/
    // see also: https://bugs.kde.org/show_bug.cgi?id=291074
    const QString replaceTabsConfigKey = QStringLiteral("replace-tabs");
    const auto oldReplaceTabsConfigValue = document->configValue(replaceTabsConfigKey);
    document->setConfigValue(replaceTabsConfigKey, false);

    const auto& formatter = currentFormatter().formatter();
    document->setText(
        formatter.formatSourceWithStyle(currentStyle, formatter.previewText(currentStyle, mimeType), QUrl(), mimeType));

    document->setConfigValue(replaceTabsConfigKey, oldReplaceTabsConfigValue);

    ui.previewArea->show();
    view->setCursorPosition(KTextEditor::Cursor(0, 0));

    document->setReadWrite(false);
}

QListWidgetItem& SourceFormatterSelectionEditPrivate::addStyleItem(SourceFormatterStyle& style, StyleCategory category,
                                                                   NewItemPosition position)
{
    Q_ASSERT_X((category == StyleCategory::UserDefined) == isUserDefinedStyle(style), Q_FUNC_INFO,
               "Wrong style category!");

    auto* const item = new QListWidgetItem(style.caption());
    item->setData(styleItemDataRole, QVariant::fromValue(&style));
    if (category == StyleCategory::UserDefined) {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }

    switch (position) {
    case NewItemPosition::Bottom:
        ui.styleList->addItem(item);
        break;
    case NewItemPosition::Top:
        ui.styleList->insertItem(0, item);
        break;
    }

    return *item;
}

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

    for (const auto& lang : languages) {
        const QString& name = lang.name();
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

void SourceFormatterSelectionEditPrivate::addMimeTypes(const SourceFormatterStyle::MimeList& mimeTypes,
                                                       FormatterData& formatter)
{
    for (const auto& item : mimeTypes) {
        QMimeType mime = QMimeDatabase().mimeTypeForName(item.mimeType);
        if (!mime.isValid()) {
            qCWarning(SHELL) << "formatter plugin" << formatter.name() << "supports unknown MIME type entry"
                             << item.mimeType;
            continue;
        }
        auto& lang = addSupportingFormatterToLanguage(item.highlightMode, formatter);
        lang.addMimeType(std::move(mime));
    }
}

LanguageSettings& SourceFormatterSelectionEditPrivate::addSupportingFormatterToLanguage(const QString& languageName,
                                                                                        FormatterData& formatter)
{
    Q_ASSERT_X(!languageName.isEmpty(), Q_FUNC_INFO,
               "Empty language name should not be displayed in the UI and should be skipped earlier.");
    const auto it = languageLowerBound(languageName);
    if (it == languages.end() || it->name() != languageName) {
        return *languages.emplace(it, languageName, formatter);
    }
    it->addSupportingFormatter(formatter);
    return *it;
}

SourceFormatterSelectionEdit::SourceFormatterSelectionEdit(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new SourceFormatterSelectionEditPrivate)
{
    Q_D(SourceFormatterSelectionEdit);

    d->ui.setupUi(this);
    // Aligning to the left prevents the widgets on the left side from moving right/left whenever
    // style description and preview on the right side become hidden/shown (when a different style
    // is selected or the current style is unselected).
    d->ui.mainLayout->setAlignment(Qt::AlignLeft);

    connect(d->ui.cbLanguages, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, &SourceFormatterSelectionEdit::selectLanguage);
    connect(d->ui.cbFormatters, QOverload<int>::of(&KComboBox::currentIndexChanged),
            this, &SourceFormatterSelectionEdit::selectFormatter);
    connect(d->ui.styleList, &QListWidget::itemSelectionChanged, this,
            &SourceFormatterSelectionEdit::styleSelectionChanged);
    connect(d->ui.btnDelStyle, &QPushButton::clicked, this, &SourceFormatterSelectionEdit::deleteStyle);
    connect(d->ui.btnNewStyle, &QPushButton::clicked, this, &SourceFormatterSelectionEdit::newStyle);
    connect(d->ui.btnEditStyle, &QPushButton::clicked, this, &SourceFormatterSelectionEdit::editStyle);
    connect(d->ui.styleList, &QListWidget::itemChanged, this, &SourceFormatterSelectionEdit::styleNameChanged);

    const auto showWhatsThisOnClick = [](QAbstractButton* button) {
        connect(button, &QAbstractButton::clicked, button, [button] {
            QWhatsThis::showText(button->mapToGlobal(QPoint{0, 0}), button->whatsThis(), button);
        });
    };
    showWhatsThisOnClick(d->ui.usageHelpButton);
    showWhatsThisOnClick(d->ui.formatterDescriptionButton);

    d->document = KTextEditor::Editor::instance()->createDocument(this);
    d->document->setReadWrite(false);

    d->view = d->document->createView(d->ui.textEditor);
    d->view->setStatusBarEnabled(false);

    auto *layout2 = new QVBoxLayout(d->ui.textEditor);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(d->view);
    d->ui.textEditor->setLayout(layout2);

    d->view->setConfigValue(QStringLiteral("dynamic-word-wrap"), false);
    d->view->setConfigValue(QStringLiteral("icon-bar"), false);
    d->view->setConfigValue(QStringLiteral("scrollbar-minimap"), false);
    d->view->show();

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

SourceFormatterSelectionEdit::~SourceFormatterSelectionEdit() = default;

void SourceFormatterSelectionEdit::addSourceFormatterNoUi(ISourceFormatter* ifmt)
{
    Q_D(SourceFormatterSelectionEdit);

    const QString formatterName = ifmt->name();
    qCDebug(SHELL) << "Adding source formatter:" << formatterName;

    if (std::any_of(d->formatters.cbegin(), d->formatters.cend(), [&formatterName](const auto& f) {
            return formatterName == f->name();
        })) {
        qCWarning(SHELL) << "formatter plugin" << formatterName
                         << "loading which was already seen before by SourceFormatterSelectionEdit";
        return;
    }

    d->formatters.push_back(std::make_unique<FormatterData>(
        *ifmt, Core::self()->sourceFormatterControllerInternal()->stylesForFormatter(*ifmt)));
    auto& formatter = *d->formatters.back();

    // The loop below can invalidate currentLanguagePtr; resetUi() selects the first language anyway.
    d->currentLanguagePtr = nullptr;

    // Built-in styles share the same MIME list object. User-defined styles usually have MIME lists equal to the
    // shared built-in list. addedMimeLists allows to quickly skip duplicate lists as an optimization.
    // Note that a single addedMimeLists object cannot be shared by consecutive calls to this function, because
    // formatter is different in each call, and formatter is added to language settings in the loop below.
    std::vector<SourceFormatterStyle::MimeList> addedMimeLists;
    formatter.forEachStyle([d, &formatter, &addedMimeLists](const SourceFormatterStyle& style) {
        auto mimeTypes = style.mimeTypes();
        if (std::find(addedMimeLists.cbegin(), addedMimeLists.cend(), mimeTypes) != addedMimeLists.cend()) {
            return; // this is a duplicate list
        }
        addedMimeLists.push_back(std::move(mimeTypes));
        d->addMimeTypes(addedMimeLists.back(), formatter);
    });
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

    const auto formatterIt = std::find_if(d->formatters.cbegin(), d->formatters.cend(), [ifmt](const auto& f) {
        return ifmt == &f->formatter();
    });
    if (formatterIt == d->formatters.cend()) {
        qCWarning(SHELL) << "formatter plugin" << ifmt->name() << "unloading which was not seen before by SourceFormatterSelectionEdit";
        return;
    }

    // The loop below can invalidate currentLanguagePtr; resetUi() selects the first language anyway.
    d->currentLanguagePtr = nullptr;

    for (auto languageIt = d->languages.begin(); languageIt != d->languages.end();) {
        if (languageIt->removeSupportingFormatter(**formatterIt)) {
            ++languageIt;
        } else {
            // Remove the language, for which no supporting formatters remain.
            languageIt = d->languages.erase(languageIt);
        }
    }

    d->formatters.erase(formatterIt);

    resetUi();
}

void SourceFormatterSelectionEdit::loadSettings(const KConfigGroup& config)
{
    Q_D(SourceFormatterSelectionEdit);

    for (auto& lang : d->languages) {
        lang.readSelectedFormatterAndStyle(config);
    }
    resetUi();
}

void SourceFormatterSelectionEdit::resetUi()
{
    Q_D(SourceFormatterSelectionEdit);

    qCDebug(SHELL) << "Resetting UI";

    d->currentLanguagePtr = nullptr;

    if (d->languages.empty()) {
        {
            const QSignalBlocker blocker(d->ui.cbLanguages);
            d->ui.cbLanguages->clear();
        }
        {
            const QSignalBlocker blocker(d->ui.cbFormatters);
            d->ui.cbFormatters->clear();
        }
        d->ui.formatterDescriptionButton->setWhatsThis(QString{});
        d->ui.usageHintLabel->hide();
        {
            const QSignalBlocker blocker(d->ui.styleList);
            d->ui.styleList->clear();
        }

        d->updateUiForCurrentStyle();
    } else {
        {
            const QSignalBlocker blocker(d->ui.cbLanguages);
            d->ui.cbLanguages->clear();
            d->fillLanguageCombobox();
        }
        Q_ASSERT(d->ui.cbLanguages->count() == static_cast<int>(d->languages.size()));
        selectLanguage(d->ui.cbLanguages->currentIndex());
    }
}

void SourceFormatterSelectionEdit::saveSettings(KConfigGroup& config) const
{
    Q_D(const SourceFormatterSelectionEdit);

    // Store possibly modified user-defined styles. Store globally to allow reusing styles across sessions.
    KConfigGroup globalConfig = Core::self()->sourceFormatterControllerInternal()->globalConfig();
    for (const auto& formatter : d->formatters) {
        KConfigGroup fmtgrp = globalConfig.group(formatter->name());

        // Delete all user-defined styles so we don't leave behind styles deleted in the UI.
        const auto oldStyleGroups = fmtgrp.groupList();
        for (const QString& subgrp : oldStyleGroups) {
            if (subgrp.startsWith(userStyleNamePrefix)) {
                fmtgrp.deleteGroup( subgrp );
            }
        }

        formatter->forEachUserDefinedStyle([&fmtgrp](const SourceFormatterStyle& style) {
            KConfigGroup styleGroup = fmtgrp.group(style.name());
            styleGroup.writeEntry(SourceFormatterController::styleCaptionKey(), style.caption());
            styleGroup.writeEntry(SourceFormatterController::styleShowPreviewKey(), style.usePreview());
            styleGroup.writeEntry(SourceFormatterController::styleContentKey(), style.content());
            styleGroup.writeEntry(SourceFormatterController::styleMimeTypesKey(), style.mimeTypesVariant());
            styleGroup.writeEntry(SourceFormatterController::styleSampleKey(), style.overrideSample());
        });
    }
    globalConfig.sync();

    // Store formatter and style selection for each language.
    for (const auto& lang : d->languages) {
        lang.saveSettings(config);
    }
}

void SourceFormatterSelectionEdit::selectLanguage(int index)
{
    Q_D(SourceFormatterSelectionEdit);

    Q_ASSERT(index >= 0);
    Q_ASSERT(d->ui.cbLanguages->currentIndex() == index);

    Q_ASSERT(d->currentLanguagePtr != &d->languageSelectedInUi());
    d->currentLanguagePtr = &d->languageSelectedInUi();

    {
        const QSignalBlocker blocker(d->ui.cbFormatters);
        d->ui.cbFormatters->clear();

        for (auto* formatter : d->currentLanguage().supportingFormatters()) {
            d->ui.cbFormatters->addItem(formatter->formatter().caption(), QVariant::fromValue(formatter));
            if (formatter == &d->currentLanguage().selectedFormatter()) {
                d->ui.cbFormatters->setCurrentIndex(d->ui.cbFormatters->count() - 1);
            }
        }
        Q_ASSERT_X(&d->currentLanguage().selectedFormatter() == &d->formatterSelectedInUi(), Q_FUNC_INFO,
                   "The selected formatter is not among the supporting formatters!");
    }

    d->updateUiForCurrentFormatter();
    // Selecting a language does not change configuration => don't emit changed().
}

void SourceFormatterSelectionEdit::selectFormatter(int index)
{
    Q_D(SourceFormatterSelectionEdit);

    Q_ASSERT(index >= 0);
    Q_ASSERT(d->ui.cbFormatters->currentIndex() == index);

    const bool styleWasSelected = d->currentLanguage().selectedStyle();

    Q_ASSERT(&d->currentLanguage().selectedFormatter() != &d->formatterSelectedInUi());
    d->currentLanguage().setSelectedFormatter(d->formatterSelectedInUi());

    d->updateUiForCurrentFormatter();

    // Switching between formatters does not affect configuration if style remains
    // unselected => don't emit changed() then.
    Q_ASSERT(!d->currentLanguage().selectedStyle());
    if (styleWasSelected) {
        emit changed();
    }
}

void SourceFormatterSelectionEdit::styleSelectionChanged()
{
    Q_D(SourceFormatterSelectionEdit);

    Q_ASSERT(d->currentLanguage().selectedStyle() != d->styleSelectedInUi());
    d->currentLanguage().setSelectedStyle(d->styleSelectedInUi());

    d->updateUiForCurrentStyle();
    emit changed();
}

void SourceFormatterSelectionEdit::deleteStyle()
{
    Q_D(SourceFormatterSelectionEdit);

    const auto& currentStyle = d->validCurrentStyle();

    QStringList otherLanguageNames;
    std::vector<LanguageSettings*> otherLanguages;
    for (auto& lang : d->languages) {
        if (&lang != &d->currentLanguage() && lang.selectedStyle() == &currentStyle) {
            otherLanguageNames.push_back(lang.name());
            otherLanguages.push_back(&lang);
        }
    }
    // The deleted style can be used in other sessions or projects. But we show the warning dialog only if it
    // is selected for another language in the current session or project model (!otherLanguageNames.empty()).
    // Checking style selections in all other sessions is complicated, in all other projects - impossible.
    // Showing the warning dialog every time a style is deleted, even if the user just created it, can be
    // annoying. So the current behavior makes sense.
    if (!otherLanguageNames.empty()
        && KMessageBox::warningContinueCancel(
               this,
               i18n("The style %1 is also used for the following languages:\n%2.\nAre you sure you want to delete it?",
                    currentStyle.caption(), otherLanguageNames.join(QLatin1Char('\n'))),
               i18nc("@title:window", "Deleting Style"))
            != KMessageBox::Continue) {
        return;
    }

    const auto* const currentStyleItem = d->ui.styleList->currentItem();
    d->assertValidSelectedStyleItem(currentStyleItem);
    {
        const QSignalBlocker blocker(d->ui.styleList);
        delete currentStyleItem;
        // QListWidget selects the next item in the list when the currently selected item is destroyed.
        // Clear the selection for consistency with other languages where the deleted style was selected.
        d->ui.styleList->clearSelection();
    }

    d->currentLanguage().unselectStyle(currentStyle);
    for (auto* lang : otherLanguages) {
        lang->unselectStyle(currentStyle);
    }
    d->currentFormatter().removeStyle(currentStyle);

    d->updateUiForCurrentStyle();
    emit changed();
}

void SourceFormatterSelectionEdit::editStyle()
{
    Q_D(SourceFormatterSelectionEdit);

    auto& currentStyle = d->validCurrentStyle();

    Q_ASSERT_X(d->currentFormatter().formatter().hasEditStyleWidget(), Q_FUNC_INFO,
               "The Edit... button must be disabled if the current style is not editable.");
    KDevelop::ScopedDialog<EditStyleDialog> dlg(d->currentFormatter().formatter(),
                                                d->currentLanguage().defaultMimeType(), currentStyle, this);
    if (dlg->exec() == QDialog::Rejected) {
        return; // nothing changed
    }

    QString updatedContent = dlg->content();
    const bool updatedUsePreview = dlg->usePreview();
    if (updatedUsePreview == currentStyle.usePreview() && updatedContent == currentStyle.content()) {
        return; // nothing changed
    }

    currentStyle.setContent(std::move(updatedContent));
    currentStyle.setUsePreview(updatedUsePreview);

    // Don't call updateStyleButtons(), because editing a style doesn't affect the buttons.
    d->updatePreview();
    emit changed();
}

void SourceFormatterSelectionEdit::newStyle()
{
    Q_D(SourceFormatterSelectionEdit);

    const auto& currentStyle = d->validCurrentStyle();
    auto& newStyle = d->currentFormatter().addNewStyle();
    newStyle.copyDataFrom(currentStyle);
    newStyle.setCaption(i18n("New %1", currentStyle.caption()));

    d->currentLanguage().setSelectedStyle(&newStyle);

    // Don't insert the new item into the correct ordered position, because the user will probably enter a
    // different caption (which affects ordering) right away. Note that when the user renames a style, it is
    // not immediately moved into its new ordered position to avoid the "jumping" of the renamed style.
    // Always keeping style items in their correct UI order positions is not trivial to implement, which is
    // another reason not to do it.
    // User-defined styles are displayed on top of predefined styles, so the eventual ordered position of
    // the new item is most likely closer to the top than to the bottom => place it at the top of the list.
    auto& newStyleItem = d->addStyleItem(newStyle, StyleCategory::UserDefined, NewItemPosition::Top);
    {
        const QSignalBlocker blocker(d->ui.styleList);
        // An edited item is not automatically selected, which results in weird UI behavior:
        // the source style (currentStyle) remains selected after the editing finishes.
        // Select the new style here to prevent this confusion.
        d->ui.styleList->setCurrentItem(&newStyleItem);
    }
    d->ui.styleList->editItem(&newStyleItem);

    d->updateUiForCurrentStyle();
    emit changed();
}

void SourceFormatterSelectionEdit::styleNameChanged(QListWidgetItem* item)
{
    Q_D(SourceFormatterSelectionEdit);

    d->assertValidSelectedStyleItem(item);
    d->validCurrentStyle().setCaption(item->text());

    // Don't call updateUiForCurrentStyle(), because neither style buttons nor preview depend on style captions.
    emit changed();
}

#include "moc_sourceformatterselectionedit.cpp"
