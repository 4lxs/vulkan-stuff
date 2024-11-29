#include "loadMesh.hpp"

#include <spdlog/spdlog.h>
#include <stb/stb_image.h>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <glm/gtx/quaternion.hpp>

#include "engine.hpp"

std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(
    std::filesystem::path const& filePath) {
  spdlog::info("Loading GLTF: {}", filePath.string());

  auto expectedData = fastgltf::GltfDataBuffer::FromPath(filePath);
  if (!bool(expectedData)) {
    spdlog::error("failed to read file: {}",
                  fastgltf::getErrorMessage(expectedData.error()));
    return std::nullopt;
  }
  auto& data = expectedData.get();

  constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers;

  fastgltf::Asset gltf;
  fastgltf::Parser parser{};

  auto load = parser.loadGltfBinary(data, filePath.parent_path(), gltfOptions);
  if (load) {
    gltf = std::move(load.get());
  } else {
    spdlog::error("Failed to load glTF: {} \n",
                  fastgltf::to_underlying(load.error()));
    return {};
  }

  std::vector<std::shared_ptr<MeshAsset>> meshes;

  std::vector<uint32_t> indices;
  std::vector<Vertex> vertices;
  for (fastgltf::Mesh& mesh : gltf.meshes) {
    // clear the mesh arrays each mesh, we dont want to merge them by error
    indices.clear();
    vertices.clear();

    std::vector<GeoSurface> surfaces;

    for (auto&& primitive : mesh.primitives) {
      GeoSurface newSurface{};
      newSurface.startIndex = (uint32_t)indices.size();
      newSurface.count =
          (uint32_t)gltf.accessors[primitive.indicesAccessor.value()].count;

      size_t initial_vtx = vertices.size();

      // load indexes
      {
        fastgltf::Accessor& indexaccessor =
            gltf.accessors[primitive.indicesAccessor.value()];
        indices.reserve(indices.size() + indexaccessor.count);

        fastgltf::iterateAccessor<std::uint32_t>(
            gltf, indexaccessor,
            [&](std::uint32_t idx) { indices.push_back(idx + initial_vtx); });
      }

      // load vertex positions
      {
        fastgltf::Accessor& posAccessor =
            gltf.accessors[primitive.findAttribute("POSITION")->accessorIndex];
        vertices.resize(vertices.size() + posAccessor.count);

        fastgltf::iterateAccessorWithIndex<glm::vec3>(
            gltf, posAccessor, [&](glm::vec3 pos, size_t index) {
              Vertex newvtx;
              newvtx.position = pos;
              // newvtx.normal = {1, 0, 0};
              newvtx.color = glm::vec4{1.};
              vertices[initial_vtx + index] = newvtx;
            });
      }

      // fastgltf::Attribute* normals = primitive.findAttribute("NORMAL");
      // if (normals != primitive.attributes.end()) {
      //   fastgltf::iterateAccessorWithIndex<glm::vec3>(
      //       gltf, gltf.accessors[(*normals).second],
      //       [&](glm::vec3 v, size_t index) {
      //         vertices[initial_vtx + index].normal = v;
      //       });
      // }

      // load UVs
      fastgltf::Attribute* texCoord = primitive.findAttribute("TEXCOORD_0");
      if (texCoord != primitive.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec2>(
            gltf, gltf.accessors[(*texCoord).accessorIndex],
            [&](glm::vec2 texc, size_t index) {
              vertices[initial_vtx + index].texCoord = texc;
            });
      }

      // load vertex colors
      fastgltf::Attribute* colors = primitive.findAttribute("COLOR_0");
      if (colors != primitive.attributes.end()) {
        fastgltf::iterateAccessorWithIndex<glm::vec4>(
            gltf, gltf.accessors[(*colors).accessorIndex],
            [&](glm::vec4 col, size_t index) {
              vertices[initial_vtx + index].color = col;
            });
      }
      surfaces.push_back(newSurface);
    }

    // // display the vertex normals
    // constexpr bool OverrideColors = true;
    // if (OverrideColors) {
    //   for (Vertex& vtx : vertices) {
    //     vtx.color = glm::vec4(vtx.normal, 1.f);
    //   }
    // }
    auto asset = std::make_shared<MeshAsset>(
        std::string(mesh.name), std::move(surfaces),
        Engine::instance().upload_mesh(vertices, indices));
    meshes.emplace_back(std::move(asset));
  }

  return meshes;
}
