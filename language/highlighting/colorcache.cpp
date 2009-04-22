/*
 * This file is part of KDevelop
 *
 * Copyright 2009 Milian Wolff <mail@milianw.de>
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

#include "colorcache.h"
#include "configurablecolors.h"

#include <KColorScheme>
#include <KColorUtils>
#include <KGlobalSettings>

#include "../../interfaces/icore.h"
#include "../../interfaces/ilanguagecontroller.h"
#include "../../interfaces/icompletionsettings.h"
#include "../../interfaces/idocument.h"
#include "../../interfaces/idocumentcontroller.h"
#include "../../interfaces/ilanguage.h"
#include "../interfaces/ilanguagesupport.h"
#include "../duchain/duchain.h"

#include "configurablecolors.h"

// ######### start interpolation

uint totalColorInterpolationStepCount = 6;
uint interpolationWaypoints[] = {0xff0000, 0xff9900, 0x00ff00, 0x00aaff, 0x0000ff, 0xaa00ff};
//Do less steps when interpolating to/from green: Green is very dominant, and different mixed green tones are hard to distinguish(and always seem green).
uint interpolationLengths[] = {0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xff};

uint totalColorInterpolationSteps()
{
  uint ret = 0;
  for(uint a = 0; a < totalColorInterpolationStepCount; ++a)
    ret += interpolationLengths[a];
  return ret;
}

///Generates a color from the color wheel. @param step Step-number, one of totalColorInterpolationSteps
QColor interpolate(uint step)
{
  uint waypoint = 0;
  while(step > interpolationLengths[waypoint]) {
    step -= interpolationLengths[waypoint];
    ++waypoint;
  }

  uint nextWaypoint = (waypoint + 1) % totalColorInterpolationStepCount;

  return KColorUtils::mix( QColor(interpolationWaypoints[waypoint]), QColor(interpolationWaypoints[nextWaypoint]),
                           float(step) / float(interpolationLengths[waypoint]) );
}

// ######### end interpolation
namespace KDevelop {

ColorCache* ColorCache::m_self = 0;

ColorCache::ColorCache(QObject* parent)
  : QObject(parent), m_defaultColors(0), m_validColorCount(0), m_colorOffset(0),
    m_localColorRatio(0), m_globalColorRatio(0)
{
  Q_ASSERT(m_self == 0);

  m_self = this;

  connect(ICore::self()->languageController()->completionSettings(), SIGNAL(settingsChanged(ICompletionSettings*)),
           this, SLOT(adaptToColorChanges()));

  connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
           this, SLOT(adaptToColorChanges()));

  // first time initilialization

  m_localColorRatio = ICore::self()->languageController()->completionSettings()->localColorizationLevel();
  m_globalColorRatio = ICore::self()->languageController()->completionSettings()->globalColorizationLevel();

  ///@todo Find the correct text foreground color from kate! The palette thing below returns some strange other color.
  m_foregroundColor = KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::NormalText).color();

  m_defaultColors = new CodeHighlightingColors(this);

  generateColors(10);
}

ColorCache::~ColorCache()
{
  m_self = 0;
  delete m_defaultColors;
}

ColorCache* ColorCache::self()
{
  return m_self;
}

void ColorCache::initialize()
{
  if (m_self) {
    return;
  }

  m_self = new ColorCache;
}

void ColorCache::generateColors(uint count)
{
  m_colors.clear();
  uint step = totalColorInterpolationSteps() / count;
  uint currentPos = m_colorOffset;
  kDebug() << "text color:" << m_foregroundColor;
  for(uint a = 0; a < count; ++a) {
    m_colors.append( blendLocalColor( interpolate( currentPos ) ) );
    kDebug() << "color" << a << "interpolated from" << currentPos << " < " << totalColorInterpolationSteps() << ":" << (void*) m_colors.last().rgb();
    currentPos += step;
  }
  m_validColorCount = m_colors.count();
  m_colors.append(m_foregroundColor);
}

void ColorCache::ColorCache::adaptToColorChanges()
{
  m_localColorRatio = ICore::self()->languageController()->completionSettings()->localColorizationLevel();
  m_globalColorRatio = ICore::self()->languageController()->completionSettings()->globalColorizationLevel();
  ///@todo Find the correct text foreground color from kate! The palette thing below returns some strange other color.
  m_foregroundColor = KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::NormalText).color();

  delete m_defaultColors;

  m_defaultColors = new CodeHighlightingColors(this);

  generateColors(10);

  emit colorsGotChanged();

  // rehighlight open documents
  foreach (IDocument* doc, ICore::self()->documentController()->openDocuments()) {
    if ( TopDUContext* top = DUChain::self()->chainForDocument(doc->url()) ) {
      foreach ( ILanguage* lang, ICore::self()->languageController()->languagesForUrl(doc->url()) ) {
        if ( ILanguageSupport* langSupport = lang->languageSupport() ) {
          if ( const ICodeHighlighting* highlighting = langSupport->codeHighlighting() ) {
            highlighting->highlightDUChain(top);
          }
        }
      }
    }
  }
}

QColor ColorCache::blend(QColor color, uchar ratio) const
{
  if ( KColorUtils::luma(m_foregroundColor) >= 0.5 ) {
    // for dark color schemes, produce a fitting color first
    color = KColorUtils::tint(m_foregroundColor, color, 0.5).rgb();
  }
  // adapt contrast
  return KColorUtils::mix( m_foregroundColor, color, float(ratio) / float(0xff) );
}

QColor ColorCache::blendGlobalColor(QColor color) const
{
  return blend(color, m_globalColorRatio);
}

QColor ColorCache::blendLocalColor(QColor color) const
{
  return blend(color, m_localColorRatio);
}

CodeHighlightingColors* ColorCache::defaultColors() const
{
  return m_defaultColors;
}

QColor ColorCache::generatedColor(uint num) const
{
  return m_colors[num];
}

uint ColorCache::validColorCount() const
{
  return m_validColorCount;
}

}

#include "colorcache.moc"

// kate: space-indent on; indent-width 2; replace-trailing-space-save on; show-tabs on; tab-indents on; tab-width 2;
