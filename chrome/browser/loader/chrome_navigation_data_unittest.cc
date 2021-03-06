// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/loader/chrome_navigation_data.h"

#include <memory>

#include "base/memory/ptr_util.h"
#include "components/data_reduction_proxy/core/browser/data_reduction_proxy_data.h"
#include "components/previews/core/previews_user_data.h"
#include "content/public/browser/navigation_data.h"
#include "testing/gtest/include/gtest/gtest.h"

class ChromeNavigationDataTest : public testing::Test {
 public:
  ChromeNavigationDataTest() {}
  ~ChromeNavigationDataTest() override {}
};

TEST_F(ChromeNavigationDataTest, AddingDataReductionProxyData) {
  std::unique_ptr<ChromeNavigationData> data(new ChromeNavigationData());
  data_reduction_proxy::DataReductionProxyData* data_reduction_proxy_data =
      new data_reduction_proxy::DataReductionProxyData();
  data->SetDataReductionProxyData(base::WrapUnique(data_reduction_proxy_data));
  EXPECT_EQ(data_reduction_proxy_data, data->GetDataReductionProxyData());
}

TEST_F(ChromeNavigationDataTest, AddingPreviewsUserData) {
  ChromeNavigationData data;
  EXPECT_FALSE(data.previews_user_data());
  data.set_previews_user_data(std::make_unique<previews::PreviewsUserData>(1u));
  EXPECT_TRUE(data.previews_user_data());
}

TEST_F(ChromeNavigationDataTest, Clone) {
  ChromeNavigationData data;
  EXPECT_FALSE(data.GetDataReductionProxyData());
  data.SetDataReductionProxyData(
      std::make_unique<data_reduction_proxy::DataReductionProxyData>());
  EXPECT_FALSE(data.previews_user_data());
  data.set_previews_user_data(std::make_unique<previews::PreviewsUserData>(1u));

  std::unique_ptr<content::NavigationData> clone_data = data.Clone();
  ChromeNavigationData* clone_chrome_data =
      static_cast<ChromeNavigationData*>(clone_data.get());
  EXPECT_NE(&data, clone_data.get());
  EXPECT_NE(&data, clone_chrome_data);
  EXPECT_NE(data.GetDataReductionProxyData(),
            clone_chrome_data->GetDataReductionProxyData());
  EXPECT_NE(data.previews_user_data(), clone_chrome_data->previews_user_data());
  EXPECT_TRUE(data.GetDataReductionProxyData());
  EXPECT_TRUE(data.previews_user_data());
  EXPECT_TRUE(clone_chrome_data->GetDataReductionProxyData());
  EXPECT_TRUE(clone_chrome_data->previews_user_data());
}
