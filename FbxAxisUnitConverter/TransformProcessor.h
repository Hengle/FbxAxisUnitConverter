#pragma once
#include <fbxsdk.h>

class TransformProcessor
{
public:
    static void ProcessNode(FbxNode* node, const FbxAMatrix& convMatrix, double scale);

private:
    static void ProcessNodeRecursive(FbxNode* node, const FbxAMatrix& convMatrix, double scale);
};
