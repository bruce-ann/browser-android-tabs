// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/viz/service/frame_sinks/direct_layer_tree_frame_sink.h"

#include <memory>

#include "cc/test/fake_layer_tree_frame_sink_client.h"
#include "cc/test/fake_output_surface.h"
#include "cc/test/test_context_provider.h"
#include "cc/test/test_shared_bitmap_manager.h"
#include "components/viz/common/display/renderer_settings.h"
#include "components/viz/common/frame_sinks/begin_frame_source.h"
#include "components/viz/common/frame_sinks/delay_based_time_source.h"
#include "components/viz/common/surfaces/frame_sink_id.h"
#include "components/viz/common/surfaces/parent_local_surface_id_allocator.h"
#include "components/viz/service/display/display.h"
#include "components/viz/service/display/display_scheduler.h"
#include "components/viz/service/frame_sinks/compositor_frame_sink_support_manager.h"
#include "components/viz/service/frame_sinks/frame_sink_manager_impl.h"
#include "components/viz/test/begin_frame_args_test.h"
#include "components/viz/test/compositor_frame_helpers.h"
#include "components/viz/test/ordered_simple_task_runner.h"
#include "components/viz/test/test_gpu_memory_buffer_manager.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace viz {
namespace {

static constexpr FrameSinkId kArbitraryFrameSinkId(1, 1);

class TestDirectLayerTreeFrameSink : public DirectLayerTreeFrameSink {
 public:
  using DirectLayerTreeFrameSink::DirectLayerTreeFrameSink;
};

class TestCompositorFrameSinkSupportManager
    : public CompositorFrameSinkSupportManager {
 public:
  explicit TestCompositorFrameSinkSupportManager(
      FrameSinkManagerImpl* frame_sink_manager)
      : frame_sink_manager_(frame_sink_manager) {}
  ~TestCompositorFrameSinkSupportManager() override = default;

  std::unique_ptr<CompositorFrameSinkSupport> CreateCompositorFrameSinkSupport(
      mojom::CompositorFrameSinkClient* client,
      const FrameSinkId& frame_sink_id,
      bool is_root,
      bool needs_sync_points) override {
    return std::make_unique<CompositorFrameSinkSupport>(
        client, frame_sink_manager_, frame_sink_id, is_root, needs_sync_points);
  }

 private:
  FrameSinkManagerImpl* const frame_sink_manager_;

  DISALLOW_COPY_AND_ASSIGN(TestCompositorFrameSinkSupportManager);
};

class DirectLayerTreeFrameSinkTest : public testing::Test {
 public:
  DirectLayerTreeFrameSinkTest()
      : now_src_(new base::SimpleTestTickClock()),
        task_runner_(new cc::OrderedSimpleTaskRunner(now_src_.get(), true)),
        display_size_(1920, 1080),
        display_rect_(display_size_),
        support_manager_(&frame_sink_manager_),
        context_provider_(cc::TestContextProvider::Create()) {
    auto display_output_surface = cc::FakeOutputSurface::Create3d();
    display_output_surface_ = display_output_surface.get();

    begin_frame_source_ = std::make_unique<BackToBackBeginFrameSource>(
        std::make_unique<DelayBasedTimeSource>(task_runner_.get()));

    int max_frames_pending = 2;
    auto scheduler = std::make_unique<DisplayScheduler>(
        begin_frame_source_.get(), task_runner_.get(), max_frames_pending);

    display_ = std::make_unique<Display>(
        &bitmap_manager_, RendererSettings(), kArbitraryFrameSinkId,
        std::move(display_output_surface), std::move(scheduler), task_runner_);
    layer_tree_frame_sink_ = std::make_unique<TestDirectLayerTreeFrameSink>(
        kArbitraryFrameSinkId, &support_manager_, &frame_sink_manager_,
        display_.get(), nullptr /* display_client */, context_provider_,
        nullptr, task_runner_, &gpu_memory_buffer_manager_, &bitmap_manager_);
    layer_tree_frame_sink_->BindToClient(&layer_tree_frame_sink_client_);
    display_->Resize(display_size_);
    display_->SetVisible(true);

    EXPECT_FALSE(
        layer_tree_frame_sink_client_.did_lose_layer_tree_frame_sink_called());
  }

  ~DirectLayerTreeFrameSinkTest() override {
    layer_tree_frame_sink_->DetachFromClient();
  }

  void SwapBuffersWithDamage(const gfx::Rect& damage_rect) {
    auto frame = CompositorFrameBuilder()
                     .AddRenderPass(display_rect_, damage_rect)
                     .Build();
    layer_tree_frame_sink_->SubmitCompositorFrame(std::move(frame));
  }

  void SetUp() override {
    // Draw the first frame to start in an "unlocked" state.
    SwapBuffersWithDamage(display_rect_);

    EXPECT_EQ(0u, display_output_surface_->num_sent_frames());
    task_runner_->RunUntilIdle();
    EXPECT_EQ(1u, display_output_surface_->num_sent_frames());
  }

 protected:
  std::unique_ptr<base::SimpleTestTickClock> now_src_;
  scoped_refptr<cc::OrderedSimpleTaskRunner> task_runner_;

  const gfx::Size display_size_;
  const gfx::Rect display_rect_;
  FrameSinkManagerImpl frame_sink_manager_;
  TestCompositorFrameSinkSupportManager support_manager_;
  cc::TestSharedBitmapManager bitmap_manager_;
  TestGpuMemoryBufferManager gpu_memory_buffer_manager_;

  scoped_refptr<cc::TestContextProvider> context_provider_;
  cc::FakeOutputSurface* display_output_surface_ = nullptr;
  std::unique_ptr<BackToBackBeginFrameSource> begin_frame_source_;
  std::unique_ptr<Display> display_;
  cc::FakeLayerTreeFrameSinkClient layer_tree_frame_sink_client_;
  std::unique_ptr<TestDirectLayerTreeFrameSink> layer_tree_frame_sink_;
};

TEST_F(DirectLayerTreeFrameSinkTest, DamageTriggersSwapBuffers) {
  SwapBuffersWithDamage(display_rect_);
  EXPECT_EQ(1u, display_output_surface_->num_sent_frames());
  task_runner_->RunUntilIdle();
  EXPECT_EQ(2u, display_output_surface_->num_sent_frames());
}

TEST_F(DirectLayerTreeFrameSinkTest, NoDamageDoesNotTriggerSwapBuffers) {
  SwapBuffersWithDamage(gfx::Rect());
  EXPECT_EQ(1u, display_output_surface_->num_sent_frames());
  task_runner_->RunUntilIdle();
  EXPECT_EQ(1u, display_output_surface_->num_sent_frames());
}

TEST_F(DirectLayerTreeFrameSinkTest, SuspendedDoesNotTriggerSwapBuffers) {
  SwapBuffersWithDamage(display_rect_);
  EXPECT_EQ(1u, display_output_surface_->num_sent_frames());
  display_output_surface_->set_suspended_for_recycle(true);
  task_runner_->RunUntilIdle();
  EXPECT_EQ(1u, display_output_surface_->num_sent_frames());
  SwapBuffersWithDamage(display_rect_);
  task_runner_->RunUntilIdle();
  EXPECT_EQ(1u, display_output_surface_->num_sent_frames());
  display_output_surface_->set_suspended_for_recycle(false);
  SwapBuffersWithDamage(display_rect_);
  task_runner_->RunUntilIdle();
  EXPECT_EQ(2u, display_output_surface_->num_sent_frames());
}

}  // namespace
}  // namespace viz
