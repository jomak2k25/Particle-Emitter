#pragma once
#include "../../Common/d3dUtil.h"


struct ToonMaterialConstants : public MaterialConstants
{
	float OutlineThreshold = 0.0f;
};
struct ToonMaterial : public Material
{
	float OutlineThreshold = 0.0f;
};