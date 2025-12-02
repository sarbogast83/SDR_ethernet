# FUll SDR with Ethernet

NOTE: 2025 Cloned from "dougwen/radio_periph_lab" for educational purposes

Note : if you are building in windows and vivado is not installed in c:\Xilinx\Vivado\2022.1, you will have to change one thing
I included the settings64.bat file in the make_project.bat just to save a step.  Change that to your Install directory

run make_project.bat (windows) or make_project.sh (linux) to build the project all the way through SD card creation.  You can of course
edit in the GUI and debug in Vitis GUI afterwards as well.  The Vivado project is in "vivado" and the vitis workspace will be in "vitis"

Only downside of this (haven't fixed it yet) the C code for the processor is copied into the Vitis workspace, not linked from the original
version controlled SRC directory.  So, if you change it, you have to copy it back there.  There is a solution to this of course, but haven't 
done it yet

The base distributed project for the radio peripheral laboratory

A couple of other notes to smooth things along :

- Note that the Makefile that is created for you with the Create/Import peripheral wizard doesn't work when you go to build your software Vitis if you are using Windows.  Before packaging your peripheral, make sure to copy the Makefile from the doug_custom peripheral in this repository in place of the Makefile that the tool created as part of your software driver
- I'm not convinced that right clicking on the "Update Hardware Platform" works when you've changed the underlying hardware XSA file.  The safest thing to do here (this is where the script really helps) is to just delete the entire Vitis directory and regenerate it.  To regenerate a Vitis directory (workspace with all the software), you can certainly do it manually as in all the other labs; however I just run the last line of "make_project.bat" from the command prompt
- Make sure to use a standard command prompt, and not the windows powershell.
- If you are experiencing long build times (i.e. waiting for an excessive amount of time for impl_1 to complete) this may be that you don't have enough memory to handle the level of parallelism that I asked for in the script.  Edit "impl.tcl" to change the line from "-jobs 7" to "-jobs 3" if you don't have 16GB of memory.  Your computer probably doesn't have enough memory to synthesize 7 things in parallel, and is constantly swapping to disk

# Features
Hardware is developed from the radio_periph_lab with the following additions. The control address in the full_radio block has been updated to manage the two least significant bits. Bit(0) continues to control reset of the system, write 1 to control address to reset. Bit(1) controls the  m_axis_tvalid of full_radio_0, write 2 to the control address to clear the reset and initialize tvalid signal for samples. Additionally, m_axis_tlast has be set to '1' for funtionality with the addtional fifo. 

The radio stream has been split with axis_broadcast into two singals, one to the codec and the second into a axi_fifo. The fifo is connected back to the PS allowing samples to be read back into the PS. the m_axis_tvalid needs to be activated with the control bit to write samples to the fifo. 

A software package, located in scr has been developed for the PS to accomplish the following tasks. Serve a web page from the Zybo. From the page, one can initalize the radio and start a UPD transmision to a target IP. Set the fake ADC and tuner frequencies, control UDP transmission (set the tvalid control bit). Execute a short example song and test thoughput. Test fifo read bandwidth. 

Note: initiallizing the radio clears the UDP stream while runing the fifo test enables the stream.  