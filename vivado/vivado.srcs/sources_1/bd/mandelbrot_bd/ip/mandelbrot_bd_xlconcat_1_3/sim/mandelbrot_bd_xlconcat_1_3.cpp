// (c) Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// (c) Copyright 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
// 
// This file contains confidential and proprietary information
// of AMD and is protected under U.S. and international copyright
// and other intellectual property laws.
// 
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// AMD, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND AMD HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) AMD shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or AMD had been advised of the
// possibility of the same.
// 
// CRITICAL APPLICATIONS
// AMD products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of AMD products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
// 
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
// 
// DO NOT MODIFY THIS FILE.


#include "mandelbrot_bd_xlconcat_1_3_sc.h"

#include "mandelbrot_bd_xlconcat_1_3.h"

#include "mandelbrot_bd_xlconcat_1_3_core.h"

#include <map>
#include <string>





#ifdef XILINX_SIMULATOR
mandelbrot_bd_xlconcat_1_3::mandelbrot_bd_xlconcat_1_3(const sc_core::sc_module_name& nm) : mandelbrot_bd_xlconcat_1_3_sc(nm), In0("In0"), In1("In1"), dout("dout")
{

  // initialize pins
  mp_impl->In0(In0);
  mp_impl->In1(In1);
  mp_impl->dout(dout);

}

void mandelbrot_bd_xlconcat_1_3::before_end_of_elaboration()
{
}

#endif // XILINX_SIMULATOR




#ifdef XM_SYSTEMC
mandelbrot_bd_xlconcat_1_3::mandelbrot_bd_xlconcat_1_3(const sc_core::sc_module_name& nm) : mandelbrot_bd_xlconcat_1_3_sc(nm), In0("In0"), In1("In1"), dout("dout")
{

  // initialize pins
  mp_impl->In0(In0);
  mp_impl->In1(In1);
  mp_impl->dout(dout);

}

void mandelbrot_bd_xlconcat_1_3::before_end_of_elaboration()
{
}

#endif // XM_SYSTEMC




#ifdef RIVIERA
mandelbrot_bd_xlconcat_1_3::mandelbrot_bd_xlconcat_1_3(const sc_core::sc_module_name& nm) : mandelbrot_bd_xlconcat_1_3_sc(nm), In0("In0"), In1("In1"), dout("dout")
{

  // initialize pins
  mp_impl->In0(In0);
  mp_impl->In1(In1);
  mp_impl->dout(dout);

}

void mandelbrot_bd_xlconcat_1_3::before_end_of_elaboration()
{
}

#endif // RIVIERA




#ifdef VCSSYSTEMC
mandelbrot_bd_xlconcat_1_3::mandelbrot_bd_xlconcat_1_3(const sc_core::sc_module_name& nm) : mandelbrot_bd_xlconcat_1_3_sc(nm),  In0("In0"), In1("In1"), dout("dout")
{
  // initialize pins
  mp_impl->In0(In0);
  mp_impl->In1(In1);
  mp_impl->dout(dout);

  // Instantiate Socket Stubs


}

void mandelbrot_bd_xlconcat_1_3::before_end_of_elaboration()
{
}

#endif // VCSSYSTEMC




#ifdef MTI_SYSTEMC
mandelbrot_bd_xlconcat_1_3::mandelbrot_bd_xlconcat_1_3(const sc_core::sc_module_name& nm) : mandelbrot_bd_xlconcat_1_3_sc(nm),  In0("In0"), In1("In1"), dout("dout")
{
  // initialize pins
  mp_impl->In0(In0);
  mp_impl->In1(In1);
  mp_impl->dout(dout);

  // Instantiate Socket Stubs


}

void mandelbrot_bd_xlconcat_1_3::before_end_of_elaboration()
{
}

#endif // MTI_SYSTEMC




mandelbrot_bd_xlconcat_1_3::~mandelbrot_bd_xlconcat_1_3()
{
}

#ifdef MTI_SYSTEMC
SC_MODULE_EXPORT(mandelbrot_bd_xlconcat_1_3);
#endif

#ifdef XM_SYSTEMC
XMSC_MODULE_EXPORT(mandelbrot_bd_xlconcat_1_3);
#endif

#ifdef RIVIERA
SC_MODULE_EXPORT(mandelbrot_bd_xlconcat_1_3);
SC_REGISTER_BV(64);
#endif

