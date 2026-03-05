#include "globals.h"

void tools()   {
  String tools[51] = {







    "New Gate",
    "Laguna_Resaw",
    "Laguna_3000",
    "Laguna SE",
    "CNC HDM",
    "CNC XXL",
    "Jet Combo Sander",
    "NA",
    "NA",
    "NA",
    "Supermax_2",
    "Supermax_1",
    "16 Jointer",
    "NA",
    "Dust Collector",
    "Edge Sander",
    "12 inch Jointer",
    "8 inch Jointer",
    "NA",
    "NA",
    "NA",
    "NA",
    "NA",
    "NA",
    "NA",
    "Robust Lathe",
    "Miter Saw",
    "NA",
    "Dewalt Planer",
    "20 inch Planer",
    "Router table #1",
    "Router table #2",
    "NA",
    "Enlon Spindle Sander",
    "Powermatic Spindle",
    "SawStop #1",
    "SawStop #2",
    "Pegas",
    "Combo Sander #2",
    "SawStop#2",



    "Jet 12-21 Lathe",

    "15 inch planer",




   "Static Presure Sensor",


  };
  toolInt = machineID.toInt();
  toolString = tools[toolInt];

  String gTypes[51] = {


    "B",// 00 New Gate         
    "A",// 01 Laguna_Resaw     
    "A",// 02 Laguna_3000      
    "A",// 03 Laguna_SE        
    "C",// 04 CNC HDM          
    "C",// 05 CNC XXL          
    "B",// 06 Jet Combo Sander   
    "Z",//
    "Z",//
    "Z",//
    "A",// 10 Supermax_2       
    "A",// 11 Supermax_1       
    "A",// 12 16" Jointer
    "Z",//
    "M",// 14 Dust Collector   
    "B",// 15 Edge Sander      
    "A",// 16 12 inch Jointer  
    "A",// 17 8 inch Jointer   
    "Z",//
    "Z",//
    "Z",//
    "Z",//
    "Z",//
    "Z",//
    "Z",//  
    "M",// 25 Robust Lathe    
    "P",// 26 Miter Saw       
    "Z",//
    "Z",// 27 Dewalt Planer   
    "A",// 29 20 inch Planer  
    "B",// 30 Router table #1 
    "B",// 31 Router table #2 
    "Z",//    
    "B",// 33 Enlon Spindle Sander 
    "B",// 34 Powermatic Spindle   
    "A",// 35 SawStop #1      
    "Y",//      
    "W",//
    "B",// 38 Combo Sander #2
    "A",// 39 SawStop #2
    "X",//
    "T",//
    "L",//
    "M",// 43 Jet Lathe
    "Z",//
    "A",// 45 Powermatic 15" Planer
    "Z",//
    "Z",//
    "Z",//
    "Z",//           
    "S",// 50 Static Pressure Sensor 
  };
 
 
  gateType = gTypes[toolInt];

}
