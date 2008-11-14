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

#ifndef V8_JSREGEXP_H_
#define V8_JSREGEXP_H_

namespace v8 { namespace internal {

class RegExpImpl {
 public:
  // Creates a regular expression literal in the old space.
  // This function calls the garbage collector if necessary.
  static Handle<Object> CreateRegExpLiteral(Handle<JSFunction> constructor,
                                            Handle<String> pattern,
                                            Handle<String> flags,
                                            bool* has_pending_exception);

  // Returns a string representation of a regular expression.
  // Implements RegExp.prototype.toString, see ECMA-262 section 15.10.6.4.
  // This function calls the garbage collector if necessary.
  static Handle<String> ToString(Handle<Object> value);

  static Handle<Object> Compile(Handle<JSRegExp> re,
                                Handle<String> pattern,
                                Handle<String> flags);

  // Implements RegExp.prototype.exec(string) function.
  // See ECMA-262 section 15.10.6.2.
  // This function calls the garbage collector if necessary.
  static Handle<Object> Exec(Handle<JSRegExp> regexp,
                             Handle<String> subject,
                             Handle<Object> index);

  // Call RegExp.prototyp.exec(string) in a loop.
  // Used by String.prototype.match and String.prototype.replace.
  // This function calls the garbage collector if necessary.
  static Handle<Object> ExecGlobal(Handle<JSRegExp> regexp,
                                   Handle<String> subject);

  // Stores an uncompiled RegExp pattern in the JSRegExp object.
  // It will be compiled by JSCRE when first executed.
  static Handle<Object> JscrePrepare(Handle<JSRegExp> re,
                                     Handle<String> pattern,
                                     JSRegExp::Flags flags);

  // Stores a compiled RegExp pattern in the JSRegExp object.
  // The pattern is compiled by Regexp2000.
  static Handle<Object> Re2kPrepare(Handle<JSRegExp> re,
                                    Handle<String> pattern,
                                    JSRegExp::Flags flags,
                                    Handle<FixedArray> re2k_data);


  // Compile the pattern using JSCRE and store the result in the
  // JSRegExp object.
  static Handle<Object> JscreCompile(Handle<JSRegExp> re);

  static Handle<Object> AtomCompile(Handle<JSRegExp> re,
                                    Handle<String> pattern,
                                    JSRegExp::Flags flags,
                                    Handle<String> match_pattern);
  static Handle<Object> AtomExec(Handle<JSRegExp> regexp,
                                 Handle<String> subject,
                                 Handle<Object> index);

  static Handle<Object> AtomExecGlobal(Handle<JSRegExp> regexp,
                                       Handle<String> subject);

  static Handle<Object> JscreCompile(Handle<JSRegExp> re,
                                     Handle<String> pattern,
                                     JSRegExp::Flags flags);

  // Execute a compiled JSCRE pattern.
  static Handle<Object> JscreExec(Handle<JSRegExp> regexp,
                                  Handle<String> subject,
                                  Handle<Object> index);

  // Execute a Regexp2000 bytecode pattern.
  static Handle<Object> Re2kExec(Handle<JSRegExp> regexp,
                                 Handle<String> subject,
                                 Handle<Object> index);

  static Handle<Object> JscreExecGlobal(Handle<JSRegExp> regexp,
                                        Handle<String> subject);

  static Handle<Object> Re2kExecGlobal(Handle<JSRegExp> regexp,
                                       Handle<String> subject);

  static void NewSpaceCollectionPrologue();
  static void OldSpaceCollectionPrologue();

  // Converts a source string to a 16 bit flat string.  The string
  // will be either sequential or it will be a SlicedString backed
  // by a flat string.
  static Handle<String> StringToTwoByte(Handle<String> pattern);
  static Handle<String> CachedStringToTwoByte(Handle<String> pattern);

  static const int kRe2kImplementationIndex = 0;
  static const int kRe2kNumberOfCapturesIndex = 1;
  static const int kRe2kNumberOfRegistersIndex = 2;
  static const int kRe2kCodeIndex = 3;
  static const int kRe2kDataLength = 4;

