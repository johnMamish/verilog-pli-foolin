#include <stdlib.h>
#include <stdio.h>

// This file is part of the Verilog standard; it provides C prototypes for the Verilog PLI.
#include "vpi_user.h"

/**
 * This routine is the thing that's actually run by the VPI-compatible Verilog simulator; it's
 * called a "calltf routine".
 */
PLI_INT32 hello_calltf()
{
    // As opposed to printf, which just sends its output to the OS output channel, vpi_printf sends
    // its output to the simulator's output channel.
    vpi_printf("Hello, world!\n");

    return 0;
}

/**
 * Each VPI calltf routine that we want to use needs to have an executable "registration" function
 * that the verilog simulator can run so it knows basic properties about the function.
 */
void hello_register()
{
    s_vpi_systf_data tfd;

    // Set up the data structure with relevant information about our hello_calltf() function
    tfd.type = vpiSysTask;

    // This tells what the return type of the function will be. It can be one of
    // vpiIntFunc (int32_t), vpiRealFunc (double), vpiTimeFunc (uint64_t),
    // vpiSizedFunc (Verilog bitvector), or vpiSizedSignedFunc (Verilog signed bitvector).
    tfd.sysfunctype = vpiIntFunc;

    // Name of the task. Must be preceded with '$'.
    tfd.tfname = "$hello";

    // Function pointer to calltf that should be executed.
    tfd.calltf = hello_calltf;

    // compiletf functions are called once for each PLI-defined system task at the very beginning
    // of simulation (before time = 0). compiletf functions are intended to be used to validate
    // argument types. This can be useful for speeding things up if a PLI-defined system task is
    // called millions of times in a tight loop; you just need to check once at the beginning of
    // simulation to see if the invocations have the correct argument types. You can always do the
    // argument checking in your calltf function, but it might be slower.
    //
    // They are optional; if your calltf function doesn't have an associated compiletf function,
    // it should be left NULL.
    tfd.compiletf = NULL;

    // sizetf routines are called before calltf and tell the simulator how many bits to expect to
    // be returned for a given input.
    tfd.sizetf = NULL;

    // if user_data is set, it will be passed to the tf routines as a "utility" argument. It can do
    // a few cool things, like allowing the same calltf to be registered for 2 different system
    // functions, but have their user_data differentiate them (this could be used to reduce code
    // duplication).
    // user_data can also be very important when a calltf is called multiple times, and the
    // different instances need to differentiate between each other.
    tfd.user_data = NULL;

    // Register our function's information with the simulator so it can be called.
    vpi_register_systf(&tfd);
}

/**
 * Out of everything here, this is the only thing whose specific name is important; the verilog
 * simulator will look for arrays of function pointers named vlog_startup_routines and execute all
 * of the functions in the array.
 *
 * When linking in a closed-source binary, this will need to be in its own C file.
 */
void (*vlog_startup_routines[])() = {
    hello_register,
    0
};
