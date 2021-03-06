//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

// Constant data that varies per frame.

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbMaterial : register(b1)
{
	float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float  gRoughness;
	float4x4 gMatTransform;
    float gOutlineThreshold;
};

// Constant data that varies per material.
cbuffer cbPass : register(b2)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};
 
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    //Vector in the opposit direction of light
    float3 toLightW = normalize(-gLights[0].Direction);

    //Calculates the intensity of the light based on the angle between the normal and the light vector
    float intensity = 0.0f;
    intensity += dot(toLightW, pin.NormalW);

    //Create the initial pixel colour normally, multiplying the albedo by the light's diffuse colour
    float4 litColor = gDiffuseAlbedo * float4(gLights[0].Strength.rgb, 1.0f);

    //Creates a highlight based on material shininess and light, camera and normal vectors
    if (dot(normalize(toLightW + toEyeW), pin.NormalW) > (0.97f+(gRoughness*0.03f)))
        litColor.rgb = clamp((gLights[0].Strength*2),0.0f,1.0f);
    //Defines brackets of intensity used to define how illuminated a pixel fragment will be
    else if (intensity > 0.7f)
        litColor = float4(1.0f, 1.0f, 1.0f, 1.0f) * litColor;
    else if (intensity > 0.35f)
        litColor = float4(0.8f, 0.8f, 0.8f, 1.8f) * litColor;
    else if (intensity > 0.05f)
        litColor = float4(0.4f, 0.4f, 0.4f, 1.0f) * litColor;
    else if (intensity < 0.05f)
        litColor = float4(0.2f, 0.2f, 0.2f, 1.0f) * litColor;
    //Generates an outline based on the material and angle with the camera
    if (dot(toEyeW, pin.NormalW) < gOutlineThreshold) litColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    
    // Common convention to take alpha from diffuse material.
    litColor.a = gDiffuseAlbedo.a;

    return litColor;
}