  static const int kJscreNumberOfCapturesIndex = 0;
  static const int kJscreInternalIndex = 1;
  static const int kJscreDataLength = 2;

 private:
  static String* last_ascii_string_;
  static String* two_byte_cached_string_;

  static int JscreNumberOfCaptures(Handle<JSRegExp> re);
  static ByteArray* JscreInternal(Handle<JSRegExp> re);

  static int Re2kNumberOfCaptures(Handle<JSRegExp> re);
  static int Re2kNumberOfRegisters(Handle<JSRegExp> re);
  static Handle<ByteArray> Re2kCode(Handle<JSRegExp> re);

  // Call jsRegExpExecute once
  static Handle<Object> JscreExecOnce(Handle<JSRegExp> regexp,
                                      int num_captures,
                                      Handle<String> subject,
                                      int previous_index,
                                      const uc16* utf8_subject,
                                      int* ovector,
                                      int ovector_length);

  static Handle<Object> Re2kExecOnce(Handle<JSRegExp> regexp,
                                     int num_captures,
                                     Handle<String> subject,
                                     int previous_index,
                                     const uc16* utf8_subject,
                                     int* ovector,
                                     int ovector_length);

  // Set the subject cache.  The previous string buffer is not deleted, so the
  // caller should ensure that it doesn't leak.
  static void SetSubjectCache(String* subject,
                              char* utf8_subject,
                              int uft8_length,
                              int character_position,
                              int utf8_position);

  // A one element cache of the last utf8_subject string and its length.  The
  // subject JS String object is cached in the heap.  We also cache a
  // translation between position and utf8 position.
  static char* utf8_subject_cache_;
  static int utf8_length_cache_;
  static int utf8_position_;
  static int character_position_;
};


class CharacterRange {
 public:
  // For compatibility with the CHECK_OK macro
  CharacterRange(void* null) { ASSERT_EQ(NULL, null); }  //NOLINT
  CharacterRange(uc16 from, uc16 to)
    : from_(from),
      to_(to) {
  }
  static void AddClassEscape(uc16 type, ZoneList<CharacterRange>* ranges);
  static inline CharacterRange Singleton(uc16 value) {
    return CharacterRange(value, value);
  }
  static inline CharacterRange Range(uc16 from, uc16 to) {
    ASSERT(from <= to);
    return CharacterRange(from, to);
  }
  bool Contains(uc16 i) { return from_ <= i && i <= to_; }
  uc16 from() const { return from_; }
  void set_from(uc16 value) { from_ = value; }
  uc16 to() const { return to_; }
  void set_to(uc16 value) { to_ = value; }
  bool is_valid() { return from_ <= to_; }
  bool IsSingleton() { return (from_ == to_); }
 private:
  uc16 from_;
  uc16 to_;
};


template <typename Node, class Callback>
static void DoForEach(Node* node, Callback callback);


// A zone splay tree.  The config type parameter encapsulates the
// different configurations of a concrete splay tree:
//
//   typedef Key: the key type
//   typedef Value: the value type
//   static const kNoKey: the dummy key used when no key is set
//   static const kNoValue: the dummy value used to initialize nodes
//   int (Compare)(Key& a, Key& b) -> {-1, 0, 1}: comparison function
//
template <typename Config>
class ZoneSplayTree : public ZoneObject {
 public:
  typedef typename Config::Key Key;
  typedef typename Config::Value Value;

  class Locator;

  ZoneSplayTree() : root_(NULL) { }

  // Inserts the given key in this tree with the given value.  Returns
  // true if a node was inserted, otherwise false.  If found the locator
  // is enabled and provides access to the mapping for the key.
  bool Insert(const Key& key, Locator* locator);

  // Looks up the key in this tree and returns true if it was found,
  // otherwise false.  If the node is found the locator is enabled and
  // provides access to the mapping for the key.
  bool Find(const Key& key, Locator* locator);

