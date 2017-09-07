struct VertexIn
{
    float2 Position : POSITION0;
    float2 TexCoord : TEXCOORD0;
};

struct VertexOut
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

void main(in VertexIn input, out VertexOut output)
{
    output.Position = float4(input.Position, 0, 1);
    output.TexCoord = input.TexCoord;
}
