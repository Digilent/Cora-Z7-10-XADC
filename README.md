This is a demonstration of how to use Cora Z7-10's analog input pins. Once the project is programmed onto the board, a serial terminal application, such as Tera Term, should be used to view the voltage level of the selected analog input channel. A channel select index can be incremented by pressing button 0 and decremented by pressing button 1. The table below shows how the channel select index is used to select from the Cora's various analog inputs. RGB LED 0 is used to show the sign of the selected analog input. If the voltage read is greater than 0.5 Volts, the RGB LED is green; if the voltage read is less than -0.5 Volts, the RGB LED is red.

| Index | Analog Input/s |
| 0 | VP/VN Dedicated Differential Input |
| 1 | Shield Header A6-A7 Differential Input |
| 2 | Shield Header A8-A9 Differential Input |
| 3 | Shield Header A10-A11 Differential Input |
| 3 | Shield Header A0 Single-Ended Input |
| 3 | Shield Header A1 Single-Ended Input |
| 3 | Shield Header A2 Single-Ended Input |
| 3 | Shield Header A3 Single-Ended Input |
| 3 | Shield Header A4 Single-Ended Input |
| 3 | Shield Header A5 Single-Ended Input |

For the Cora Z7-07S version of this project, see [this repo](https://github.com/Digilent/Cora-Z7-07S-XADC).

WARNING!!! This project is only supported in the 2017.4 version of Vivado.

In order to program the project onto an FPGA:

1. 	Download the most recent release ZIP archive (not the source ZIP) from the repo's [releases page](https://github.com/Digilent/Cora-Z7-10-XADC/releases).

2. 	Extract the downloaded ZIP. Find the "Cora-Z7-10-XADC.xpr" file in the "vivado" subfolder of the extracted archive. This will launch an archived version of the project, in which a bitstream has already been generated.

3.  Select File -> Export -> Export Hardware from the bar at the top of the Vivado GUI. Select <Local to Project> as the location to export to and make sure that the Include bitstream box is checked, then click OK.

4.  Select File -> Launch SDK from the bar at the top of the Vivado GUI. Select <Local to Project> as the workspace location and exported location, then click OK.

5.  Vivado SDK will open and the hardware platform created by Vivado will be imported. Select File -> New -> Application Project from the bar at the top of the SDK GUI. Give the application project a name without spaces. Leave all other settings default, then click Next.

6.  From the list of template applications, select "Empty Application", then click Finish.

7.  Copy all of the files from the "sdk" subfolder of the extracted archive into the "src" subfolder of your new application project. If SDK does not automatically rebuild the project, select Project -> Build All from the bar at the top of the SDK GUI.

8. 	Select Xilinx -> Program FPGA from the menu at the top of the Xilinx SDK GUI. Click OK.

9.  Right click on the application project in the Project Explorer to the left, then select Run As -> Launch on Hardware (System Debugger).

10.  The application will now be running on your Cora. It can be interacted with as described in the first paragraph of this README.

For more information on how this project is version controlled, see the [digilent-vivado-scripts repo](https://github.com/artvvb/digilent-vivado-scripts), which contains several Python and TCL scripts for maintaining a Vivado project on Github.
