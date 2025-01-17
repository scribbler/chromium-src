// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/editing/spellcheck/spell_checker.h"

#include "third_party/blink/renderer/core/editing/editor.h"
#include "third_party/blink/renderer/core/editing/ephemeral_range.h"
#include "third_party/blink/renderer/core/editing/frame_selection.h"
#include "third_party/blink/renderer/core/editing/markers/document_marker_controller.h"
#include "third_party/blink/renderer/core/editing/markers/spell_check_marker.h"
#include "third_party/blink/renderer/core/editing/selection_template.h"
#include "third_party/blink/renderer/core/editing/spellcheck/spell_check_requester.h"
#include "third_party/blink/renderer/core/editing/spellcheck/spell_check_test_base.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/frame/local_frame_view.h"
#include "third_party/blink/renderer/core/frame/settings.h"
#include "third_party/blink/renderer/core/html/forms/html_input_element.h"

namespace blink {

class SpellCheckerTest : public SpellCheckTestBase {
 protected:
  unsigned LayoutCount() const {
    return Page().GetFrameView().LayoutCountForTesting();
  }
  DummyPageHolder& Page() const { return GetDummyPageHolder(); }

  void ForceLayout();
};

void SpellCheckerTest::ForceLayout() {
  LocalFrameView& frame_view = Page().GetFrameView();
  IntRect frame_rect = frame_view.FrameRect();
  frame_rect.SetWidth(frame_rect.Width() + 1);
  frame_rect.SetHeight(frame_rect.Height() + 1);
  Page().GetFrameView().SetFrameRect(frame_rect);
  GetDocument().UpdateStyleAndLayout();
}

TEST_F(SpellCheckerTest, AdvanceToNextMisspellingWithEmptyInputNoCrash) {
  SetBodyContent("<input placeholder='placeholder'>abc");
  UpdateAllLifecyclePhasesForTest();
  Element* input = GetDocument().QuerySelector("input");
  input->focus();
  // Do not crash in advanceToNextMisspelling.
  GetSpellChecker().AdvanceToNextMisspelling(false);
}

// Regression test for crbug.com/701309
TEST_F(SpellCheckerTest, AdvanceToNextMisspellingWithImageInTableNoCrash) {
  SetBodyContent(
      "<div contenteditable>"
      "<table><tr><td>"
      "<img src=foo.jpg>"
      "</td></tr></table>"
      "zz zz zz"
      "</div>");
  GetDocument().QuerySelector("div")->focus();
  UpdateAllLifecyclePhasesForTest();

  // Do not crash in advanceToNextMisspelling.
  GetSpellChecker().AdvanceToNextMisspelling(false);
}

// Regression test for crbug.com/728801
TEST_F(SpellCheckerTest, AdvancedToNextMisspellingWrapSearchNoCrash) {
  SetBodyContent("<div contenteditable>  zz zz zz  </div>");

  Element* div = GetDocument().QuerySelector("div");
  div->focus();
  Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .Collapse(Position::LastPositionInNode(*div))
          .Build());
  UpdateAllLifecyclePhasesForTest();

  GetSpellChecker().AdvanceToNextMisspelling(false);
}

TEST_F(SpellCheckerTest, SpellCheckDoesNotCauseUpdateLayout) {
  SetBodyContent("<input>");
  HTMLInputElement* input =
      ToHTMLInputElement(GetDocument().QuerySelector("input"));
  input->focus();
  input->setValue("Hello, input field");
  GetDocument().UpdateStyleAndLayout();

  Position new_position(input->InnerEditorElement()->firstChild(), 3);
  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder().Collapse(new_position).Build());
  ASSERT_EQ(3u, input->selectionStart());

  EXPECT_TRUE(GetSpellChecker().IsSpellCheckingEnabled());
  ForceLayout();
  unsigned start_count = LayoutCount();
  GetSpellChecker().RespondToChangedSelection();
  EXPECT_EQ(start_count, LayoutCount());
}

