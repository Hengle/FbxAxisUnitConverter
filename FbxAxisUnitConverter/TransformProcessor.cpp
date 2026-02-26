#include "TransformProcessor.h"
#include <cmath>

// 3x3 サブ行列が単位行列かどうか確認（axis swizzle が不要なケースの判定）
static bool IsRotationIdentity(const FbxAMatrix& m)
{
    for (int r = 0; r < 3; ++r)
    {
        FbxVector4 row = m.GetRow(r);
        for (int c = 0; c < 3; ++c)
        {
            double expected = (r == c) ? 1.0 : 0.0;
            if (std::abs(row[c] - expected) > 1e-6) return false;
        }
    }
    return true;
}

void TransformProcessor::ApplySingleNode(FbxNode* node,
                                         const FbxAMatrix& corrInv,
                                         const FbxAMatrix& corrMatrix,
                                         double scaleInv)
{
    if (!node) return;

    // --- LclTranslation: 逆回転（軸置換）+ 単位スケール除去 ---
    // FbxAMatrix::MultT は行ベクトル規約 (v^T * M)。
    // M_eff(corrInv) = Rx(+90°) なので corrInv.MultT(v) が fileSrc→preNorm 変換を行う。
    // T_orig = M_eff(corrInv) * T_baked = corrInv.MultT(T_baked)
    {
        FbxDouble3 t = node->LclTranslation.Get();
        FbxVector4 tVec(t[0], t[1], t[2], 0.0);
        FbxVector4 tRestored = corrInv.MultT(tVec);
        node->LclTranslation.Set(FbxDouble3(
            tRestored[0] * scaleInv,
            tRestored[1] * scaleInv,
            tRestored[2] * scaleInv));
    }

    // --- LclRotation: 逆変換 R_orig = M_eff(corrInv) * R_baked ---
    // FbxAMatrix の * は行ベクトル規約: M_eff(A*B) = M_eff(B) * M_eff(A)（順序逆）
    // したがって "srcRot * corrInv" の実効 = M_eff(corrInv) * M_eff(srcRot) = Rx(+90°) * R_baked
    if (!IsRotationIdentity(corrMatrix))
    {
        FbxDouble3 r = node->LclRotation.Get();
        FbxAMatrix srcRot;
        srcRot.SetR(FbxVector4(r[0], r[1], r[2], 0.0));
        FbxAMatrix rRestored = srcRot * corrInv;
        FbxVector4 euler = rRestored.GetR();
        node->LclRotation.Set(FbxDouble3(euler[0], euler[1], euler[2]));
    }

    // --- LclScaling: 軸置換 + 単位スケール除去 ---
    // corrInv の各行から置換マッピングを逆引きし、scaleInv を乗算する
    {
        FbxDouble3 s = node->LclScaling.Get();
        FbxDouble3 sRestored(1.0, 1.0, 1.0);
        for (int dstAxis = 0; dstAxis < 3; ++dstAxis)
        {
            FbxVector4 row = corrInv.GetRow(dstAxis);
            for (int srcAxis = 0; srcAxis < 3; ++srcAxis)
            {
                if (std::abs(row[srcAxis]) > 0.5)
                {
                    sRestored[dstAxis] = s[srcAxis] * scaleInv;
                    break;
                }
            }
        }
        node->LclScaling.Set(sRestored);
    }
}

void TransformProcessor::ProcessNode(FbxNode* node, const FbxAMatrix& convMatrix, double scale)
{
    if (!node) return;
    ProcessNodeRecursive(node, convMatrix, scale);
}

void TransformProcessor::ProcessNodeRecursive(FbxNode* node, const FbxAMatrix& convMatrix, double scale)
{
    // --- LclTranslation: rotate (axis swizzle) + scale ---
    {
        FbxDouble3 t = node->LclTranslation.Get();
        FbxVector4 tVec(t[0] * scale, t[1] * scale, t[2] * scale, 0.0);
        FbxVector4 tNew = convMatrix.MultT(tVec);
        node->LclTranslation.Set(FbxDouble3(tNew[0], tNew[1], tNew[2]));
    }

    // --- LclRotation: change of basis
    //   R_dst = M * R_src * M^(-1)  (column-vector convention)
    //
    //   NOTE: SetR/GetR のオイラー角ラウンドトリップは一意ではない（Y角が ±90° を
    //   超えるケースで等価な別解に収束することがある）。変換行列が単位行列の場合は
    //   値を保持するためスキップする。
    // ---
    if (!IsRotationIdentity(convMatrix))
    {
        FbxDouble3 rD3 = node->LclRotation.Get();
        FbxVector4 rVec(rD3[0], rD3[1], rD3[2], 0.0);

        // Build rotation-only matrix from Euler angles
        FbxAMatrix srcRot;
        srcRot.SetR(rVec);

        // Change of basis: M * R_src * M^(-1)
        FbxAMatrix convInv = convMatrix.Inverse();
        FbxAMatrix dstRot  = convMatrix * srcRot * convInv;

        FbxVector4 dstEuler = dstRot.GetR();
        node->LclRotation.Set(FbxDouble3(dstEuler[0], dstEuler[1], dstEuler[2]));
    }

    // --- LclScaling: permute axes (no sign, no scale factor)
    //   For each destination axis (row of convMatrix), the source axis
    //   is the column where the non-zero entry appears.
    // ---
    {
        FbxDouble3 s = node->LclScaling.Get();
        FbxDouble3 sNew(1.0, 1.0, 1.0);

        for (int dstAxis = 0; dstAxis < 3; ++dstAxis)
        {
            FbxVector4 row = convMatrix.GetRow(dstAxis);
            for (int srcAxis = 0; srcAxis < 3; ++srcAxis)
            {
                if (std::abs(row[srcAxis]) > 0.5)
                {
                    sNew[dstAxis] = s[srcAxis];
                    break;
                }
            }
        }
        node->LclScaling.Set(sNew);
    }

    // Recurse into children
    int childCount = node->GetChildCount();
    for (int i = 0; i < childCount; ++i)
        ProcessNodeRecursive(node->GetChild(i), convMatrix, scale);
}
