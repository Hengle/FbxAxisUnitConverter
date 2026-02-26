// Test_ZupToYup.cpp
//
// Z-up (+Y into screen) -> Y-up (-Z into screen) 変換の数値検証。
//
// 変換行列:
//   M = | 1  0  0 |   row0
//       | 0  0  1 |   row1
//       | 0 -1  0 |   row2
//
// 列ベクトル規約: v' = M * v  (FbxAMatrix::MultT)

#include "TestHelpers.h"
#include "TransformProcessor.h"

// ----------------------------------------------------------------
// LclTranslation: (0, 0, 1) -> (0, 1, 0)
// ----------------------------------------------------------------
TEST(ZupToYup, LclTranslationConverted)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("n",
        FbxDouble3(0, 0, 1),
        FbxDouble3(0, 0, 0),
        FbxDouble3(1, 1, 1));

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 1.0);

    ExpectDouble3Near(node->LclTranslation.Get(), FbxDouble3(0, 1, 0));
}

// ----------------------------------------------------------------
// LclRotation: Z 軸 90° 回転 -> Y 軸 90° 回転
// Z-up 系での Z 軸回転は Y-up 系での Y 軸回転に相当する
// ----------------------------------------------------------------
TEST(ZupToYup, LclRotationConverted)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("n",
        FbxDouble3(0, 0, 0),
        FbxDouble3(0, 0, 90),   // Z 軸 90°
        FbxDouble3(1, 1, 1));

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 1.0);

    // M * R_zrot90 * M^{-1} = R_yrot90
    // -> LclRotation.Y ≈ 90, X ≈ 0, Z ≈ 0
    FbxDouble3 rot = node->LclRotation.Get();
    EXPECT_NEAR(rot[0],  0.0, 1e-4) << "X rotation should be ~0";
    EXPECT_NEAR(rot[1], 90.0, 1e-4) << "Y rotation should be ~90";
    EXPECT_NEAR(rot[2],  0.0, 1e-4) << "Z rotation should be ~0";
}

// ----------------------------------------------------------------
// LclScaling: (2, 3, 5) -> (2, 5, 3)
// Z が Y へ、Y が Z へ入れ替わる
// ----------------------------------------------------------------
TEST(ZupToYup, LclScalingPermuted)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("n",
        FbxDouble3(0, 0, 0),
        FbxDouble3(0, 0, 0),
        FbxDouble3(2, 3, 5));

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 1.0);

    // row1 picks srcZ  -> dstY = s[2] = 5
    // row2 picks srcY  -> dstZ = s[1] = 3
    ExpectDouble3Near(node->LclScaling.Get(), FbxDouble3(2, 5, 3));
}

// ----------------------------------------------------------------
// LclTranslation with scale factor 100 (cm -> mm 等)
// ----------------------------------------------------------------
TEST(ZupToYup, LclTranslationWithScale)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("n",
        FbxDouble3(0, 0, 1),
        FbxDouble3(0, 0, 0),
        FbxDouble3(1, 1, 1));

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 100.0);

    // scale=100 applied before swizzle: (0,0,1)*100 = (0,0,100) -> M*(0,0,100) = (0,100,0)
    ExpectDouble3Near(node->LclTranslation.Get(), FbxDouble3(0, 100, 0));
}
