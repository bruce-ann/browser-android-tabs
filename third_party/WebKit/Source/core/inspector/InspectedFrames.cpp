// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/inspector/InspectedFrames.h"

#include "core/dom/Document.h"
#include "core/frame/LocalFrame.h"

namespace blink {

InspectedFrames::InspectedFrames(LocalFrame* root,
                                 const String& devtools_frame_token)
    : root_(root), devtools_frame_token_(devtools_frame_token) {}

InspectedFrames::Iterator InspectedFrames::begin() {
  return Iterator(root_, root_);
}

InspectedFrames::Iterator InspectedFrames::end() {
  return Iterator(root_, nullptr);
}

bool InspectedFrames::Contains(LocalFrame* frame) const {
  return frame->GetProbeSink() == root_->GetProbeSink();
}

LocalFrame* InspectedFrames::FrameWithSecurityOrigin(
    const String& origin_raw_string) {
  for (LocalFrame* frame : *this) {
    if (frame->GetDocument()->GetSecurityOrigin()->ToRawString() ==
        origin_raw_string)
      return frame;
  }
  return nullptr;
}

InspectedFrames::Iterator::Iterator(LocalFrame* root, LocalFrame* current)
    : root_(root), current_(current) {}

InspectedFrames::Iterator& InspectedFrames::Iterator::operator++() {
  if (!current_)
    return *this;
  Frame* frame = current_->Tree().TraverseNext(root_);
  current_ = nullptr;
  for (; frame; frame = frame->Tree().TraverseNext(root_)) {
    if (!frame->IsLocalFrame())
      continue;
    LocalFrame* local = ToLocalFrame(frame);
    if (local->GetProbeSink() == root_->GetProbeSink()) {
      current_ = local;
      break;
    }
  }
  return *this;
}

InspectedFrames::Iterator InspectedFrames::Iterator::operator++(int) {
  LocalFrame* old = current_;
  ++*this;
  return Iterator(root_, old);
}

bool InspectedFrames::Iterator::operator==(const Iterator& other) {
  return current_ == other.current_ && root_ == other.root_;
}

bool InspectedFrames::Iterator::operator!=(const Iterator& other) {
  return !(*this == other);
}

void InspectedFrames::Trace(blink::Visitor* visitor) {
  visitor->Trace(root_);
}

}  // namespace blink
