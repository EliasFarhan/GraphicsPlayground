#include <argh.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <log.h>
#include <fmt/core.h>
#include <filesystem>

namespace fs = std::filesystem;

void ConvertFile(const std::string_view inpath, const std::string_view outpath)
{
    if(!fs::exists(inpath))
    {
        core::LogError(fmt::format("Input file {} does not exist", inpath));
        return;
    }
    Assimp::Importer importer;
    const auto* scene = importer.ReadFile(inpath.data(), aiProcess_Triangulate | aiProcess_FlipUVs |
                                                         aiProcess_GenNormals);
    aiScene* tmpScene = nullptr;
    aiCopyScene(scene, &tmpScene);
    Assimp::Exporter exporter;
    exporter.Export(tmpScene, "gltf2", outpath.data());
    aiFreeScene(tmpScene);
}

int main(int argc, char** argv)
{
    argh::parser parser(argc, argv);
    const auto inpath = parser[1];
    const auto outpath = parser[2];
    //core::LogDebug(fmt::format("argNmb: {}, arg1: {}, arg2: {}", parser.size(), inpath, outpath));
    ConvertFile(inpath, outpath);
    return 0;
}