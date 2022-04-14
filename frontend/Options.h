#ifndef LANG_OPTIONS_H
#define LANG_OPTIONS_H

/** C/C++ language options. */
struct LanguageOptions {

#define LANG_OPT(Name, Bits, Default, Description) unsigned Name : Bits;
  // clang-format off
  LANG_OPT(C99,         1, 0, "C99")
  LANG_OPT(C11,         1, 0, "C11")
  LANG_OPT(C17,         1, 0, "C17")
  LANG_OPT(C2x,         1, 0, "C2x")

  LANG_OPT(CPlusPlus,   1, 0, "C++")
  LANG_OPT(CPlusPlus11, 1, 0, "C++11")
  LANG_OPT(CPlusPlus14, 1, 0, "C++14")
  LANG_OPT(CPlusPlus17, 1, 0, "C++17")
  LANG_OPT(CPlusPlus20, 1, 0, "C++20")
  LANG_OPT(CPlusPlus2b, 1, 0, "C++2b")

  LANG_OPT(Trigraphs,   1, 0, "Trigraphs")
  LANG_OPT(Digraphs,    1, 0, "Digraphs")
  // clang-format on
};

#endif
