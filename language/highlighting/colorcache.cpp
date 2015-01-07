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

#include "../../interfaces/icore.h"
#include "../../interfaces/ilanguagecontroller.h"
#include "../../interfaces/icompletionsettings.h"
#include "../../interfaces/idocument.h"
#include "../../interfaces/idocumentcontroller.h"
#include "../interfaces/ilanguagesupport.h"
#include "../duchain/duchain.h"
#include "../duchain/duchainlock.h"
#include "util/debug.h"

#include <KColorUtils>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/ConfigInterface>

#define ifDebug(x)

// ######### start interpolation

static const uint totalColorInterpolationStepCount = 6;
static const uint interpolationWaypoints[] = {0xff0000, 0xff9900, 0x00ff00, 0x00aaff, 0x0000ff, 0xaa00ff};
//Do less steps when interpolating to/from green: Green is very dominant, and different mixed green tones are hard to distinguish(and always seem green).
static const uint interpolationLengths[] = {0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xff};

static const uint totalGeneratedColors = 10;

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

  updateColorsFromScheme(); // default / fallback
  updateColorsFromSettings();

  connect(ICore::self()->languageController()->completionSettings(), &ICompletionSettings::settingsChanged,
           this, &ColorCache::updateColorsFromSettings, Qt::QueuedConnection);

  connect(ICore::self()->documentController(), &IDocumentController::documentActivated,
          this, &ColorCache::slotDocumentActivated);

  bool hadDoc = tryActiveDocument();

  updateInternal();

  m_self = this;

  if (!hadDoc) {
    // try to update later on again
    QMetaObject::invokeMethod(this, "tryActiveDocument", Qt::QueuedConnection);
  }
}

bool ColorCache::tryActiveDocument()
{
  KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
  if ( view ) {
    updateColorsFromView(view);
    return true;
  }
  return false;
}

ColorCache::~ColorCache()
{
  m_self = 0;
  delete m_defaultColors;
  m_defaultColors = 0;
}

ColorCache* ColorCache::self()
{
  if (!m_self) {
    m_self = new ColorCache;
  }
  return m_self;
}

void ColorCache::generateColors()
{
  if ( m_defaultColors ) {
    delete m_defaultColors;
  }

  m_defaultColors = new CodeHighlightingColors(this);

  m_colors.clear();
  uint step = totalColorInterpolationSteps() / totalGeneratedColors;
  uint currentPos = m_colorOffset;
  ifDebug(qCDebug(LANGUAGE) << "text color:" << m_foregroundColor;)
  for(uint a = 0; a < totalGeneratedColors; ++a) {
    m_colors.append( blendLocalColor( interpolate( currentPos ) ) );
    ifDebug(qCDebug(LANGUAGE) << "color" << a << "interpolated from" << currentPos << " < " << totalColorInterpolationSteps() << ":" << (void*) m_colors.last().rgb();)
    currentPos += step;
  }
  m_validColorCount = m_colors.count();
  m_colors.append(m_foregroundColor);
}

void ColorCache::slotDocumentActivated()
{
  KTextEditor::View* view = ICore::self()->documentController()->activeTextDocumentView();
  ifDebug(qCDebug(LANGUAGE) << "doc activated:" << doc;)
  if ( view ) {
    updateColorsFromView(view);
  }
}

void ColorCache::slotViewSettingsChanged()
{
  KTextEditor::View* view = qobject_cast<KTextEditor::View*>(sender());
  Q_ASSERT(view);

  ifDebug(qCDebug(LANGUAGE) << "settings changed" << view;)
  updateColorsFromView(view);
}

