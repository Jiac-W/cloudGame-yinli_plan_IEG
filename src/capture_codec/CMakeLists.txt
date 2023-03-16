cmake_minimum_required(VERSION 2.8.12)
project(capture_codec)

# message("default build type is ${CMAKE_BUILD_TYPE}")
# set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE RELEASE)
message("after set, build type is ${CMAKE_BUILD_TYPE}")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# 设置ffmpeg依赖库及头文件所在目录，并存进指定变量
#for Ubuntu16.04 ffmpeg4.3.2
set(ffmpeg_headers_DIR /usr/include/x86_64-linux-gnu)
set(ffmpeg_libs_DIR /usr/lib/x86_64-linux-gnu)

# # # # for 云服务器 CentOS 4.x 版本
# set(ffmpeg_libs_DIR /usr/local/ffmpeg/lib)
# set(ffmpeg_headers_DIR /usr/local/ffmpeg/include)

# 对于find_package找不到的外部依赖库，可以用add_library添加
# SHARED表示添加的是动态库
# IMPORTED表示是引入已经存在的动态库
add_library( avcodec SHARED IMPORTED)
add_library( avfilter SHARED IMPORTED )
add_library( swresample SHARED IMPORTED )
add_library( swscale SHARED IMPORTED )
add_library( avformat SHARED IMPORTED )
add_library( avutil SHARED IMPORTED )

#指定所添加依赖库的导入路径
set_target_properties( avcodec PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavcodec.so )
set_target_properties( avfilter PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavfilter.so )
set_target_properties( swresample PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libswresample.so )
set_target_properties( swscale PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libswscale.so )
set_target_properties( avformat PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavformat.so )
set_target_properties( avutil PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_DIR}/libavutil.so )

include_directories(
  include ${catkin_INCLUDE_DIRS}
  include ${ffmpeg_headers_DIR} )
link_directories(${ffmpeg_libs_DIR} )

add_compile_options(-std=c++11)

add_executable(stream_pusher_with_rtsp
  src/stream_pusher_with_rtsp.cpp
)
target_link_libraries(stream_pusher_with_rtsp
  avdevice avfilter swscale avformat avcodec avutil swresample
)

add_executable(stream_pusher_with_rtmp
  src/stream_pusher_with_rtmp.cpp
)
target_link_libraries(stream_pusher_with_rtmp
  avdevice avfilter swscale avformat avcodec avutil swresample
)

add_executable(cgi_server
  src/cgi_server.cpp
)
target_link_libraries(cgi_server
)

add_executable(get_time
  src/get_time.cpp
)
target_link_libraries(get_time
)