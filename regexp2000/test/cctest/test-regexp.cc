// Copyright 2006-2008 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <stdlib.h>
#include <set>

#include "v8.h"

#include "cctest.h"
#include "zone-inl.h"
#include "parser.h"
#include "ast.h"
#include "jsregexp-inl.h"


using namespace v8::internal;


class RegExpTestCase {
 public:
  RegExpTestCase()
    : pattern_(NULL),
      flags_(NULL),
      input_(NULL),
      compile_error_(NULL) { }
  RegExpTestCase(const char* pattern,
                 const char* flags,
                 const char* input,
                 const char* compile_error)
    : pattern_(pattern),
      flags_(flags),
      input_(input),
      compile_error_(compile_error) { }
  const char* pattern() const { return pattern_; }
  bool expect_error() const { return compile_error_ != NULL; }
 private:
  const char* pattern_;
  const char* flags_;
  const char* input_;
  const char* compile_error_;
};


static SmartPointer<char> Parse(const char* input) {
  v8::HandleScope scope;
  unibrow::Utf8InputBuffer<> buffer(input, strlen(input));
  ZoneScope zone_scope(DELETE_ON_EXIT);
  Handle<String> error;
  RegExpTree* node = v8::internal::ParseRegExp(&buffer, &error);
  CHECK(node != NULL);
  CHECK(error.is_null());
  SmartPointer<char> output = node->ToString();
  return output;
}


#define CHECK_PARSE_EQ(input, expected) CHECK_EQ(expected, *Parse(input))


