struct VertexIn
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

Texture2D SrcImage;
sampler   SrcSampler;

float4 main(in VertexIn input) : SV_TARGET
{
    return float4(SrcImage.Sample(SrcSampler, input.TexCoord).rrr, 1);
}
