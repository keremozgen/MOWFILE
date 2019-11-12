#
# project: MOWFILE
#
cmake_minimum_required(VERSION 2.8)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

# include_directories(src)

fips_setup(PROJECT MOWFILE)
fips_project(mowfile)
fips_begin_app(mowfile cmdline)
fips_vs_warning_level(3)
if (FIPS_MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
fips_files(mowfile.c mowfile.h)
if (FIPS_ANDROID)
        fips_dir(android)
        fips_files(android_native_app_glue.c android_native_app_glue.h)
        fips_libs(android log)
endif()
fips_end_app()
fips_finish()


