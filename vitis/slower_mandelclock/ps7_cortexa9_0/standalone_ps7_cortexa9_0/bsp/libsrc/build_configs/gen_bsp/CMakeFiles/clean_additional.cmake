# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\ps7_cortexa9_0\\standalone_ps7_cortexa9_0\\bsp\\include\\sleep.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\ps7_cortexa9_0\\standalone_ps7_cortexa9_0\\bsp\\include\\xiltimer.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\ps7_cortexa9_0\\standalone_ps7_cortexa9_0\\bsp\\include\\xtimer_config.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\ps7_cortexa9_0\\standalone_ps7_cortexa9_0\\bsp\\lib\\libxiltimer.a"
  )
endif()
