# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\include\\lwipopts.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\include\\sleep.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\include\\xemac_ieee_reg.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\include\\xemacpsif_hw.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\include\\xiltimer.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\include\\xlwipconfig.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\include\\xtimer_config.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\lib\\liblwip220.a"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\axi_blot\\ps7_cortexa9_1\\bsp_axi\\bsp\\lib\\libxiltimer.a"
  )
endif()
