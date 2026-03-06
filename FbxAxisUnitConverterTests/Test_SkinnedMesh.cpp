// Test_SkinnedMesh.cpp
//
// FbxSkin / FbxCluster の軸変換検証 (Z-up → Y-up)
//
// 変換行列 M (ZupToYup):
//   行ベクトル規約 v' = v * M
//   src X -> dst X  : (1,0,0) -> (1,0,0)
//   src Y -> dst -Z : (0,1,0) -> (0,0,-1)
//   src Z -> dst  Y : (0,0,1) -> (0,1,0)

#include "TestHelpers.h"
#include "GeometryProcessor.h"

// ---------------------------------------------------------------------------
// Helper: 平行移動のみの FbxAMatrix を作る
// ---------------------------------------------------------------------------
static FbxAMatrix MakeTranslationMatrix(double x, double y, double z)
{
    FbxAMatrix m;
    m.SetIdentity();
    m.SetT(FbxVector4(x, y, z, 1.0));
    return m;
}

// ---------------------------------------------------------------------------
// Helper: メッシュに FbxSkin + FbxCluster を追加し、クラスターを返す
// ---------------------------------------------------------------------------
static FbxCluster* AddSkinCluster(
    FbxScene*         scene,
    FbxMesh*          mesh,
    const FbxAMatrix& transformMatrix,
    const FbxAMatrix& transformLinkMatrix)
{
    FbxSkin*    skin    = FbxSkin::Create(scene, "skin");
    FbxCluster* cluster = FbxCluster::Create(scene, "cluster");
    cluster->SetTransformMatrix(transformMatrix);
    cluster->SetTransformLinkMatrix(transformLinkMatrix);
    skin->AddCluster(cluster);
    mesh->AddDeformer(skin);
    return cluster;
}

// ---------------------------------------------------------------------------
// TransformLinkMatrix の平行移動が変換される
// (0, 0, 1) --ZupToYup--> (0, 1, 0)
// ---------------------------------------------------------------------------
TEST(SkinnedMesh, TransformLinkMatrixTranslationZtoY)
{
    FbxTestContext ctx;
    FbxNode* node = ctx.AddNode("mesh_node");
    FbxMesh* mesh = ctx.AttachTriangle(node);

    FbxAMatrix identity; identity.SetIdentity();
    FbxAMatrix tlm = MakeTranslationMatrix(0, 0, 1);

    FbxCluster* cluster = AddSkinCluster(ctx.scene, mesh, identity, tlm);

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    GeometryProcessor gp(false);
    gp.ProcessScene(ctx.scene, M, 1.0);

    FbxAMatrix result;
    cluster->GetTransformLinkMatrix(result);
    FbxVector4 t = result.GetT();

    EXPECT_NEAR(t[0], 0.0, 1e-4) << "X should be 0";
    EXPECT_NEAR(t[1], 1.0, 1e-4) << "Y should be 1 (was Z)";
    EXPECT_NEAR(t[2], 0.0, 1e-4) << "Z should be 0";
}

// ---------------------------------------------------------------------------
// TransformMatrix の平行移動: X 軸は変換後も不変
// (1, 0, 0) --ZupToYup--> (1, 0, 0)
// ---------------------------------------------------------------------------
TEST(SkinnedMesh, TransformMatrixXAxisUnchanged)
{
    FbxTestContext ctx;
    FbxNode* node = ctx.AddNode("mesh_node");
    FbxMesh* mesh = ctx.AttachTriangle(node);

    FbxAMatrix tm = MakeTranslationMatrix(1, 0, 0);
    FbxAMatrix identity; identity.SetIdentity();

    FbxCluster* cluster = AddSkinCluster(ctx.scene, mesh, tm, identity);

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    GeometryProcessor gp(false);
    gp.ProcessScene(ctx.scene, M, 1.0);

    FbxAMatrix result;
    cluster->GetTransformMatrix(result);
    FbxVector4 t = result.GetT();

    EXPECT_NEAR(t[0], 1.0, 1e-4) << "X should remain 1";
    EXPECT_NEAR(t[1], 0.0, 1e-4) << "Y should be 0";
    EXPECT_NEAR(t[2], 0.0, 1e-4) << "Z should be 0";
}

