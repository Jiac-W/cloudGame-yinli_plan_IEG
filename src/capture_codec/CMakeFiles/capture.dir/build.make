# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/wu/tencent/video_capture/src/capture_codec

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/wu/tencent/video_capture/src/capture_codec

# Include any dependencies generated for this target.
include CMakeFiles/capture.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/capture.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/capture.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/capture.dir/flags.make

CMakeFiles/capture.dir/src/capture.cpp.o: CMakeFiles/capture.dir/flags.make
CMakeFiles/capture.dir/src/capture.cpp.o: src/capture.cpp
CMakeFiles/capture.dir/src/capture.cpp.o: CMakeFiles/capture.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/wu/tencent/video_capture/src/capture_codec/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/capture.dir/src/capture.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/capture.dir/src/capture.cpp.o -MF CMakeFiles/capture.dir/src/capture.cpp.o.d -o CMakeFiles/capture.dir/src/capture.cpp.o -c /home/wu/tencent/video_capture/src/capture_codec/src/capture.cpp

CMakeFiles/capture.dir/src/capture.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/capture.dir/src/capture.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/wu/tencent/video_capture/src/capture_codec/src/capture.cpp > CMakeFiles/capture.dir/src/capture.cpp.i

CMakeFiles/capture.dir/src/capture.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/capture.dir/src/capture.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/wu/tencent/video_capture/src/capture_codec/src/capture.cpp -o CMakeFiles/capture.dir/src/capture.cpp.s

# Object files for target capture
capture_OBJECTS = \
"CMakeFiles/capture.dir/src/capture.cpp.o"

# External object files for target capture
capture_EXTERNAL_OBJECTS =

capture: CMakeFiles/capture.dir/src/capture.cpp.o
capture: CMakeFiles/capture.dir/build.make
capture: /usr/lib/x86_64-linux-gnu/libavfilter.so
capture: /usr/lib/x86_64-linux-gnu/libswscale.so
capture: /usr/lib/x86_64-linux-gnu/libavformat.so
capture: /usr/lib/x86_64-linux-gnu/libavcodec.so
capture: /usr/lib/x86_64-linux-gnu/libavutil.so
capture: /usr/lib/x86_64-linux-gnu/libswresample.so
capture: CMakeFiles/capture.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/wu/tencent/video_capture/src/capture_codec/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable capture"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/capture.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/capture.dir/build: capture
.PHONY : CMakeFiles/capture.dir/build

CMakeFiles/capture.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/capture.dir/cmake_clean.cmake
.PHONY : CMakeFiles/capture.dir/clean

CMakeFiles/capture.dir/depend:
	cd /home/wu/tencent/video_capture/src/capture_codec && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/wu/tencent/video_capture/src/capture_codec /home/wu/tencent/video_capture/src/capture_codec /home/wu/tencent/video_capture/src/capture_codec /home/wu/tencent/video_capture/src/capture_codec /home/wu/tencent/video_capture/src/capture_codec/CMakeFiles/capture.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/capture.dir/depend