TEST_F(SpellCheckerTest, MarkAndReplaceForHandlesMultipleReplacements) {
  SetBodyContent(
      "<div contenteditable>"
      "spllchck"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();
  EphemeralRange range_to_check =
      EphemeralRange(Position(text, 0), Position(text, 8));

  SpellCheckRequest* request = SpellCheckRequest::Create(range_to_check, 0);

  TextCheckingResult result;
  result.decoration = TextDecorationType::kTextDecorationTypeSpelling;
  result.location = 0;
  result.length = 8;
  result.replacements = Vector<String>({"spellcheck", "spillchuck"});

  GetDocument().GetFrame()->GetSpellChecker().MarkAndReplaceFor(
      request, Vector<TextCheckingResult>({result}));

  ASSERT_EQ(1u, GetDocument().Markers().Markers().size());

  // The Spelling marker's description should be a newline-separated list of the
  // suggested replacements
  EXPECT_EQ(
      "spellcheck\nspillchuck",
      ToSpellCheckMarker(GetDocument().Markers().Markers()[0])->Description());
}

TEST_F(SpellCheckerTest, GetSpellCheckMarkerUnderSelection_FirstCharSelected) {
  SetBodyContent(
      "<div contenteditable>"
      "spllchck"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 8)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 0), Position(text, 1))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(8u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest, GetSpellCheckMarkerUnderSelection_LastCharSelected) {
  SetBodyContent(
      "<div contenteditable>"
      "spllchck"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 8)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 7), Position(text, 8))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(8u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest,
       GetSpellCheckMarkerUnderSelection_SingleCharWordSelected) {
  SetBodyContent(
      "<div contenteditable>"
      "s"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 1)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 0), Position(text, 1))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(1u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest,
       GetSpellCheckMarkerUnderSelection_CaretLeftOfSingleCharWord) {
  SetBodyContent(
      "<div contenteditable>"
      "s"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 1)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 0), Position(text, 0))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(1u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest,
       GetSpellCheckMarkerUnderSelection_CaretRightOfSingleCharWord) {
  SetBodyContent(
      "<div contenteditable>"
      "s"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 1)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 1), Position(text, 1))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(1u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest,
       GetSpellCheckMarkerUnderSelection_CaretLeftOfMultiCharWord) {
  SetBodyContent(
      "<div contenteditable>"
      "spllchck"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 8)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 0), Position(text, 0))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(8u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest,
       GetSpellCheckMarkerUnderSelection_CaretRightOfMultiCharWord) {
  SetBodyContent(
      "<div contenteditable>"
      "spllchck"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 8)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 8), Position(text, 8))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(8u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest, GetSpellCheckMarkerUnderSelection_CaretMiddleOfWord) {
  SetBodyContent(
      "<div contenteditable>"
      "spllchck"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 8)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 4), Position(text, 4))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(text, result.first);
  ASSERT_NE(nullptr, result.second);
  EXPECT_EQ(0u, result.second->StartOffset());
  EXPECT_EQ(8u, result.second->EndOffset());
}

TEST_F(SpellCheckerTest,
       GetSpellCheckMarkerUnderSelection_CaretOneCharLeftOfMisspelling) {
  SetBodyContent(
      "<div contenteditable>"
      "a spllchck"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 2), Position(text, 10)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 1), Position(text, 1))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(nullptr, result.first);
}

TEST_F(SpellCheckerTest,
       GetSpellCheckMarkerUnderSelection_CaretOneCharRightOfMisspelling) {
  SetBodyContent(
      "<div contenteditable>"
      "spllchck a"
      "</div>");
  Element* div = GetDocument().QuerySelector("div");
  Node* text = div->firstChild();

  GetDocument().Markers().AddSpellingMarker(
      EphemeralRange(Position(text, 0), Position(text, 8)));

  GetDocument().GetFrame()->Selection().SetSelectionAndEndTyping(
      SelectionInDOMTree::Builder()
          .SetBaseAndExtent(Position(text, 9), Position(text, 9))
          .Build());

  std::pair<Node*, SpellCheckMarker*> result =
      GetDocument()
          .GetFrame()
          ->GetSpellChecker()
          .GetSpellCheckMarkerUnderSelection();
  EXPECT_EQ(nullptr, result.first);
}

}  // namespace blink