TEST(Parser) {
  V8::Initialize(NULL);
  CHECK_PARSE_EQ("abc", "'abc'");
  CHECK_PARSE_EQ("", "%");
  CHECK_PARSE_EQ("abc|def", "(| 'abc' 'def')");
  CHECK_PARSE_EQ("abc|def|ghi", "(| 'abc' 'def' 'ghi')");
  CHECK_PARSE_EQ("^xxx$", "(: @^i 'xxx' @$i)");
  CHECK_PARSE_EQ("ab\\b\\d\\bcd", "(: 'ab' @b [0-9] @b 'cd')");
  CHECK_PARSE_EQ("\\w|\\d", "(| [0-9 A-Z _ a-z] [0-9])");
  CHECK_PARSE_EQ("a*", "(# 0 - g 'a')");
  CHECK_PARSE_EQ("a*?", "(# 0 - n 'a')");
  CHECK_PARSE_EQ("abc+", "(# 1 - g 'abc')");
  CHECK_PARSE_EQ("abc+?", "(# 1 - n 'abc')");
  CHECK_PARSE_EQ("xyz?", "(# 0 1 g 'xyz')");
  CHECK_PARSE_EQ("xyz??", "(# 0 1 n 'xyz')");
  CHECK_PARSE_EQ("xyz{0,1}", "(# 0 1 g 'xyz')");
  CHECK_PARSE_EQ("xyz{0,1}?", "(# 0 1 n 'xyz')");
  CHECK_PARSE_EQ("xyz{93}", "(# 93 93 g 'xyz')");
  CHECK_PARSE_EQ("xyz{93}?", "(# 93 93 n 'xyz')");
  CHECK_PARSE_EQ("xyz{1,32}", "(# 1 32 g 'xyz')");
  CHECK_PARSE_EQ("xyz{1,32}?", "(# 1 32 n 'xyz')");
  CHECK_PARSE_EQ("xyz{1,}", "(# 1 - g 'xyz')");
  CHECK_PARSE_EQ("xyz{1,}?", "(# 1 - n 'xyz')");
  CHECK_PARSE_EQ("a\\fb\\nc\\rd\\te\\vf", "'a\fb\nc\rd\te\vf'");
  CHECK_PARSE_EQ("a\\nb\\bc", "(: 'a\nb' @b 'c')");
  CHECK_PARSE_EQ("(?:foo)", "'foo'");
  CHECK_PARSE_EQ("(?: foo )", "' foo '");
  CHECK_PARSE_EQ("(foo|bar|baz)", "(^ (| 'foo' 'bar' 'baz'))");
  CHECK_PARSE_EQ("foo|(bar|baz)|quux", "(| 'foo' (^ (| 'bar' 'baz')) 'quux')");
  CHECK_PARSE_EQ("foo(?=bar)baz", "(: 'foo' (-> + 'bar') 'baz')");
  CHECK_PARSE_EQ("foo(?!bar)baz", "(: 'foo' (-> - 'bar') 'baz')");
  CHECK_PARSE_EQ("()", "(^ %)");
  CHECK_PARSE_EQ("(?=)", "(-> + %)");
  CHECK_PARSE_EQ("[]", "%");
  CHECK_PARSE_EQ("[x]", "[x]");
  CHECK_PARSE_EQ("[xyz]", "[x y z]");
  CHECK_PARSE_EQ("[a-zA-Z0-9]", "[a-z A-Z 0-9]");
  CHECK_PARSE_EQ("[-123]", "[- 1 2 3]");
  CHECK_PARSE_EQ("[^123]", "^[1 2 3]");
  CHECK_PARSE_EQ("]", "']'");
  CHECK_PARSE_EQ("}", "'}'");
  CHECK_PARSE_EQ("[a-b-c]", "[a-b - c]");
  CHECK_PARSE_EQ("[\\d]", "[0-9]");
  CHECK_PARSE_EQ("[x\\dz]", "[x 0-9 z]");
  CHECK_PARSE_EQ("[\\d-z]", "[0-9 - z]");
  CHECK_PARSE_EQ("[\\d-\\d]", "[0-9 - 0-9]");
  CHECK_PARSE_EQ("\\cj\\cJ\\ci\\cI\\ck\\cK", "'\n\n\t\t\v\v'");
  CHECK_PARSE_EQ("\\c!", "'c!'");
  CHECK_PARSE_EQ("\\c_", "'c_'");
  CHECK_PARSE_EQ("\\c~", "'c~'");
  CHECK_PARSE_EQ("[a\\]c]", "[a ] c]");
  CHECK_PARSE_EQ("\\[\\]\\{\\}\\(\\)\\%\\^\\#\\ ", "'[]{}()%^# '");
  CHECK_PARSE_EQ("[\\[\\]\\{\\}\\(\\)\\%\\^\\#\\ ]", "[[ ] { } ( ) % ^ #  ]");
  CHECK_PARSE_EQ("\\0", "'\0'");
  CHECK_PARSE_EQ("\\8", "'8'");
  CHECK_PARSE_EQ("\\9", "'9'");
  CHECK_PARSE_EQ("\\11", "'\t'");
  CHECK_PARSE_EQ("\\11a", "'\ta'");
  CHECK_PARSE_EQ("\\011", "'\t'");
  CHECK_PARSE_EQ("\\00011", "'\00011'");
  CHECK_PARSE_EQ("\\118", "'\t8'");
  CHECK_PARSE_EQ("\\111", "'I'");
  CHECK_PARSE_EQ("\\1111", "'I1'");
  CHECK_PARSE_EQ("(x)(x)(x)\\1", "(: (^ 'x') (^ 'x') (^ 'x') (<- 1))");
  CHECK_PARSE_EQ("(x)(x)(x)\\2", "(: (^ 'x') (^ 'x') (^ 'x') (<- 2))");
  CHECK_PARSE_EQ("(x)(x)(x)\\3", "(: (^ 'x') (^ 'x') (^ 'x') (<- 3))");
  CHECK_PARSE_EQ("(x)(x)(x)\\4", "(: (^ 'x') (^ 'x') (^ 'x') '\x04')");
  CHECK_PARSE_EQ("(x)(x)(x)\\1*", "(: (^ 'x') (^ 'x') (^ 'x')"
                               " (# 0 - g (<- 1)))");
  CHECK_PARSE_EQ("(x)(x)(x)\\2*", "(: (^ 'x') (^ 'x') (^ 'x')"
                               " (# 0 - g (<- 2)))");
  CHECK_PARSE_EQ("(x)(x)(x)\\3*", "(: (^ 'x') (^ 'x') (^ 'x')"
                               " (# 0 - g (<- 3)))");
  CHECK_PARSE_EQ("(x)(x)(x)\\4*", "(: (^ 'x') (^ 'x') (^ 'x')"
                               " (# 0 - g '\x04'))");
  CHECK_PARSE_EQ("(x)(x)(x)(x)(x)(x)(x)(x)(x)(x)\\10",
              "(: (^ 'x') (^ 'x') (^ 'x') (^ 'x') (^ 'x') (^ 'x')"
              " (^ 'x') (^ 'x') (^ 'x') (^ 'x') (<- 10))");
  CHECK_PARSE_EQ("(x)(x)(x)(x)(x)(x)(x)(x)(x)(x)\\11",
              "(: (^ 'x') (^ 'x') (^ 'x') (^ 'x') (^ 'x') (^ 'x')"
              " (^ 'x') (^ 'x') (^ 'x') (^ 'x') '\x09')");
  CHECK_PARSE_EQ("[\\0]", "[\0]");
  CHECK_PARSE_EQ("[\\11]", "[\t]");
  CHECK_PARSE_EQ("[\\11a]", "[\t a]");
  CHECK_PARSE_EQ("[\\011]", "[\t]");
  CHECK_PARSE_EQ("[\\00011]", "[\000 1 1]");
  CHECK_PARSE_EQ("[\\118]", "[\t 8]");
  CHECK_PARSE_EQ("[\\111]", "[I]");
  CHECK_PARSE_EQ("[\\1111]", "[I 1]");
  CHECK_PARSE_EQ("\\x34", "'\x34'");
  CHECK_PARSE_EQ("\\x3z", "'x3z'");
  CHECK_PARSE_EQ("\\u0034", "'\x34'");
  CHECK_PARSE_EQ("\\u003z", "'u003z'");
}


