-- Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
-- Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2025.1 (win64) Build 6140274 Thu May 22 00:12:29 MDT 2025
-- Date        : Fri Aug  1 21:09:38 2025
-- Host        : NICKYSEXYPCV2 running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub
--               s:/Codes/mandelbrot_super_accelerator/ip_repo/AXI_BROT_1_0/src/mult_gen_0/mult_gen_0_stub.vhdl
-- Design      : mult_gen_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7z020clg400-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity mult_gen_0 is
  Port ( 
    A : in STD_LOGIC_VECTOR ( 11 downto 0 );
    B : in STD_LOGIC_VECTOR ( 63 downto 0 );
    P : out STD_LOGIC_VECTOR ( 63 downto 0 )
  );

  attribute CHECK_LICENSE_TYPE : string;
  attribute CHECK_LICENSE_TYPE of mult_gen_0 : entity is "mult_gen_0,mult_gen_v12_0_23,{}";
  attribute core_generation_info : string;
  attribute core_generation_info of mult_gen_0 : entity is "mult_gen_0,mult_gen_v12_0_23,{x_ipProduct=Vivado 2025.1,x_ipVendor=xilinx.com,x_ipLibrary=ip,x_ipName=mult_gen,x_ipVersion=12.0,x_ipCoreRevision=23,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED,C_VERBOSITY=0,C_MODEL_TYPE=0,C_OPTIMIZE_GOAL=1,C_XDEVICEFAMILY=zynq,C_HAS_CE=0,C_HAS_SCLR=0,C_LATENCY=0,C_A_WIDTH=12,C_A_TYPE=1,C_B_WIDTH=64,C_B_TYPE=0,C_OUT_HIGH=63,C_OUT_LOW=0,C_MULT_TYPE=0,C_CE_OVERRIDES_SCLR=0,C_CCM_IMP=0,C_B_VALUE=10000001,C_HAS_ZERO_DETECT=0,C_ROUND_OUTPUT=0,C_ROUND_PT=0}";
  attribute downgradeipidentifiedwarnings : string;
  attribute downgradeipidentifiedwarnings of mult_gen_0 : entity is "yes";
end mult_gen_0;

architecture stub of mult_gen_0 is
  attribute syn_black_box : boolean;
  attribute black_box_pad_pin : string;
  attribute syn_black_box of stub : architecture is true;
  attribute black_box_pad_pin of stub : architecture is "A[11:0],B[63:0],P[63:0]";
  attribute x_interface_info : string;
  attribute x_interface_info of A : signal is "xilinx.com:signal:data:1.0 a_intf DATA";
  attribute x_interface_mode : string;
  attribute x_interface_mode of A : signal is "slave a_intf";
  attribute x_interface_parameter : string;
  attribute x_interface_parameter of A : signal is "XIL_INTERFACENAME a_intf, LAYERED_METADATA undef";
  attribute x_interface_info of B : signal is "xilinx.com:signal:data:1.0 b_intf DATA";
  attribute x_interface_mode of B : signal is "slave b_intf";
  attribute x_interface_parameter of B : signal is "XIL_INTERFACENAME b_intf, LAYERED_METADATA undef";
  attribute x_interface_info of P : signal is "xilinx.com:signal:data:1.0 p_intf DATA";
  attribute x_interface_mode of P : signal is "master p_intf";
  attribute x_interface_parameter of P : signal is "XIL_INTERFACENAME p_intf, LAYERED_METADATA undef";
  attribute x_core_info : string;
  attribute x_core_info of stub : architecture is "mult_gen_v12_0_23,Vivado 2025.1";
begin
end;
