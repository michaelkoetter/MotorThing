///////////////////////////////////////////////////////////////////////////////
// Utility subroutines
///////////////////////////////////////////////////////////////////////////////

// Move to the position in accumulator
Util_MoveTo:
  MVP REL, 0, 0
  AAP 0, 0
  WAIT POS, 0, 0
  RSUB

// wait for milliseconds in accumulator
Util_Wait:
  CALCX SWAP
  GGP   132, 0             // get timer (1ms resolution)
  CALCX ADD                // now + X
  CSUB  Util_WaitUntil
  RSUB

// wait until the timer value in accumulator is reached
Util_WaitUntil:
  CALCX SWAP
  _WaitUntil:
    GGP   132, 0		    // get timer (1ms resolution)
    CALCX SUB              // ticks - until
    COMP  0
    JC    LT, _WaitUntil   // if < 0, keep waiting
  RSUB