# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\diskio.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\ff.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\ffconf.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\sleep.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xilffs.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xilffs_config.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xilrsa.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xiltimer.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\include\\xtimer_config.h"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\lib\\libxilffs.a"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\lib\\libxilrsa.a"
  "S:\\Codes\\mandelbrot_super_accelerator\\vitis\\slower_mandelclock\\zynq_fsbl\\zynq_fsbl_bsp\\lib\\libxiltimer.a"
  )
endif()
