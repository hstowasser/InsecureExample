# 
# Synthesis run script generated by Vivado
# 

proc create_report { reportName command } {
  set status "."
  append status $reportName ".fail"
  if { [file exists $status] } {
    eval file delete [glob $status]
  }
  send_msg_id runtcl-4 info "Executing : $command"
  set retval [eval catch { $command } msg]
  if { $retval != 0 } {
    set fp [open $status w]
    close $fp
    send_msg_id runtcl-5 warning "$msg"
  }
}
set_msg_config -id {HDL-1065} -limit 10000
create_project -in_memory -part xc7z007sclg400-1

set_param project.singleFileAddWarning.threshold 0
set_param project.compositeFile.enableAutoGeneration 0
set_param synth.vivado.isSynthRun true
set_msg_config -source 4 -id {IP_Flow 19-2162} -severity warning -new_severity info
set_property webtalk.parent_dir /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/proj/test_reduced/test_reduced.cache/wt [current_project]
set_property parent.project_path /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/proj/test_reduced/test_reduced.xpr [current_project]
set_property XPM_LIBRARIES {XPM_CDC XPM_FIFO XPM_MEMORY} [current_project]
set_property default_lib xil_defaultlib [current_project]
set_property target_language VHDL [current_project]
set_property board_part digilentinc.com:cora-z7-07s:part0:1.0 [current_project]
set_property ip_repo_paths {
  /home/heiko/Documents/Sandbox/Another_Goddam_IP_repo
  /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/repo
} [current_project]
set_property ip_output_repo /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/repo/cache [current_project]
set_property ip_cache_permissions {read write} [current_project]
read_vhdl -library xil_defaultlib /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/hdl/system_wrapper.vhd
add_files /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/system.bd
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axi_dma_0_0_1/system_axi_dma_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axi_dma_0_0_1/system_axi_dma_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axi_dma_0_0_1/system_axi_dma_0_0_clocks.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axi_intc_0_0_1/system_axi_intc_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axi_intc_0_0_1/system_axi_intc_0_0_clocks.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axi_intc_0_0_1/system_axi_intc_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axis_data_fifo_0_0_1/system_axis_data_fifo_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axis_data_fifo_0_0_1/system_axis_data_fifo_0_0/system_axis_data_fifo_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_axis_data_fifo_0_0_1/system_axis_data_fifo_0_0/system_axis_data_fifo_0_0_clocks.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_blk_mem_gen_0_0_1/system_blk_mem_gen_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_clk_wiz_25M_0_1/system_clk_wiz_25M_0_board.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_clk_wiz_25M_0_1/system_clk_wiz_25M_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_clk_wiz_25M_0_1/system_clk_wiz_25M_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_data_lmb_bram_if_cntlr_1_0_1/system_data_lmb_bram_if_cntlr_1_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_data_lmb_v10_1_0_1/system_data_lmb_v10_1_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_data_lmb_v10_1_0_1/system_data_lmb_v10_1_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_dma_axi_bram_ctrl_1_0_1/system_dma_axi_bram_ctrl_1_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_dma_blk_mem_gen_1_0_1/system_dma_blk_mem_gen_1_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_fifo_count_axi_gpio_0_0_1/system_fifo_count_axi_gpio_0_0_board.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_fifo_count_axi_gpio_0_0_1/system_fifo_count_axi_gpio_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_fifo_count_axi_gpio_0_0_1/system_fifo_count_axi_gpio_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_ins_lmb_bram_if_cntlr_0_0_1/system_ins_lmb_bram_if_cntlr_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_ins_lmb_v10_0_0_1/system_ins_lmb_v10_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_ins_lmb_v10_0_0_1/system_ins_lmb_v10_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_int_axi_gpio_0_0_1/system_int_axi_gpio_0_0_board.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_int_axi_gpio_0_0_1/system_int_axi_gpio_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_int_axi_gpio_0_0_1/system_int_axi_gpio_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_mb_dma_axi_bram_ctrl_0_0_1/system_mb_dma_axi_bram_ctrl_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_microblaze_0_0_1/system_microblaze_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_microblaze_0_0_1/system_microblaze_0_0_ooc_debug.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_microblaze_0_0_1/system_microblaze_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_proc_sys_reset_0_0_1/system_proc_sys_reset_0_0_board.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_proc_sys_reset_0_0_1/system_proc_sys_reset_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_proc_sys_reset_0_0_1/system_proc_sys_reset_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_processing_system7_0_0_1/system_processing_system7_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_rst_ps7_0_100M_0_1/system_rst_ps7_0_100M_0_board.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_rst_ps7_0_100M_0_1/system_rst_ps7_0_100M_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_rst_ps7_0_100M_0_1/system_rst_ps7_0_100M_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_xadc_wiz_0_0_1/system_xadc_wiz_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_xadc_wiz_0_0_1/system_xadc_wiz_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_xbar_1_1/system_xbar_1_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_xbar_0_1/system_xbar_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_clk_wiz_0_0/system_clk_wiz_0_0_board.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_clk_wiz_0_0/system_clk_wiz_0_0.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_clk_wiz_0_0/system_clk_wiz_0_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_auto_pc_2/system_auto_pc_2_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_auto_pc_1/system_auto_pc_1_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/ip/system_auto_pc_0/system_auto_pc_0_ooc.xdc]
set_property used_in_implementation false [get_files -all /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/bd/system/system_ooc.xdc]

# Mark all dcp files as not used in implementation to prevent them from being
# stitched into the results of this synthesis run. Any black boxes in the
# design are intentionally left as such for best results. Dcp files will be
# stitched into the design at a later time, either when this synthesis run is
# opened, or when it is stitched into a dependent implementation run.
foreach dcp [get_files -quiet -all -filter file_type=="Design\ Checkpoint"] {
  set_property used_in_implementation false $dcp
}
read_xdc /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/constraints/Cora-Z7-Master.xdc
set_property used_in_implementation false [get_files /home/heiko/Desktop/Hardware_Sandbox/2020-ectf-insecure-example_copy/pl/src/constraints/Cora-Z7-Master.xdc]

read_xdc dont_touch.xdc
set_property used_in_implementation false [get_files dont_touch.xdc]

synth_design -top system_wrapper -part xc7z007sclg400-1 -flatten_hierarchy none -directive RuntimeOptimized -fsm_extraction off


# disable binary constraint mode for synth run checkpoints
set_param constraints.enableBinaryConstraints false
write_checkpoint -force -noxdef system_wrapper.dcp
create_report "synth_1_synth_report_utilization_0" "report_utilization -file system_wrapper_utilization_synth.rpt -pb system_wrapper_utilization_synth.pb"
