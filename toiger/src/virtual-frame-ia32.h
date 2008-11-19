// Copyright 2008 the V8 project authors. All rights reserved.
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

#ifndef V8_VIRTUAL_FRAME_IA32_H_
#define V8_VIRTUAL_FRAME_IA32_H_

#include "macro-assembler.h"

namespace v8 { namespace internal {

// -------------------------------------------------------------------------
// Virtual frame elements
//
// The internal elements of the virtual frames.  Elements are (currently) of
// only one kind, in-memory.  Their actual location is given by their
// position in the virtual frame.

class FrameElement BASE_EMBEDDED {
 public:
  enum Type { MEMORY, CONSTANT, LAST_TYPE = CONSTANT };

  FrameElement() : type_(MEMORY) {}

  explicit FrameElement(Handle<Object> value) : type_(CONSTANT | kDirtyBit) {
    data_.handle_ = value.location();
  }

  Type type() const { return static_cast<Type>(type_ & kTypeMask); }

  bool is_dirty() const {
    STATIC_ASSERT(kDirtyBit > LAST_TYPE);
    return (type_ & kDirtyBit) != 0;
  }

  void set_dirty() {
    ASSERT(type() != MEMORY);
    type_ = type_ | kDirtyBit;
  }

  void clear_dirty() {
    ASSERT(type() != MEMORY);
    type_ = type_ & ~kDirtyBit;
  }

  Handle<Object> handle() const {
    ASSERT(type() == CONSTANT);
    return Handle<Object>(data_.handle_);
  }

 private:
  static const int kDirtyBit = 1 << 8;
  static const int kTypeMask = kDirtyBit - 1;

  // The element's type and a dirty bit.  The dirty bit can be cleared
  // for non-memory elements to indicate that the element agrees with
  // the value in memory in the actual frame.
  int type_;

  union {
    Object** handle_;
  } data_;
};


// -------------------------------------------------------------------------
// Virtual frames
//
// The virtual frame is an abstraction of the physical stack frame.  It
// encapsulates the parameters, frame-allocated locals, and the expression
// stack.  It supports push/pop operations on the expression stack, as well
// as random access to the expression stack elements, locals, and
// parameters.

class VirtualFrame : public Malloced {
 public:
  // Construct an initial virtual frame on entry to a JS function.
  explicit VirtualFrame(CodeGenerator* cgen);

  // Construct a virtual frame as a clone of an existing one.
  explicit VirtualFrame(VirtualFrame* original);

  // The height of the virtual expression stack.
  int height() const {
    return elements_.length() - expression_base_index();
  }

  // Add extra in-memory elements to the top of the frame to match an actual
  // frame (eg, the frame after an exception handler is pushed).  No code is
  // emitted.
  void Adjust(int count);

  // Forget elements from the top of the frame to match an actual frame (eg,
  // the frame after a runtime call).  No code is emitted.
  void Forget(int count);

  // Spill all values from the frame to memory.
  void SpillAll();

  // Ensure that this frame is in a state where an arbitrary frame of the
  // right size could be merged to it.  May emit code.
  void EnsureMergable();

  // Make this virtual frame have a state identical to an expected virtual
  // frame.  As a side effect, code may be emitted to make this frame match
  // the expected one.
  void MergeTo(VirtualFrame* expected);

  // Emit code for the physical JS entry and exit frame sequences.  After
  // calling Enter, the virtual frame is ready for use; and after calling
  // Exit it should not be used.  Note that Enter does not allocate space in
  // the physical frame for storing frame-allocated locals.
  void Enter();
  void Exit();

  // Allocate and initialize the frame-allocated locals.  The number of
  // locals is known from the frame's code generator's state (specifically
  // its scope).  As a side effect, code may be emitted.
  void AllocateStackSlots(int count);

  // The current top of the expression stack as an assembly operand.
  Operand Top() const { return Operand(esp, 0); }

  // An element of the expression stack as an assembly operand.
  Operand ElementAt(int index) const {
    return Operand(esp, index * kPointerSize);
  }

