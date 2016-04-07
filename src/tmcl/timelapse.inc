//////////////////////////////////////////////////////////////////////////////
// Timelapse Slider Program
// Implementation of a Shoot-Move-Shoot sequence in TMCL
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// User Variables - Sequence Parameters
//////////////////////////////////////////////////////////////////////////////
Shots               = 0
IntervalTicks       = 1
StabilizeTicks      = 2
ShootTicks          = 3
Reverse             = 4

//////////////////////////////////////////////////////////////////////////////
// User Variables - Status
//////////////////////////////////////////////////////////////////////////////
CurrentShot         = 10
// the actual interval ticks - can be used to monitor timing
ActualIntervalTicks = 11

//////////////////////////////////////////////////////////////////////////////
// Coordinates
//////////////////////////////////////////////////////////////////////////////
EndCoordinate       = 1

//////////////////////////////////////////////////////////////////////////////
// Internal Variables
//////////////////////////////////////////////////////////////////////////////
// calculated position increment between shots
PositionIncrement   = 20
// time of next move
NextMoveTicks       = 21
// start of the loop iteration
LoopStartTicks      = 22


Timelapse_Main:
  //reset current shot & actual interval ticks
  SGP   CurrentShot, 2, 0    	
  SGP   ActualIntervalTicks, 2, 0

  // Copy EndCoordinate from EEPROM to RAM
  // This is required for some modules that default to store coordindates in RAM only 
  // - other modules will probably ignore it
  GCO   EndCoordinate, 255

  // calculate the position increment between shots
  // PositionIncrement = (EndPosition - StartPosition) / (Shots - 1)
  CSUB  Timelapse_Get_StartPosition
  CALCX SWAP
  CSUB  Timelapse_Get_EndPosition
  CALCX SUB                               // EndPosition - StartPosition
  CALCX SWAP
  GGP   Shots, 2
  CALC  SUB, 1 						 // Shots - 1
  CALCX SWAP
  CALCX DIV					          // (EndPosition - StartPosition) / (Shots - 1)
  AGP   PositionIncrement, 2

  CSUB  Timelapse_Get_StartPosition
  CSUB  Util_MoveTo

Timelapse_Loop:
  GGP   132, 0				        // get tick timer (1ms resolution)
  AGP   LoopStartTicks, 2
  // calculate next move time: now + IntervalTicks
  CALCX SWAP
  GGP   IntervalTicks, 2
  CALCX ADD
  AGP   NextMoveTicks, 2

  // calculate next position and move
  // target position = StartPosition + (CurrentShot * PositionIncrement)
  GGP   CurrentShot, 2
  CALCX SWAP
  GGP   PositionIncrement, 2
  CALCX MUL					          // CurrentShot * PositionIncrement
  CALCX SWAP
  CSUB  Timelapse_Get_StartPosition
  CALCX ADD					          // StartPosition + (CurrentShot * PositionIncrement)
  CSUB  Util_MoveTo			          // set new target position

  // wait for StabilizeTicks
  GGP   StabilizeTicks, 2
  CSUB  Util_Wait

  // increment CurrentShot
  GGP   CurrentShot, 2
  CALC  ADD, 1				        // CurrentShot + 1
  AGP   CurrentShot, 2

  // shoot
  SIO 0, 2, 1    //change DOUT0

  // wait for ShootTicks
  GGP   ShootTicks, 2
  CSUB  Util_Wait

  SIO 0, 2, 0    //change DOUT0

  // wait until NextMoveTicks
  GGP   NextMoveTicks, 2
  CSUB  Util_WaitUntil

  // check if we should stop (current shot >= shots)
  GGP   CurrentShot, 2
  CALCX SWAP
  GGP   Shots, 2
  CALCX SUB					          // Shots - CurrentShot
  COMP  0
  JC    LE, Timelapse_Stop
  // otherwise, continue loop

  // calculate actual interval ticks
  GGP   LoopStartTicks, 2
  CALCX SWAP
  GGP   132, 0				        // get tick timer (1ms resolution)
  CALCX SUB
  AGP   ActualIntervalTicks, 2

  JA    Timelapse_Loop


Timelapse_Get_StartPosition:
  GGP   Reverse, 2
  JC    NZ, _Get_StartPosition_Reverse

  // in normal mode, our start position is always 0
  CALC  LOAD, 0
  RSUB

_Get_StartPosition_Reverse:
  // in reverse mode, our start position is the EndCoordinate
  GCO   EndCoordinate, 0
  RSUB

Timelapse_Get_EndPosition:
  GGP   Reverse, 2
  JC    NZ, _Get_EndPosition_Reverse

  // in normal mode, our end position is the EndCoordinate
  GCO   EndCoordinate, 0
  RSUB

_Get_EndPosition_Reverse:
  // in reverse mode, our end position is always 0
  CALC  LOAD, 0
  RSUB


Timelapse_Stop:
  //reset current shot & actual interval ticks
  SGP   CurrentShot, 2, 0    	
  SGP   ActualIntervalTicks, 2, 0

  // move to start
  CSUB  Timelapse_Get_StartPosition
  CSUB  Util_MoveTo
