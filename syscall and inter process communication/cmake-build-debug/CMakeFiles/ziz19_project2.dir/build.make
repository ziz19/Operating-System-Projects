# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = C:\Users\zhuan\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\173.4674.29\bin\cmake\bin\cmake.exe

# The command to remove a file.
RM = C:\Users\zhuan\AppData\Local\JetBrains\Toolbox\apps\CLion\ch-0\173.4674.29\bin\cmake\bin\cmake.exe -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\ComputerScience\CS1550\ziz19-project2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\ComputerScience\CS1550\ziz19-project2\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/ziz19_project2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ziz19_project2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ziz19_project2.dir/flags.make

CMakeFiles/ziz19_project2.dir/sys.c.obj: CMakeFiles/ziz19_project2.dir/flags.make
CMakeFiles/ziz19_project2.dir/sys.c.obj: ../sys.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\ComputerScience\CS1550\ziz19-project2\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/ziz19_project2.dir/sys.c.obj"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\ziz19_project2.dir\sys.c.obj   -c D:\ComputerScience\CS1550\ziz19-project2\sys.c

CMakeFiles/ziz19_project2.dir/sys.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ziz19_project2.dir/sys.c.i"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\ComputerScience\CS1550\ziz19-project2\sys.c > CMakeFiles\ziz19_project2.dir\sys.c.i

CMakeFiles/ziz19_project2.dir/sys.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ziz19_project2.dir/sys.c.s"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S D:\ComputerScience\CS1550\ziz19-project2\sys.c -o CMakeFiles\ziz19_project2.dir\sys.c.s

CMakeFiles/ziz19_project2.dir/sys.c.obj.requires:

.PHONY : CMakeFiles/ziz19_project2.dir/sys.c.obj.requires

CMakeFiles/ziz19_project2.dir/sys.c.obj.provides: CMakeFiles/ziz19_project2.dir/sys.c.obj.requires
	$(MAKE) -f CMakeFiles\ziz19_project2.dir\build.make CMakeFiles/ziz19_project2.dir/sys.c.obj.provides.build
.PHONY : CMakeFiles/ziz19_project2.dir/sys.c.obj.provides

CMakeFiles/ziz19_project2.dir/sys.c.obj.provides.build: CMakeFiles/ziz19_project2.dir/sys.c.obj


CMakeFiles/ziz19_project2.dir/prodcons.c.obj: CMakeFiles/ziz19_project2.dir/flags.make
CMakeFiles/ziz19_project2.dir/prodcons.c.obj: ../prodcons.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\ComputerScience\CS1550\ziz19-project2\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/ziz19_project2.dir/prodcons.c.obj"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\ziz19_project2.dir\prodcons.c.obj   -c D:\ComputerScience\CS1550\ziz19-project2\prodcons.c

CMakeFiles/ziz19_project2.dir/prodcons.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ziz19_project2.dir/prodcons.c.i"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\ComputerScience\CS1550\ziz19-project2\prodcons.c > CMakeFiles\ziz19_project2.dir\prodcons.c.i

CMakeFiles/ziz19_project2.dir/prodcons.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ziz19_project2.dir/prodcons.c.s"
	C:\MinGW\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S D:\ComputerScience\CS1550\ziz19-project2\prodcons.c -o CMakeFiles\ziz19_project2.dir\prodcons.c.s

CMakeFiles/ziz19_project2.dir/prodcons.c.obj.requires:

.PHONY : CMakeFiles/ziz19_project2.dir/prodcons.c.obj.requires

CMakeFiles/ziz19_project2.dir/prodcons.c.obj.provides: CMakeFiles/ziz19_project2.dir/prodcons.c.obj.requires
	$(MAKE) -f CMakeFiles\ziz19_project2.dir\build.make CMakeFiles/ziz19_project2.dir/prodcons.c.obj.provides.build
.PHONY : CMakeFiles/ziz19_project2.dir/prodcons.c.obj.provides

CMakeFiles/ziz19_project2.dir/prodcons.c.obj.provides.build: CMakeFiles/ziz19_project2.dir/prodcons.c.obj


# Object files for target ziz19_project2
ziz19_project2_OBJECTS = \
"CMakeFiles/ziz19_project2.dir/sys.c.obj" \
"CMakeFiles/ziz19_project2.dir/prodcons.c.obj"

# External object files for target ziz19_project2
ziz19_project2_EXTERNAL_OBJECTS =

ziz19_project2.exe: CMakeFiles/ziz19_project2.dir/sys.c.obj
ziz19_project2.exe: CMakeFiles/ziz19_project2.dir/prodcons.c.obj
ziz19_project2.exe: CMakeFiles/ziz19_project2.dir/build.make
ziz19_project2.exe: CMakeFiles/ziz19_project2.dir/linklibs.rsp
ziz19_project2.exe: CMakeFiles/ziz19_project2.dir/objects1.rsp
ziz19_project2.exe: CMakeFiles/ziz19_project2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\ComputerScience\CS1550\ziz19-project2\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable ziz19_project2.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\ziz19_project2.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ziz19_project2.dir/build: ziz19_project2.exe

.PHONY : CMakeFiles/ziz19_project2.dir/build

CMakeFiles/ziz19_project2.dir/requires: CMakeFiles/ziz19_project2.dir/sys.c.obj.requires
CMakeFiles/ziz19_project2.dir/requires: CMakeFiles/ziz19_project2.dir/prodcons.c.obj.requires

.PHONY : CMakeFiles/ziz19_project2.dir/requires

CMakeFiles/ziz19_project2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\ziz19_project2.dir\cmake_clean.cmake
.PHONY : CMakeFiles/ziz19_project2.dir/clean

CMakeFiles/ziz19_project2.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" D:\ComputerScience\CS1550\ziz19-project2 D:\ComputerScience\CS1550\ziz19-project2 D:\ComputerScience\CS1550\ziz19-project2\cmake-build-debug D:\ComputerScience\CS1550\ziz19-project2\cmake-build-debug D:\ComputerScience\CS1550\ziz19-project2\cmake-build-debug\CMakeFiles\ziz19_project2.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ziz19_project2.dir/depend

