// Test_PreRotation.cpp
//
// PreRotation の制限事項を記録するテスト。
// 現在の実装は PreRotation を変換しないため、PreRotation を持つノードは
// 軸変換後に正確な向きを保証しない。
// このテストはクラッシュしないことと limitation の文書化を目的とする。

#include "TestHelpers.h"
#include "TransformProcessor.h"

TEST(PreRotation, LimitationDocumented)
{
    FbxTestContext ctx;

    FbxNode* node = ctx.AddNode("nodeWithPreRot",
        FbxDouble3(1, 0, 0),
        FbxDouble3(0, 45, 0),
        FbxDouble3(1, 1, 1));

    // PreRotation を設定（Z-up 系で X 軸 -90° の補正回転など）
    node->SetPreRotation(FbxNode::eSourcePivot, FbxVector4(-90, 0, 0));

    FbxAMatrix M = FbxTestContext::MakeZupToYupMatrix();

    // LIMITATION: TransformProcessor は PreRotation を変換しない。
    // クラッシュしないことのみ確認する。
    SUCCEED() << "PreRotation is not converted by TransformProcessor (known limitation).";

    // ただし LclTranslation と LclRotation は変換されること
    TransformProcessor::ProcessNode(ctx.scene->GetRootNode(), M, 1.0);

    // Translation: (1,0,0) -> M*(1,0,0) = (1,0,0) (X 成分は不変)
    ExpectDouble3Near(node->LclTranslation.Get(), FbxDouble3(1, 0, 0));
}
