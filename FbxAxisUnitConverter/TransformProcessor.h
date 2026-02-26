#pragma once
#include <fbxsdk.h>

class TransformProcessor
{
public:
    static void ProcessNode(FbxNode* node, const FbxAMatrix& convMatrix, double scale);

    // Pre-normalization用: 1ノードに逆補正変換を適用する（再帰なし）
    // corrInv : corrMatrix の逆行列（preNorm→fileSrc の逆）
    // corrMatrix : preNorm→fileSrc の変換行列（回転変換の基底変換に使用）
    // scaleInv   : 1.0 / corrScale（fileSrcUnit / preNormUnit の逆数）
    static void ApplySingleNode(FbxNode* node,
                                const FbxAMatrix& corrInv,
                                const FbxAMatrix& corrMatrix,
                                double scaleInv);

private:
    static void ProcessNodeRecursive(FbxNode* node, const FbxAMatrix& convMatrix, double scale);
};
