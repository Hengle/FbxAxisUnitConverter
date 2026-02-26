// Test_IdentityConversion.cpp
//
// 回帰テスト: Identity 変換行列を適用しても LclTranslation / LclRotation / LclScaling
// の値が変化しないことを確認する。
// (バグ発見: Y > 90° のオイラー角をラウンドトリップすると等価だが異なる値に変換される
//  問題の回帰テスト。Identity では IsRotationIdentity() が true になりスキップされる)

#include "TestHelpers.h"
#include "TransformProcessor.h"

// ----------------------------------------------------------------
// ケース1: Y 角が 90° 超のノードが Identity 変換後も値不変
// ----------------------------------------------------------------
TEST(IdentityConversion, RotationPreservedOver90Deg)
{
    FbxTestContext ctx;

    // Y 軸回転 120° (90° を超えた値)
    FbxNode* node = ctx.AddNode("node1",
        FbxDouble3(0, 0, 0),
        FbxDouble3(0, 120, 0),
        FbxDouble3(1, 1, 1));

    FbxAMatrix identity = FbxTestContext::MakeIdentityMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), identity, 1.0);

    FbxDouble3 rot = node->LclRotation.Get();
    // Identity 変換ではオイラー角は変化しないこと
    ExpectDouble3Near(rot, FbxDouble3(0, 120, 0));
}

// ----------------------------------------------------------------
// ケース2: LclTranslation が Identity 変換後も不変
// ----------------------------------------------------------------
TEST(IdentityConversion, TranslationPreserved)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("node2",
        FbxDouble3(3.5, -2.0, 7.0),
        FbxDouble3(0, 0, 0),
        FbxDouble3(1, 1, 1));

    FbxAMatrix identity = FbxTestContext::MakeIdentityMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), identity, 1.0);

    FbxDouble3 t = node->LclTranslation.Get();
    ExpectDouble3Near(t, FbxDouble3(3.5, -2.0, 7.0));
}

// ----------------------------------------------------------------
// ケース3: LclScaling が Identity 変換後も不変
// ----------------------------------------------------------------
TEST(IdentityConversion, ScalingPreserved)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("node3",
        FbxDouble3(0, 0, 0),
        FbxDouble3(0, 0, 0),
        FbxDouble3(2, 3, 5));

    FbxAMatrix identity = FbxTestContext::MakeIdentityMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), identity, 1.0);

    FbxDouble3 s = node->LclScaling.Get();
    ExpectDouble3Near(s, FbxDouble3(2, 3, 5));
}
