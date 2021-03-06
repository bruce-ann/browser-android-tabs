// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_REMOTING_END2END_RENDERER_H_
#define MEDIA_REMOTING_END2END_RENDERER_H_

#include <vector>

#include "base/memory/weak_ptr.h"
#include "media/base/demuxer_stream.h"
#include "media/base/renderer.h"
#include "media/remoting/rpc_broker.h"

namespace media {
namespace remoting {

class SharedSession;
class RendererController;
class CourierRenderer;
class Receiver;

// Simulates the media remoting pipeline.
class End2EndTestRenderer final : public Renderer {
 public:
  explicit End2EndTestRenderer(std::unique_ptr<Renderer> renderer);
  ~End2EndTestRenderer() override;

  // Renderer implementation.
  void Initialize(MediaResource* media_resource,
                  RendererClient* client,
                  const PipelineStatusCB& init_cb) override;
  void SetCdm(CdmContext* cdm_context,
              const CdmAttachedCB& cdm_attached_cb) override;
  void Flush(const base::Closure& flush_cb) override;
  void StartPlayingFrom(base::TimeDelta time) override;
  void SetPlaybackRate(double playback_rate) override;
  void SetVolume(float volume) override;
  base::TimeDelta GetMediaTime() override;

 private:
  // Called to send RPC messages to |receiver_|.
  void SendMessageToSink(const std::vector<uint8_t>& message);

  // Called to send frame data to |receiver_|.
  void SendFrameToSink(const std::vector<uint8_t>& data,
                       DemuxerStream::Type type);

  // Called when receives RPC messages from |receiver_|.
  void OnMessageFromSink(std::unique_ptr<std::vector<uint8_t>> message);

  // The session that is used by |controller_| to create the data pipes.
  scoped_refptr<SharedSession> shared_session_;
  std::unique_ptr<RendererController> controller_;
  std::unique_ptr<CourierRenderer> courier_renderer_;

  // The RpcBroker to handle the RPC messages to/from |receiver_|.
  RpcBroker receiver_rpc_broker_;

  // A receiver that renders media streams.
  std::unique_ptr<Receiver> receiver_;

  base::WeakPtrFactory<End2EndTestRenderer> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(End2EndTestRenderer);
};

}  // namespace remoting
}  // namespace media

#endif  // MEDIA_REMOTING_END2END_RENDERER_H_
