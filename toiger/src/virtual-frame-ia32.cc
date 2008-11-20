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

#include "v8.h"

#include "codegen.h"
#include "codegen-inl.h"
#include "virtual-frame.h"

namespace v8 { namespace internal {

#define __ masm_->

// -------------------------------------------------------------------------
// VirtualFrame implementation.

// On entry to a function, the virtual frame already contains the receiver,
// the parameters, and a return address.  All frame elements are in memory.
VirtualFrame::VirtualFrame(CodeGenerator* cgen)
    : masm_(cgen->masm()),
      elements_(0),
      parameter_count_(cgen->scope()->num_parameters()),
      local_count_(0),
      stack_pointer_(parameter_count_ + 1),  // 0-based index of TOS.
      frame_pointer_(kIllegalIndex) {
  for (int i = 0; i < parameter_count_ + 2; i++) {
    elements_.Add(FrameElement());
  }
}


// When cloned, a frame is a deep copy of the original.
VirtualFrame::VirtualFrame(VirtualFrame* original)
    : masm_(original->masm_),
      elements_(original->elements_.length()),
      parameter_count_(original->parameter_count_),
      local_count_(original->local_count_),
      stack_pointer_(original->stack_pointer_),
      frame_pointer_(original->frame_pointer_) {
  // Copy all the elements from the original.
  for (int i = 0; i < original->elements_.length(); i++) {
    elements_.Add(original->elements_[i]);
  }
}


// Modify the state of the virtual frame to match the actual frame by adding
// extra in-memory elements to the top of the virtual frame.  The extra
// elements will be externally materialized on the actual frame (eg, by
// pushing an exception handler).  No code is emitted.
void VirtualFrame::Adjust(int count) {
  ASSERT(count >= 0);
  ASSERT(stack_pointer_ == elements_.length() - 1);

  for (int i = 0; i < count; i++) {
    elements_.Add(FrameElement());
  }
  stack_pointer_ += count;
}


// Modify the state of the virtual frame to match the actual frame by
// removing elements from the top of the virtual frame.  The elements will
// be externally popped from the actual frame (eg, by a runtime call).  No
// code is emitted.
void VirtualFrame::Forget(int count) {
  ASSERT(count >= 0);
  ASSERT(stack_pointer_ == elements_.length() - 1);
  ASSERT(elements_.length() >= count);

  stack_pointer_ -= count;
  for (int i = 0; i < count; i++) {
    elements_.RemoveLast();
  }
}


// Clear the dirty bit for the element at a given index.  We can only
// allocate space in the actual frame for the virtual element immediately
// above the stack pointer.
void VirtualFrame::SyncElementAt(int index) {
  FrameElement element = elements_[index];

  if (element.is_dirty()) {
    if (index <= stack_pointer_) {
      // Write elements below the stack pointer to their (already allocated)
      // actual frame location.
      if (element.is_constant()) {
        __ Set(Operand(ebp, fp_relative(index)), Immediate(element.handle()));
      } else {
        ASSERT(element.is_register());
        __ mov(Operand(ebp, fp_relative(index)), element.reg());
      }
    } else {
      // Push elements above the stack pointer to allocate space and sync
      // them.  Space should have already been allocated in the actual frame
      // for all the elements below this one.
      ASSERT(index == stack_pointer_ + 1);
      stack_pointer_++;
      if (element.is_constant()) {
        __ push(Immediate(element.handle()));
      } else {
        ASSERT(element.is_register());
        __ push(element.reg());
      }
    }
  }
}


// Make the type of the element at a given index be MEMORY.  We can only
// allocate space in the actual frame for the virtual element immediately
// above the stack pointer.
void VirtualFrame::SpillElementAt(int index) {
  SyncElementAt(index);
  // The element is now in memory.
  elements_[index] = FrameElement();
}


// Make the type of all elements be MEMORY.
void VirtualFrame::SpillAll() {
  for (int i = 0; i < elements_.length(); i++) {
    SpillElementAt(i);
  }
}


void VirtualFrame::PrepareForCall(int frame_arg_count) {
  ASSERT(height() >= frame_arg_count);

  // Below the stack pointer, spill all registers.
  for (int i = 0; i <= stack_pointer_; i++) {
    if (elements_[i].is_register()) {
      SpillElementAt(i);
    }
  }

  // Above the stack pointer, spill registers and sync everything else (ie,
  // constants).
  for (int i = stack_pointer_ + 1; i < elements_.length(); i++) {
    if (elements_[i].is_register()) {
      SpillElementAt(i);
    } else {
      SyncElementAt(i);
    }
  }

  // Forget the frame elements that will be popped by the call.
  Forget(frame_arg_count);
}


void VirtualFrame::EnsureMergable() {
  // We cannot merge to a frame that has constants as elements, because an
  // arbitrary frame might not have constants in those locations.
  //
  // We cannot merge to a frame that has registers as elements because we
  // haven't implemented merging for such frames yet.
  SpillAll();
}


void VirtualFrame::MergeTo(VirtualFrame* expected) {
  ASSERT(masm_ == expected->masm_);
  ASSERT(elements_.length() == expected->elements_.length());
  ASSERT(parameter_count_ == expected->parameter_count_);
  ASSERT(local_count_ == expected->local_count_);
  ASSERT(frame_pointer_ == expected->frame_pointer_);

  // Mergable frames do not have constants and they do not (currently) have
  // registers.  They are always fully spilled, so the only thing needed to
  // make this frame match the expected one is to spill everything.
  //
  // TODO(): Implement a non-stupid way of merging frames.
  SpillAll();

  ASSERT(stack_pointer_ == expected->stack_pointer_);
}


void VirtualFrame::Enter() {
  Comment cmnt(masm_, "[ Enter JS frame");
  EmitPush(ebp);

  frame_pointer_ = stack_pointer_;
  __ mov(ebp, Operand(esp));

  // Store the context and the function in the frame.
  FrameElement context(esi);
  context.clear_dirty();
  elements_.Add(context);
  stack_pointer_++;
  __ push(esi);

  FrameElement function(edi);
  function.clear_dirty();
  elements_.Add(function);
  stack_pointer_++;
  __ push(edi);

  // Clear the function slot when generating debug code.
  if (FLAG_debug_code) {
    SpillElementAt(stack_pointer_);
    __ Set(edi, Immediate(reinterpret_cast<int>(kZapValue)));
  }
}


void VirtualFrame::Exit() {
  Comment cmnt(masm_, "[ Exit JS frame");
  // Record the location of the JS exit code for patching when setting
  // break point.
  __ RecordJSReturn();

  // Avoid using the leave instruction here, because it is too
  // short. We need the return sequence to be a least the size of a
  // call instruction to support patching the exit code in the
  // debugger. See VisitReturnStatement for the full return sequence.
  __ mov(esp, Operand(ebp));
  stack_pointer_ = frame_pointer_;
  for (int i = elements_.length() - 1; i > stack_pointer_; i--) {
    elements_.RemoveLast();
  }

  frame_pointer_ = kIllegalIndex;
  EmitPop(ebp);
}


void VirtualFrame::AllocateStackSlots(int count) {
  ASSERT(height() == 0);
  local_count_ = count;

  if (count > 0) {
    Comment cmnt(masm_, "[ Allocate space for locals");
    // The locals are constants (the undefined value), but we sync them with
    // the actual frame to allocate space for spilling them.
    FrameElement initial_value(Factory::undefined_value());
    initial_value.clear_dirty();
    __ Set(eax, Immediate(Factory::undefined_value()));
    for (int i = 0; i < count; i++) {
      elements_.Add(initial_value);
      stack_pointer_++;
      __ push(eax);
    }
  }
}


void VirtualFrame::PushTryHandler(HandlerType type) {
  // Grow the expression stack by handler size less two (the return address
  // is already pushed by a call instruction, and PushTryHandler from the
  // macro assembler will leave the top of stack in the eax register to be
  // pushed separately).
  Adjust(kHandlerSize - 2);
  __ PushTryHandler(IN_JAVASCRIPT, type);
  // TODO(1222589): remove the reliance of PushTryHandler on a cached TOS
  EmitPush(eax);
}


void VirtualFrame::CallStub(CodeStub* stub, int frame_arg_count) {
  PrepareForCall(frame_arg_count);
  __ CallStub(stub);
}


void VirtualFrame::CallRuntime(Runtime::Function* f, int frame_arg_count) {
  PrepareForCall(frame_arg_count);
  __ CallRuntime(f, frame_arg_count);
}


void VirtualFrame::CallRuntime(Runtime::FunctionId id, int frame_arg_count) {
  PrepareForCall(frame_arg_count);
  __ CallRuntime(id, frame_arg_count);
}


void VirtualFrame::InvokeBuiltin(Builtins::JavaScript id,
                                 InvokeFlag flag,
                                 int frame_arg_count) {
  PrepareForCall(frame_arg_count);
  __ InvokeBuiltin(id, flag);
}


void VirtualFrame::CallCodeObject(Handle<Code> code,
                                  RelocInfo::Mode rmode,
                                  int frame_arg_count) {
  PrepareForCall(frame_arg_count);
  __ call(code, rmode);
}


void VirtualFrame::Drop(int count) {
  ASSERT(height() >= count);

  // Discard elements above the stack pointer.
  while (count > 0 && stack_pointer_ < elements_.length() - 1) {
    elements_.RemoveLast();
  }

  // Discard the rest of the elements and lower the stack pointer.
  Forget(count);
  if (count > 0) {
    __ add(Operand(esp), Immediate(count * kPointerSize));
  }
}


void VirtualFrame::Drop() { Drop(1); }


void VirtualFrame::EmitPop(Register reg) {
  ASSERT(stack_pointer_ == elements_.length() - 1);
  stack_pointer_--;
  elements_.RemoveLast();
  __ pop(reg);
}


void VirtualFrame::EmitPop(Operand operand) {
  ASSERT(stack_pointer_ == elements_.length() - 1);
  stack_pointer_--;
  elements_.RemoveLast();
  __ pop(operand);
}


void VirtualFrame::EmitPush(Register reg) {
  ASSERT(stack_pointer_ == elements_.length() - 1);
  elements_.Add(FrameElement());
  stack_pointer_++;
  __ push(reg);
}


void VirtualFrame::EmitPush(Operand operand) {
  ASSERT(stack_pointer_ == elements_.length() - 1);
  elements_.Add(FrameElement());
  stack_pointer_++;
  __ push(operand);
}


void VirtualFrame::EmitPush(Immediate immediate) {
  ASSERT(stack_pointer_ == elements_.length() - 1);
  elements_.Add(FrameElement());
  stack_pointer_++;
  __ push(immediate);
}


#undef __

} }  // namespace v8::internal
