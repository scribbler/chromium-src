// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/webui/web_ui_ios_controller_factory_registry.h"

#include <stddef.h>
#include <memory>

#include "base/no_destructor.h"
#include "ios/web/public/webui/web_ui_ios_controller.h"
#include "url/gurl.h"
#include "url/url_constants.h"

namespace web {
namespace {
// Returns the global list of registered factories.
std::vector<WebUIIOSControllerFactory*>& GetGlobalFactories() {
  static base::NoDestructor<std::vector<WebUIIOSControllerFactory*>> factories;
  return *factories;
}
}  // namespace

void WebUIIOSControllerFactory::RegisterFactory(
    WebUIIOSControllerFactory* factory) {
  GetGlobalFactories().push_back(factory);
}

WebUIIOSControllerFactoryRegistry*
WebUIIOSControllerFactoryRegistry::GetInstance() {
  static base::NoDestructor<WebUIIOSControllerFactoryRegistry> instance;
  return instance.get();
}

bool WebUIIOSControllerFactoryRegistry::HasWebUIIOSControllerForURL(
    const GURL& url) const {
  for (WebUIIOSControllerFactory* factory : GetGlobalFactories()) {
    if (factory->HasWebUIIOSControllerForURL(url))
      return true;
  }
  return false;
}

std::unique_ptr<WebUIIOSController>
WebUIIOSControllerFactoryRegistry::CreateWebUIIOSControllerForURL(
    WebUIIOS* web_ui,
    const GURL& url) const {
  for (WebUIIOSControllerFactory* factory : GetGlobalFactories()) {
    auto controller = factory->CreateWebUIIOSControllerForURL(web_ui, url);
    if (controller)
      return controller;
  }
  return nullptr;
}

WebUIIOSControllerFactoryRegistry::WebUIIOSControllerFactoryRegistry() {
}

WebUIIOSControllerFactoryRegistry::~WebUIIOSControllerFactoryRegistry() {
}

}  // namespace web
