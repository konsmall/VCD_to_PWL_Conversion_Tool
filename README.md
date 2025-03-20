# VCD_to_PWL_Conversion_Tool
A tool that converts the VCD output waveforms from the VVP verilog simulator into PWL that can be inputted as waveform for virtuoso and other such programs. 

The VCD variables option file contains the VCD input names that will mapped into the individual PWL files.

The Extra variables file contains extra options pertaining the conversion of the logical waveform of the vcd to the digital waveforms of the PWL. Such options are the rising/fall time, the high/low voltage, time scale and so on.

The singular VCD input will be translated into multiple files, each a different waveform for each bit, based on the relative user input.

This tool was developed to certify the operational integrity of the analog memristor crossbar that was smulated digitally in the RISC-V architecture with an IMC memristor crossbar memory [CPU_32bit_pipelined_Virtual_Memristor_Memory] (https://github.com/konsmall/CPU_32bit_pipelined_In_Memory_Computing_Virtual_Memristor_Memory)
