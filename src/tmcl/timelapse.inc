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
EaseShots           = 5

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

// trapezoidal motion profile parameters
// Acceleration is calculated at start
Acceleration        = 20
// Velocity is updated for every interval
Velocity            = 21
// TargetPosition is updated for every interval
TargetPosition      = 22

// time of next move
NextMoveTicks       = 23
// start of the loop iteration
LoopStartTicks      = 24

Temp1               = 30
Temp2               = 31
Temp3               = 32

// We scale our position values to +/- 2^31 to reduce rounding errors
// The max. position is +/- 2^23 so we can scale by 2^8
PositionScale       = 256


Timelapse_Main:
  //reset current shot & actual interval ticks
  SGP   CurrentShot, 2, 0
  SGP   ActualIntervalTicks, 2, 0

  // reset trapezoidal motion profile params
  SGP   Acceleration, 2, 0
  SGP   Velocity, 2, 0

  // Copy EndCoordinate from EEPROM to RAM
  // This is required for some modules that default to store coordindates in RAM only
  // - other modules will probably ignore it
  GCO   EndCoordinate, 255

  // Calculate trapezoidal motion profile Acceleration
  // Acceleration   = Distance / (Shots - EaseShots) / EaseShots
  //   Distance     = EndPosition - StartPosition
  CSUB  Timelapse_Get_StartPosition
  CALCX SWAP
  CSUB  Timelapse_Get_EndPosition
  CALCX SUB                               // Distance = EndPosition - StartPosition
  CALC  MUL, PositionScale                // Scaled Distance = PositionScale * Distance
  AGP   Temp1, 2                          // => Temp1


  GGP   EaseShots, 2
  CALCX SWAP
  GGP   Shots, 2
  CALCX SUB                               // (Shots - EaseShots)

  CALCX SWAP
  GGP   Temp1, 2                          // Scaled Distance
  CALCX DIV                               // ... / (Shots - EaseShots)

  CALCX SWAP
  GGP   EaseShots, 2
  CALCX SWAP
  CALCX DIV                               // ... / EaseShots
  AGP   Acceleration, 2                   // => Acceleration

  // Set scaled start position
  CSUB  Timelapse_Get_StartPosition
  CALC  MUL, PositionScale
  AGP   TargetPosition, 2

Timelapse_Loop:
  GGP   132, 0				        // get tick timer (1ms resolution)
  AGP   LoopStartTicks, 2

  // calculate next move time: now + IntervalTicks
  CALCX SWAP
  GGP   IntervalTicks, 2
  CALCX ADD
  AGP   NextMoveTicks, 2

  // Move to next TargetPosition
  GGP   TargetPosition, 2
  CALC  DIV, PositionScale
  CSUB  Util_MoveTo

  // wait for StabilizeTicks
  GGP   StabilizeTicks, 2
  CSUB  Util_Wait

  // shoot
  SIO 0, 2, 1    //change DOUT0

  // wait for ShootTicks
  GGP   ShootTicks, 2
  CSUB  Util_Wait

  SIO 0, 2, 0    //change DOUT0

  // increment CurrentShot
  GGP   CurrentShot, 2
  CALC  ADD, 1				        // CurrentShot + 1
  AGP   CurrentShot, 2

  // Calculate next TargetPosition
  // Position    = Position + Velocity + (Acceleration / 2)
  //   Velocity  = Velocity + Acceleration
  CSUB  Timelapse_Get_Acceleration
  CALCX SWAP
  GGP   Velocity, 2
  CALCX ADD                          // Velocity + Acceleration
  AGP   Velocity, 2                  // => Velocity

  CSUB  Timelapse_Get_Acceleration
  CALC  DIV, 2                        // (Acceleration / 2)
  CALCX SWAP
  GGP   Velocity, 2
  CALCX ADD                           // ... + Velocity
  CALCX SWAP
  GGP   TargetPosition, 2
  CALCX ADD                           // ... + Position

  // store next TargetPosition
  AGP   TargetPosition, 2

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

// Get the current acceleration
// Depends on the current phase of the trapezoidal motion profile
Timelapse_Get_Acceleration:
  GGP   CurrentShot, 2
  CALCX SWAP
  GGP   EaseShots, 2
  CALCX SUB					  // EaseShots - CurrentShot
  COMP  0
  JC    LT, _Phase_Slew             // CurrentShot > EaseShots ?

_Phase_Accelerate:
  GGP   Acceleration, 2
  RSUB

_Phase_Slew:
  GGP   EaseShots, 2
  CALCX SWAP
  GGP   Shots, 2
  CALCX SUB                         // Shots - EaseShots
  CALCX SWAP
  GGP   CurrentShot, 2
  CALCX SUB                         // CurrentShot - (Shots - EaseShots)
  COMP  0
  JC    GT, _Phase_Decelerate       // CurrentShot > (Shots - EaseShots) ?

  CALC  LOAD, 0
  RSUB

_Phase_Decelerate:
  GGP   Acceleration, 2
  CALC  MUL, -1
  RSUB

Timelapse_Stop:
  //reset current shot & actual interval ticks
  SGP   CurrentShot, 2, 0
  SGP   ActualIntervalTicks, 2, 0

  // move to start
  CSUB  Timelapse_Get_StartPosition
  CSUB  Util_MoveTo
