// Test_UnitOnly.cpp
//
// 単位変換のみ (axis は Identity) のテスト。
// scale=100 のとき:
//   - LclTranslation: 各成分が 100 倍になる
//   - LclRotation: 変化なし (角度は単位に依存しない)
//   - LclScaling: 変化なし (比率は単位に依存しない)

#include "TestHelpers.h"
#include "TransformProcessor.h"

TEST(UnitOnly, TranslationScaled_RotationScalingUnchanged)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("n",
        FbxDouble3(1, 2, 3),
        FbxDouble3(30, 45, 60),
        FbxDouble3(2, 3, 5));

    FbxAMatrix identity = FbxTestContext::MakeIdentityMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), identity, 100.0);

    // Translation: x100
    ExpectDouble3Near(node->LclTranslation.Get(), FbxDouble3(100, 200, 300));

    // Rotation: unchanged (identity matrix skips rotation conversion)
    ExpectDouble3Near(node->LclRotation.Get(), FbxDouble3(30, 45, 60));

    // Scaling: unchanged
    ExpectDouble3Near(node->LclScaling.Get(), FbxDouble3(2, 3, 5));
}
