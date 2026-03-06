#pragma once
#include <fbxsdk.h>
#include <set>

class GeometryProcessor
{
public:
    explicit GeometryProcessor(bool flipWinding);

    void ProcessScene(FbxScene* scene, const FbxAMatrix& convMatrix, double scale);

private:
    void ProcessMesh(FbxMesh* mesh, const FbxAMatrix& convMatrix, const FbxAMatrix& convInv, double scale);
    void ProcessSkin(FbxMesh* mesh, const FbxAMatrix& convMatrix, const FbxAMatrix& convInv, double scale);
    void ProcessPoses(FbxScene* scene, const FbxAMatrix& convMatrix, const FbxAMatrix& convInv, double scale);
    void FlipWindingOrder(FbxMesh* mesh);

    bool mFlipWinding;
    std::set<FbxMesh*>    mProcessedMeshes;
    std::set<FbxCluster*> mProcessedClusters;
};
