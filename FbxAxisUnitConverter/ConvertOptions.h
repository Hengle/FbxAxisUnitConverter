#pragma once
#include <string>
#include <optional>
#include <fbxsdk.h>

struct AxisVector
{
    int axis = 1;  // 0=X, 1=Y, 2=Z
    int sign = 1;  // +1 or -1

    bool IsOrthogonalTo(const AxisVector& other) const
    {
        return axis != other.axis;
    }
};

struct ConvertOptions
{
    std::string inputPath;
    std::string outputPath;

    std::optional<AxisVector> dstUp;
    std::optional<AxisVector> dstForward;

    std::optional<AxisVector> srcUp;
    std::optional<AxisVector> srcForward;

    std::optional<FbxSystemUnit> srcUnit;
    std::optional<FbxSystemUnit> dstUnit;
};
