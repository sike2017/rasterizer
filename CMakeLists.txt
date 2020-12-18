# CMakeList.txt: rasterizer 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("rasterizer")
include_directories(.) # 把 CMakeListis.txt 所在目录引入 include 查找路径

# 将源代码添加到此项目的可执行文件。
add_executable (rasterizer "stb/stb_image.h" "stb/stb_image_write.h" "display/image_png.cpp" "display/image_png.h" "display/painter.cpp" "display/painter.h" "display/wdisplayer.cpp" "display/wdisplayer.h" "log/log.cpp" "log/log.h" "rasterizer.cpp" "rasterizer.h" "main.cpp" "color.h")

# TODO: 如有需要，请添加测试并安装目标。
