#pragma once
#include <fbxsdk.h>

class AnimProcessor
{
public:
    static void ProcessAnimation(FbxScene* scene, const FbxAMatrix& convMatrix, double scale);
};
