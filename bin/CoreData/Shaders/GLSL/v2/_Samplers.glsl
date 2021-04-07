#ifndef _SAMPLERS_GLSL_
#define _SAMPLERS_GLSL_

#ifndef _CONFIG_GLSL_
    #error Include _Config.glsl before _Samplers.glsl
#endif

#ifdef URHO3D_PIXEL_SHADER

SAMPLER(0, sampler2D sDiffMap)
SAMPLER(0, samplerCube sDiffCubeMap)
SAMPLER(1, sampler2D sNormalMap)
SAMPLER(2, sampler2D sSpecMap)
SAMPLER(3, sampler2D sEmissiveMap)
SAMPLER(4, sampler2D sEnvMap)
SAMPLER(4, samplerCube sEnvCubeMap)
SAMPLER(8, sampler2D sLightRampMap)
SAMPLER(9, sampler2D sLightSpotMap)
SAMPLER(9, samplerCube sLightCubeMap)
#if defined(URHO3D_VARIANCE_SHADOW_MAP) || defined(GL_ES)
    SAMPLER_HIGHP(10, sampler2D sShadowMap)
#else
    SAMPLER_HIGHP(10, sampler2DShadow sShadowMap)
#endif
#ifndef GL_ES
    SAMPLER(5, sampler3D sVolumeMap)
    SAMPLER(13, sampler2D sDepthBuffer)
    SAMPLER(15, samplerCube sZoneCubeMap)
    SAMPLER(15, sampler3D sZoneVolumeMap)
#endif

#ifdef URHO3D_MATERIAL_HAS_DIFFUSE
    #if URHO3D_MATERIAL_DIFFUSE_HINT == 0
        #define DiffMap_ToGamma(color)  Texture_ToGamma_0(color)
        #define DiffMap_ToLinear(color) Texture_ToLinear_0(color)
        #define DiffMap_ToLight(color)  Texture_ToLight_0(color)
    #elif URHO3D_MATERIAL_DIFFUSE_HINT == 1
        #define DiffMap_ToGamma(color)  Texture_ToGamma_1(color)
        #define DiffMap_ToLinear(color) Texture_ToLinear_1(color)
        #define DiffMap_ToLight(color)  Texture_ToLight_1(color)
    #endif
#endif

#ifdef URHO3D_MATERIAL_HAS_EMISSIVE
    #if URHO3D_MATERIAL_EMISSIVE_HINT == 0
        #define EmissiveMap_ToGamma(color)  Texture_ToGamma_0(color)
        #define EmissiveMap_ToLinear(color) Texture_ToLinear_0(color)
        #define EmissiveMap_ToLight(color)  Texture_ToLight_0(color)
    #elif URHO3D_MATERIAL_EMISSIVE_HINT == 1
        #define EmissiveMap_ToGamma(color)  Texture_ToGamma_1(color)
        #define EmissiveMap_ToLinear(color) Texture_ToLinear_1(color)
        #define EmissiveMap_ToLight(color)  Texture_ToLight_1(color)
    #endif
#endif

vec3 DecodeNormal(vec4 normalInput)
{
    #ifdef PACKEDNORMAL
        vec3 normal;
        normal.xy = normalInput.ag * 2.0 - 1.0;
        normal.z = sqrt(max(1.0 - dot(normal.xy, normal.xy), 0.0));
        return normal;
    #else
        return normalize(normalInput.rgb * 2.0 - 1.0);
    #endif
}

float ReconstructDepth(float hwDepth)
{
    return dot(vec2(hwDepth, cDepthReconstruct.y / (hwDepth - cDepthReconstruct.x)), cDepthReconstruct.zw);
}

#endif // URHO3D_PIXEL_SHADER
#endif // _SAMPLERS_GLSL_