  // Finds the mapping with the greatest key less than or equal to the
  // given key.
  bool FindGreatestLessThan(const Key& key, Locator* locator);

  // Find the mapping with the greatest key in this tree.
  bool FindGreatest(Locator* locator);

  // Finds the mapping with the least key greater than or equal to the
  // given key.
  bool FindLeastGreaterThan(const Key& key, Locator* locator);

  // Find the mapping with the least key in this tree.
  bool FindLeast(Locator* locator);

  // Remove the node with the given key from the tree.
  bool Remove(const Key& key);

  bool is_empty() { return root_ == NULL; }

  // Perform the splay operation for the given key. Moves the node with
  // the given key to the top of the tree.  If no node has the given
  // key, the last node on the search path is moved to the top of the
  // tree.
  void Splay(const Key& key);

  class Node : public ZoneObject {
   public:
    Node(const Key& key, const Value& value)
      : key_(key),
        value_(value),
        left_(NULL),
        right_(NULL) { }
     Key key() { return key_; }
     Value value() { return value_; }
     Node* left() { return left_; }
     Node* right() { return right_; }
   private:
    friend class ZoneSplayTree;
    friend class Locator;
    Key key_;
    Value value_;
    Node* left_;
    Node* right_;
  };

  // A locator provides access to a node in the tree without actually
  // exposing the node.
  class Locator {
   public:
    explicit Locator(Node* node) : node_(node) { }
    Locator() : node_(NULL) { }
    const Key& key() { return node_->key_; }
    Value& value() { return node_->value_; }
    void set_value(const Value& value) { node_->value_ = value; }
    inline void bind(Node* node) { node_ = node; }
   private:
    Node* node_;
  };

  template <class Callback>
  void ForEach(Callback c) {
    DoForEach<typename ZoneSplayTree<Config>::Node, Callback>(root_, c);
  }

 private:
  Node* root_;
};


// A set of unsigned integers that behaves especially well on small
// integers (< 32).  May do zone-allocation.
class OutSet: public ZoneObject {
 public:
  OutSet() : first_(0), remaining_(NULL), successors_(NULL) { }
  OutSet* Extend(unsigned value);
  bool Get(unsigned value);
  static const unsigned kFirstLimit = 32;
 private:

  // Destructively set a value in this set.  In most cases you want
  // to use Extend instead to ensure that only one instance exists
  // that contains the same values.
  void Set(unsigned value);

  // The successors are a list of sets that contain the same values
  // as this set and the one more value that is not present in this
  // set.
  ZoneList<OutSet*>* successors() { return successors_; }

  OutSet(uint32_t first, ZoneList<unsigned>* remaining)
    : first_(first), remaining_(remaining), successors_(NULL) { }
  uint32_t first_;
  ZoneList<unsigned>* remaining_;
  ZoneList<OutSet*>* successors_;
};


// A mapping from integers, specified as ranges, to a set of integers.
// Used for mapping character ranges to choices.
class DispatchTable {
 public:
  class Entry {
   public:
    Entry()
      : from_(0), to_(0), out_set_(NULL) { }
    Entry(uc16 from, uc16 to, OutSet* out_set)
      : from_(from), to_(to), out_set_(out_set) { }
    uc16 from() { return from_; }
    uc16 to() { return to_; }
    void set_to(uc16 value) { to_ = value; }
    void AddValue(int value) { out_set_ = out_set_->Extend(value); }
    OutSet* out_set() { return out_set_; }
   private:
    uc16 from_;
    uc16 to_;
    OutSet* out_set_;
  };

  class Config {
   public:
    typedef uc16 Key;
    typedef Entry Value;
    static const uc16 kNoKey;
    static const Entry kNoValue;
    static inline int Compare(uc16 a, uc16 b) {
      if (a == b)
        return 0;
      else if (a < b)
        return -1;
      else
        return 1;
    }
  };