static void ExpectError(const char* input,
                        const char* expected) {
  v8::HandleScope scope;
  unibrow::Utf8InputBuffer<> buffer(input, strlen(input));
  ZoneScope zone_scope(DELETE_ON_EXIT);
  Handle<String> error;
  RegExpTree* node = v8::internal::ParseRegExp(&buffer, &error);
  CHECK(node == NULL);
  CHECK(!error.is_null());
  SmartPointer<char> str = error->ToCString(ALLOW_NULLS);
  CHECK_EQ(expected, *str);
}


TEST(Errors) {
  V8::Initialize(NULL);
  const char* kEndBackslash = "\\ at end of pattern";
  ExpectError("\\", kEndBackslash);
  const char* kInvalidQuantifier = "Invalid quantifier";
  ExpectError("a{}", kInvalidQuantifier);
  ExpectError("a{,}", kInvalidQuantifier);
  ExpectError("a{", kInvalidQuantifier);
  ExpectError("a{z}", kInvalidQuantifier);
  ExpectError("a{1z}", kInvalidQuantifier);
  ExpectError("a{12z}", kInvalidQuantifier);
  ExpectError("a{12,", kInvalidQuantifier);
  ExpectError("a{12,3b", kInvalidQuantifier);
  const char* kUnterminatedGroup = "Unterminated group";
  ExpectError("(foo", kUnterminatedGroup);
  const char* kInvalidGroup = "Invalid group";
  ExpectError("(?", kInvalidGroup);
  const char* kUnterminatedCharacterClass = "Unterminated character class";
  ExpectError("[", kUnterminatedCharacterClass);
  ExpectError("[a-", kUnterminatedCharacterClass);
  const char* kIllegalCharacterClass = "Illegal character class";
  ExpectError("[a-\\w]", kIllegalCharacterClass);
  const char* kEndControl = "\\c at end of pattern";
  ExpectError("\\c", kEndControl);
}


static bool IsDigit(uc16 c) {
  return ('0' <= c && c <= '9');
}


static bool NotDigit(uc16 c) {
  return !IsDigit(c);
}


static bool IsWhiteSpace(uc16 c) {
  switch (c) {
    case 0x09: case 0x0B: case 0x0C: case 0x20: case 0xA0:
      return true;
    default:
      return unibrow::Space::Is(c);
  }
}


static bool NotWhiteSpace(uc16 c) {
  return !IsWhiteSpace(c);
}


static bool IsWord(uc16 c) {
  return ('a' <= c && c <= 'z')
      || ('A' <= c && c <= 'Z')
      || ('0' <= c && c <= '9')
      || (c == '_');
}


static bool NotWord(uc16 c) {
  return !IsWord(c);
}


static bool Dot(uc16 c) {
  return true;
}


static void TestCharacterClassEscapes(uc16 c, bool (pred)(uc16 c)) {
  ZoneScope scope(DELETE_ON_EXIT);
  ZoneList<CharacterRange>* ranges = new ZoneList<CharacterRange>(2);
  CharacterRange::AddClassEscape(c, ranges);
  for (unsigned i = 0; i < (1 << 16); i++) {
    bool in_class = false;
    for (int j = 0; !in_class && j < ranges->length(); j++) {
      CharacterRange& range = ranges->at(j);
      in_class = (range.from() <= i && i <= range.to());
    }
    CHECK_EQ(pred(i), in_class);
  }
}


TEST(CharacterClassEscapes) {
  TestCharacterClassEscapes('.', Dot);
  TestCharacterClassEscapes('d', IsDigit);
  TestCharacterClassEscapes('D', NotDigit);
  TestCharacterClassEscapes('s', IsWhiteSpace);
  TestCharacterClassEscapes('S', NotWhiteSpace);
  TestCharacterClassEscapes('w', IsWord);
  TestCharacterClassEscapes('W', NotWord);
}


static void Execute(bool expected, const char* input, const char* str) {
  v8::HandleScope scope;
  unibrow::Utf8InputBuffer<> buffer(input, strlen(input));
  ZoneScope zone_scope(DELETE_ON_EXIT);
  Handle<String> error;
  RegExpTree* tree = v8::internal::ParseRegExp(&buffer, &error);
  CHECK(tree != NULL);
  CHECK(error.is_null());
  RegExpNode<const char>* node = RegExpEngine::Compile<const char>(tree);
  bool outcome = RegExpEngine::Execute(node, CStrVector(str));
  CHECK_EQ(outcome, expected);
}


