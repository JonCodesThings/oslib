workspace "oslib_workspace"
  configurations { "Debug", "Release" }
  platforms { "x86", "x86_64" }
  location "build"

project "oslib"
  kind "StaticLib"
  language "C"
  includedirs { "." }
  filter "system:Windows"
    files { "include/oslib/*.h", "src/win32/*.c" }
  filter "system:Linux"
    files { "include/oslib/*.h", "src/posix/*.c" }