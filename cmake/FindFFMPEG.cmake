# FindFFMPEG - defines FFMPEG libraries
# FFMPEG_INCLUDE_DIRS -> Folder for FFMPEG includes
# FFMPEG_LIBRARIES -> libraries to load

set(FFMPEG_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include/ffmpeg)
include_directories(${FFMPEG_INCLUDE_DIRS})

set(FFMPEG_LIBRARIES avcodec avdevice avfilter avformat avutil postproc swresample swscale)

