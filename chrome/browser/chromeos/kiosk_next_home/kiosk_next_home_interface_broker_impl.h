// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_KIOSK_NEXT_HOME_KIOSK_NEXT_HOME_INTERFACE_BROKER_IMPL_H_
#define CHROME_BROWSER_CHROMEOS_KIOSK_NEXT_HOME_KIOSK_NEXT_HOME_INTERFACE_BROKER_IMPL_H_

#include <memory>

#include "base/macros.h"
#include "base/supports_user_data.h"
#include "chrome/browser/chromeos/kiosk_next_home/mojom/kiosk_next_home_interface_broker.mojom.h"
#include "mojo/public/cpp/bindings/binding_set.h"
#include "services/identity/public/mojom/identity_accessor.mojom.h"

namespace service_manager {
class Connector;
}

namespace chromeos {
namespace kiosk_next_home {

// Implementation of interface responsible for brokering other interfaces needed
// to support Kiosk Next Home functionality.
class KioskNextHomeInterfaceBrokerImpl
    : public mojom::KioskNextHomeInterfaceBroker,
      public base::SupportsUserData::Data {
 public:
  explicit KioskNextHomeInterfaceBrokerImpl(
      service_manager::Connector* connector);
  ~KioskNextHomeInterfaceBrokerImpl() override;

  // Binds client requests to this implementation.
  void BindRequest(mojom::KioskNextHomeInterfaceBrokerRequest request);

  // mojom::KioskNextHomeInterfaceBroker:
  void GetIdentityAccessor(
      ::identity::mojom::IdentityAccessorRequest request) override;

 private:
  mojo::BindingSet<mojom::KioskNextHomeInterfaceBroker> bindings_;
  // Clone of BrowserContext's Connector, which allows binding to other
  // services.
  std::unique_ptr<service_manager::Connector> connector_;

  DISALLOW_COPY_AND_ASSIGN(KioskNextHomeInterfaceBrokerImpl);
};

}  // namespace kiosk_next_home
}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_KIOSK_NEXT_HOME_KIOSK_NEXT_HOME_INTERFACE_BROKER_IMPL_H_