  // A frame-allocated local as an assembly operand.
  Operand LocalAt(int index) const {
    ASSERT(0 <= index);
    ASSERT(index < local_count_);
    return Operand(ebp, kLocal0Offset - index * kPointerSize);
  }

  // The function frame slot.
  Operand Function() const { return Operand(ebp, kFunctionOffset); }

  // The context frame slot.
  Operand Context() const { return Operand(ebp, kContextOffset); }

  // A parameter as an assembly operand.
  Operand ParameterAt(int index) const {
    ASSERT(-1 <= index);  // -1 is the receiver.
    ASSERT(index < parameter_count_);
    return Operand(ebp, (1 + parameter_count_ - index) * kPointerSize);
  }

  // The receiver frame slot.
  Operand Receiver() const { return ParameterAt(-1); }

  // Push a try-catch or try-finally handler on top of the virtual frame.
  void PushTryHandler(HandlerType type);

  // Call a code stub, given the number of arguments it expects on (and
  // removes from) the top of the physical frame.
  void CallStub(CodeStub* stub, int frame_arg_count);

  // Call the runtime, given the number of arguments expected on (and
  // removed from) the top of the physical frame.
  void CallRuntime(Runtime::Function* f, int frame_arg_count);
  void CallRuntime(Runtime::FunctionId id, int frame_arg_count);

  // Invoke a builtin, given the number of arguments it expects on (and
  // removes from) the top of the physical frame.
  void InvokeBuiltin(Builtins::JavaScript id,
                     InvokeFlag flag,
                     int frame_arg_count);

  // Call into a JS code object, given the number of arguments it expects on
  // (and removes from) the top of the physical frame.
  void CallCodeObject(Handle<Code> ic,
                      RelocInfo::Mode rmode,
                      int frame_arg_count);

  // Drop a number of elements from the top of the expression stack.  May
  // emit code to affect the physical frame.  Does not clobber any registers
  // excepting possibly the stack pointer.
  void Drop(int count);

  // Drop one element.
  void Drop();

  // Pop and save an element from the top of the expression stack and emit a
  // corresponding pop instruction.
  void EmitPop(Register reg);
  void EmitPop(Operand operand);

  // Push an element on top of the expression stack and emit a corresponding
  // push instruction.
  void EmitPush(Register reg);
  void EmitPush(Operand operand);
  void EmitPush(Immediate immediate);

 private:
  // An illegal index into the virtual frame.
  static const int kIllegalIndex = -1;

  static const int kLocal0Offset = JavaScriptFrameConstants::kLocal0Offset;
  static const int kFunctionOffset = JavaScriptFrameConstants::kFunctionOffset;
  static const int kContextOffset = StandardFrameConstants::kContextOffset;

  static const int kHandlerSize = StackHandlerConstants::kSize / kPointerSize;

  MacroAssembler* masm_;

  List<FrameElement> elements_;

  int parameter_count_;
  int local_count_;

  // The index of the element that is at the processor's stack pointer
  // (the esp register).
  int stack_pointer_;

  // The index of the element that is at the processor's frame pointer
  // (the ebp register).
  int frame_pointer_;

  // The index of the first parameter.  The receiver lies below the first
  // parameter.
  int param0_index() const { return 1; }

  // The index of the first local.  Between the parameters and the locals
  // lie the return address, the saved frame pointer, the context, and the
  // function.
  int local0_index() const { return param0_index() + parameter_count_ + 4; }

  // The index of the base of the expression stack.
  int expression_base_index() const { return local0_index() + local_count_; }

  // Convert a frame index into a frame pointer relative offset into the
  // actual stack.
  int fp_relative(int index) const {
    return (frame_pointer_ - index) * kPointerSize;
  }

  // Spill the topmost elements of the frame to memory (eg, they are the
  // arguments to a call) and all registers.
  void PrepareForCall(int count);
};


} }  // namespace v8::internal

#endif  // V8_VIRTUAL_FRAME_IA32_H_
