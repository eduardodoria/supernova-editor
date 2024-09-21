// Based on sokol_gfx.h

#ifndef GLLOADER_H
#define GLLOADER_H

#if defined(_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
    #define _SOKOL_USE_WIN32_GL_LOADER (1)
    #pragma comment (lib, "kernel32")   // GetProcAddress()
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #ifndef GL_SILENCE_DEPRECATION
        #define GL_SILENCE_DEPRECATION
    #endif
    #include <OpenGL/gl3.h>
#elif defined(__linux__) || defined(__unix__)
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
#endif

// optional GL loader definitions (only on Win32)
#if defined(_SOKOL_USE_WIN32_GL_LOADER)
    #define __gl_h_ 1
    #define __gl32_h_ 1
    #define __gl31_h_ 1
    #define __GL_H__ 1
    #define __glext_h_ 1
    #define __GLEXT_H_ 1
    #define __gltypes_h_ 1
    #define __glcorearb_h_ 1
    #define __gl_glcorearb_h_ 1
    #define GL_APIENTRY APIENTRY

    typedef unsigned int  GLenum;
    typedef unsigned int  GLuint;
    typedef int  GLsizei;
    typedef char  GLchar;
    typedef ptrdiff_t  GLintptr;
    typedef ptrdiff_t  GLsizeiptr;
    typedef double  GLclampd;
    typedef unsigned short  GLushort;
    typedef unsigned char  GLubyte;
    typedef unsigned char  GLboolean;
    typedef uint64_t  GLuint64;
    typedef double  GLdouble;
    typedef unsigned short  GLhalf;
    typedef float  GLclampf;
    typedef unsigned int  GLbitfield;
    typedef signed char  GLbyte;
    typedef short  GLshort;
    typedef void  GLvoid;
    typedef int64_t  GLint64;
    typedef float  GLfloat;
    typedef int  GLint;
    #define GL_INT_2_10_10_10_REV 0x8D9F
    #define GL_R32F 0x822E
    #define GL_PROGRAM_POINT_SIZE 0x8642
    #define GL_DEPTH_ATTACHMENT 0x8D00
    #define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
    #define GL_COLOR_ATTACHMENT2 0x8CE2
    #define GL_COLOR_ATTACHMENT0 0x8CE0
    #define GL_R16F 0x822D
    #define GL_COLOR_ATTACHMENT22 0x8CF6
    #define GL_DRAW_FRAMEBUFFER 0x8CA9
    #define GL_FRAMEBUFFER_COMPLETE 0x8CD5
    #define GL_NUM_EXTENSIONS 0x821D
    #define GL_INFO_LOG_LENGTH 0x8B84
    #define GL_VERTEX_SHADER 0x8B31
    #define GL_INCR 0x1E02
    #define GL_DYNAMIC_DRAW 0x88E8
    #define GL_STATIC_DRAW 0x88E4
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
    #define GL_TEXTURE_CUBE_MAP 0x8513
    #define GL_FUNC_SUBTRACT 0x800A
    #define GL_FUNC_REVERSE_SUBTRACT 0x800B
    #define GL_CONSTANT_COLOR 0x8001
    #define GL_DECR_WRAP 0x8508
    #define GL_R8 0x8229
    #define GL_LINEAR_MIPMAP_LINEAR 0x2703
    #define GL_ELEMENT_ARRAY_BUFFER 0x8893
    #define GL_SHORT 0x1402
    #define GL_DEPTH_TEST 0x0B71
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
    #define GL_LINK_STATUS 0x8B82
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
    #define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
    #define GL_RGBA16F 0x881A
    #define GL_CONSTANT_ALPHA 0x8003
    #define GL_READ_FRAMEBUFFER 0x8CA8
    #define GL_TEXTURE0 0x84C0
    #define GL_TEXTURE_MIN_LOD 0x813A
    #define GL_CLAMP_TO_EDGE 0x812F
    #define GL_UNSIGNED_SHORT_5_6_5 0x8363
    #define GL_TEXTURE_WRAP_R 0x8072
    #define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
    #define GL_NEAREST_MIPMAP_NEAREST 0x2700
    #define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
    #define GL_SRC_ALPHA_SATURATE 0x0308
    #define GL_STREAM_DRAW 0x88E0
    #define GL_ONE 1
    #define GL_NEAREST_MIPMAP_LINEAR 0x2702
    #define GL_RGB10_A2 0x8059
    #define GL_RGBA8 0x8058
    #define GL_SRGB8_ALPHA8 0x8C43
    #define GL_COLOR_ATTACHMENT1 0x8CE1
    #define GL_RGBA4 0x8056
    #define GL_RGB8 0x8051
    #define GL_ARRAY_BUFFER 0x8892
    #define GL_STENCIL 0x1802
    #define GL_TEXTURE_2D 0x0DE1
    #define GL_DEPTH 0x1801
    #define GL_FRONT 0x0404
    #define GL_STENCIL_BUFFER_BIT 0x00000400
    #define GL_REPEAT 0x2901
    #define GL_RGBA 0x1908
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
    #define GL_DECR 0x1E03
    #define GL_FRAGMENT_SHADER 0x8B30
    #define GL_FLOAT 0x1406
    #define GL_TEXTURE_MAX_LOD 0x813B
    #define GL_DEPTH_COMPONENT 0x1902
    #define GL_ONE_MINUS_DST_ALPHA 0x0305
    #define GL_COLOR 0x1800
    #define GL_TEXTURE_2D_ARRAY 0x8C1A
    #define GL_TRIANGLES 0x0004
    #define GL_UNSIGNED_BYTE 0x1401
    #define GL_TEXTURE_MAG_FILTER 0x2800
    #define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
    #define GL_NONE 0
    #define GL_SRC_COLOR 0x0300
    #define GL_BYTE 0x1400
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
    #define GL_LINE_STRIP 0x0003
    #define GL_TEXTURE_3D 0x806F
    #define GL_CW 0x0900
    #define GL_LINEAR 0x2601
    #define GL_RENDERBUFFER 0x8D41
    #define GL_GEQUAL 0x0206
    #define GL_COLOR_BUFFER_BIT 0x00004000
    #define GL_RGBA32F 0x8814
    #define GL_BLEND 0x0BE2
    #define GL_ONE_MINUS_SRC_ALPHA 0x0303
    #define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
    #define GL_TEXTURE_WRAP_T 0x2803
    #define GL_TEXTURE_WRAP_S 0x2802
    #define GL_TEXTURE_MIN_FILTER 0x2801
    #define GL_LINEAR_MIPMAP_NEAREST 0x2701
    #define GL_EXTENSIONS 0x1F03
    #define GL_NO_ERROR 0
    #define GL_REPLACE 0x1E01
    #define GL_KEEP 0x1E00
    #define GL_CCW 0x0901
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
    #define GL_RGB 0x1907
    #define GL_TRIANGLE_STRIP 0x0005
    #define GL_FALSE 0
    #define GL_ZERO 0
    #define GL_CULL_FACE 0x0B44
    #define GL_INVERT 0x150A
    #define GL_INT 0x1404
    #define GL_UNSIGNED_INT 0x1405
    #define GL_UNSIGNED_SHORT 0x1403
    #define GL_NEAREST 0x2600
    #define GL_SCISSOR_TEST 0x0C11
    #define GL_LEQUAL 0x0203
    #define GL_STENCIL_TEST 0x0B90
    #define GL_DITHER 0x0BD0
    #define GL_DEPTH_COMPONENT32F 0x8CAC
    #define GL_EQUAL 0x0202
    #define GL_FRAMEBUFFER 0x8D40
    #define GL_RGB5 0x8050
    #define GL_LINES 0x0001
    #define GL_DEPTH_BUFFER_BIT 0x00000100
    #define GL_SRC_ALPHA 0x0302
    #define GL_INCR_WRAP 0x8507
    #define GL_LESS 0x0201
    #define GL_MULTISAMPLE 0x809D
    #define GL_FRAMEBUFFER_BINDING 0x8CA6
    #define GL_BACK 0x0405
    #define GL_ALWAYS 0x0207
    #define GL_FUNC_ADD 0x8006
    #define GL_ONE_MINUS_DST_COLOR 0x0307
    #define GL_NOTEQUAL 0x0205
    #define GL_DST_COLOR 0x0306
    #define GL_COMPILE_STATUS 0x8B81
    #define GL_RED 0x1903
    #define GL_COLOR_ATTACHMENT3 0x8CE3
    #define GL_DST_ALPHA 0x0304
    #define GL_RGB5_A1 0x8057
    #define GL_GREATER 0x0204
    #define GL_POLYGON_OFFSET_FILL 0x8037
    #define GL_TRUE 1
    #define GL_NEVER 0x0200
    #define GL_POINTS 0x0000
    #define GL_ONE_MINUS_SRC_COLOR 0x0301
    #define GL_MIRRORED_REPEAT 0x8370
    #define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
    #define GL_R11F_G11F_B10F 0x8C3A
    #define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
    #define GL_RGB9_E5 0x8C3D
    #define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
    #define GL_RGBA32UI 0x8D70
    #define GL_RGB32UI 0x8D71
    #define GL_RGBA16UI 0x8D76
    #define GL_RGB16UI 0x8D77
    #define GL_RGBA8UI 0x8D7C
    #define GL_RGB8UI 0x8D7D
    #define GL_RGBA32I 0x8D82
    #define GL_RGB32I 0x8D83
    #define GL_RGBA16I 0x8D88
    #define GL_RGB16I 0x8D89
    #define GL_RGBA8I 0x8D8E
    #define GL_RGB8I 0x8D8F
    #define GL_RED_INTEGER 0x8D94
    #define GL_RG 0x8227
    #define GL_RG_INTEGER 0x8228
    #define GL_R8 0x8229
    #define GL_R16 0x822A
    #define GL_RG8 0x822B
    #define GL_RG16 0x822C
    #define GL_R16F 0x822D
    #define GL_R32F 0x822E
    #define GL_RG16F 0x822F
    #define GL_RG32F 0x8230
    #define GL_R8I 0x8231
    #define GL_R8UI 0x8232
    #define GL_R16I 0x8233
    #define GL_R16UI 0x8234
    #define GL_R32I 0x8235
    #define GL_R32UI 0x8236
    #define GL_RG8I 0x8237
    #define GL_RG8UI 0x8238
    #define GL_RG16I 0x8239
    #define GL_RG16UI 0x823A
    #define GL_RG32I 0x823B
    #define GL_RG32UI 0x823C
    #define GL_RGBA_INTEGER 0x8D99
    #define GL_R8_SNORM 0x8F94
    #define GL_RG8_SNORM 0x8F95
    #define GL_RGB8_SNORM 0x8F96
    #define GL_RGBA8_SNORM 0x8F97
    #define GL_R16_SNORM 0x8F98
    #define GL_RG16_SNORM 0x8F99
    #define GL_RGB16_SNORM 0x8F9A
    #define GL_RGBA16_SNORM 0x8F9B
    #define GL_RGBA16 0x805B
    #define GL_MAX_TEXTURE_SIZE 0x0D33
    #define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
    #define GL_MAX_3D_TEXTURE_SIZE 0x8073
    #define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
    #define GL_MAX_VERTEX_ATTRIBS 0x8869
    #define GL_CLAMP_TO_BORDER 0x812D
    #define GL_TEXTURE_BORDER_COLOR 0x1004
    #define GL_CURRENT_PROGRAM 0x8B8D
    #define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
    #define GL_UNPACK_ALIGNMENT 0x0CF5
    #define GL_FRAMEBUFFER_SRGB 0x8DB9
    #define GL_TEXTURE_COMPARE_MODE 0x884C
    #define GL_TEXTURE_COMPARE_FUNC 0x884D
    #define GL_COMPARE_REF_TO_TEXTURE 0x884E
    #define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
    #define GL_TEXTURE_MAX_LEVEL 0x813D
    #define GL_FRAMEBUFFER_UNDEFINED 0x8219
    #define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
    #define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
    #define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
    #define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
    #define GL_MAJOR_VERSION 0x821B
    #define GL_MINOR_VERSION 0x821C
#endif

#endif /* GLLOADER_H */