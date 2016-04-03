//////////////////////////////////////////////////////////////////////////////
// Timelapse Slider Program
// Implementation of a Shoot-Move-Shoot sequence in TMCL
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// User Variables - Sequence Parameters
//////////////////////////////////////////////////////////////////////////////
StartPosition       = 0
EndPosition         = 1
Shots               = 2
IntervalTicks       = 3
StabilizeTicks      = 4
ShootTicks          = 5

//////////////////////////////////////////////////////////////////////////////
// User Variables - Status
//////////////////////////////////////////////////////////////////////////////
CurrentShot         = 10
// the actual interval ticks - can be used to monitor timing
ActualIntervalTicks = 11

//////////////////////////////////////////////////////////////////////////////
// Internal Variables
//////////////////////////////////////////////////////////////////////////////
// calculated position increment between shots
PositionIncrement   = 20
// time of next move
NextMoveTicks       = 21
// start of the loop iteration
LoopStartTicks      = 22

//////////////////////////////////////////////////////////////////////////////
// Set Variables - TODO these should be set by the MCU later
//////////////////////////////////////////////////////////////////////////////
// SGP   StartPosition,    2,  0
// SGP   EndPosition,      2,  100000
// SGP   Shots,            2,  10
// SGP   IntervalTicks,    2,  1000
// SGP   StabilizeTicks,   2,  500
// SGP   ShootTicks,       2,  100

Timelapse_Main:
  SAP   138, 0, 0             //set ramp mode (position)
  SGP   CurrentShot, 2, 0    	//reset current shot & actual interval ticks
  SGP   ActualIntervalTicks, 2, 0

  // calculate the position increment between shots
  // PositionIncrement = (EndPosition - StartPosition) / Shots
  GGP   StartPosition, 2
  CALCX SWAP
  GGP   EndPosition, 2
  CALCX SUB					          // EndPosition - StartPosition
  CALCX SWAP
  GGP   Shots, 2
  CALCX SWAP
  CALCX DIV					          // (EndPosition - StartPosition) / Shots
  AGP   PositionIncrement, 2

  GGP   StartPosition, 2
  CSUB  Util_MoveTo

Loop:
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
  GGP   StartPosition, 2
  CALCX ADD					          // StartPosition + (CurrentShot * PositionIncrement)
  AAP   0, 0					        // set new target position
  WAIT  POS, 0, 0			        // wait until we reach it

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
  JC    LE, Stop
  // otherwise, continue loop

  // calculate actual interval ticks
  GGP   LoopStartTicks, 2
  CALCX SWAP
  GGP   132, 0				        // get tick timer (1ms resolution)
  CALCX SUB
  AGP   ActualIntervalTicks, 2

  JA    Loop

Stop:
  SGP   CurrentShot, 2, 0    	//reset current shot & actual interval ticks
  SGP   ActualIntervalTicks, 2, 0

  GGP   StartPosition, 2
  CSUB  Util_MoveTo