// ---------------------------------------------------------------------------
// TransformLinkMatrix にスケールが適用される
// (0, 0, 1) --ZupToYup, scale=100--> (0, 100, 0)
// ---------------------------------------------------------------------------
TEST(SkinnedMesh, TransformLinkMatrixScaleApplied)
{
    FbxTestContext ctx;
    FbxNode* node = ctx.AddNode("mesh_node");
    FbxMesh* mesh = ctx.AttachTriangle(node);

    FbxAMatrix identity; identity.SetIdentity();
    FbxAMatrix tlm = MakeTranslationMatrix(0, 0, 1);

    FbxCluster* cluster = AddSkinCluster(ctx.scene, mesh, identity, tlm);

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    GeometryProcessor gp(false);
    gp.ProcessScene(ctx.scene, M, 100.0);

    FbxAMatrix result;
    cluster->GetTransformLinkMatrix(result);
    FbxVector4 t = result.GetT();

    EXPECT_NEAR(t[0],   0.0, 1e-4) << "X should be 0";
    EXPECT_NEAR(t[1], 100.0, 1e-4) << "Y should be 100 (was Z, scale=100)";
    EXPECT_NEAR(t[2],   0.0, 1e-4) << "Z should be 0";
}

// ---------------------------------------------------------------------------
// TransformLinkMatrix の Y 平行移動は -Z に写像される
// (0, 1, 0) --ZupToYup--> (0, 0, -1)
// ---------------------------------------------------------------------------
TEST(SkinnedMesh, TransformLinkMatrixTranslationYtoNegZ)
{
    FbxTestContext ctx;
    FbxNode* node = ctx.AddNode("mesh_node");
    FbxMesh* mesh = ctx.AttachTriangle(node);

    FbxAMatrix identity; identity.SetIdentity();
    FbxAMatrix tlm = MakeTranslationMatrix(0, 1, 0);

    FbxCluster* cluster = AddSkinCluster(ctx.scene, mesh, identity, tlm);

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    GeometryProcessor gp(false);
    gp.ProcessScene(ctx.scene, M, 1.0);

    FbxAMatrix result;
    cluster->GetTransformLinkMatrix(result);
    FbxVector4 t = result.GetT();

    EXPECT_NEAR(t[0],  0.0, 1e-4) << "X should be 0";
    EXPECT_NEAR(t[1],  0.0, 1e-4) << "Y should be 0";
    EXPECT_NEAR(t[2], -1.0, 1e-4) << "Z should be -1 (was Y)";
}

// ---------------------------------------------------------------------------
// 同じクラスターが複数回処理されないことを確認 (dedup)
// 2つの FbxSkin が同じ FbxCluster を参照しても変換は1回だけ
// ---------------------------------------------------------------------------
TEST(SkinnedMesh, ClusterNotProcessedTwice)
{
    FbxTestContext ctx;
    FbxNode* node = ctx.AddNode("mesh_node");
    FbxMesh* mesh = ctx.AttachTriangle(node);

    FbxAMatrix identity; identity.SetIdentity();
    FbxAMatrix tlm = MakeTranslationMatrix(0, 0, 1);

    // 同じ cluster を 2つの skin に登録する
    FbxCluster* cluster = FbxCluster::Create(ctx.scene, "shared_cluster");
    cluster->SetTransformMatrix(identity);
    cluster->SetTransformLinkMatrix(tlm);

    FbxSkin* skin1 = FbxSkin::Create(ctx.scene, "skin1");
    skin1->AddCluster(cluster);
    mesh->AddDeformer(skin1);

    FbxSkin* skin2 = FbxSkin::Create(ctx.scene, "skin2");
    skin2->AddCluster(cluster);
    mesh->AddDeformer(skin2);

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    GeometryProcessor gp(false);
    gp.ProcessScene(ctx.scene, M, 1.0);

    // 2回変換されていれば (0,0,1) -> (0,1,0) -> (0,0,1) に戻るが
    // 1回だけなら (0,1,0) のまま
    FbxAMatrix result;
    cluster->GetTransformLinkMatrix(result);
    FbxVector4 t = result.GetT();

    EXPECT_NEAR(t[0], 0.0, 1e-4) << "X should be 0 (not double-converted)";
    EXPECT_NEAR(t[1], 1.0, 1e-4) << "Y should be 1 (not double-converted)";
    EXPECT_NEAR(t[2], 0.0, 1e-4) << "Z should be 0 (not double-converted)";
}
