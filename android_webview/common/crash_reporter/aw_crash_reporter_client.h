// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ANDROID_WEBVIEW_COMMON_CRASH_REPORTER_AW_CRASH_REPORTER_CLIENT_H_
#define ANDROID_WEBVIEW_COMMON_CRASH_REPORTER_AW_CRASH_REPORTER_CLIENT_H_

#include <string>

namespace android_webview {

void EnableCrashReporter(const std::string& process_type);

}  // namespace android_webview

#endif  // ANDROID_WEBVIEW_COMMON_CRASH_REPORTER_AW_CRASH_REPORTER_CLIENT_H_
