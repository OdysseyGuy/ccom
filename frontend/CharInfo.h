#ifndef CHARINFO_H
#define CHARINFO_H

#include <cstdint>

extern uint16_t InfoTable[256];

enum {
  CHAR_HORZ_WS = 0x0001, // '\t', '\f', '\v'.  Note, no '\0'
  CHAR_VERT_WS = 0x0002, // Carriage Return '\r', Line Feed '\n'
  CHAR_SPACE = 0x0004,   // ' '
  CHAR_DIGIT = 0x0008,   // 0-9
  CHAR_XLETTER = 0x0010, // (hex alphabets) a-f,A-F
  CHAR_UPPER = 0x0020,   // A-Z
  CHAR_LOWER = 0x0040,   // a-z
  CHAR_UNDER = 0x0080,   // _
  CHAR_PERIOD = 0x0100,  // .
  CHAR_RAWDEL = 0x0200,  // {}[]#<>%:;?*+-/^&|~!=,"'
  CHAR_PUNCT = 0x0400,   // `$@()
};

inline bool IsAscii(char C) { return static_cast<unsigned char>(C) <= 127; }
inline bool IsAscii(unsigned char C) { return C <= 127; }
inline bool IsAscii(uint32_t C) { return C <= 127; }

/**
 * Returns true if this character is horizontal or vertical ASCII whitespace:
 *  '\r', '\n', ' ', \t', '\f', '\v'
 *
 * Note this returns false for '\0'
 */
inline bool IsWhitespace(unsigned char C) {
  return (InfoTable[C] & (CHAR_HORZ_WS | CHAR_VERT_WS | CHAR_SPACE));
}

/**
 * Returns true if this character is a horizontal ASCII whitespace:
 *  ' ', \t', '\f', '\v'
 *
 * Note this returns false for '\0'
 */
inline bool IsHorizontalWhitespace(unsigned char C) {
  return (InfoTable[C] & (CHAR_HORZ_WS | CHAR_SPACE));
}

/**
 * Returns true if this character is a vertical ASCII whitespace:
 *  '\r', '\n'
 *
 * Note this returns false for '\0'
 */
inline bool IsVerticalWhitespace(unsigned char C) {
  return (InfoTable[C] & (CHAR_VERT_WS));
}

inline bool IsPreprocessingNumber(unsigned char C) {}

#endif