  void AddRange(CharacterRange range, int value);
  OutSet* Get(uc16 value);
  void Dump();

  template <typename Callback>
  void ForEach(Callback callback) { return tree()->ForEach(callback); }
 private:
  // There can't be a static empty set since it allocates its
  // successors in a zone and caches them.
  OutSet* empty() { return &empty_; }
  OutSet empty_;
  ZoneSplayTree<Config>* tree() { return &tree_; }
  ZoneSplayTree<Config> tree_;
};


#define FOR_EACH_NODE_TYPE(VISIT)                                    \
  VISIT(End)                                                         \
  VISIT(Atom)                                                        \
  VISIT(Action)                                                      \
  VISIT(Choice)                                                      \
  VISIT(Backreference)                                               \
  VISIT(CharacterClass)


class RegExpNode: public ZoneObject {
 public:
  virtual ~RegExpNode() { }
  virtual void Accept(NodeVisitor* visitor) = 0;
  // Generates a goto to this node or actually generates the code at this point.
  // Until the implementation is complete we will return true for success and
  // false for failure.
  bool GoTo(RegExpCompiler* compiler);
  void EmitAddress(RegExpCompiler* compiler);
  // Until the implementation is complete we will return true for success and
  // false for failure.
  virtual bool Emit(RegExpCompiler* compiler) = 0;
 private:
  Label label;
};


class SeqRegExpNode: public RegExpNode {
 public:
  explicit SeqRegExpNode(RegExpNode* on_success)
    : on_success_(on_success) { }
  RegExpNode* on_success() { return on_success_; }
  virtual bool Emit(RegExpCompiler* compiler) { return false; }
 private:
  RegExpNode* on_success_;
};


class ActionNode: public SeqRegExpNode {
 public:
  enum Type {
    STORE_REGISTER,
    INCREMENT_REGISTER,
    STORE_POSITION,
    RESTORE_POSITION,
    BEGIN_SUBMATCH,
    ESCAPE_SUBMATCH,
    END_SUBMATCH
  };
  static ActionNode* StoreRegister(int reg, int val, RegExpNode* on_success);
  static ActionNode* IncrementRegister(int reg, RegExpNode* on_success);
  static ActionNode* StorePosition(int reg, RegExpNode* on_success);
  static ActionNode* RestorePosition(int reg, RegExpNode* on_success);
  static ActionNode* BeginSubmatch(RegExpNode* on_success);
  static ActionNode* EscapeSubmatch(RegExpNode* on_success);
  static ActionNode* EndSubmatch(RegExpNode* on_success);
  virtual void Accept(NodeVisitor* visitor);
  virtual bool Emit(RegExpCompiler* compiler);
 private:
  union {
    struct {
      int reg;
      int value;
    } u_store_register;
    struct {
      int reg;
    } u_increment_register;
    struct {
      int reg;
    } u_position_register;
  } data_;
  ActionNode(Type type, RegExpNode* on_success)
    : SeqRegExpNode(on_success),
      type_(type) { }
  Type type_;
  friend class DotPrinter;
};


class AtomNode: public SeqRegExpNode {
 public:
  AtomNode(Vector<const uc16> data,
           RegExpNode* on_success,
           RegExpNode* on_failure)
    : SeqRegExpNode(on_success),
      on_failure_(on_failure),
      data_(data) { }
  virtual void Accept(NodeVisitor* visitor);
  Vector<const uc16> data() { return data_; }
  RegExpNode* on_failure() { return on_failure_; }
  virtual bool Emit(RegExpCompiler* compiler) { return false; }
 private:
  RegExpNode* on_failure_;
  Vector<const uc16> data_;
};


class BackreferenceNode: public SeqRegExpNode {
 public:
  BackreferenceNode(int start_reg,
                    int end_reg,
                    RegExpNode* on_success,
                    RegExpNode* on_failure)
    : SeqRegExpNode(on_success),
      on_failure_(on_failure),
      start_reg_(start_reg),
      end_reg_(end_reg) { }
  virtual void Accept(NodeVisitor* visitor);
  RegExpNode* on_failure() { return on_failure_; }
  int start_register() { return start_reg_; }
  int end_register() { return end_reg_; }
  virtual bool Emit(RegExpCompiler* compiler) { return false; }
 private:
  RegExpNode* on_failure_;
  int start_reg_;
  int end_reg_;
};


class CharacterClassNode: public SeqRegExpNode {
 public:
  CharacterClassNode(ZoneList<CharacterRange>* ranges,
                     bool is_negated,
                     RegExpNode* on_success,
                     RegExpNode* on_failure)
    : SeqRegExpNode(on_success),
      on_failure_(on_failure),
      ranges_(ranges),
      is_negated_(is_negated ) { }
  virtual void Accept(NodeVisitor* visitor);
  ZoneList<CharacterRange>* ranges() { return ranges_; }
  bool is_negated() { return is_negated_; }
  RegExpNode* on_failure() { return on_failure_; }
  virtual bool Emit(RegExpCompiler* compiler) { return false; }
  static void AddInverseToTable(ZoneList<CharacterRange>* ranges,
                                DispatchTable* table,
                                int index);
 private:
  RegExpNode* on_failure_;
  ZoneList<CharacterRange>* ranges_;
  bool is_negated_;
};


class EndNode: public RegExpNode {
 public:
  enum Action { ACCEPT, BACKTRACK };
  virtual void Accept(NodeVisitor* visitor);
  static EndNode* GetAccept() { return &kAccept; }
  static EndNode* GetBacktrack() { return &kBacktrack; }
  virtual bool Emit(RegExpCompiler* compiler);
 private:
  explicit EndNode(Action action) : action_(action) { }
  Action action_;
  static EndNode kAccept;
  static EndNode kBacktrack;
};


class Guard: public ZoneObject {
 public:
  enum Relation { LT, GEQ };
  Guard(int reg, Relation op, int value)
    : reg_(reg),
      op_(op),
      value_(value) { }
  int reg() { return reg_; }
  Relation op() { return op_; }
  int value() { return value_; }
 private:
  int reg_;
  Relation op_;
  int value_;
};


class GuardedAlternative {
 public:
  explicit GuardedAlternative(RegExpNode* node) : node_(node), guards_(NULL) { }
  void AddGuard(Guard* guard);
  RegExpNode* node() { return node_; }
  ZoneList<Guard*>* guards() { return guards_; }
 private:
  RegExpNode* node_;
  ZoneList<Guard*>* guards_;
};


class ChoiceNode: public RegExpNode {
 public:
  explicit ChoiceNode(int expected_size, RegExpNode* on_failure)
    : on_failure_(on_failure),
      choices_(new ZoneList<GuardedAlternative>(expected_size)),
      visited_(false) { }
  virtual void Accept(NodeVisitor* visitor);
  void AddChild(GuardedAlternative node) { choices()->Add(node); }
  ZoneList<GuardedAlternative>* choices() { return choices_; }
  DispatchTable* table() { return &table_; }
  RegExpNode* on_failure() { return on_failure_; }
  virtual bool Emit(RegExpCompiler* compiler);
  bool visited() { return visited_; }
  void set_visited(bool value) { visited_ = value; }
 private:
  RegExpNode* on_failure_;
  ZoneList<GuardedAlternative>* choices_;
  DispatchTable table_;
  bool visited_;
};


struct RegExpParseResult {
  RegExpTree* tree;
  bool has_character_escapes;
  Handle<String> error;
  int capture_count;
};


class RegExpEngine: public AllStatic {
 public:
  static Handle<FixedArray> Compile(RegExpParseResult* input,
                                    RegExpNode** node_return,
                                    bool ignore_case);
  static void DotPrint(const char* label, RegExpNode* node);
};


class RegExpCompiler;


} }  // namespace v8::internal

#endif  // V8_JSREGEXP_H_
