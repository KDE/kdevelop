#include "highlight.h"

#include "kwdoc.h"

//#include <kapp.h>
#include <kcharsets.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kmimemagic.h>

#include <qfile.h>
#include <qgrid.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlist.h>
#include <qgrpbox.h>
#include <qtstream.h>
//#include <qregexp.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <kbuttonbox.h>


//#include <X11/Xlib.h> //used in getXFontList()

// ISO/IEC 9899:1990 (aka ANSI C)
// "interrupt" isn´t an ANSI keyword, but an extension of some C compilers
const char *cKeywords[] = {
  "break", "case", "continue", "default", "do", "else", "enum", "extern",
  "for", "goto", "if", "interrupt",  "return", "sizeof", "struct", "switch",
  "typedef", "union", "while", 0L};

const char *cTypes[] = {
  "auto", "char", "const", "double", "float", "int", "long", "register",
  "short", "signed", "static",
  "unsigned", "void", "volatile", 0L};

//  ISO/IEC 14882:1998 . Sec. 2.11.1 (aka ANSI C++)
// keyword "const" (apart from a type spec.) is also a keyword, so it is named inside this array
const char *cppKeywords[] = {
  "asm", "catch", "class", "const_cast", "const", "delete", "dynamic_cast",
  "explicit", "export", "false", "friend", "inline", "namespace", "new", "operator",
  "private", "protected", "public", "reinterpret_cast", "static_cast", "template",
  "this", "throw", "true", "try", "typeid", "typename", "using", "virtual",
  // alternative representations  (these words are reserved and shall not be used otherwise)
  //  ISO/IEC 14882:1998 . Sec. 2.11.2
  "and", "and_eq", "bitand", "bitor", "compl", "not", "not_eq", "or", "or_eq", "xor",
  "xor_eq",
  
  "bad_cast", "bad_typeid","except","finally","type_info", "xalloc",
  // ADDED FOR TESTING
  "Q_EXPORT","Q_OBJECT","K_DCOP","SLOT","SIGNAL", "slots","signals",
  // QT 2.1 macros
  "Q_PROPERTY", "Q_ENUMS","Q_SETS","Q_OVERRIDE","Q_CLASSINFO",0L}; 

 const char *cppTypes[] = {
  "bool", "wchar_t", "mutable", 0L};

const char *idlKeywords[] = {
	"module", "interface", "struct", "case", "enum", "typedef","signal", "slot",
	"attribute", "readonly", "context", "oneway", "union", "in", "out", "inout",
	0L};

const char *idlTypes[] = {
	"long", "short", "unsigned", "double", "octet", "sequence", "char", "wchar", "string",
	"wstring", "any", "fixed", "Object", "void", "boolean", 0L};


const char *javaKeywords[] = {
  "abstract", "break", "case", "cast", "catch", "class", "continue",
  "default", "do", "else", "extends", "false", "finally", "for", "future",
  "generic", "goto", "if", "implements", "import", "inner", "instanceof",
  "interface", "native", "new", "null", "operator", "outer", "package",
  "private", "protected", "public", "rest", "return", "super", "switch",
  "synchronized", "this", "throws", "throw", "transient", "true", "try",
  "var", "volatile", "while", 0L};

const char *javaTypes[] = {
  "boolean", "byte", "char", "const", "double", "final", "float", "int",
  "long", "short", "static", "void", 0L};

const char *bashKeywords[] = {
  "break","case","done","do","elif","else","esac","exit","export","fi","for",
  "function","if","in","return","select","then","until","while",".",0L};

const char *modulaKeywords[] = {
  "BEGIN","CONST","DEFINITION","DIV","DO","ELSE","ELSIF","END","FOR","FROM",
  "IF","IMPLEMENTATION","IMPORT","MODULE","MOD","PROCEDURE","RECORD","REPEAT",
  "RETURN","THEN","TYPE","VAR","WHILE","WITH","|",0L};

const char *pascalKeywords[] = {
  // Ancient DOS Turbo Pascal keywords:
  //"absolute", "out",
  // Generic pascal keywords:
  "and", "array", "asm", "begin", "case", "const", "div", "do", "downto", "else",
  "end", "for", "function", "goto", "if", "implementation", "in", "interface",
  "label", "mod", "nil", "not", "of", "on", "operator", "or", "packed",
  "procedure", "program", "record", "repeat", "self", "set", "shl", "shr", "then",
  "to", "type", "unit", "until", "uses", "var", "while", "with", "xor",
  // Borland pascal keywords:
  "break", "continue", "constructor", "destructor", "inherited", "inline", "object",
  "private", "public",
  // Borland Delphi keywords
  "as", "at", "automated", "class", "dispinterface", "except", "exports",
  "finalization", "finally", "initialization", "is", "library", "on", "property",
  "published", "raise", "resourcestring", "threadvar", "try",
  // FPC keywords (www.freepascal.org)
  "dispose", "exit", "false", "new", "true",
  0L};

const char *pascalTypes[] = {
  "Integer", "Cardinal",
  "ShortInt", "SmallInt", "LongInt", "Int64", "Byte", "Word", "LongWord",
  "Char", "AnsiChar", "WideChar",
  "Boolean", "ByteBool", "WordBool", "LongBool",
  "Single", "Double", "Extended", "Comp",  "Currency", "Real", "Real48"
  "String", "ShortString", "AnsiString", "WideString",
  "Pointer", "Variant",
  "File", "Text",
  0L};

const char *adaKeywords[] = {
  "abort","abs","accept","access","all","and","array","at","begin","body",
  "case","constant","declare","delay","delta","digits","do","else","elsif",
  "end","entry","exception","exit","for", "function","generic","goto","if",
  "in","is","limited","loop","mod","new", "not","null","of","or","others",
  "out","package","pragma","private","procedure", "raise","range","rem",
  "record","renames","return","reverse","select","separate","subtype", "task",
  "terminate","then","type","use","when","while","with","xor",0L};

const char *pythonKeywords[] = {
  "and","assert","break","class","continue","def","del","elif","else",
  "except","exec"," finally","for","from","global","if","import","in","is",
  "lambda","not","or","pass","print","raise","return","try","while",0L};

const char *perlKeywords[] = {
  "and","&&", "bless","caller","cmp","continue","dbmclose","dbmopen","do",
  "die","dump","eval","else","elsif","eq","exit","foreach","for","ge","goto",
  "gt","if","import", "last","le","local","lt","my","next","ne","no","not",
  "!","or","||", "package","ref","redo","require","return","sub","tied",
  "tie","unless","until","untie","use","wantarray","while","xor", 0L};

const char *satherKeywords[] = {
  "and","assert","attr","break!","case","class","const","else","elsif",
  "end","exception","external","false","if","include","initial","is","ITER",
  "loop","new","or","post","pre","private","protect","quit","raise",
  "readonly","result","return","ROUT","SAME","self","shared","then","true",
  "typecase","type","until!","value","void","when","while!","yield",
// new in 1.1 and pSather:
  "abstract","any","bind","fork","guard","immutable","inout","in","lock",
  "once","out","parloop","partial","par","spread","stub", 0L};

// are those Sather keywords, too?
//     "nil","do@", "do"

const char *satherSpecClassNames[] = {
  "$OB","ARRAY","AREF","AVAL","BOOL","CHAR","EXT_OB","FLTDX","FLTD","FLTX",
  "FLTI","FLT","INTI","INT","$REHASH","STR","SYS",0L};

const char *satherSpecFeatureNames[] = {
// real special features
  "create","invariant","main",
// sugar feature names
  "aget","aset","div","is_eq","is_geq","is_gt","is_leq","is_lt","is_neq",
  "minus","mod","negate","not","plus","pow","times", 0L};


//char cEscapeChars[] = "abefnrtv\"\'\\";
//char perlEscapeChars[] = "tnrfbaeluLUEQ";


const char fontSizes[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,24,26,28,32,48,64,0};

//default item style indexes
const int dsNormal = 0;
const int dsKeyword = 1;
const int dsDataType = 2;
const int dsDecVal = 3;
const int dsBaseN = 4;
const int dsFloat = 5;
const int dsChar = 6;
const int dsString = 7;
const int dsComment = 8;
const int dsOthers = 9;


bool testWw(char c) {
  static unsigned char data[] = {0,0,0,0,0,0,255,3,254,255,255,135,254,255,255,7};
  if (c & 128) return false;
  return !(data[c >> 3] & (1 << (c & 7)));
}

// ---------------------------------------------------------------------------

HlItem::HlItem(int attribute, int context)
  : attr(attribute), ctx(context) {
}

// ---------------------------------------------------------------------------

HlItemWw::HlItemWw(int attribute, int context)
  : HlItem(attribute,context) {
}

// ---------------------------------------------------------------------------

HlCharDetect::HlCharDetect(int attribute, int context, char c)
  : HlItem(attribute,context), sChar(c) {
}

const char *HlCharDetect::checkHgl(const char *str) {
    if (*str == sChar) return str + 1;
   return 0L;
  

}

// ---------------------------------------------------------------------------

Hl2CharDetect::Hl2CharDetect(int attribute, int context, const char *s)
  : HlItem(attribute,context) {
  sChar[0] = s[0];
  sChar[1] = s[1];
}

const char *Hl2CharDetect::checkHgl(const char *str) {
  if (str[0] == sChar[0] && str[1] == sChar[1]) return str + 2;
  return 0L;
}

// ---------------------------------------------------------------------------

HlStringDetect::HlStringDetect(int attribute, int context, const char *s)
  : HlItem(attribute,context) {
  len = strlen(s);
  str = new char[len];
  memcpy(str,s,len);
}

HlStringDetect::~HlStringDetect() {
  delete str;
}

const char *HlStringDetect::checkHgl(const char *s) {
  if (memcmp(s,str,len) == 0) return s + len;
  return 0L;
}

