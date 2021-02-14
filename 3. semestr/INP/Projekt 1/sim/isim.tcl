proc isim_script {} {

   add_divider "Signals of the Vigenere Interface"
   add_wave_label "" "CLK" /testbench/clk
   add_wave_label "" "RST" /testbench/rst
   add_wave_label "-radix ascii" "DATA" /testbench/tb_data
   add_wave_label "-radix ascii" "KEY" /testbench/tb_key
   add_wave_label "-radix ascii" "CODE" /testbench/tb_code

   add_divider "Vigenere Inner Signals"

   add_wave_label "-radix dec" "shift" /testbench/uut/shift
   add_wave_label "-radix ascii" "corPositiveShifted" /testbench/uut/corPositiveShifted
   add_wave_label "-radix ascii" "corNegativeShifted" /testbench/uut/corNegativeShifted
   add_wave_label "" "presentState" /testbench/uut/presentState
   add_wave_label "" "nextState" /testbench/uut/nextState
   add_wave_label "" "forceErr" /testbench/uut/forceErr
   add_wave_label "" "fsmOutput" /testbench/uut/fsmOutput

   run 8 ns
}
