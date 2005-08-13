/*
 * Two macros are defined here: ANTLR_PARSER_PREAMBLE and ANTLR_LEXER_PREAMBLE.
 * They encapsulate the application specific extensions for the classes
 * AdaParser and AdaLexer which are defined in ada.g.
 * This keeps ada.g independent of the application.
 *
 * Kdevelop version:
 * (C) 2003  Oliver M. Kellogg  (okellogg@users.sourceforge.net)
 */
#ifndef _PREAMBLES_H_
#define _PREAMBLES_H_

#include <string>
#include "problemreporter.h"

#define ANTLR_PARSER_PREAMBLE  \
  private:                                                                    \
    unsigned int m_numberOfErrors;                                            \
    ProblemReporter* m_problemReporter;                                       \
                                                                              \
  public:                                                                     \
    void resetErrors ()                          { m_numberOfErrors = 0; }    \
    unsigned int numberOfErrors () const         { return m_numberOfErrors; } \
    void setProblemReporter (ProblemReporter* r) { m_problemReporter = r; }   \
                                                                              \
    void reportError (const antlr::RecognitionException& ex) {                \
      m_problemReporter->reportError                                          \
             (ex.toString().c_str (),                                         \
              ex.getFilename().c_str (),                                      \
              ex.getLine (),                                                  \
              ex.getColumn ());                                               \
      ++m_numberOfErrors;                                                     \
    }                                                                         \
                                                                              \
    void reportError (const std::string& errorMessage) {                      \
      m_problemReporter->reportError                                          \
             (errorMessage.c_str(),                                           \
              getFilename ().c_str(),                                         \
              LT(1)->getLine (),                                              \
              LT(1)->getColumn ());                                           \
      ++m_numberOfErrors;                                                     \
    }                                                                         \
                                                                              \
    void reportMessage (const std::string& message) {                         \
      m_problemReporter->reportMessage                                        \
             (message.c_str (),                                               \
              getFilename ().c_str (),                                        \
              LT(1)->getLine (),                                              \
              LT(1)->getColumn ());                                           \
    }

#define ANTLR_LEXER_PREAMBLE  \
  private:                                                                    \
    unsigned int m_numberOfErrors;                                            \
    ProblemReporter* m_problemReporter;                                       \
                                                                              \
  public:                                                                     \
    void resetErrors ()                          { m_numberOfErrors = 0; }    \
    unsigned int numberOfErrors () const         { return m_numberOfErrors; } \
    void setProblemReporter (ProblemReporter* r) { m_problemReporter = r; }   \
                                                                              \
    void reportError (const antlr::RecognitionException& ex) {                \
      m_problemReporter->reportError                                          \
             (ex.toString ().c_str (),                                        \
              ex.getFilename ().c_str (),                                     \
              ex.getLine (),                                                  \
              ex.getColumn ());                                               \
      ++m_numberOfErrors;                                                     \
    }                                                                         \
                                                                              \
    void reportError (const std::string& errorMessage) {                      \
      m_problemReporter->reportError                                          \
             (errorMessage.c_str (),                                          \
              getFilename().c_str (),                                         \
              getLine (),                                                     \
              getColumn ());                                                  \
      ++m_numberOfErrors;                                                     \
    }                                                                         \
                                                                              \
    void reportWarning (const std::string& warnMessage) {                     \
      m_problemReporter->reportWarning                                        \
             (warnMessage.c_str (),                                           \
              getFilename ().c_str (),                                        \
              getLine (),                                                     \
              getColumn ());                                                  \
    }

#endif  // _PREAMBLES_H_

