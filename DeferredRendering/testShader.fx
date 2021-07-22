float4x4 WVP;
float4x4 W;

texture Tex;

sampler Samp = sampler_state
{
	Texture = <Tex>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = NONE;

	AddressU = Clamp;
	AddressV = Clamp;
};

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexUV : TEXCOORD0;
};

struct VS_OUT
{
	float4			vPosition: POSITION;
	float4			vNormal : NORMAL;
	float2		 	vTexUV : TEXCOORD0;
};

VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;



	Out.vPosition = mul(vector(In.vPosition, 1.f), WVP);
	Out.vNormal = normalize(mul(vector(In.vNormal, 0.f), W));
	Out.vTexUV = In.vTexUV;

	return Out;
}

struct PS_IN
{
	float4			vPosition: POSITION;
	float4			vNormal : NORMAL;
	float2		 	vTexUV : TEXCOORD0;
};

struct PS_OUT
{
	float4	vDiffuse : COLOR0;
	float4  vDiffuse2 : COLOR1;
	float4	vNormal : COLOR2;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT		Out = (PS_OUT)0;

	Out.vDiffuse = tex2D(Samp, In.vTexUV);
	Out.vDiffuse.a = 1.f;

	Out.vDiffuse2 = tex2D(Samp, In.vTexUV);
	Out.vDiffuse2.a = 1.f;

	Out.vNormal = vector(In.vNormal.xyz * 0.5f + 0.5f, 0.f);

	return Out;
}


technique TShader
{
	pass P0
	{
		VertexShader = compile vs_3_0 VS_MAIN();
		PixelShader = compile ps_3_0 PS_MAIN();
	}
};