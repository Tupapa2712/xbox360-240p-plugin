#ifndef _SDL_shaders_src_h
#define _SDL_shaders_src_h

/* Passthrough: el UNICO shader que sigue viviendo embebido en el binario.
 *
 * Todos los demas (Sharp-Bilinear, LCD3x, Scanlines, CRT-Geom, CRT-Lottes,
 * CRT-Easymode, HQ2x/3x/4x, xBR...) se han sacado a ficheros de texto en
 * <appDir>\assets\shaders, en formato de preset estilo RetroArch (.hlslp +
 * .hlsl), y se cargan y compilan en runtime. Ver:
 *   - src/video/salvia_shader_api.h  (frontera app <-> backend de video)
 *   - src/video/shaderpreset.cpp     (descubrimiento, parser, LUTs)
 *   - utils/extract_shaders.py       (script que genero los assets iniciales)
 *
 * Este se queda aqui a proposito, como RED DE SEGURIDAD: en Xenon no hay
 * pipeline de funcion fija, asi que enganchar un pixel shader NULL da PANTALLA
 * NEGRA. initShaders() lo compila lo primero y lo usa siempre que un preset no
 * exista, no compile, o solo describa estado de sampler (Nearest y Bilinear,
 * que se diferencian unicamente en filter_linear0). */
const static char* g_strShaderNormalSource =
    " float fFilterType : register(c0);            "
    "                                              "
    " struct PS_IN                                 "
    " {                                            "
    "     float2 TexCoord : TEXCOORD0;             "
    " };                                           "
    "                                              "
    " sampler2D detail : register(s0);             "
    "                                              "
    " float4 main( PS_IN In ) : COLOR0             "
    " {                                            "
    "     float4 color = tex2D( detail, In.TexCoord ); "
    "     return color;                            "
    " }     ";

/* CRT 240p horizontal Sharp-Bilinear (area box filter) + strict Point-Y:
 * Estira horizontalmente resoluciones como 256px o 512px a 640x240 (4:3 completo)
 * sin shimmering en scroll ni desenfoque excesivo, manteniendo el eje Y 100% 1:1
 * (centro exacto de texel por scanline, cero sangrado vertical).
 * c1 = (texW, texH, displayW / texW, 0). Compilar con PS_FLAGS_FULL_PRECISION. */
const static char* g_strShaderCrtSharpXPointYSource =
    " float4 textureDims : register(c1);                                           \n"
    " struct PS_IN                                                                 \n"
    " {                                                                            \n"
    "     float2 TexCoord : TEXCOORD0;                                             \n"
    " };                                                                           \n"
    " sampler2D detail : register(s0);                                             \n"
    " float4 main( PS_IN In ) : COLOR0                                             \n"
    " {                                                                            \n"
    "     float tx = In.TexCoord.x * textureDims.x;                                \n"
    "     float p  = tx - 0.5;                                                     \n"
    "     float fl = floor(p);                                                     \n"
    "     float fr = p - fl;                                                       \n"
    "     float sx = max(textureDims.z, 1.0);                                      \n"
    "     float u  = (fl + saturate((fr - 0.5) * sx + 0.5) + 0.5) / textureDims.x; \n"
    "     float v  = (floor(In.TexCoord.y * textureDims.y) + 0.5) / textureDims.y; \n"
    "     return tex2D( detail, float2(u, v) );                                    \n"
    " }                                                                            \n";

#endif
