#include "TransformProcessor.h"
#include <cmath>

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
    //   R_dst = M^(-1) * R_src * M  (row-vector convention: v_dst = v_src * M)
    //   with column-vector (FBX): equivalent to M * R_src * M^(-1)
    // ---
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
