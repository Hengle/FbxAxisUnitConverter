#pragma once
#include "ConvertOptions.h"
#include <fbxsdk.h>

class FbxAxisUnitConverter
{
public:
    explicit FbxAxisUnitConverter(const ConvertOptions& options);
    ~FbxAxisUnitConverter();

    // メイン処理。成功時は 0 を返す
    int Run();

private:
    void BuildConversionMatrix();
    void ApplyGlobalSettings();
    void ProcessScene();

    const ConvertOptions& mOptions;
    FbxManager*  mManager = nullptr;
    FbxScene*    mScene   = nullptr;

    FbxAMatrix   mConvMatrix;   // 変換行列（回転・スウィズル）
    double       mScaleFactor = 1.0;
    bool         mFlipWinding = false;
};
