#ifndef FAKESUPPORT_PART_H
#define FAKESUPPORT_PART_H

#include <kdevlanguagesupport.h>

class FakeLanguageSupport: public KDevLanguageSupport
{
  Q_OBJECT
public:
  FakeLanguageSupport(QObject *parent, const char *name, const QStringList &args);
  virtual ~FakeLanguageSupport();

  /**@return The feature set of the language. This is e.g. used
  by the class view to decide which organizer items to display and which not.*/
  virtual Features features() const;

  /**@return A typical mimetype list for the support language, this list
  should be configurable in the languagesupport dialog.*/
  virtual KMimeType::List mimeTypes() const;

  /**Formats a Tag as used by the persistant symbol store to the human-readable convention.
  @param tag Tag to format.*/
  virtual QString formatTag(const Tag& tag) const;

  /**Formats a KDevCodeItem as used by the CodeModel to the human-readable convention.
  @param item Symbol to format.
  @param shortDescription Show short description of a symbol. For example, when
  formatting functions short description could be a function signature without
  the return type and argument default values.*/
  virtual QString formatModelItem(const KDevCodeItem *item, bool shortDescription=false) const;

  /**Formats a canonicalized class path as used by the symbol store to the
  human-readable convention. For example, the C++ support part formats the
  string "KParts.Part" into "KParts::Part".
  @param name Class name.*/
  virtual QString formatClassName(const QString &name) const;

  /**The opposite of @ref formatClassName. Reverts formatting.
  @param name Class name.*/
  virtual QString unformatClassName(const QString &name) const;

private slots:
  void test_codemodel();
};

#endif // FAKESUPPORT_PART_H
