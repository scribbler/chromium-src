// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_CHROME_CLEANER_ENGINES_BROKER_CLEANER_ENGINE_REQUESTS_IMPL_H_
#define CHROME_CHROME_CLEANER_ENGINES_BROKER_CLEANER_ENGINE_REQUESTS_IMPL_H_

#include <memory>
#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "chrome/chrome_cleaner/constants/uws_id.h"
#include "chrome/chrome_cleaner/engines/broker/cleaner_sandbox_interface.h"
#include "chrome/chrome_cleaner/engines/broker/interface_metadata_observer.h"
#include "chrome/chrome_cleaner/interfaces/cleaner_engine_requests.mojom.h"
#include "chrome/chrome_cleaner/ipc/mojo_task_runner.h"
#include "chrome/chrome_cleaner/zip_archiver/sandboxed_zip_archiver.h"
#include "mojo/public/cpp/bindings/associated_binding.h"

namespace chrome_cleaner {

std::unique_ptr<chrome_cleaner::FileRemoverAPI>
CreateFileRemoverWithDigestVerifier(
    const std::vector<UwSId>& enabled_uws,
    std::unique_ptr<SandboxedZipArchiver> archiver,
    const base::RepeatingClosure& reboot_needed_callback);

class CleanerEngineRequestsImpl : public mojom::CleanerEngineRequests {
 public:
  CleanerEngineRequestsImpl(
      scoped_refptr<MojoTaskRunner> mojo_task_runner,
      InterfaceMetadataObserver* metadata_observer,
      std::unique_ptr<chrome_cleaner::FileRemoverAPI> file_remover);
  ~CleanerEngineRequestsImpl() override;

  void Bind(mojom::CleanerEngineRequestsAssociatedPtrInfo* ptr_info);

  // mojom::CleanerEngineRequests
  void SandboxDeleteFile(const base::FilePath& file_name,
                         SandboxDeleteFileCallback result_callback) override;
  void SandboxDeleteFilePostReboot(
      const base::FilePath& file_name,
      SandboxDeleteFilePostRebootCallback result_callback) override;
  void SandboxNtDeleteRegistryKey(
      const String16EmbeddedNulls& key,
      SandboxNtDeleteRegistryKeyCallback result_callback) override;
  void SandboxNtDeleteRegistryValue(
      const String16EmbeddedNulls& key,
      const String16EmbeddedNulls& value_name,
      SandboxNtDeleteRegistryValueCallback result_callback) override;
  void SandboxNtChangeRegistryValue(
      const String16EmbeddedNulls& key,
      const String16EmbeddedNulls& value_name,
      const String16EmbeddedNulls& new_value,
      SandboxNtChangeRegistryValueCallback result_callback) override;
  void SandboxDeleteService(
      const base::string16& name,
      SandboxDeleteServiceCallback result_callback) override;
  void SandboxDeleteTask(const base::string16& name,
                         SandboxDeleteServiceCallback result_callback) override;
  void SandboxTerminateProcess(
      uint32_t process_id,
      SandboxTerminateProcessCallback result_callback) override;

 private:
  bool NtDeleteRegistryKey(const String16EmbeddedNulls& key);
  bool NtDeleteRegistryValue(const String16EmbeddedNulls& key,
                             const String16EmbeddedNulls& value_name);
  bool NtChangeRegistryValue(const String16EmbeddedNulls& key,
                             const String16EmbeddedNulls& value_name,
                             const String16EmbeddedNulls& new_value);
  bool DeleteService(const base::string16& name);
  bool DeleteTask(const base::string16& name);
  bool TerminateProcess(uint32_t process_id);

  scoped_refptr<MojoTaskRunner> mojo_task_runner_;
  mojo::AssociatedBinding<mojom::CleanerEngineRequests> binding_;
  InterfaceMetadataObserver* metadata_observer_ = nullptr;
  std::unique_ptr<chrome_cleaner::FileRemoverAPI> file_remover_;
};

}  // namespace chrome_cleaner

#endif  // CHROME_CHROME_CLEANER_ENGINES_BROKER_CLEANER_ENGINE_REQUESTS_IMPL_H_
