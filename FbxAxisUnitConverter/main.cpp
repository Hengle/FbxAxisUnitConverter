#include "ArgumentParser.h"
#include "FbxAxisUnitConverter.h"
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        ConvertOptions opts = ArgumentParser::Parse(argc, argv);
        FbxAxisUnitConverter converter(opts);
        return converter.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Error] " << e.what() << "\n";
        return 1;
    }
}
