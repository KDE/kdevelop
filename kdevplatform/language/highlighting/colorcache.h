/*
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_COLORCACHE_H
#define KDEVPLATFORM_COLORCACHE_H

#include <QObject>
#include <QVector>
#include <QColor>
#include <QPointer>

#include <KSyntaxHighlighting/Repository>

#include <interfaces/icompletionsettings.h>
#include <language/languageexport.h>

namespace KSyntaxHighlighting {
class Theme;
}

namespace KTextEditor {
class Document;
class View;
}

namespace KDevelop {
class ConfigurableHighlightingColors;
class IDocument;

/**
 * A singleton which holds the global default colors, adapted to the current color scheme
 */
class KDEVPLATFORMLANGUAGE_EXPORT ColorCache
    : public QObject
{
    Q_OBJECT

public:
    ~ColorCache() override;

    /// access the global color cache
    static ColorCache* self();

    /// adapt a given foreground color to the current color scheme
    /// @p ratio between 0 and 255 where 0 gives @see m_foregroundColor
    /// and 255 gives @p color
    ///
    /// @note if you are looking for a background color, simply setting an alpha
    ///       value should work.
    QColor blend(QColor color, uchar ratio) const;

    /// adapt a given background color to the current color scheme
    /// @p ratio between 0 and 255 where 0 gives @see m_foregroundColor
    /// and 255 gives @p color
    ///
    /// @note if you are looking for a background color, simply setting an alpha
    ///       value should work.
    QColor blendBackground(QColor color, uchar ratio) const;

    /// blend a color for local colorization according to the user settings
    /// @see blend()
    QColor blendLocalColor(QColor color) const;

    /// blend a color for global colorization according to the user settings
    /// @see blend()
    QColor blendGlobalColor(QColor color) const;

    /// access the default colors
    ConfigurableHighlightingColors* defaultColors() const;

    /**
     * @returns a primary color if @p num less primaryColorCount and a supplementary color if @p num >= primaryColorCount and < validColorCount
     * @see validColorCount()
     * @see primaryColorCount()
     */
    QColor generatedColor(uint num) const;

    /**
     * @returns the number of primary and supplementary colors
     *
     * @see generatedColor()
     * @see primaryColorCount()
     */
    uint validColorCount() const;

    /**
     * @returns number of primary colors
     *
     * When you run out of primary colors use supplementary colors
     */
    uint primaryColorCount() const;

    /// access the foreground color
    QColor foregroundColor() const;

Q_SIGNALS:
    /// will be emitted whenever the colors got changed
    /// @see update()
    void colorsGotChanged();

private Q_SLOTS:
    /// if necessary, adapt to the colors of this document
    void slotDocumentActivated();
    /// settings got changed, update to the settings of the sender
    void slotViewSettingsChanged();

    /// will regenerate colors from global KDE color scheme
    void updateColorsFromScheme();
    /// will regenerate colors with the proper intensity settings
    void updateColorsFromSettings();

    /// regenerate colors and emits @p colorsGotChanged()
    /// and finally triggers a rehighlight of the opened documents
    void updateInternal();

    bool tryActiveDocument();

private:
    explicit ColorCache(QObject* parent = nullptr);
    static ColorCache* m_self;

    /// get @p totalGeneratedColors colors from the color wheel and adapt them to the current color scheme
    void generateColors();

    /// calls @c updateInternal() delayed to prevent double loading of language plugins.
    void update();

    /// try to access the KatePart settings for the given doc or fallback to the global KDE scheme
    /// and update the colors if necessary
    /// @see generateColors(), updateColorsFromScheme()
    void updateColorsFromView(KTextEditor::View* view);

    bool updateColorsFromTheme(const KSyntaxHighlighting::Theme& theme);

    void updateDefaultColorsFromSource();

    /// the default colors for the different types
    ConfigurableHighlightingColors* m_defaultColors;

    /// the generated colors
    QVector<QColor> m_colors;

    uint m_validColorCount;

    uint m_primaryColorCount;

    /// the text color for the current color scheme
    QColor m_foregroundColor;

    /// the editor background color for the current color scheme
    QColor m_backgroundColor;

    /// How generated colors for local variables should be mixed with the foreground color.
    /// Between 0 and 255, where 255 means only foreground color, and 0 only the chosen color.
    uchar m_localColorRatio;

    /// How global colors (i.e. for types, uses, etc.) should be mixed with the foreground color.
    /// Between 0 and 255, where 255 means only foreground color, and 0 only the chosen color.
    uchar m_globalColorRatio;

    ICompletionSettings::GlobalColorSource m_globalColorSource;

    /// Whether declarations have to be rendered with a bold style or not.
    bool m_boldDeclarations;

    /// The view we are listening to for setting changes.
    QPointer<KTextEditor::View> m_view;

    KSyntaxHighlighting::Repository m_schemeRepo;
};
}

#endif // KDEVPLATFORM_COLORCACHE_H
