// Test_DeepHierarchy.cpp
//
// 5 段ネストした階層の全ノードが正しく変換されることを確認する。
// Z-up -> Y-up 変換: translation の Z 成分が Y へ移動する。

#include "TestHelpers.h"
#include "TransformProcessor.h"

TEST(DeepHierarchy, AllNodesConverted)
{
    FbxTestContext ctx;

    // 5 段ネスト: root -> n1 -> n2 -> n3 -> n4 -> n5
    // 各ノードの translation = (0, 0, depth_index) (Z-up 系)
    FbxNode* parent = ctx.scene->GetRootNode();
    FbxNode* nodes[5];
    for (int i = 0; i < 5; ++i)
    {
        char name[16];
        snprintf(name, sizeof(name), "n%d", i + 1);
        FbxDouble3 t(0, 0, static_cast<double>(i + 1));
        nodes[i] = ctx.AddChildNode(parent, name, t);
        parent = nodes[i];
    }

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 1.0);

    // 各ノード: (0, 0, i+1) -> M*(0, 0, i+1) = (0, i+1, 0)
    for (int i = 0; i < 5; ++i)
    {
        FbxDouble3 t = nodes[i]->LclTranslation.Get();
        double expected = static_cast<double>(i + 1);
        EXPECT_NEAR(t[0], 0.0,      1e-4) << "node" << (i+1) << " x";
        EXPECT_NEAR(t[1], expected, 1e-4) << "node" << (i+1) << " y";
        EXPECT_NEAR(t[2], 0.0,      1e-4) << "node" << (i+1) << " z";
    }
}
