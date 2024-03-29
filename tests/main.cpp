
/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_BUILDING_APPLICATION     1
#define PD_WITH_DEADCODE            1
#define PD_WITH_MAIN                0
#define PD_WITH_DUMMY               1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../src/amalgam.cpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../libraries/T/T.c"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "t_random.c"
#include "t_atomic.c"
#include "t_float.c"
#include "t_pointer32.c"
#include "t_pointer64.c"
#include "t_ringbuffer.c"
#include "t_clocks.c"
#include "t_time.c"
#include "t_utf8.c"
#include "t_soundfile.c"
#include "t_memory.c"
#include "t_benchmark.c"
#include "t_hoeldrich.c"
#include "t_buffer.c"
#include "t_utils.c"
#include "t_expr.c"
#include "t_prim.cpp"
#include "t_MICA.cpp"
#include "t_MIR.cpp"
#include "t_corpus.cpp"
#include "t_tralala.c"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int main()
{
    return (ttt_testAll() != TTT_GOOD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