void ColorCache::updateColorsFromView(KTextEditor::View* view)
{
  if ( !view ) {
    // yeah, the HighlightInterface methods returning an Attribute
    // require a View... kill me for that mess
    return;
  }

  QColor foreground(QColor::Invalid);
  QColor background(QColor::Invalid);

  KTextEditor::Attribute::Ptr style = view->defaultStyleAttribute(KTextEditor::dsNormal);
  foreground = style->foreground().color();
  if (style->hasProperty(QTextFormat::BackgroundBrush)) {
    background = style->background().color();
  }

  // FIXME: this is in kateview
//     qCDebug(LANGUAGE) << "got foreground:" << foreground.name() << "old is:" << m_foregroundColor.name();
  //NOTE: this slot is defined in KatePart > 4.4, see ApiDocs of the ConfigInterface

  // the signal is not defined in ConfigInterface, but according to the docs it should be
  // can't use new signal slot syntax here, since ConfigInterface is not a QObject
  if ( KTextEditor::View* view = m_view.data() ) {
    Q_ASSERT(qobject_cast<KTextEditor::ConfigInterface*>(view));
    // we only listen to a single view, i.e. the active one
    disconnect(view, SIGNAL(configChanged()), this, SLOT(slotViewSettingsChanged()));
  }
  Q_ASSERT(qobject_cast<KTextEditor::ConfigInterface*>(view));
  connect(view, SIGNAL(configChanged()), this, SLOT(slotViewSettingsChanged()));
  m_view = view;

  if ( !foreground.isValid() ) {
    // fallback to colorscheme variant
    ifDebug(qCDebug(LANGUAGE) << "updating from scheme";)
    updateColorsFromScheme();
  } else if ( m_foregroundColor != foreground || m_backgroundColor != background ) {
    m_foregroundColor = foreground;
    m_backgroundColor = background;

    ifDebug(qCDebug(LANGUAGE) << "updating from document";)
    update();
  }
}

void ColorCache::updateColorsFromScheme()
{
  KColorScheme scheme(QPalette::Normal, KColorScheme::View);

  QColor foreground = scheme.foreground(KColorScheme::NormalText).color();
  QColor background = scheme.background(KColorScheme::NormalBackground).color();

  if ( foreground != m_foregroundColor || background != m_backgroundColor ) {
    m_foregroundColor = foreground;
    m_backgroundColor = background;
    update();
  }
}

void ColorCache::updateColorsFromSettings()
{
  int localRatio = ICore::self()->languageController()->completionSettings()->localColorizationLevel();
  int globalRatio = ICore::self()->languageController()->completionSettings()->globalColorizationLevel();

  if ( localRatio != m_localColorRatio || globalRatio != m_globalColorRatio ) {
    m_localColorRatio = localRatio;
    m_globalColorRatio = globalRatio;
    update();
  }
}

void ColorCache::update()
{
  if ( !m_self ) {
    ifDebug(qCDebug(LANGUAGE) << "not updating - still initializating";)
    // don't update on startup, updateInternal is called directly there
    return;
  }

  QMetaObject::invokeMethod(this, "updateInternal", Qt::QueuedConnection);
}

void ColorCache::updateInternal()
{
  ifDebug(qCDebug(LANGUAGE) << "update internal" << m_self;)
  generateColors();

  if ( !m_self ) {
    // don't do anything else fancy on startup
    return;
  }

  emit colorsGotChanged();

  // rehighlight open documents
  foreach (IDocument* doc, ICore::self()->documentController()->openDocuments()) {
    foreach (auto lang, ICore::self()->languageController()->languagesForUrl(doc->url())) {
      ReferencedTopDUContext top;
      {
        DUChainReadLocker lock;
        top = lang->standardContext(doc->url());
      }

      if(top) {
        if ( ICodeHighlighting* highlighting = lang->codeHighlighting() ) {
          highlighting->highlightDUChain(top);
        }
      }
    }
  }
}

QColor ColorCache::blend(QColor color, uchar ratio) const
{
  Q_ASSERT(m_backgroundColor.isValid());
  Q_ASSERT(m_foregroundColor.isValid());
  if ( KColorUtils::luma(m_foregroundColor) > KColorUtils::luma(m_backgroundColor) ) {
    // for dark color schemes, produce a fitting color first
    color = KColorUtils::tint(m_foregroundColor, color, 0.5);
  }
  // adapt contrast
  return KColorUtils::mix( m_foregroundColor, color, float(ratio) / float(0xff) );
}

QColor ColorCache::blendBackground(QColor color, uchar ratio) const
{
/*  if ( KColorUtils::luma(m_foregroundColor) > KColorUtils::luma(m_backgroundColor) ) {
    // for dark color schemes, produce a fitting color first
    color = KColorUtils::tint(m_foregroundColor, color, 0.5).rgb();
  }*/
  // adapt contrast
  return KColorUtils::mix( m_backgroundColor, color, float(ratio) / float(0xff) );
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
  Q_ASSERT(m_defaultColors);
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

QColor ColorCache::foregroundColor() const
{
  return m_foregroundColor;
}

}


// kate: space-indent on; indent-width 2; replace-trailing-space-save on; show-tabs on; tab-indents on; tab-width 2;
