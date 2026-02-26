// Test_RotationOrder.cpp
//
// RotationOrder の制限事項テスト。
//
// 現在の実装は EXYZOrder (デフォルト) のみ正しく処理する。
// ZYX などの非デフォルト RotationOrder を持つノードは limitation として記録し、
// クラッシュしないことのみ確認する。

#include "TestHelpers.h"
#include "TransformProcessor.h"

// ----------------------------------------------------------------
// ZYX RotationOrder: limitation 記録 (クラッシュしないことを確認)
// ----------------------------------------------------------------
TEST(RotationOrder, ZYX_LimitationDocumented)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("nodeZYX",
        FbxDouble3(0, 0, 1),
        FbxDouble3(30, 45, 60),
        FbxDouble3(1, 1, 1));

    // ZYX 順に設定
    node->SetRotationOrder(FbxNode::eSourcePivot, eEulerZYX);

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();

    // LIMITATION: 非デフォルト RotationOrder は正しく変換されない。
    // クラッシュしないことのみ確認する。
    SUCCEED() << "Non-default RotationOrder (ZYX) is not correctly handled (known limitation).";

    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 1.0);
    // ここに到達すれば OK (クラッシュなし)
}

// ----------------------------------------------------------------
// XYZ (デフォルト) RotationOrder: 正しく変換されること
// ----------------------------------------------------------------
TEST(RotationOrder, XYZ_CorrectlyConverted)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("nodeXYZ",
        FbxDouble3(0, 0, 0),
        FbxDouble3(0, 0, 90),   // Z 軸 90°
        FbxDouble3(1, 1, 1));

    // デフォルトは eEulerXYZ なので明示設定不要だが念のため
    node->SetRotationOrder(FbxNode::eSourcePivot, eEulerXYZ);

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 1.0);

    // Z-up Z 軸 90° -> Y-up Y 軸 90°
    FbxDouble3 rot = node->LclRotation.Get();
    EXPECT_NEAR(rot[0],  0.0, 1e-4) << "X rotation";
    EXPECT_NEAR(rot[1], 90.0, 1e-4) << "Y rotation should be ~90";
    EXPECT_NEAR(rot[2],  0.0, 1e-4) << "Z rotation";
}
