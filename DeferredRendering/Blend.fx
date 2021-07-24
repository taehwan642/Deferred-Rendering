
texture		diffuseTexture;

sampler DiffuseSampler = sampler_state
{
	texture = diffuseTexture;
	minfilter = linear;
	magfilter = linear;
	mipfilter = linear;
};

texture		lightAccumulatedTexture;

sampler ShadeSampler = sampler_state
{
	texture = lightAccumulatedTexture;
};

struct PS_IN
{
	vector		vPosition : POSITION;
	float2		vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	vector		vColor : COLOR0;
};


PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector		vDiffuseInfo = tex2D(DiffuseSampler, In.vTexUV);
	vector		vShadeInfo = tex2D(ShadeSampler, In.vTexUV);

	Out.vColor = vDiffuseInfo * (vShadeInfo);
	Out.vColor.a = 1.f;

	return Out;
}

technique Default_Technique
{
	pass Default_Rendering
	{
		ZwriteEnable = false;
		AlphaTestEnable = true;
		Alpharef = 0;
		AlphaFunc = Greater;

		VertexShader = NULL;
		PixelShader = compile ps_3_0 PS_MAIN();
	}
}