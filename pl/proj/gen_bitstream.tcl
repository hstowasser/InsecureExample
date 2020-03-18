#******************************************************************
# gen_bitstream.tcl: Tcl script for automating bitstream generation
#
# This file contains tcl commands for running the synthesis,
# implementation, and bitstream generation steps.
#
# Note: You may need to run
#  * reset_run synth_1, reset_run impl_1, etc.
# if this is run without fully generating the bitstream
#******************************************************************


set origin_dir "."
#set project_bd "$origin_dir/src/bd/system/system.bd"

# Set the project name
set project_name "test_reduced"

# Use project name variable, if specified in the tcl shell
if { [info exists ::user_project_name] } {
  set project_name $::user_project_name
}

set synth_comp "synth_design Complete!"
set impl_run "Running Design Initialization..."
set bitstream_comp "write_bitstream Complete!"

if { $::argc > 0 } {
  puts $::argc
  for {set i 0} {$i < 4} {incr i} {
    set option [string trim [lindex $::argv $i]]
    switch -regexp -- $option {
      "--project_file" { incr i; set project_file [lindex $::argv $i] }
      "--project_name" { incr i; set project_name [lindex $::argv $i] }
      default {
        if { [regexp {^-} $option] } {
          puts "ERROR: Unknown option '$option' specified, expecting --project_file arg.\n"
          #return 1
        }
      }
    }
  }

}
set project_bd "$origin_dir/$project_name/$project_name.srcs/sources_1/bd/system/system.bd"
open_project $project_file
open_bd_design $project_bd

# synthesis
puts "Launching Synthesis"
save_bd_design
launch_runs synth_1 -jobs 2

time {
  while { [get_property STATUS [get_runs synth_1]] != $synth_comp } {
  }
}

# implementation
puts "Launching Implementation"
save_bd_design
launch_runs impl_1 -jobs 2

#time {
#  while { [get_property STATUS [get_runs impl_1]] == $impl_run } {
#  }
#}

wait_on_run impl_1


# generate bitstream
puts "Launching Write Bitstream"
launch_runs impl_1 -to_step write_bitstream -jobs 2


time {
  while { [get_property STATUS [get_runs impl_1]] != $bitstream_comp } {
  }
}
