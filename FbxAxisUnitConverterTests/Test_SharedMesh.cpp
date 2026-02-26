// Test_SharedMesh.cpp
//
// 同一 FbxMesh インスタンスを複数のノードで共有している場合、
// GeometryProcessor が1回だけ処理することを確認する。
// (2回変換されると頂点が2倍ずれる)

#include "TestHelpers.h"
#include "GeometryProcessor.h"

TEST(SharedMesh, ProcessedOnlyOnce)
{
    FbxTestContext ctx;

    // ノード A にメッシュをアタッチ
    FbxNode* nodeA = ctx.AddNode("nodeA");
    FbxMesh* mesh = ctx.AttachTriangle(nodeA,
        FbxVector4(0, 0, 1),
        FbxVector4(1, 0, 1),
        FbxVector4(0, 1, 1));

    // ノード B に同じメッシュを共有
    FbxNode* nodeB = ctx.AddNode("nodeB");
    nodeB->SetNodeAttribute(mesh);

    // Z-up -> Y-up 変換行列
    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();

    GeometryProcessor geoProc(false);
    geoProc.ProcessScene(ctx.scene, M, 1.0);

    // 頂点0: (0,0,1) -> M*(0,0,1) = (0,1,0)
    // 2回処理されていたら (0,1,0) -> M*(0,1,0) = (0,0,-1) になってしまう
    FbxVector4 v0 = mesh->GetControlPointAt(0);
    EXPECT_NEAR(v0[0],  0.0, 1e-4) << "vertex0.x";
    EXPECT_NEAR(v0[1],  1.0, 1e-4) << "vertex0.y (should be 1, not double-transformed)";
    EXPECT_NEAR(v0[2],  0.0, 1e-4) << "vertex0.z";
}
