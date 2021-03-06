///////////////////////////////////////////////////////////////////////
// Wendy - a simple game engine
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////

#include <wendy/Config.hpp>

#include <wendy/Core.hpp>
#include <wendy/Bimap.hpp>

#include <wendy/Texture.hpp>
#include <wendy/RenderBuffer.hpp>
#include <wendy/Program.hpp>
#include <wendy/RenderContext.hpp>
#include <wendy/Pass.hpp>
#include <wendy/Material.hpp>

#include <algorithm>

#include <pugixml.hpp>

namespace wendy
{

namespace
{

Bimap<std::string, PolygonFace> polygonFaceMap;
Bimap<std::string, BlendFactor> blendFactorMap;
Bimap<std::string, FragmentFunction> functionMap;
Bimap<std::string, StencilOp> operationMap;
Bimap<std::string, FilterMode> filterModeMap;
Bimap<std::string, AddressMode> addressModeMap;
Bimap<std::string, RenderPhase> phaseMap;

void initializeMaps()
{
  if (polygonFaceMap.isEmpty())
  {
    polygonFaceMap["none"] = FACE_NONE;
    polygonFaceMap["front"] = FACE_FRONT;
    polygonFaceMap["back"] = FACE_BACK;
    polygonFaceMap["both"] = FACE_BOTH;
    polygonFaceMap.setDefaults("", FACE_NONE);
  }

  if (blendFactorMap.isEmpty())
  {
    blendFactorMap["zero"] = BLEND_ZERO;
    blendFactorMap["one"] = BLEND_ONE;
    blendFactorMap["src color"] = BLEND_SRC_COLOR;
    blendFactorMap["dst color"] = BLEND_DST_COLOR;
    blendFactorMap["src alpha"] = BLEND_SRC_ALPHA;
    blendFactorMap["dst alpha"] = BLEND_DST_ALPHA;
    blendFactorMap["one minus src color"] = BLEND_ONE_MINUS_SRC_COLOR;
    blendFactorMap["one minus dst color"] = BLEND_ONE_MINUS_DST_COLOR;
    blendFactorMap["one minus src alpha"] = BLEND_ONE_MINUS_SRC_ALPHA;
    blendFactorMap["one minus dst alpha"] = BLEND_ONE_MINUS_DST_ALPHA;
  }

  if (functionMap.isEmpty())
  {
    functionMap["never"] = ALLOW_NEVER;
    functionMap["always"] = ALLOW_ALWAYS;
    functionMap["equal"] = ALLOW_EQUAL;
    functionMap["not equal"] = ALLOW_NOT_EQUAL;
    functionMap["lesser"] = ALLOW_LESSER;
    functionMap["lesser or equal"] = ALLOW_LESSER_EQUAL;
    functionMap["greater"] = ALLOW_GREATER;
    functionMap["greater or equal"] = ALLOW_GREATER_EQUAL;
  }

  if (operationMap.isEmpty())
  {
    operationMap["keep"] = STENCIL_KEEP;
    operationMap["zero"] = STENCIL_ZERO;
    operationMap["replace"] = STENCIL_REPLACE;
    operationMap["increase"] = STENCIL_INCREASE;
    operationMap["decrease"] = STENCIL_DECREASE;
    operationMap["invert"] = STENCIL_INVERT;
    operationMap["increase wrap"] = STENCIL_INCREASE_WRAP;
    operationMap["decrease wrap"] = STENCIL_DECREASE_WRAP;
  }

  if (addressModeMap.isEmpty())
  {
    addressModeMap["wrap"] = ADDRESS_WRAP;
    addressModeMap["clamp"] = ADDRESS_CLAMP;
  }

  if (filterModeMap.isEmpty())
  {
    filterModeMap["nearest"] = FILTER_NEAREST;
    filterModeMap["bilinear"] = FILTER_BILINEAR;
    filterModeMap["trilinear"] = FILTER_TRILINEAR;
  }

  if (phaseMap.isEmpty())
  {
    phaseMap[""] = RENDER_DEFAULT;
    phaseMap["default"] = RENDER_DEFAULT;
    phaseMap["shadowmap"] = RENDER_SHADOWMAP;
  }
}

const uint MATERIAL_XML_VERSION = 12;

} /*namespace*/

bool parsePass(RenderContext& context, Pass& pass, pugi::xml_node root)
{
  initializeMaps();

  ResourceCache& cache = context.cache();

  if (pugi::xml_node node = root.child("blending"))
  {
    if (pugi::xml_attribute a = node.attribute("src"))
    {
      if (blendFactorMap.hasKey(a.value()))
        pass.setBlendFactors(blendFactorMap[a.value()], pass.dstFactor());
      else
      {
        logError("Invalid source blend factor %s", a.value());
        return false;
      }
    }

    if (pugi::xml_attribute a = node.attribute("dst"))
    {
      if (blendFactorMap.hasKey(a.value()))
        pass.setBlendFactors(pass.srcFactor(), blendFactorMap[a.value()]);
      else
      {
        logError("Invalid destination blend factor %s", a.value());
        return false;
      }
    }
  }

  if (pugi::xml_node node = root.child("color"))
  {
    if (pugi::xml_attribute a = node.attribute("writing"))
      pass.setColorWriting(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("multisampling"))
      pass.setMultisampling(a.as_bool());
  }

  if (pugi::xml_node node = root.child("depth"))
  {
    if (pugi::xml_attribute a = node.attribute("testing"))
      pass.setDepthTesting(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("writing"))
      pass.setDepthWriting(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("function"))
    {
      if (functionMap.hasKey(a.value()))
        pass.setDepthFunction(functionMap[a.value()]);
      else
      {
        logError("Invalid depth function %s", a.value());
        return false;
      }
    }
  }

  if (pugi::xml_node node = root.child("stencil"))
  {
    if (pugi::xml_attribute a = node.attribute("testing"))
      pass.setStencilTesting(a.as_bool());

    for (pugi::xml_node child : node.children())
    {
      const PolygonFace face = polygonFaceMap[child.name()];
      if (face == FACE_NONE)
        continue;

      if (pugi::xml_attribute a = node.attribute("mask"))
        pass.setStencilWriteMask(face, a.as_uint());

      if (pugi::xml_attribute a = node.attribute("reference"))
        pass.setStencilReference(face, a.as_uint());

      if (pugi::xml_attribute a = node.attribute("stencilFail"))
      {
        if (functionMap.hasKey(a.value()))
          pass.setStencilFailOperation(face, operationMap[a.value()]);
        else
        {
          logError("Invalid stencil fail operation %s", a.value());
          return false;
        }
      }

      if (pugi::xml_attribute a = node.attribute("depthFail"))
      {
        if (functionMap.hasKey(a.value()))
          pass.setDepthFailOperation(face, operationMap[a.value()]);
        else
        {
          logError("Invalid depth fail operation %s", a.value());
          return false;
        }
      }

      if (pugi::xml_attribute a = node.attribute("depthPass"))
      {
        if (functionMap.hasKey(a.value()))
          pass.setDepthPassOperation(face, operationMap[a.value()]);
        else
        {
          logError("Invalid depth pass operation %s", a.value());
          return false;
        }
      }

      if (pugi::xml_attribute a = node.attribute("function"))
      {
        if (functionMap.hasKey(a.value()))
          pass.setStencilFunction(face, functionMap[a.value()]);
        else
        {
          logError("Invalid stencil function %s", a.value());
          return false;
        }
      }
    }
  }

  if (pugi::xml_node node = root.child("polygon"))
  {
    if (pugi::xml_attribute a = node.attribute("wireframe"))
      pass.setWireframe(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("cull"))
    {
      if (polygonFaceMap.hasKey(a.value()))
        pass.setCullFace(polygonFaceMap[a.value()]);
      else
      {
        logError("Invalid cull face %s", a.value());
        return false;
      }
    }
  }

  if (pugi::xml_node node = root.child("line"))
  {
    if (pugi::xml_attribute a = node.attribute("smoothing"))
      pass.setLineSmoothing(a.as_bool());

    if (pugi::xml_attribute a = node.attribute("width"))
      pass.setLineWidth(a.as_float());
  }

  if (pugi::xml_node node = root.child("program"))
  {
    const std::string vertexShaderName(node.attribute("vs").value());
    if (vertexShaderName.empty())
    {
      logError("No vertex shader specified");
      return false;
    }

    const std::string fragmentShaderName(node.attribute("fs").value());
    if (fragmentShaderName.empty())
    {
      logError("No fragment shader specified");
      return false;
    }

    Ref<Program> program = Program::read(context,
                                         vertexShaderName,
                                         fragmentShaderName);
    if (!program)
    {
      logError("Failed to load program");
      return false;
    }

    pass.setProgram(program);

    for (auto u : node.children("uniform"))
    {
      const std::string uniformName(u.attribute("name").value());
      if (uniformName.empty())
      {
        logWarning("Program %s lists unnamed uniform",
                   program->name().c_str());

        continue;
      }

      const Uniform* uniform = program->findUniform(uniformName.c_str());
      if (!uniform)
      {
        logWarning("Program %s does not have uniform %s",
                   program->name().c_str(),
                   uniformName.c_str());

        continue;
      }

      if (uniform->isSampler())
      {
        Ref<Texture> texture;

        if (pugi::xml_attribute a = u.attribute("image"))
        {
          TextureParams params(TextureType(uniform->type()), TF_NONE);

          if (u.attribute("mipmapped").as_bool())
            params.flags |= TF_MIPMAPPED;

          if (u.attribute("sRGB").as_bool())
            params.flags |= TF_SRGB;

          if (pugi::xml_attribute a = u.attribute("filter"))
          {
            if (filterModeMap.hasKey(a.value()))
              params.filterMode = filterModeMap[a.value()];
            else
            {
              logError("Invalid filter mode name %s", a.value());
              return false;
            }
          }

          if (pugi::xml_attribute a = u.attribute("address"))
          {
            if (addressModeMap.hasKey(a.value()))
              params.addressMode = addressModeMap[a.value()];
            else
            {
              logError("Invalid address mode name %s", a.value());
              return false;
            }
          }

          if (pugi::xml_attribute a = u.attribute("anisotropy"))
            params.maxAnisotropy = a.as_float();

          texture = Texture::read(context, params, a.value());
        }
        else if (pugi::xml_attribute a = u.attribute("texture"))
          texture = cache.find<Texture>(a.value());
        else
        {
          logError("No texture specified for uniform %s of program %s",
                   uniformName.c_str(),
                   program->name().c_str());

          return false;
        }

        if (!texture)
        {
          logError("Failed to find texture for uniform %s of program %s",
                   uniformName.c_str(),
                   program->name().c_str());

          return false;
        }

        pass.setUniformTexture(uniformName.c_str(), texture);
      }
      else
      {
        pugi::xml_attribute attribute = u.attribute("value");
        if (!attribute)
        {
          logError("Missing value for uniform %s of program %s",
                   uniformName.c_str(),
                   program->name().c_str());

          return false;
        }

        switch (uniform->type())
        {
          case UNIFORM_INT:
            pass.setUniformState(uniformName.c_str(), attribute.as_int());
            break;
          case UNIFORM_UINT:
            pass.setUniformState(uniformName.c_str(), attribute.as_uint());
            break;
          case UNIFORM_FLOAT:
            pass.setUniformState(uniformName.c_str(), attribute.as_float());
            break;
          case UNIFORM_VEC2:
            pass.setUniformState(uniformName.c_str(), vec2Cast(attribute.value()));
            break;
          case UNIFORM_VEC3:
            pass.setUniformState(uniformName.c_str(), vec3Cast(attribute.value()));
            break;
          case UNIFORM_VEC4:
            pass.setUniformState(uniformName.c_str(), vec4Cast(attribute.value()));
            break;
          case UNIFORM_MAT2:
            pass.setUniformState(uniformName.c_str(), mat2Cast(attribute.value()));
            break;
          case UNIFORM_MAT3:
            pass.setUniformState(uniformName.c_str(), mat3Cast(attribute.value()));
            break;
          case UNIFORM_MAT4:
            pass.setUniformState(uniformName.c_str(), mat4Cast(attribute.value()));
            break;
        }
      }
    }
  }

  return true;
}

Ref<Material> Material::create(const ResourceInfo& info, RenderContext& context)
{
  return new Material(info);
}

Ref<Material> Material::read(RenderContext& context, const std::string& name)
{
  initializeMaps();

  if (Material* cached = context.cache().find<Material>(name))
    return cached;

  const Path path = context.cache().findFile(name);
  if (path.isEmpty())
  {
    logError("Failed to find material %s", name.c_str());
    return nullptr;
  }

  pugi::xml_document document;

  const pugi::xml_parse_result result = document.load_file(path.name().c_str());
  if (!result)
  {
    logError("Failed to load material %s: %s",
             name.c_str(),
             result.description());
    return nullptr;
  }

  pugi::xml_node root = document.child("material");
  if (!root || root.attribute("version").as_uint() != MATERIAL_XML_VERSION)
  {
    logError("Material file format mismatch in %s", name.c_str());
    return nullptr;
  }

  Ref<Material> material = Material::create(ResourceInfo(context.cache(), name, path), context);

  for (auto pn : root.children("pass"))
  {
    const std::string phaseName(pn.attribute("phase").value());
    if (!phaseMap.hasKey(phaseName))
    {
      logError("Invalid render phase %s in material %s",
               phaseName.c_str(),
               name.c_str());
      return nullptr;
    }

    if (!parsePass(context, material->pass(phaseMap[phaseName]), pn))
    {
      logError("Failed to parse pass for material %s", name.c_str());
      return nullptr;
    }
  }

  return material;
}

Material::Material(const ResourceInfo& info):
  Resource(info)
{
}

} /*namespace wendy*/