// ---------------------------------------------------------------------------

HlRangeDetect::HlRangeDetect(int attribute, int context, const char *s)
  : HlItem(attribute,context) {
  sChar[0] = s[0];
  sChar[1] = s[1];
}

const char *HlRangeDetect::checkHgl(const char *s) {
  if (*s == sChar[0]) {
    do {
      s++;
      if (!*s) return 0L;
    } while (*s != sChar[1]);
    return s + 1;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

KeywordData::KeywordData(const char *str) {
  len = strlen(str);
  s = new char[len];
  memcpy(s,str,len);
}

KeywordData::~KeywordData() {
  delete s;
}

// ---------------------------------------------------------------------------

HlKeyword::HlKeyword(int attribute, int context)
  : HlItemWw(attribute,context) {
  words.setAutoDelete(true);
}

HlKeyword::~HlKeyword() {
}


void HlKeyword::addWord(const char *s) {
  KeywordData *word;
  word = new KeywordData(s);
  words.append(word);
}

void HlKeyword::addList(const char **list) {

  while (*list) {
    addWord(*list);
    list++;
  }
}

const char *HlKeyword::checkHgl(const char *s) {
 int z, count, len;
  KeywordData *word;

  count = words.count();
  len = strlen(s);
  for (z = 0; z < count; z++) {
      word = words.at(z);
      if (len>=word->len && memcmp(s,word->s,word->len) == 0) {
          return s + word->len;
      }
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlCaseInsensitiveKeyword::HlCaseInsensitiveKeyword(int attribute, int context)
  : HlKeyword(attribute,context) {
}

HlCaseInsensitiveKeyword::~HlCaseInsensitiveKeyword() {
}


const char *HlCaseInsensitiveKeyword::checkHgl(const char *s) {
  int z, count;
  KeywordData *word;

  count = words.count();
  for (z = 0; z < count; z++) {
    word = words.at(z);
    if (strncasecmp(s,word->s,word->len) == 0) {
      return s + word->len;
    }
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlInt::HlInt(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlInt::checkHgl(const char *str) {
  const char *s;

  s = str;
  while (*s >= '0' && *s <= '9') s++;
  if (s > str) return s;
  return 0L;
}

// ---------------------------------------------------------------------------

HlFloat::HlFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlFloat::checkHgl(const char *s) {
  bool b, p;

  b = false;
  while (*s >= '0' && *s <= '9') {
    s++;
    b = true;
  }
  if (p = (*s == '.')) {
    s++;
    while (*s >= '0' && *s <= '9') {
      s++;
      b = true;
    }
  }
  if (!b) return 0L;
  if (*s == 'E' || *s == 'e') s++; else return (p) ? s : 0L;
  if (*s == '-') s++;
  b = false;
  while (*s >= '0' && *s <= '9') {
    s++;
    b = true;
  }
  if (b) return s; else return 0L;
}

// ---------------------------------------------------------------------------

HlCSymbol::HlCSymbol(int attribute, int context)
  : HlItemWw(attribute,context) {
}

bool isCSymbol(char c) {
  static unsigned char data[] = {255,255,255,255,28,128,255,3,255,255,255,151,255,255,255,7};
  if (c & 128) return false;
  return !(data[c >> 3] & (1 << (c & 7)));
}

const char *HlCSymbol::checkHgl(const char *str) {
  const char *s = str;
  while ( *s == ' ' || *s == '\t' ) s++;
  if ( isCSymbol(*s) && ( *s != '\'' ) ){
    s++;
    return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlCInt::HlCInt(int attribute, int context)
  : HlInt(attribute,context) {
}

const char *HlCInt::checkHgl(const char *s) {

  if (*s == '0') s++; else s = HlInt::checkHgl(s);
  if (s && (*s == 'L' || *s == 'l' || *s == 'U' || *s == 'u')) s++;
  return s;
}

// ---------------------------------------------------------------------------

HlCOct::HlCOct(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlCOct::checkHgl(const char *str) {
  const char *s;

  if (*str == '0') {
    str++;
    s = str;
    while (*s >= '0' && *s <= '7') s++;
    if (s > str) {
      if (*s == 'L' || *s == 'l' || *s == 'U' || *s == 'u') s++;
      return s;
    }
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlCHex::HlCHex(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlCHex::checkHgl(const char *str) {
  const char *s;

  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
    str += 2;
    s = str;
    while ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f')) s++;
    if (s > str) {
      if (*s == 'L' || *s == 'l' || *s == 'U' || *s == 'u') s++;
      return s;
    }
  }
  return 0L;
}
 
// ---------------------------------------------------------------------------

HlCFloat::HlCFloat(int attribute, int context)
  : HlFloat(attribute,context) {
}

const char *HlCFloat::checkHgl(const char *s) {

  s = HlFloat::checkHgl(s);
  if (s && (*s == 'F' || *s == 'f')) s++;
  return s;
}

// ---------------------------------------------------------------------------

HlLineContinue::HlLineContinue(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlLineContinue::checkHgl(const char *s) {
  if (*s == '\\') return s + 1;
  return 0L;
}

// ---------------------------------------------------------------------------

HlCStringChar::HlCStringChar(int attribute, int context)
  : HlItem(attribute,context) {
}

//checks for hex and oct (for example \x1b or \033)
const char *checkCharHexOct(const char *str) {
  const char *s;
  int n;

  s = str;
  if (*s == 'x') {
    n = 0;
    do {
      s++;
      n *= 16;
      if (*s >= '0' && *s <= '9') n += *s - '0';
      else if (*s >= 'A' && *s <= 'F') n += *s - 'A' + 10;
      else if (*s >= 'a' && *s <= 'f') n += *s - 'a' + 10;
      else break;
      if (n >= 256) return 0L;
    } while (true);
    if (s - str == 1) return 0L;
  } else {
    if (!(*s >= '0' && *s <= '7')) return 0L;
    n = *s - '0';
    do {
      s++;
      n *= 8;
      if (*s >= '0' && *s <= '7') n += *s - '0'; else break;
      if (n >= 256) return s;
    } while (s - str < 3);
  }
  return s;
}

//checks for C escape chars like \n
const char *checkEscapedChar(const char *s) {

  if (s[0] == '\\' && s[1] != 0) {
    s++;
    if (strchr("abefnrtv\"\'\\",*s)) {
      s++;
    } else return checkCharHexOct(s); /*if (*s == 'x') {
      n = 0;
      do {
        s++;
        n *= 16;
        if (*s >= '0' && *s <= '9') n += *s - '0';
        else if (*s >= 'A' && *s <= 'F') n += *s - 'A' + 10;
        else if (*s >= 'a' && *s <= 'f') n += *s - 'a' + 10;
        else break;
        if (n >= 256) return 0L;
      } while (true);
      if (s - str == 2) return 0L;
    } else {
      if (!(*s >= '0' && *s <= '7')) return 0L;
      n = *s - '0';
      do {
        s++;
        n *= 8;
        if (*s >= '0' && *s <= '7') n += *s - '0'; else break;
        if (n >= 256) return s;
      } while (s - str < 4);
    }        */
    return s;
  }
  return 0L;
}

const char *HlCStringChar::checkHgl(const char *str) {
  return checkEscapedChar(str);
}

// ---------------------------------------------------------------------------

HlCChar::HlCChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlCChar::checkHgl(const char *str) {
  const char *s;

  if (str[0] == '\'' && str[1] != 0 && str[1] != '\'') {
    s = checkEscapedChar(&str[1]); //try to match escaped char
    if (!s) s = &str[2];           //match single non-escaped char
    if (*s == '\'') return s + 1;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlCPrep::HlCPrep(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlCPrep::checkHgl(const char *s) {

  while (*s == ' ' || *s == '\t') s++;
  if (*s == '#') {
    s++;
    return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlHtmlTag::HlHtmlTag(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlHtmlTag::checkHgl(const char *s) {
  while (*s == ' ' || *s == '\t') s++;
  while (*s != ' ' && *s != '\t' && *s != '>' && *s != '\0') s++;
  return s;
}

// ---------------------------------------------------------------------------

HlHtmlValue::HlHtmlValue(int attribute, int context)
  : HlItem(attribute,context) {
}

const char *HlHtmlValue::checkHgl(const char *s) {
  while (*s == ' ' || *s == '\t') s++;
  if (*s == '\"') {
    do {
      s++;
      if (!*s) return 0L;
    } while (*s != '\"');
    s++;
  } else {
    while (*s != ' ' && *s != '\t' && *s != '>' && *s != '\0') s++;
  }
  return s;
}

// ---------------------------------------------------------------------------

HlShellComment::HlShellComment(int attribute, int context)
  : HlCharDetect(attribute,context,'#') {
}

// ---------------------------------------------------------------------------

HlMHex::HlMHex(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlMHex::checkHgl(const char *s) {

  if (*s >= '0' && *s <= '9') {
    s++;
    while ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F')) s++;
    if (*s == 'H') return s + 1;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlAdaDec::HlAdaDec(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaDec::checkHgl(const char *s) {
  const char *str;

  if (*s >= '0' && *s <= '9') {
    s++;
    while ((*s >= '0' && *s <= '9') || *s == '_') s++;
    if (*s != 'e' && *s != 'E') return s;
    s++;
    str = s;
    while ((*s >= '0' && *s <= '9') || *s == '_') s++;
    if (s > str) return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlAdaBaseN::HlAdaBaseN(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaBaseN::checkHgl(const char *s) {
  int base;
  char c1, c2, c3;
  const char *str;

  base = 0;
  while (*s >= '0' && *s <= '9') {
    base *= 10;
    base += *s - '0';
    if (base > 16) return 0L;
    s++;
  }
  if (base >= 2 && *s == '#') {
    s++;
    c1 = '0' + ((base <= 10) ? base : 10);
    c2 = 'A' + base - 10;
    c3 = 'a' + base - 10;
    while ((*s >= '0' && *s < c1) || (*s >= 'A' && *s < c2)
      || (*s >= 'a' && *s < c3) || *s == '_') {
      s++;
    }
    if (*s == '#') {
      s++;
      if (*s != 'e' && *s != 'E') return s;
      s++;
      str = s;
      while ((*s >= '0' && *s <= '9') || *s == '_') s++;
      if (s > str) return s;
    }
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlAdaFloat::HlAdaFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaFloat::checkHgl(const char *s) {
  const char *str;

  str = s;
  while (*s >= '0' && *s <= '9') s++;
  if (s > str && *s == '.') {
    s++;
    str = s;
    while (*s >= '0' && *s <= '9') s++;
    if (s > str) {
      if (*s != 'e' && *s != 'E') return s;
      s++;
      if (*s == '-') s++;
      str = s;
      while ((*s >= '0' && *s <= '9') || *s == '_') s++;
      if (s > str) return s;
    }
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlAdaChar::HlAdaChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlAdaChar::checkHgl(const char *s) {
  if (s[0] == '\'' && s[1] && s[2] == '\'') return s + 3;
  return 0L;
}

// ---------------------------------------------------------------------------

HlSatherClassname::HlSatherClassname(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlSatherClassname::checkHgl(const char *s) {
  if (*s == '$') s++;
  if (*s >= 'A' && *s <= 'Z') {
    s++;
    while ((*s >= 'A' && *s <= 'Z')
           || (*s >= '0' && *s <= '9')
           || *s == '_') s++;
    return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlSatherIdent::HlSatherIdent(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlSatherIdent::checkHgl(const char *s) {
  if ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z')) {
    s++;
    while ((*s >= 'a' && *s <= 'z')
           || (*s >= 'A' && *s <= 'Z')
           || (*s >= '0' && *s <= '9')
           || *s == '_') s++;
    if (*s == '!') s++;
    return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlSatherDec::HlSatherDec(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlSatherDec::checkHgl(const char *s) {
  if (*s >= '0' && *s <= '9') {
    s++;
    while ((*s >= '0' && *s <= '9') || *s == '_') s++;
    if (*s == 'i') s++;
    return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlSatherBaseN::HlSatherBaseN(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlSatherBaseN::checkHgl(const char *s) {
  if (*s == '0') {
    s++;
    if (*s == 'x') {
      s++;
      while ((*s >= '0' && *s <= '9')
             || (*s >= 'a' && *s <= 'f')
             || (*s >= 'A' && *s <= 'F')
             || *s == '_') s++;
    } else if (*s == 'o') {
      s++;
      while ((*s >= '0' && *s <= '7') || *s == '_') s++;
    } else if (*s == 'b') {
      s++;
      while (*s == '0' || *s == '1' || *s == '_') s++;
    } else
      return 0L;
    if (*s == 'i') s++;
    return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlSatherFloat::HlSatherFloat(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlSatherFloat::checkHgl(const char *s) {
  if (*s >= '0' && *s <= '9') {
    s++;
    while ((*s >= '0' && *s <= '9') || *s == '_') s++;
    if (*s == '.') {
      s++;
      while (*s >= '0' && *s <= '9') s++;
      if (*s == 'e' || *s == 'E') {
        s++;
        if (*s == '-') s++;
        if (*s >= '0' && *s <= '9') {
          s++;
          while ((*s >= '0' && *s <= '9') || *s == '_') s++;
        } else
          return 0L;
      }
      if (*s == 'i') return s+1;
      if (*s == 'd') s++;
      if (*s == 'x') s++;
		// "dx" is allowed too
      return s;
    }
  }
  return 0L;
}

// ---------------------------------------------------------------------------

HlSatherChar::HlSatherChar(int attribute, int context)
  : HlItemWw(attribute,context) {
}

const char *HlSatherChar::checkHgl(const char *s) {
  if (*s == '\'') {
    s++;
    if (*s == '\\') {
      s++;
      if (*s == 'a' || *s == 'b' || *s == 'f' || *s == 'n'
          || *s == 'r' || *s == 't' || *s == 'v' || *s == '\\'
          || *s == '\'' || *s == '\"') s++;
      else if (*s>='0' && *s<='7')
        while (*s>='0' && *s<='7') s++;
      else
        return 0L;
    } else if (*s != '\0') s++;
  }
  if (*s == '\'')
    return s+1;
  else
    return 0L;
}

// ---------------------------------------------------------------------------

HlSatherString::HlSatherString(int attribute, int context)
  : HlItemWw(attribute, context) {
}

const char *HlSatherString::checkHgl(const char *s) {
  if (*s == '\"') {
    s++;
    while (*s != '\"') {
      if (*s == '\\')
        s++;
      if (*s == '\n' || *s == '\0')
        return s;
      s++;
    }
    return s+1;
  }
  return 0L;
}


// ---------------------------------------------------------------------------

HlLatexTag::HlLatexTag(int attribute, int context)
  : HlItem(attribute, context) {
}

const char *HlLatexTag::checkHgl(const char *s) {
  const char *str;

  if (*s == '\\') {
    s++;
    if (*s == ' ' || *s == '/' || *s == '\\') return s +1;
    str = s;
    while ((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z')
      || (*s >= '0' && *s <= '9') || *s == '@') {
      s++;
    }
    if (s != str) return s;
  } else if (*s == '$') return s +1;
  return 0L;
}

//---------------------------------------------------------------------------

HlLatexChar::HlLatexChar(int attribute, int context)
  : HlItem(attribute, context) {
}

const char *HlLatexChar::checkHgl(const char *s) {
  if (*s == '\\') {
    s++;
    if (*s && strchr("{}$&#_%", *s)) return s +1;
  }/* else if (*s == '"') {
    s++;
    if (*s && (*s < '0' || *s > '9')) return s +1;
  } */
  return 0L;
}

// ---------------------------------------------------------------------------

HlLatexParam::HlLatexParam(int attribute, int context)
  : HlItem(attribute, context) {
}

const char *HlLatexParam::checkHgl(const char *s) {
  if (*s == '#') {
    s++;
    while (*s >= '0' && *s <= '9') {
      s++;
    }
    return s;
  }
  return 0L;
}

// ---------------------------------------------------------------------------

ItemStyle::ItemStyle() : selCol(Qt::white), bold(false), italic(false) {
}

ItemStyle::ItemStyle(const QColor &col, const QColor &selCol,
  bool bold, bool italic)
  : col(col), selCol(selCol), bold(bold), italic(italic) {
}

// ---------------------------------------------------------------------------

ItemFont::ItemFont() :
  family("courier"),
  size(12),
  charset("")
{
}

// ---------------------------------------------------------------------------

ItemData::ItemData(const QString& name, int defStyleNum) :
  name(name),
  defStyleNum(defStyleNum),
  defStyle(true),
  defFont(true)
{
}

ItemData::ItemData(const QString& name, int defStyleNum,
                    const QColor &col, const QColor &selCol,
                    bool bold, bool italic) :
  ItemStyle(col,selCol,bold,italic),
  name(name),
  defStyleNum(defStyleNum),
  defStyle(false),
  defFont(true) {
}

// ---------------------------------------------------------------------------

HlData::HlData(const QString& wildcards, const QString& mimetypes) :
  wildcards(wildcards),
  mimetypes(mimetypes)
{
  itemDataList.setAutoDelete(true);
}

// ---------------------------------------------------------------------------

Highlight::Highlight(const QString& name) :
	iName(name),
	refCount(0)
{
}

Highlight::~Highlight() {
}

KConfig *Highlight::getKConfig() {
  KConfig *config = KGlobal::config();
  config->setGroup((QString) iName + " Highlight");
  return config;
}

QString Highlight::getWildcards()
{
  KConfig *config = getKConfig();
	if (config)
  	return config->readEntry("Wildcards", dw);
	return dw;
}


QString Highlight::getMimetypes()
{
  KConfig *config = getKConfig();
	if (config)
	  return config->readEntry("Mimetypes", dm);
	return dm;
}


HlData *Highlight::getData() {
  KConfig *config;
  HlData *hlData;

  config = getKConfig();

  hlData = new HlData(
    config->readEntry("Wildcards",dw),config->readEntry("Mimetypes",dm));
  getItemDataList(hlData->itemDataList,config);
  return hlData;
}

void Highlight::setData(HlData *hlData) {
  KConfig *config;

  config = getKConfig();

  config->writeEntry("Wildcards",hlData->wildcards);
  config->writeEntry("Mimetypes",hlData->mimetypes);

  setItemDataList(hlData->itemDataList,config);
}

void Highlight::getItemDataList(ItemDataList &list) {
  KConfig *config;

  config = getKConfig();
  getItemDataList(list,config);
}

void Highlight::getItemDataList(ItemDataList &list, KConfig *config) {
  ItemData *p;
  QString s;
  QRgb col, selCol;
  char family[96];
  char charset[48];

  list.clear();
  list.setAutoDelete(true);
  createItemData(list);

  for (p = list.first(); p != 0L; p = list.next()) {
    s = config->readEntry(p->name);
    p->size=p->printSize=10;
    if (!s.isEmpty()) {
      sscanf(s,"%d,%X,%X,%d,%d,%d,%95[^,],%d,%47[^,],%d",
        &p->defStyle,&col,&selCol,&p->bold,&p->italic,
        &p->defFont,family,&p->size,charset, &p->printSize);
      p->col.setRgb(col);
      p->selCol.setRgb(selCol);
      p->family = family;
      p->charset = charset;
    }
  }
}

void Highlight::setItemDataList(ItemDataList &list, KConfig *config) {
  ItemData *p;
  char s[200];

  for (p = list.first(); p != 0L; p = list.next()) {
    sprintf(s,"%d,%X,%X,%d,%d,%d,%1.95s,%d,%1.47s,%d",
      p->defStyle,p->col.rgb(),p->selCol.rgb(),p->bold,p->italic,
      p->defFont,p->family.data(),p->size,p->charset.data(),p->printSize);
    config->writeEntry(p->name,s);
  }
}

QString Highlight::name() {
  return iName;
}

void Highlight::use() {
  if (refCount == 0) init();
  refCount++;
}

void Highlight::release() {
  refCount--;
  if (refCount == 0) done();
}

int Highlight::doHighlight(int, TextLine *textLine) {

  if (textLine)
  {
    textLine->setAttribs(0,0,textLine->length());
    textLine->setAttr(0);
  }
  return 0;
}

int Highlight::doPreHighlight( QList<TextLine> & /*contents*/ ) {
  return 0;
}

void Highlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
}


void Highlight::init() {
}

void Highlight::done() {
}

bool Highlight::containsFiletype(const QString& ext)
{
  QString w = getWildcards();
  if (w.isEmpty())
    return false;

  w = ";" + w + ";";
  return w.find(";*."+ext+ ";") != -1;
}

bool Highlight::containsMimetype(const QString& mimetype)
{
  QString w = getMimetypes();
  if (w.isEmpty())
    return false;

  w = ";" + w + ";";
  return w.find(";"+mimetype+ ";") != -1;
}

// ---------------------------------------------------------------------------

HlContext::HlContext(int attribute, int lineEndContext)
  : attr(attribute), ctx(lineEndContext) {
  items.setAutoDelete(true);
}

// ---------------------------------------------------------------------------

GenHighlight::GenHighlight(const char *name) : Highlight(name) {
  for ( int z = 0; z < nContexts; z++) contextList[z] = 0L;
}


int GenHighlight::doHighlight(int ctxNum, TextLine *textLine){
  if (!textLine) return 0;

  HlContext *context;
  const char *str, *s1, *s2;
  char lastChar;
  HlItem *item;

  context = contextList[ctxNum];

  str = textLine->getString();
  lastChar = 0;

  s1 = str;
  while (*s1) {
    for (item = context->items.first(); item != 0L; item = context->items.next()) {
      if (item->startEnable(lastChar) || isCSymbol(*s1)) {
        s2 = item->checkHgl(s1);
        if (s2 > s1) {
          if (item->endEnable(*s2) || isCSymbol(*s1)) {
            textLine->setAttribs(item->attr,s1 - str,s2 - str);
            ctxNum = item->ctx;
            context = contextList[ctxNum];
            s1 = s2 - 1;
            goto found;
          }
        }
      }
    }
    // nothing found: set attribute of one char
    textLine->setAttribs(context->attr,s1 - str,s1 - str + 1);

    found:
    lastChar = *s1;
    s1++;
  }
  //set "end of line"-properties
  textLine->setAttr(context->attr);
  //return new context
  return context->ctx;
}

int GenHighlight::doPreHighlight( QList<TextLine> &contents )
{
  HlContext *context;
  HlItem *item;
  TextLine *textLine;
  const char *str, *s1, *s2;
  char lastChar;
  int ctxNum= 0;

  context = contextList[0];

#define LOCAL_CONTEXT
#ifdef LOCAL_CONTEXT
  // cache only those context items locally that lead to a context switch
  // test only those possibilities that lead to a context switch
  HlContext *localContext[nContexts];
  for (int i= 0; i < nContexts; ++i )
  {
    if ( contextList[i] ) {
      HlContext *c;
      context = contextList[i];
      localContext[i] = c = new HlContext( context->attr, context->ctx );
      c->items.setAutoDelete(false);
      for (item = context->items.first(); item != 0L; item = context->items.next()) {
        if ( item->ctx == i )
          continue; // not interested in this one
        c->items.append(item);
      }
    } else
      localContext[i] = 0L;
  }
#endif

  int lastLine = (int) contents.count() -1;
  for (int line= 0; line < lastLine; ++line ) {
    textLine = contents.at(line);
    str = textLine->getString();
    s1 = str;
    lastChar = 0;

#ifdef LOCAL_CONTEXT
    context = localContext[ctxNum];
#else
    context = contextList[ctxNum];
#endif
    while (*s1) {
      for (item = context->items.first(); item != 0L; item = context->items.next()) {
        if (item->startEnable(lastChar) || isCSymbol(*s1)) {
          s2 = item->checkHgl(s1);
          if (s2 > s1) {
            if (item->endEnable(*s2) || isCSymbol(*s1)) {
              ctxNum = item->ctx;
#ifdef LOCAL_CONTEXT
              context = localContext[ctxNum];
#else
              context = contextList[ctxNum];
#endif
              s1 = s2 - 1;
              goto found;
            }
          }
        }
      }
found:
      lastChar = *s1;
      s1++;
    }
    //set "end of line"-properties
    ctxNum= context->ctx;
    textLine->setContext(ctxNum);
  }
#ifdef LOCAL_CONTEXT
  for (int i = 0; i < nContexts; ++i )
  {
    delete localContext[i];
    localContext[i] = 0L;
  }
#endif
  return 0;
}

void GenHighlight::init() {
  int z;

  for (z = 0; z < nContexts; z++) {
    delete contextList[z];
    contextList[z] = 0L;
  }
  makeContextList();
}

void GenHighlight::done() {
  int z;

  for (z = 0; z < nContexts; z++) { delete contextList[z]; contextList[z] = 0L; }
}

// ---------------------------------------------------------------------------

CHighlight::CHighlight(const QString& name) :
  GenHighlight(name)
{
  dw = "*.c";
  dm = "text/x-csrc";
}

CHighlight::~CHighlight() {
}

void CHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
  list.append(new ItemData("Keyword",dsKeyword));
  list.append(new ItemData("Data Type",dsDataType));
  list.append(new ItemData("Decimal",dsDecVal));
  list.append(new ItemData("Octal",dsBaseN));
  list.append(new ItemData("Hex",dsBaseN));
  list.append(new ItemData("Float",dsFloat));
  list.append(new ItemData("Char",dsChar));
  list.append(new ItemData("String",dsString));
  list.append(new ItemData("String Char",dsChar));
  list.append(new ItemData("Comment",dsComment));
  list.append(new ItemData("Preprocessor",dsOthers));
  list.append(new ItemData("Prep. Lib",dsOthers,Qt::darkYellow,Qt::yellow,false,false));
  list.append(new ItemData("Symbol",dsNormal));
}

void CHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword, *dataType;

  //normal context
  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(dataType = new HlKeyword(2,0));
    c->items.append(new HlCFloat(6,0));
    c->items.append(new HlCOct(4,0));
    c->items.append(new HlCHex(5,0));
    c->items.append(new HlCInt(3,0));
    c->items.append(new HlCChar(7,0));
    c->items.append(new HlCharDetect(8,1,'"'));
    c->items.append(new Hl2CharDetect(10,2,"//"));
    c->items.append(new Hl2CharDetect(10,3,"/*"));
    c->items.append(new HlCPrep(11,4));
		c->items.append(new HlCSymbol(13,0));
  //string context
  contextList[1] = c = new HlContext(8,0);
    c->items.append(new HlLineContinue(8,6));
    c->items.append(new HlCStringChar(9,1));
    c->items.append(new HlCharDetect(8,0,'"'));
  //one line comment context
  contextList[2] = new HlContext(10,0);
  //multi line comment context
  contextList[3] = c = new HlContext(10,3);
    c->items.append(new Hl2CharDetect(10,0,"*/"));
  //preprocessor context
  contextList[4] = c = new HlContext(11,0);
    c->items.append(new HlLineContinue(11,7));
    c->items.append(new HlRangeDetect(12,4,"\"\""));
    c->items.append(new HlRangeDetect(12,4,"<>"));
    c->items.append(new Hl2CharDetect(10,2,"//"));
    c->items.append(new Hl2CharDetect(10,5,"/*"));
  //preprocessor multiline comment context
  contextList[5] = c = new HlContext(10,5);
    c->items.append(new Hl2CharDetect(10,4,"*/"));
  //string line continue
  contextList[6] = new HlContext(0,1);
  //preprocessor string line continue
  contextList[7] = new HlContext(0,4);

  setKeywords(keyword, dataType);
}

void CHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {

  keyword->addList(cKeywords);
  dataType->addList(cTypes);
}

// ---------------------------------------------------------------------------

CppHighlight::CppHighlight(const QString& name) : CHighlight(name) {
  dw = "*.cpp;*.h;*.C;*.cc";
  dm = "text/x-c++src;text/x-c++hdr;text/x-chdr";
}

CppHighlight::~CppHighlight() {
}

void CppHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {

  keyword->addList(cKeywords);
  keyword->addList(cppKeywords);
  dataType->addList(cTypes);
  dataType->addList(cppTypes);
}

// ---------------------------------------------------------------------------

PascalHighlight::PascalHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.pp;*.pas;*.inc";
  dm = "text/x-pascal";
}

PascalHighlight::~PascalHighlight() {
}

void PascalHighlight::createItemData(ItemDataList &list) {
  list.append(new ItemData("Normal Text",dsNormal));   // 0
  list.append(new ItemData("Keyword",dsKeyword));      // 1
  list.append(new ItemData("Data Type",dsDataType));   // 2
  list.append(new ItemData("Number",dsDecVal));        // 3
  list.append(new ItemData("String",dsString));        // 4
  list.append(new ItemData("Directive",dsOthers));     // 5
  list.append(new ItemData("Comment",dsComment));      // 6
}

void PascalHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword, *dataType;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlCaseInsensitiveKeyword(1,0));
    c->items.append(dataType = new HlCaseInsensitiveKeyword(2,0));
    c->items.append(new HlFloat(3,0));
    c->items.append(new HlInt(3,0));
    // TODO: Pascal hex $1234
    c->items.append(new HlCharDetect(4,1,'\''));
    c->items.append(new HlStringDetect(5,2,"(*$"));
    c->items.append(new Hl2CharDetect(5,3,"{$"));
    c->items.append(new Hl2CharDetect(6,4,"(*"));
    c->items.append(new HlCharDetect(6,5,'{'));
    c->items.append(new Hl2CharDetect(6,6,"//"));

  // string context
  contextList[1] = c = new HlContext(4,0);
    c->items.append(new HlCharDetect(4,0,'\''));
  // TODO: detect '''' or 'Holger''s Jokes are silly'

  // (*$ directive context
  contextList[2] = c = new HlContext(5,2);
    c->items.append(new Hl2CharDetect(5,0,"*)"));
  // {$ directive context
  contextList[3] = c = new HlContext(5,3);
    c->items.append(new HlCharDetect(5,0,'}'));
  // (* comment context
  contextList[4] = c = new HlContext(6,4);
    c->items.append(new Hl2CharDetect(6,0,"*)"));
  // { comment context
  contextList[5] = c = new HlContext(6,5);
    c->items.append(new HlCharDetect(6,0,'}'));
  // one line context
  contextList[6] = c = new HlContext(6,0);

  keyword->addList(pascalKeywords);
  dataType->addList(pascalTypes);
}

// ---------------------------------------------------------------------------

IdlHighlight::IdlHighlight(const QString& name) : CHighlight(name) {
  dw = "*.idl";
  dm = "text/x-idl";
}

IdlHighlight::~IdlHighlight() {
}

void IdlHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {
  keyword->addList(idlKeywords);
  dataType->addList(idlTypes);
}

// ---------------------------------------------------------------------------

JavaHighlight::JavaHighlight(const QString& name) : CHighlight(name) {
  dw = "*.java";
  dm = "text/x-java";
}

JavaHighlight::~JavaHighlight() {
}

void JavaHighlight::setKeywords(HlKeyword *keyword, HlKeyword *dataType) {

  keyword->addList(javaKeywords);
  dataType->addList(javaTypes);
}

// ---------------------------------------------------------------------------

HtmlHighlight::HtmlHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.html;*.htm";
  dm = "text/html";
}

HtmlHighlight::~HtmlHighlight() {
}

void HtmlHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
  list.append(new ItemData("Char",dsChar,Qt::darkGreen,Qt::green,false,false));
  list.append(new ItemData("Comment",dsComment));
  list.append(new ItemData("Tag Text",dsOthers,Qt::black,Qt::white,true,false));
  list.append(new ItemData("Tag",dsKeyword,Qt::darkMagenta,Qt::magenta,true,false));
  list.append(new ItemData("Tag Value",dsDecVal,Qt::darkCyan,Qt::cyan,false,false));
}

void HtmlHighlight::makeContextList() {
  HlContext *c;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlRangeDetect(1,0,"&;"));
    c->items.append(new HlStringDetect(2,1,"<!--"));
    c->items.append(new HlStringDetect(2,2,"<COMMENT>"));
    c->items.append(new HlCharDetect(3,3,'<'));
  contextList[1] = c = new HlContext(2,1);
    c->items.append(new HlStringDetect(2,0,"-->"));
  contextList[2] = c = new HlContext(2,2);
    c->items.append(new HlStringDetect(2,0,"</COMMENT>"));
  contextList[3] = c = new HlContext(3,3);
    c->items.append(new HlHtmlTag(4,3));
    c->items.append(new HlHtmlValue(5,3));
    c->items.append(new HlCharDetect(3,0,'>'));
}

// ---------------------------------------------------------------------------

BashHighlight::BashHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.sh;";
  dm = "text/x-shellscript;application/x-shellscript";
}


BashHighlight::~BashHighlight() {
}

void BashHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
  list.append(new ItemData("Keyword",dsKeyword));
  list.append(new ItemData("Integer",dsDecVal));
  list.append(new ItemData("String",dsString));
  list.append(new ItemData("Substitution",dsOthers));//darkCyan,cyan,false,false);
  list.append(new ItemData("Comment",dsComment));
}

void BashHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCharDetect(3,1,'"'));
    c->items.append(new HlCharDetect(4,2,'`'));
    c->items.append(new HlShellComment(5,3));
  contextList[1] = c = new HlContext(3,0);
    c->items.append(new HlCharDetect(3,0,'"'));
  contextList[2] = c = new HlContext(4,0);
    c->items.append(new HlCharDetect(4,0,'`'));
  contextList[3] = new HlContext(5,0);

  keyword->addList(bashKeywords);
}

// ---------------------------------------------------------------------------

ModulaHighlight::ModulaHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.md;*.mi";
  dm = "text/x-modula-2";
}

ModulaHighlight::~ModulaHighlight() {
}

void ModulaHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
  list.append(new ItemData("Keyword",dsKeyword));
  list.append(new ItemData("Decimal",dsDecVal));
  list.append(new ItemData("Hex",dsBaseN));
  list.append(new ItemData("Float",dsFloat));
  list.append(new ItemData("String",dsString));
  list.append(new ItemData("Comment",dsComment));
}

void ModulaHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlFloat(4,0));
    c->items.append(new HlMHex(3,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCharDetect(5,1,'"'));
    c->items.append(new Hl2CharDetect(6,2,"(*"));
  contextList[1] = c = new HlContext(5,0);
    c->items.append(new HlCharDetect(5,0,'"'));
  contextList[2] = c = new HlContext(6,2);
    c->items.append(new Hl2CharDetect(6,0,"*)"));

  keyword->addList(modulaKeywords);
}

// ---------------------------------------------------------------------------

AdaHighlight::AdaHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.a";
  dm = "text/x-ada";
}

AdaHighlight::~AdaHighlight() {
}

void AdaHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
  list.append(new ItemData("Keyword",dsKeyword));
  list.append(new ItemData("Decimal",dsDecVal));
  list.append(new ItemData("Base-N",dsBaseN));
  list.append(new ItemData("Float",dsFloat));
  list.append(new ItemData("Char",dsChar));
  list.append(new ItemData("String",dsString));
  list.append(new ItemData("Comment",dsComment));
}

void AdaHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlAdaBaseN(3,0));
    c->items.append(new HlAdaDec(2,0));
    c->items.append(new HlAdaFloat(4,0));
    c->items.append(new HlAdaChar(5,0));
    c->items.append(new HlCharDetect(6,1,'"'));
    c->items.append(new Hl2CharDetect(7,2,"--"));
  contextList[1] = c = new HlContext(6,0);
    c->items.append(new HlCharDetect(6,0,'"'));
  contextList[2] = c = new HlContext(7,0);

  keyword->addList(adaKeywords);
}

// ---------------------------------------------------------------------------

PythonHighlight::PythonHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.py";
  dm = "text/x-python";
}

PythonHighlight::~PythonHighlight() {
}

void PythonHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
  list.append(new ItemData("Keyword",dsKeyword));
  list.append(new ItemData("Decimal",dsDecVal));
  list.append(new ItemData("Octal",dsBaseN));
  list.append(new ItemData("Hex",dsBaseN));
  list.append(new ItemData("Float",dsFloat));
  list.append(new ItemData("Char",dsChar));
  list.append(new ItemData("String",dsString));
  list.append(new ItemData("String Char",dsChar));
  list.append(new ItemData("Comment",dsComment));
}

void PythonHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword;

  //note that a C octal has to be detected before an int and """ before "
  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(new HlCOct(3,0));
    c->items.append(new HlInt(2,0));
    c->items.append(new HlCHex(4,0));
    c->items.append(new HlFloat(5,0));
    c->items.append(new HlCChar(6,0));
    c->items.append(new HlStringDetect(7,3,"\"\"\""));
    c->items.append(new HlStringDetect(7,4,"\'\'\'"));
    c->items.append(new HlCharDetect(7,1,'"'));
    c->items.append(new HlCharDetect(7,2,'\''));
    c->items.append(new HlCharDetect(9,5,'#'));
  contextList[1] = c = new HlContext(7,0);
    c->items.append(new HlLineContinue(7,6));
    c->items.append(new HlCStringChar(8,1));
    c->items.append(new HlCharDetect(7,0,'"'));
  contextList[2] = c = new HlContext(7,0);
    c->items.append(new HlLineContinue(7,7));
    c->items.append(new HlCStringChar(8,2));
    c->items.append(new HlCharDetect(7,0,'\''));
  contextList[3] = c = new HlContext(7,3);
    c->items.append(new HlStringDetect(7,0,"\"\"\""));
  contextList[4] = c = new HlContext(7,4);
    c->items.append(new HlStringDetect(7,0,"\'\'\'"));
  contextList[5] = new HlContext(9,0);
  contextList[6] = new HlContext(0,1);
  contextList[7] = new HlContext(0,2);

  keyword->addList(pythonKeywords);
}

// ---------------------------------------------------------------------------

PerlHighlight::PerlHighlight(const QString& name) : Highlight(name) {
  dw = "*.pl;*.pm";
  dm = "application/x-perl";
}

void PerlHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));
  list.append(new ItemData("Keyword",dsKeyword));
  list.append(new ItemData("Variable",dsDecVal));
  list.append(new ItemData("Operator",dsOthers));
  list.append(new ItemData("String",dsString));
  list.append(new ItemData("String Char",dsChar));
  list.append(new ItemData("Comment",dsComment));
  list.append(new ItemData("Pod",dsOthers,Qt::darkYellow,Qt::yellow,false,true));
}

/*
hardcoded perl highlight

Op Customary  Generic     Meaning    Interpolates         Modifiers
1     ''       q{}       Literal         no
2     ""      qq{}       Literal         yes
3     ``      qx{}       Command         yes (no for ')
4             qw{}      Word list        no
5     //       m{}    Pattern match      yes (no for ')   cgimosx
6              s{}{}   Substitution      yes (no for ')   egimosx
7             tr{}{}   Translation       no               cds
7              y{}{}   Translation       no               cds
*/
int PerlHighlight::doHighlight(int ctxNum, TextLine *textLine) {
  static const char *opList[] = {"q", "qq", "qx", "qw", "m", "s", "tr", "y"};
  static int opLenList[] = {1, 2, 2, 2, 1, 1, 2, 1};
  char delimiter;
  int op;
  int argCount;
  bool interpolating, brackets, pod;

  const char *str, *s, *s2;
  bool lastWw;
  int pos, z, l;

  //extract some states out of the context number
  delimiter = ctxNum >> 8;
  op = (ctxNum >> 5) & 7;
  argCount = (ctxNum >> 3) & 3;
  interpolating = !(ctxNum & 4);
  brackets = ctxNum & 2;
  pod = ctxNum & 1;

  //current line to process
  str = textLine->getString();
  //whole word check status of last character
  lastWw = true;
  s = str;

  //match pod documentation tags
  if (*s == '=') {
    s++;
    pod = true;
    if (!strncmp(s, "cut", 3)) {
      pod = false;
      s += 3;
      textLine->setAttribs(7, 0, 4);
    }
  }
  if (pod) {
    textLine->setAttribs(7, 0, textLine->length());
    textLine->setAttr(7);
    goto finished;
  }
  while (*s) {
    pos = s - str;
    if (op == 0 && lastWw) {
      //match keyword
      s2 = keyword->checkHgl(s);
      if (s2 && testWw(*s2)) {
        s = s2;
        textLine->setAttribs(1, pos, s - str);
        goto newContext;
      }
      //match perl operator
      for (z = 0; z < 8; z++) {
        l = opLenList[z];
        if (!memcmp(s,opList[z],l) && testWw(s[l])) {
          //operator found
          if (z < 7) z++;
          op = z;
          argCount = (op >= 6) ? 2 : 1;
          s += l;
          textLine->setAttribs(3, pos, pos + l);
          goto newContext;
        }
      }
      //match customary
      if (*s == '\'') {
        op = 1;
        interpolating = false;
      }
      if (*s == '"') {
        op = 2;
      }
      if (*s == '`') {
        op = 3;
      }
      if (*s == '/') {
        op = 5;
      }
      if (op != 0) {
        delimiter = *s;
        s++;
        argCount = 1;
        textLine->setAttribs(3, pos, pos + 1);
        goto newContext;
      }
    }
    if (!delimiter) { //not in string
      //match comment
      if (lastWw && *s == '#') {
        textLine->setAttribs(6, pos, textLine->length());
        textLine->setAttr(6);
        goto finished;
      }
      //match delimiter
      if (op != 0 && (unsigned char) *s > 32) {
        delimiter = *s;
        if (delimiter == '(') {
          delimiter = ')';
          brackets = true;
        }
        if (delimiter == '<') {
          delimiter = '>';
          brackets = true;
        }
        if (delimiter == '[') {
          delimiter = ']';
          brackets = true;
        }
        if (delimiter == '{') {
          delimiter = '}';
          brackets = true;
        }
        s++;
        if (op == 1 || op == 4 || op == 7 || (delimiter == '\'' && op != 2))
          interpolating = false;
        textLine->setAttribs(3, pos, pos + 1);
        goto newContext;
      }
    }
    if (interpolating) {
      //match variable
      if (*s == '$' || *s == '@' || *s == '%') {
        s2 = s;
        do {
          s2++;
        } while ((!testWw(*s2) || *s2 == '#') && *s2 != delimiter);
        if (s2 - s > 1) {
          s = s2;
          textLine->setAttribs(2, pos, s2 - str);
          goto newContext;
        }
      }
      //match special variables
      if (s[0] == '$' && s[1] != '\0' && s[1] != delimiter) {
        if (strchr("&`'+*./|,\\;#%=-~^:?!@$<>()[]", s[1])) {
          s += 2;
          textLine->setAttribs(2, pos, pos + 2);
          goto newContext;
        }
      }
    }
    if (delimiter) { //in string
      //match escaped char
      if (interpolating) {
        if (*s == '\\' && s[1] != '\0') {
          s++;
          s2 = checkCharHexOct(s);
          if (s2) s = s2; else s++;
          textLine->setAttribs(5, pos, s - str);
          goto newContext;
        }
      }
      //match string end
      if (delimiter == *s) {
        s++;
        argCount--;
        if (argCount < 1) {
          //match operator modifiers
          if (op == 5) while (*s && strchr("cgimosx", *s)) s++;
          if (op == 6) while (*s && strchr("egimosx", *s)) s++;
          if (op == 7) while (*s && strchr("cds", *s)) s++;
          op = 0;
        }
        textLine->setAttribs(3, pos, s - str);
        if (brackets || op == 0) {
          interpolating = true;
          delimiter = '\0'; //string end delimiter = '\0' means "not in string"
          brackets = false;
        }
      } else {
        //highlight a ordinary character in string
        s++;
        textLine->setAttribs(4, pos, pos + 1);
      }
      goto newContext;
    }
    s++;
    textLine->setAttribs(0, pos, pos + 1);
    newContext:
    lastWw = testWw(s[-1]);
  }
  textLine->setAttr(0);
  finished:

  //compose new context number
  ctxNum = delimiter << 8;
  ctxNum |= op << 5;
  ctxNum |= argCount << 3;
  if (!interpolating) ctxNum |= 4;
  if (brackets) ctxNum |= 2;
  if (pod) ctxNum |= 1;
  return ctxNum;
  //method will be called again if there are more lines to highlight
}

void PerlHighlight::init() {
  keyword = new HlKeyword(0,0);
  keyword->addList(perlKeywords);
}

void PerlHighlight::done() {
  delete keyword;
}

// ---------------------------------------------------------------------------

SatherHighlight::SatherHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.sa";
  dm = "text/x-sather";
}

SatherHighlight::~SatherHighlight() {
}

void SatherHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text",dsNormal));         // 0
  list.append(new ItemData("Keyword",dsKeyword));            // 1
  list.append(new ItemData("Special Classname", dsNormal));  // 2
  list.append(new ItemData("Classname",dsNormal));           // 3
  list.append(new ItemData("Special Featurename",dsOthers)); // 4
  list.append(new ItemData("Identifier",dsOthers));          // 5
  list.append(new ItemData("Decimal",dsDecVal));             // 6
  list.append(new ItemData("Base-N",dsBaseN));               // 7
  list.append(new ItemData("Float",dsFloat));                // 8
  list.append(new ItemData("Char",dsChar));                  // 9
  list.append(new ItemData("String",dsString));              // 10
  list.append(new ItemData("Comment",dsComment));            // 11
}

void SatherHighlight::makeContextList() {
  HlContext *c;
  HlKeyword *keyword,*spec_class,*spec_feat;

  //Normal Context
  contextList[0] = c = new HlContext(0,0);
    c->items.append(keyword = new HlKeyword(1,0));
    c->items.append(spec_class = new HlKeyword(2,0));
    c->items.append(new HlSatherClassname(3,0));
    c->items.append(spec_feat = new HlKeyword(4,0));
    c->items.append(new HlSatherIdent(5,0));
    c->items.append(new HlSatherFloat(8,0)); // check float before int
    c->items.append(new HlSatherBaseN(7,0));
    c->items.append(new HlSatherDec(6,0));
    c->items.append(new HlSatherChar(9,0));
    c->items.append(new HlSatherString(10,0));
    c->items.append(new Hl2CharDetect(11,1,"--"));
  //Comment Context
  contextList[1] = c = new HlContext(11,0);

  keyword->addList(satherKeywords);
  spec_class->addList(satherSpecClassNames);
  spec_feat->addList(satherSpecFeatureNames);
}

// ---------------------------------------------------------------------------

LatexHighlight::LatexHighlight(const QString& name) : GenHighlight(name) {
  dw = "*.tex;*.sty";
  dm = "text/x-tex";
}

LatexHighlight::~LatexHighlight() {
}

void LatexHighlight::createItemData(ItemDataList &list) {

  list.append(new ItemData("Normal Text", dsNormal));
  list.append(new ItemData("Tag/Keyword", dsKeyword));
  list.append(new ItemData("Char", dsChar));
  list.append(new ItemData("Parameter", dsDecVal));
  list.append(new ItemData("Comment", dsComment));
}

void LatexHighlight::makeContextList() {
  HlContext *c;

  //normal context
  contextList[0] = c = new HlContext(0,0);
    c->items.append(new HlLatexTag(1,0));
    c->items.append(new HlLatexChar(2,0));
    c->items.append(new HlLatexParam(3,0));
    c->items.append(new HlCharDetect(4,1,'%'));
  //one line comment context
  contextList[1] = new HlContext(4,0);
}

// ---------------------------------------------------------------------------

HlManager::HlManager() : QObject(0L) {

  hlList.setAutoDelete(true);
  hlList.append(new Highlight("Normal"));
  hlList.append(new CHighlight("C"));
  hlList.append(new CppHighlight("C++"));
  hlList.append(new JavaHighlight("Java"));
  hlList.append(new HtmlHighlight("HTML"));
  hlList.append(new BashHighlight("Bash"));
  hlList.append(new ModulaHighlight("Modula 2"));
  hlList.append(new PascalHighlight("Pascal"));
  hlList.append(new AdaHighlight("Ada"));
  hlList.append(new PythonHighlight("Python"));
  hlList.append(new PerlHighlight("Perl"));
  hlList.append(new SatherHighlight("Sather"));
  hlList.append(new LatexHighlight("Latex"));
	hlList.append(new IdlHighlight("IDL"));
}

HlManager::~HlManager() {
}

Highlight *HlManager::getHl(int n) {
  if (n < 0 || n >= (int) hlList.count())
    n = 0;
  return hlList.at(n);
}

int HlManager::defaultHl() {
  KConfig *config;

  config = KGlobal::config();
  config->setGroup("General Options");
  return nameFind(config->readEntry("Highlight"));
}


int HlManager::nameFind(const QString& name) {
  int z;

  for (z = hlList.count() - 1; z > 0; z--) {
    if (hlList.at(z)->iName == name)
      break;
  }
  return z;
}

int HlManager::findByFile(const QString& filename)
{
  if (!filename.isEmpty())
  {
    QFileInfo fileInfo(filename);
    QString ext = fileInfo.extension(false);

    if (!ext.isEmpty())
    {
      for (Highlight *highlight = hlList.first(); highlight != 0L; highlight = hlList.next())
      {
        if (highlight->containsFiletype(ext))
          return hlList.at();
      }
    }
  }
  return -1;
}

int HlManager::findByMimetype(const QString& filename)
{
  if (!filename.isEmpty())
  {
    KMimeMagic* magic = KMimeMagic::self();
    magic->setFollowLinks(true);
    KMimeMagicResult *result = magic->findFileType(filename);
    if (result && result->mimeType())
    {
      for (Highlight *highlight = hlList.first(); highlight != 0L; highlight = hlList.next())
      {
        if (highlight->containsMimetype(result->mimeType()))
          return hlList.at();
      }
    }
  }
  return -1;
}


int HlManager::getHighlight(const QString& filename)
{
  int hl;
  if ((hl = findByFile(filename)) != -1)
    return hl;

  if ((hl = findByMimetype(filename)) != -1)
    return hl;

  return defaultHl();
}


void HlManager::makeAttribs(Highlight *highlight, Attribute *a, int n) {
  ItemStyleList defaultStyleList;
  ItemStyle *defaultStyle;
  ItemFont defaultFont;
  ItemDataList itemDataList;
  ItemData *itemData;
  int z;
  QFont font, printFont;
  KCharsets * charsets = KGlobal::charsets();

  defaultStyleList.setAutoDelete(true);
  getDefaults(defaultStyleList,defaultFont);

  itemDataList.setAutoDelete(true);
  highlight->getItemDataList(itemDataList);
  for (z = 0; z < (int) itemDataList.count(); z++) {
    itemData = itemDataList.at(z);
    if (itemData->defStyle) {
      defaultStyle = defaultStyleList.at(itemData->defStyleNum);
      a[z].col = defaultStyle->col;
      a[z].selCol = defaultStyle->selCol;
      font.setBold(defaultStyle->bold);
      font.setItalic(defaultStyle->italic);
    } else {
      a[z].col = itemData->col;
      a[z].selCol = itemData->selCol;
      font.setBold(itemData->bold);
      font.setItalic(itemData->italic);
    }

    if (itemData->defFont) {
      font.setFamily(defaultFont.family);
      font.setPointSize(defaultFont.size);
      printFont=font;
      printFont.setPointSize(defaultFont.printSize);

    } else {
      font.setFamily(itemData->family);
      font.setPointSize(itemData->size);
      printFont=font;
      printFont.setPointSize(itemData->printSize);
    }

    a[z].setFont(font);
    a[z].setPrintFont(printFont);
  }
  for (; z < n; z++) {
    a[z].col = black;
    a[z].selCol = black;
    a[z].setFont(font);
    a[z].setPrintFont(printFont);
  }
}

int HlManager::defaultStyles() {
  return 10;
}

const char *HlManager::defaultStyleName(int n) {
  static const char *names[] = {
    "Normal","Keyword","Data Type","Decimal/Value","Base-N Integer",
    "Floating Point","Character","String","Comment","Others"};

  return names[n];
}

void HlManager::getDefaults(ItemStyleList &list, ItemFont &font) {
  KConfig *config;
  int z;
  ItemStyle *i;
  QString s;
  QRgb col, selCol;

  list.setAutoDelete(true);
  //ItemStyle(color, selected color, bold, italic)
  list.append(new ItemStyle(black,white,false,false));     //normal
  list.append(new ItemStyle(black,white,true,false));      //keyword
  list.append(new ItemStyle(darkRed,white,false,false));   //datatype
  list.append(new ItemStyle(blue,cyan,false,false));       //decimal/value
  list.append(new ItemStyle(darkCyan,cyan,false,false));   //base n
  list.append(new ItemStyle(darkMagenta,cyan,false,false));//float
  list.append(new ItemStyle(magenta,magenta,false,false)); //char
  list.append(new ItemStyle(red,red,false,false));         //string
  list.append(new ItemStyle(darkGray,gray,false,true));    //comment
  list.append(new ItemStyle(darkGreen,green,false,false)); //others

  config = KGlobal::config();
  config->setGroup("Default Item Styles");
  for (z = 0; z < defaultStyles(); z++) {
    i = list.at(z);
    s = config->readEntry(defaultStyleName(z));
    if (!s.isEmpty()) {
      sscanf(s,"%X,%X,%d,%d",&col,&selCol,&i->bold,&i->italic);
      i->col.setRgb(col);
      i->selCol.setRgb(selCol);
    }
  }

  config->setGroup("Default Font");
  QFont defaultFont = KGlobalSettings::fixedFont();
  font.family = config->readEntry("Family", defaultFont.family());
  font.size = config->readNumEntry("Size", defaultFont.pointSize());
  font.printSize = config->readNumEntry("PrintSize", defaultFont.pointSize());
#if QT_VERSION < 300
  // ### doesn't compile with Qt3
  font.charset = config->readEntry("Charset", QFont::encodingName(QFont::ISO_8859_1));//  "ISO-8859-1");
#else
  font.charset = config->readEntry("Charset", "ISO-8859-1");
#endif
}

void HlManager::setDefaults(ItemStyleList &list, ItemFont &font) {
  KConfig *config;
  int z;
  ItemStyle *i;
  char s[64];

  config = KGlobal::config();
  config->setGroup("Default Item Styles");
  for (z = 0; z < defaultStyles(); z++) {
    i = list.at(z);
    sprintf(s,"%X,%X,%d,%d",i->col.rgb(),i->selCol.rgb(),i->bold, i->italic);
    config->writeEntry(defaultStyleName(z),s);
  }

  config->setGroup("Default Font");
  config->writeEntry("Family",font.family);
  config->writeEntry("Size",font.size);
  config->writeEntry("PrintSize",font.printSize);
  config->writeEntry("Charset",font.charset);

  emit changed();
}


int HlManager::highlights() {
  return (int) hlList.count();
}

const char *HlManager::hlName(int n) {
  return hlList.at(n)->iName;
}

void HlManager::getHlDataList(HlDataList &list) {
  int z;

  for (z = 0; z < (int) hlList.count(); z++) {
    list.append(hlList.at(z)->getData());
  }
}

void HlManager::setHlDataList(HlDataList &list) {
  int z;

  for (z = 0; z < (int) hlList.count(); z++) {
    hlList.at(z)->setData(list.at(z));
  }

  emit changed();
}

// ---------------------------------------------------------------------------

StyleChanger::StyleChanger(QWidget *parent) :
QWidget(parent)
{
  QLabel *label;
  QGridLayout *grid1 = new QGridLayout( this, 5, 3,15,7);

  col = new KColorButton(this);
  grid1->addWidget(col,2,0);
  label = new QLabel(i18n("Normal:"),this);
  grid1->addWidget(label,1,0);
  connect(col,SIGNAL(changed(const QColor &)),this,SLOT(changed()));

  selCol = new KColorButton(this);
  grid1->addWidget(selCol,4,0);

  label = new QLabel(selCol,i18n("Selected:"),this);
  grid1->addWidget(label,3,0);
  connect(selCol,SIGNAL(changed(const QColor &)),SLOT(changed()));

  bold = new QCheckBox(i18n("Bold"),this);
  connect(bold,SIGNAL(clicked()),SLOT(changed()));
  grid1->addWidget(bold,2,1);

  italic = new QCheckBox(i18n("Italic"),this);
  grid1->addWidget(italic,3,1);
  connect(italic,SIGNAL(clicked()),SLOT(changed()));
}

void StyleChanger::setRef(ItemStyle *s) {

  style = s;
  col->setColor(style->col);
  selCol->setColor(style->selCol);
  bold->setChecked(style->bold);
  italic->setChecked(style->italic);

}

void StyleChanger::setEnabled(bool enable) {

  col->setEnabled(enable);
  selCol->setEnabled(enable);
  bold->setEnabled(enable);
  italic->setEnabled(enable);
}

void StyleChanger::changed() {

  if (style) {
    style->col = col->color();
    style->selCol = selCol->color();
    style->bold = bold->isChecked();
    style->italic = italic->isChecked();
  }
}

// ---------------------------------------------------------------------------

FontChanger::FontChanger(QWidget *parent) :
  QWidget(parent)
{
  QLabel *label;

  QStringList fontList;
  KFontChooser::getFontList(fontList, false);
  QVBoxLayout *box = new QVBoxLayout( this );

  label = new QLabel(i18n("Family:"),this);
  box->addWidget(label);

  familyCombo = new QComboBox(true,this);
  box->addWidget(familyCombo);

  connect(familyCombo,SIGNAL(activated(const QString&)),SLOT(familyChanged(const QString&)));
  familyCombo->insertStringList(fontList);

  label = new QLabel(i18n("Size:"),this);
  box->addWidget(label);
  sizeCombo = new QComboBox(true,this);
  box->addWidget(sizeCombo);

  connect(sizeCombo,SIGNAL(activated(int)),SLOT(sizeChanged(int)));
  for( int i=0; fontSizes[i] != 0; i++ ){
    sizeCombo->insertItem(QString().setNum(fontSizes[i]),i);
  }

  label = new QLabel(i18n("Charset:"),this);
  box->addWidget(label);
  charsetCombo = new QComboBox(true,this);
  box->addWidget(charsetCombo);

  connect(charsetCombo,SIGNAL(activated(const QString&)),SLOT(charsetChanged(const QString&)));

  label = new QLabel(i18n("Size (printing):"),this);
  box->addWidget(label);
  printSizeCombo = new QComboBox(true,this);
  box->addWidget(printSizeCombo);

  connect(printSizeCombo,SIGNAL(activated(int)),SLOT(printSizeChanged(int)));
  for( int i=0; fontSizes[i] != 0; i++ ){
    printSizeCombo->insertItem(QString().setNum(fontSizes[i]),i);
  }

}

void FontChanger::setRef(ItemFont *f) {
  int z, defaultZ=0;
  bool found=false;
  QFont defaultFont = KGlobalSettings::fixedFont();

  font = f;
  for (z = 0; z < (int) familyCombo->count(); z++) {
    if (font->family == familyCombo->text(z)) {
      familyCombo->setCurrentItem(z);
      found=true;
      break;
    }

    if (defaultFont.family() == familyCombo->text(z))
       defaultZ=z;
  }

  if (!found)
  {
    familyCombo->setCurrentItem(defaultZ);
    font->family = familyCombo->text(defaultZ);
  }

  found=false; defaultZ=0;
  for (z = 0; fontSizes[z] > 0; z++) {
    if (font->size == fontSizes[z]) {
      sizeCombo->setCurrentItem(z);
      found=true;
      break;
    }

    if (defaultFont.pointSize() == fontSizes[z])
      defaultZ=z;
  }

  if (!found)
  {
    font->size = fontSizes[defaultZ];
    sizeCombo->setCurrentItem(defaultZ);
  }

  found=false;
  for (z = 0; fontSizes[z] > 0; z++) {
    if (font->printSize == fontSizes[z]) {
      printSizeCombo->setCurrentItem(z);
      found=true;
      break;
    }
  }

  if (!found)
  {
    font->printSize = fontSizes[defaultZ];
    printSizeCombo->setCurrentItem(defaultZ);
  }

  displayCharsets();
}

void FontChanger::familyChanged(const QString& family) {

  font->family = family;
  displayCharsets();
}

void FontChanger::sizeChanged(int n) {

  font->size = fontSizes[n];
}

void FontChanger::printSizeChanged(int n) {

  font->printSize = fontSizes[n];
}

void FontChanger::charsetChanged(const QString& charset) {

  font->charset = charset;
}

void FontChanger::displayCharsets() {
  int z;
  QString charset;
  KCharsets *charsets;

  charsets = KGlobal::charsets();
#if QT_VERSION < 300
  QStringList lst = charsets->availableCharsetNames(font->family);
#else
  QStringList lst; // ### fails with Qt3...
#endif
//  QStrList lst = charsets->displayable(font->family);
  charsetCombo->clear();
  for(z = 0; z < (int) lst.count(); z++) {
    charset = *lst.at(z);
    charsetCombo->insertItem(charset,z);
    if (/*(QString)*/ font->charset == charset)
      charsetCombo->setCurrentItem(z);
  }
  charset = "any";
  charsetCombo->insertItem(charset,z);
  if (/*(QString)*/ font->charset == charset)
    charsetCombo->setCurrentItem(z);
}

// ---------------------------------------------------------------------------

DefaultsDialog::DefaultsDialog(HlManager *hlManager, ItemStyleList *styleList,
  ItemFont *font, QWidget *parent) : QDialog(parent,0L,true) {

  QComboBox *styleCombo;
  QLabel *label;
  FontChanger *fontChanger;
  QPushButton *button;
//  QRect r;
  int z;

  QGridLayout *grid1 = new QGridLayout( this, 2, 2,15,7);


  QVGroupBox *groupbox = new QVGroupBox(i18n("Default Item Styles"),this);
  grid1->addWidget(groupbox,0,0);

  label = new QLabel(i18n("Item:"),groupbox);
  styleCombo = new QComboBox(false,groupbox);
  connect(styleCombo,SIGNAL(activated(int)),this,SLOT(changed(int)));

  styleChanger = new StyleChanger(groupbox);
  connect(styleCombo,SIGNAL(activated(int)),this,SLOT(changed(int)));

  for (z = 0; z < hlManager->defaultStyles(); z++) {
    styleCombo->insertItem(i18n(hlManager->defaultStyleName(z)),z);
  }

  groupbox = new QVGroupBox(i18n("Default Font"),this);
  grid1->addWidget(groupbox,0,1);

  fontChanger = new FontChanger(groupbox);
  fontChanger->setRef(font);

  itemStyleList = styleList;
  changed(0);

  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  button  = bb->addButton( i18n("&OK") );

  connect(button,SIGNAL(clicked()),this,SLOT(accept()));
  button  = bb->addButton( i18n("&Cancel") );
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));
  bb->layout();
  grid1->addWidget(bb,1,1);
}

void DefaultsDialog::changed(int z) {

  styleChanger->setRef(itemStyleList->at(z));
}

// ---------------------------------------------------------------------------

HighlightDialog::HighlightDialog(HlManager *hlManager,
  HlDataList *highlightDataList, int hlNumber, QWidget *parent)
  : QDialog(parent,0L,true), hlData(0L) {

  QPushButton *button;
  QGroupBox *group;
  QLabel *label;
  int z;
  QGridLayout *grid1 = new QGridLayout( this, 3,2 ,15,7);

  group = new QGroupBox(i18n("Config Select"),this);
  grid1->addWidget(group,0,0);

  QVBoxLayout *box = new QVBoxLayout( group );
  box->setMargin( 15 );
  box->setSpacing( 15 );


  label = new QLabel(i18n("Highlight:"),group);
  box->addWidget(label);
  hlCombo = new QComboBox(false,group);
  box->addWidget(hlCombo);
  connect(hlCombo,SIGNAL(activated(int)),SLOT(hlChanged(int)));

  for (z = 0; z < hlManager->highlights(); z++) {
    hlCombo->insertItem(hlManager->hlName(z),z);
  }
  hlCombo->setCurrentItem(hlNumber);

  label = new QLabel(i18n("Item:"),group);
  box->addWidget(label);
  itemCombo = new QComboBox(false,group);
  box->addWidget(itemCombo);

  connect(itemCombo,SIGNAL(activated(int)),SLOT(itemChanged(int)));

  group = new QGroupBox(i18n("Highlight Auto Select"),this);
  grid1->addWidget(group,0,1);

  box = new QVBoxLayout( group );
  box->setMargin( 15 );
  box->setSpacing( 15 );

  label = new QLabel(i18n("File Extensions:"),group);
  box->addWidget(label);
  wildcards = new QLineEdit(group);
  box->addWidget(wildcards);

  label = new QLabel(i18n("Mime Types:"),group);
  box->addWidget(label);
  mimetypes = new QLineEdit(group);
  box->addWidget(mimetypes);


  QVGroupBox *groupbox = new QVGroupBox(i18n("Item Style"),this);
  grid1->addWidget(groupbox,1,0);

  styleDefault = new QCheckBox(i18n("Default"),groupbox);
  connect(styleDefault,SIGNAL(clicked()),SLOT(changed()));
  styleChanger = new StyleChanger(groupbox);


  groupbox = new QVGroupBox(i18n("Item Font"),this);
  grid1->addWidget(groupbox,1,1);

  fontDefault = new QCheckBox(i18n("Default"),groupbox);
  connect(fontDefault,SIGNAL(clicked()),SLOT(changed()));
  fontChanger = new FontChanger(groupbox);



  hlDataList = highlightDataList;
  hlChanged(hlNumber);
  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  button  = bb->addButton( i18n("OK") );
  button->setDefault(true);
  connect(button,SIGNAL(clicked()),this,SLOT(accept()));

  button = bb->addButton(i18n("&Cancel"));
  connect(button,SIGNAL(clicked()),this,SLOT(reject()));
  bb->layout();
  grid1->addWidget(bb,2,1);
}

void HighlightDialog::hlChanged(int z) {
  ItemData *itemData;

  writeback();

  hlData = hlDataList->at(z);

  wildcards->setText(hlData->wildcards);
  mimetypes->setText(hlData->mimetypes);

  itemCombo->clear();
  for (itemData = hlData->itemDataList.first(); itemData != 0L;
    itemData = hlData->itemDataList.next()) {
    itemCombo->insertItem(i18n(itemData->name));
  }

  itemChanged(0);

//  if (hlCombo->currentItem() != z)
//    hlCombo->setCurrentItem(z);
}

void HighlightDialog::itemChanged(int z) {

  itemData = hlData->itemDataList.at(z);

  styleDefault->setChecked(itemData->defStyle);
  styleChanger->setRef(itemData);
  styleChanger->setEnabled(!itemData->defStyle);

  fontDefault->setChecked(itemData->defFont);
  fontChanger->setRef(itemData);
  fontChanger->setEnabled(!itemData->defFont);

}

void HighlightDialog::changed() {

  itemData->defStyle = styleDefault->isChecked();
  styleChanger->setEnabled(!itemData->defStyle);
  itemData->defFont = fontDefault->isChecked();
  fontChanger->setEnabled(!itemData->defFont);
}

void HighlightDialog::writeback() {
  if (hlData) {
    hlData->wildcards = wildcards->text();
    hlData->mimetypes = mimetypes->text();
  }
}

void HighlightDialog::done(int r) {
  writeback();
  QDialog::done(r);
}
#include "highlight.moc"
