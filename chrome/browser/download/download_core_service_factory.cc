// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/download/download_core_service_factory.h"

#include "chrome/browser/download/download_core_service_impl.h"
#include "chrome/browser/history/history_service_factory.h"
#include "chrome/browser/profiles/incognito_helpers.h"
#include "chrome/browser/profiles/profile.h"
#include "components/keyed_service/content/browser_context_dependency_manager.h"

// static
DownloadCoreService* DownloadCoreServiceFactory::GetForBrowserContext(
    content::BrowserContext* context) {
  return static_cast<DownloadCoreService*>(
      GetInstance()->GetServiceForBrowserContext(context, true));
}

// static
DownloadCoreServiceFactory* DownloadCoreServiceFactory::GetInstance() {
  return base::Singleton<DownloadCoreServiceFactory>::get();
}

DownloadCoreServiceFactory::DownloadCoreServiceFactory()
    : BrowserContextKeyedServiceFactory(
          "DownloadCoreService",
          BrowserContextDependencyManager::GetInstance()) {
  DependsOn(HistoryServiceFactory::GetInstance());
}

DownloadCoreServiceFactory::~DownloadCoreServiceFactory() {}

KeyedService* DownloadCoreServiceFactory::BuildServiceInstanceFor(
    content::BrowserContext* profile) const {
  DownloadCoreService* service =
      new DownloadCoreServiceImpl(static_cast<Profile*>(profile));

  // No need for initialization; initialization can be done on first
  // use of service.

  return service;
}

content::BrowserContext* DownloadCoreServiceFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  return chrome::GetBrowserContextOwnInstanceInIncognito(context);
}
