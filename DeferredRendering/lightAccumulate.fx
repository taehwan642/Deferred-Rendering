
vector		lightDir;

texture		NormalTexture;

sampler NormalSampler = sampler_state
{
	texture = NormalTexture;
};

struct PS_IN
{
	vector		vPosition : POSITION;
	float2		vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	vector		vShade : COLOR0;
};


PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;
	vector		vNormalInfo = tex2D(NormalSampler, In.vTexUV);
	vector		vNormal = vector(vNormalInfo.xyz * 2.f - 1.f, 0.f);
	Out.vShade = saturate(dot(normalize(lightDir) * -1.f, vNormal));
	return Out;
}

technique Default_Technique
{
	pass Default_Rendering
	{
		ZwriteEnable = false;
		VertexShader = NULL;
		PixelShader = compile ps_3_0 PS_MAIN();
	}
}