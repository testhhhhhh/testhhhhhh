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


#ifndef V8_BYTECODES_RE2K_H_
#define V8_BYTECODES_RE2K_H_

namespace v8 { namespace internal {

#define BYTECODE_ITERATOR(V)    \
V(BREAK,                   0, 1) /* break offset32                          */ \
V(PUSH_CP,                 1, 5) /* push_cp offset32                        */ \
V(PUSH_BT,                 2, 5) /* push_bt address32                       */ \
V(PUSH_CAPTURE,            3, 2) /* push_capture capture_index              */ \
V(SET_CAPTURE,             4, 6) /* set_capture capture_index offset32      */ \
V(POP_CP,                  5, 1) /* pop_cp                                  */ \
V(POP_BT,                  6, 1) /* pop_bt                                  */ \
V(POP_CAPTURE,             7, 2) /* pop_capture capture_index               */ \
V(FAIL,                    8, 1) /* fail                                    */ \
V(FAIL_IF_WITHIN,          9, 5) /* fail distance32                         */ \
V(SUCCEED,                10, 1) /* succeed                                 */ \
V(ADVANCE_CP,             11, 5) /* advance_cp offset32                     */ \
V(GOTO,                   12, 5) /* goto address32                          */ \
V(LOAD_CURRENT_CHAR,      13, 5) /* load offset32                           */ \
V(CHECK_CHAR,             14, 7) /* check_char uc16 address32               */ \
V(CHECK_NOT_CHAR,         15, 7) /* check_not_char uc16 address32           */ \
V(CHECK_RANGE,            16, 9) /* check_range uc16 uc16 address32         */ \
V(CHECK_NOT_RANGE,        17, 9) /* check_not_range uc16 uc16 address32     */ \
V(CHECK_BACKREF,          18, 9) /* check_backref offset32 capture_idx ad...*/ \
V(CHECK_NOT_BACKREF,      19, 9) /* check_not_backref offset32 capture_id...*/ \
V(CHECK_BITMAP,           20,13) /* check_bitmap uc16 uc16 addr32           */ \
V(CHECK_NOT_BITMAP,       21,13) /* check_not_bitmap uc16 uc16 addr32       */

#define DECLARE_BYTECODES(name, code, length) \
  static const int BC_##name = code;
BYTECODE_ITERATOR(DECLARE_BYTECODES)
#undef DECLARE_BYTECODES


} }

#endif  // V8_BYTECODES_IA32_H_
