# Static library: all sources except main.cpp (shared with tests)
add_library(fbxconverter_lib STATIC
    ArgumentParser.cpp
    ArgumentParser.h
    AnimProcessor.cpp
    AnimProcessor.h
    ConvertOptions.h
    FbxAxisUnitConverter.cpp
    FbxAxisUnitConverter.h
    FbxFileIO.cpp
    FbxFileIO.h
    GeometryProcessor.cpp
    GeometryProcessor.h
    TransformProcessor.cpp
    TransformProcessor.h
)

target_include_directories(fbxconverter_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(fbxconverter_lib PUBLIC
    FBXSDK::FBXSDK
)

# Executable: main.cpp only, links the library
add_executable(FbxAxisUnitConverter main.cpp)

target_link_libraries(FbxAxisUnitConverter PRIVATE
    fbxconverter_lib
)
