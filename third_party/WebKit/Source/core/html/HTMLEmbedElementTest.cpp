// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/html/HTMLEmbedElement.h"

#include <memory>
#include "core/dom/Document.h"
#include "core/frame/LocalFrameView.h"
#include "core/html/HTMLObjectElement.h"
#include "core/style/ComputedStyle.h"
#include "core/testing/PageTestBase.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace blink {

class HTMLEmbedElementTest : public PageTestBase {};

TEST_F(HTMLEmbedElementTest, FallbackState) {
  // Load <object> element with a <embed> child.
  // This can be seen on sites with Flash cookies,
  // for example on www.yandex.ru
  SetHtmlInnerHTML(R"HTML(
    <div>
    <object classid='clsid:D27CDB6E-AE6D-11cf-96B8-444553540000' width='1'
    height='1' id='fco'>
    <param name='movie' value='//site.com/flash-cookie.swf'>
    <param name='allowScriptAccess' value='Always'>
    <embed src='//site.com/flash-cookie.swf' allowscriptaccess='Always'
    width='1' height='1' id='fce'>
    </object></div>
  )HTML");

  auto* object_element = GetElementById("fco");
  ASSERT_TRUE(object_element);
  HTMLObjectElement* object = ToHTMLObjectElement(object_element);

  // At this moment updatePlugin() function is not called, so
  // useFallbackContent() will return false.
  // But the element will likely to use fallback content after updatePlugin().
  EXPECT_TRUE(object->HasFallbackContent());
  EXPECT_FALSE(object->UseFallbackContent());
  EXPECT_TRUE(object->WillUseFallbackContentAtLayout());

  auto* embed_element = GetElementById("fce");
  ASSERT_TRUE(embed_element);
  HTMLEmbedElement* embed = ToHTMLEmbedElement(embed_element);

  UpdateAllLifecyclePhases();

  // We should get |true| as a result and don't trigger a DCHECK.
  EXPECT_TRUE(static_cast<Element*>(embed)->LayoutObjectIsNeeded(
      ComputedStyle::InitialStyle()));

  // This call will update fallback state of the object.
  object->UpdatePlugin();

  EXPECT_TRUE(object->HasFallbackContent());
  EXPECT_TRUE(object->UseFallbackContent());
  EXPECT_TRUE(object->WillUseFallbackContentAtLayout());

  UpdateAllLifecyclePhases();
  EXPECT_TRUE(static_cast<Element*>(embed)->LayoutObjectIsNeeded(
      ComputedStyle::InitialStyle()));
}

}  // namespace blink