TEST(Execution) {
  V8::Initialize(NULL);
  Execute(true, ".*?(?:a[bc]d|e[fg]h)", "xxxabbegh");
  Execute(true, ".*?(?:a[bc]d|e[fg]h)", "xxxabbefh");
  Execute(false, ".*?(?:a[bc]d|e[fg]h)", "xxxabbefd");
}


class TestConfig {
 public:
  typedef int Key;
  typedef int Value;
  static const int kNoKey;
  static const int kNoValue;
  static inline int Compare(int a, int b) {
    if (a < b) return -1;
    else if (a > b) return 1;
    else return 0;
  }
};


const int TestConfig::kNoKey = 0;
const int TestConfig::kNoValue = 0;


static int PseudoRandom(int i, int j) {
  return ~(~((i * 781) ^ (j * 329)));
}


TEST(SplayTreeSimple) {
  static const int kLimit = 1000;
  ZoneScope zone_scope(DELETE_ON_EXIT);
  ZoneSplayTree<TestConfig> tree;
  std::set<int> seen;
#define CHECK_MAPS_EQUAL() do {                                      \
    for (int k = 0; k < kLimit; k++)                                 \
      CHECK_EQ(seen.find(k) != seen.end(), tree.Find(k, &loc));      \
  } while (false)
  for (int i = 0; i < 50; i++) {
    for (int j = 0; j < 50; j++) {
      int next = PseudoRandom(i, j) % kLimit;
      if (seen.find(next) != seen.end()) {
        // We've already seen this one.  Check the value and remove
        // it.
        ZoneSplayTree<TestConfig>::Locator loc;
        CHECK(tree.Find(next, &loc));
        CHECK_EQ(next, loc.key());
        CHECK_EQ(3 * next, loc.value());
        tree.Remove(next);
        seen.erase(next);
        CHECK_MAPS_EQUAL();
      } else {
        // Check that it wasn't there already and then add it.
        ZoneSplayTree<TestConfig>::Locator loc;
        CHECK(!tree.Find(next, &loc));
        CHECK(tree.Insert(next, &loc));
        CHECK_EQ(next, loc.key());
        loc.set_value(3 * next);
        seen.insert(next);
        CHECK_MAPS_EQUAL();
      }
      int val = PseudoRandom(j, i) % kLimit;
      for (int k = val; k >= 0; k--) {
        if (seen.find(val) != seen.end()) {
          ZoneSplayTree<TestConfig>::Locator loc;
          CHECK(tree.FindGreatestLessThan(val, &loc));
          CHECK_EQ(loc.key(), val);
          break;
        }
      }
      val = PseudoRandom(i + j, i - j) % kLimit;
      for (int k = val; k < kLimit; k++) {
        if (seen.find(val) != seen.end()) {
          ZoneSplayTree<TestConfig>::Locator loc;
          CHECK(tree.FindLeastGreaterThan(val, &loc));
          CHECK_EQ(loc.key(), val);
          break;
        }
      }
    }
  }
}


static int CompareChars(const void* ap, const void* bp) {
  uc16 a = *static_cast<const uc16*>(ap);
  uc16 b = *static_cast<const uc16*>(bp);
  if (a < b) return -1;
  else if (a > b) return 1;
  else return 0;
}


TEST(DispatchTableConstruction) {
  // Initialize test data.
  static const int kLimit = 1000;
  static const int kRangeCount = 8;
  static const int kRangeSize = 16;
  uc16 ranges[kRangeCount][2 * kRangeSize];
  for (int i = 0; i < kRangeCount; i++) {
    uc16* range = ranges[i];
    for (int j = 0; j < 2 * kRangeSize; j++) {
      range[j] = PseudoRandom(i + 25, j + 87) % kLimit;
    }
    qsort(range, 2 * kRangeSize, sizeof(uc16), CompareChars);
  }
  // Enter test data into dispatch table.
  ZoneScope zone_scope(DELETE_ON_EXIT);
  DispatchTable table;
  for (int i = 0; i < kRangeCount; i++) {
    uc16* range = ranges[i];
    for (int j = 0; j < 2 * kRangeSize; j += 2)
      table.AddRange(CharacterRange(range[j], range[j + 1]), i);
  }
  // Check that the table looks as we would expect
  for (int p = 0; p < kLimit; p++) {
    OutSet outs = table.Get(p);
    for (int j = 0; j < kRangeCount; j++) {
      uc16* range = ranges[j];
      bool is_on = false;
      for (int k = 0; !is_on && (k < 2 * kRangeSize); k += 2)
        is_on = (range[k] <= p && p <= range[k + 1]);
      CHECK_EQ(is_on, outs.Get(j));
    }
  }
}
