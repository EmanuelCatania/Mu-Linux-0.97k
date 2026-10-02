# lib/libjpeg/CMakeLists.txt — IJG libjpeg 6b como librería estática "jpeg".
#
# Replica lib/libjpeg/libjpeg.vcxproj: mismas fuentes (en el mismo orden),
# /W2, warnings 4996/4244/4018 desactivados, WIN32;_DEBUG|NDEBUG;_LIB;_MBCS.
# Se incluye desde el CMakeLists.txt raíz (usa MU_COMMON_COMPILE_OPTIONS).

set(JPEG_SOURCES
    jpeg-6b/jcapimin.c
    jpeg-6b/jcapistd.c
    jpeg-6b/jccoefct.c
    jpeg-6b/jccolor.c
    jpeg-6b/jcdctmgr.c
    jpeg-6b/jchuff.c
    jpeg-6b/jcinit.c
    jpeg-6b/jcmainct.c
    jpeg-6b/jcmarker.c
    jpeg-6b/jcmaster.c
    jpeg-6b/jcomapi.c
    jpeg-6b/jcparam.c
    jpeg-6b/jcphuff.c
    jpeg-6b/jcprepct.c
    jpeg-6b/jcsample.c
    jpeg-6b/jctrans.c
    jpeg-6b/jdapimin.c
    jpeg-6b/jdapistd.c
    jpeg-6b/jdatadst.c
    jpeg-6b/jdatasrc.c
    jpeg-6b/jdcoefct.c
    jpeg-6b/jdcolor.c
    jpeg-6b/jddctmgr.c
    jpeg-6b/jdhuff.c
    jpeg-6b/jdinput.c
    jpeg-6b/jdmainct.c
    jpeg-6b/jdmarker.c
    jpeg-6b/jdmaster.c
    jpeg-6b/jdmerge.c
    jpeg-6b/jdphuff.c
    jpeg-6b/jdpostct.c
    jpeg-6b/jdsample.c
    jpeg-6b/jdtrans.c
    jpeg-6b/jerror.c
    jpeg-6b/jfdctflt.c
    jpeg-6b/jfdctfst.c
    jpeg-6b/jfdctint.c
    jpeg-6b/jidctflt.c
    jpeg-6b/jidctfst.c
    jpeg-6b/jidctint.c
    jpeg-6b/jidctred.c
    jpeg-6b/jquant1.c
    jpeg-6b/jquant2.c
    jpeg-6b/jutils.c
    jpeg-6b/jmemmgr.c
    jpeg-6b/jmemansi.c
)

set(JPEG_HEADERS
    jpeg-6b/jconfig.h
    jpeg-6b/jpeglib.h
    jpeg-6b/jmorecfg.h
    jpeg-6b/jerror.h
    jpeg-6b/jpegint.h
    jpeg-6b/jdct.h
    jpeg-6b/jdhuff.h
    jpeg-6b/jchuff.h
    jpeg-6b/jinclude.h
    jpeg-6b/jmemsys.h
    jpeg-6b/jversion.h
)

add_library(jpeg STATIC ${JPEG_SOURCES} ${JPEG_HEADERS})

source_group("Source Files" FILES ${JPEG_SOURCES})
source_group("Header Files" FILES ${JPEG_HEADERS})

set_target_properties(jpeg PROPERTIES
    OUTPUT_NAME jpeg
    FOLDER lib
)

target_include_directories(jpeg PRIVATE jpeg-6b)

target_compile_definitions(jpeg PRIVATE
    WIN32
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _LIB
    _MBCS
)

target_compile_options(jpeg PRIVATE
    /W2
    /wd4996
    /wd4244
    /wd4018
    ${MU_COMMON_COMPILE_OPTIONS}
    # libjpeg.vcxproj no desactiva BasicRuntimeChecks: en Debug queda /RTC1.
    $<$<CONFIG:Debug>:/RTC1>
)

# WholeProgramOptimization en Release: la .lib se arma con /LTCG.
set_property(TARGET jpeg APPEND PROPERTY STATIC_LIBRARY_OPTIONS
    $<$<CONFIG:Release>:/LTCG>)
