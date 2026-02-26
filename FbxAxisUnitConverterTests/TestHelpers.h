#pragma once
#include <gtest/gtest.h>
#include <fbxsdk.h>
#include <cmath>

// ---------------------------------------------------------------------------
// FbxDouble3 近似比較ヘルパー
// ---------------------------------------------------------------------------
inline void ExpectDouble3Near(const FbxDouble3& a, const FbxDouble3& b, double tol = 1e-4)
{
    EXPECT_NEAR(a[0], b[0], tol) << "component [0] mismatch";
    EXPECT_NEAR(a[1], b[1], tol) << "component [1] mismatch";
    EXPECT_NEAR(a[2], b[2], tol) << "component [2] mismatch";
}

// ---------------------------------------------------------------------------
// FbxTestContext — FbxManager + FbxScene の RAII ラッパー
//
// テストケースは FbxTestContext を値で保持し、各メソッドでノードやメッシュを
// 組み立てる。デストラクタで FbxManager が自動破棄される。
// ---------------------------------------------------------------------------
struct FbxTestContext
{
    FbxManager* manager = nullptr;
    FbxScene*   scene   = nullptr;

    FbxTestContext()
    {
        manager = FbxManager::Create();
        FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
        manager->SetIOSettings(ios);
        scene = FbxScene::Create(manager, "TestScene");
    }

    ~FbxTestContext()
    {
        if (scene)   { scene->Destroy();   scene   = nullptr; }
        if (manager) { manager->Destroy(); manager = nullptr; }
    }

    // コピー禁止
    FbxTestContext(const FbxTestContext&) = delete;
    FbxTestContext& operator=(const FbxTestContext&) = delete;

    // ----------------------------------------------------------------
    // AddNode: RootNode に子ノードを追加して返す
    // ----------------------------------------------------------------
    FbxNode* AddNode(
        const char* name,
        FbxDouble3 trans  = FbxDouble3(0, 0, 0),
        FbxDouble3 rot    = FbxDouble3(0, 0, 0),
        FbxDouble3 scale  = FbxDouble3(1, 1, 1))
    {
        FbxNode* node = FbxNode::Create(scene, name);
        node->LclTranslation.Set(trans);
        node->LclRotation.Set(rot);
        node->LclScaling.Set(scale);
        scene->GetRootNode()->AddChild(node);
        return node;
    }

    // ----------------------------------------------------------------
    // AddChildNode: 任意の親ノードに子ノードを追加して返す
    // ----------------------------------------------------------------
    FbxNode* AddChildNode(
        FbxNode* parent,
        const char* name,
        FbxDouble3 trans  = FbxDouble3(0, 0, 0),
        FbxDouble3 rot    = FbxDouble3(0, 0, 0),
        FbxDouble3 scale  = FbxDouble3(1, 1, 1))
    {
        FbxNode* node = FbxNode::Create(scene, name);
        node->LclTranslation.Set(trans);
        node->LclRotation.Set(rot);
        node->LclScaling.Set(scale);
        parent->AddChild(node);
        return node;
    }

    // ----------------------------------------------------------------
    // AttachTriangle: 三角形メッシュをノードにアタッチする
    // ----------------------------------------------------------------
    FbxMesh* AttachTriangle(
        FbxNode* node,
        FbxVector4 v0 = FbxVector4(0, 0, 0),
        FbxVector4 v1 = FbxVector4(1, 0, 0),
        FbxVector4 v2 = FbxVector4(0, 1, 0))
    {
        FbxMesh* mesh = FbxMesh::Create(scene, "mesh");

        mesh->InitControlPoints(3);
        mesh->SetControlPointAt(v0, 0);
        mesh->SetControlPointAt(v1, 1);
        mesh->SetControlPointAt(v2, 2);

        mesh->BeginPolygon();
        mesh->AddPolygon(0);
        mesh->AddPolygon(1);
        mesh->AddPolygon(2);
        mesh->EndPolygon();

        node->SetNodeAttribute(mesh);
        return mesh;
    }

    // ----------------------------------------------------------------
    // BuildConvMatrix: Z-up(+Y forward) -> Y-up(-Z forward) 変換行列を返す
    //   M = | 1  0  0 |
    //       | 0  0  1 |
    //       | 0 -1  0 |
    // ----------------------------------------------------------------
    static FbxAMatrix MakeZupToYupMatrix()
    {
        FbxAMatrix m;
        // Row-vector convention: v' = v * M
        // src X -> dst X : row0 = (1, 0, 0)
        // src Y -> dst -Z: row1 = (0, 0, -1)
        // src Z -> dst Y : row2 = (0, 1, 0)
        m.SetRow(0, FbxVector4( 1,  0,  0, 0));
        m.SetRow(1, FbxVector4( 0,  0, -1, 0));
        m.SetRow(2, FbxVector4( 0,  1,  0, 0));
        m.SetRow(3, FbxVector4( 0,  0,  0, 1));
        return m;
    }

    // ----------------------------------------------------------------
    // Identity 行列を返す
    // ----------------------------------------------------------------
    static FbxAMatrix MakeIdentityMatrix()
    {
        FbxAMatrix m;
        m.SetIdentity();
        return m;
    }
};
