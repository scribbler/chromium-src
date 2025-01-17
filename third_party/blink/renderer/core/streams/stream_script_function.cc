// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/streams/stream_script_function.h"

#include "third_party/blink/renderer/platform/wtf/assertions.h"

namespace blink {

StreamScriptFunction::StreamScriptFunction(ScriptState* script_state)
    : ScriptFunction(script_state) {}

void StreamScriptFunction::CallRaw(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  DCHECK_GE(args.Length(), 1);
  CallWithLocal(args[0]);
}

v8::Local<v8::Promise> StreamThenPromise(v8::Local<v8::Context> context,
                                         v8::Local<v8::Promise> promise,
                                         StreamScriptFunction* on_fulfilled,
                                         StreamScriptFunction* on_rejected) {
  v8::MaybeLocal<v8::Promise> result_maybe;
  if (!on_fulfilled) {
    DCHECK(on_rejected);
    result_maybe = promise->Catch(context, on_rejected->BindToV8Function());
  } else if (on_rejected) {
    result_maybe = promise->Then(context, on_fulfilled->BindToV8Function(),
                                 on_rejected->BindToV8Function());
  } else {
    result_maybe = promise->Then(context, on_fulfilled->BindToV8Function());
  }

  v8::Local<v8::Promise> result;
  if (!result_maybe.ToLocal(&result)) {
    DVLOG(3)
        << "assuming that failure of promise->Then() is caused by shutdown and"
           "ignoring it";
    // Try to create a dummy promise so that the calling code can continue. If
    // we can't create one, then we can't return to the calling context so we
    // have to crash. This shouldn't happen except on OOM.
    result = v8::Promise::Resolver::New(context).ToLocalChecked()->GetPromise();
  }
  return result;
}

}  // namespace blink
