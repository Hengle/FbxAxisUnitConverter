#include "AnimProcessor.h"

void AnimProcessor::ProcessAnimation(FbxScene* scene, const FbxAMatrix& convMatrix, double scale)
{
    int stackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    for (int si = 0; si < stackCount; ++si)
    {
        FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(si);
        int layerCount = stack->GetMemberCount<FbxAnimLayer>();
        for (int li = 0; li < layerCount; ++li)
        {
            // FbxAnimLayer* layer = stack->GetMember<FbxAnimLayer>(li);
            // TODO: 各ノードの Translation / Rotation カーブを変換
        }
    }
}
