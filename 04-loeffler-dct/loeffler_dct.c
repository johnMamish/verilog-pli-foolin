#include <stdlib.h>
#include <stdio.h>
#include "vpi_user.h"

/**
 * for readability of loeffler_dct_calltf function, we have this struct that groups all of the
 * expected arguments together.
 */
typedef struct loeffler_dct_tf_args
{
    vpiHandle array_in_handle;
    vpiHandle array_out_handle;
    vpiHandle image_width_handle;
    vpiHandle image_height_handle;

    int32_t width_i32;
    int32_t height_i32;
} loeffler_dct_tf_args_t;

/**
 * Convenience function that gets all of the args for a loeffler_dct PLI System Task call
 *
 * assumes that there are 4 arguments, and that the systf handle is valid.
 */
loeffler_dct_tf_args_t* loeffler_dct_tf_get_args()
{
    loeffler_dct_tf_args_t* args = calloc(1, sizeof(loeffler_dct_tf_args_t));

    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    args->array_in_handle = vpi_scan(arg_iterator);
    args->array_out_handle = vpi_scan(arg_iterator);
    args->image_width_handle = vpi_scan(arg_iterator);
    args->image_height_handle = vpi_scan(arg_iterator);

    vpi_free_object(arg_iterator);

    s_vpi_value value_s;
    value_s.format = vpiIntVal;
    vpi_get_value(args->image_width_handle, &value_s);
    args->width_i32 = value_s.value.integer;

    value_s.format = vpiIntVal;
    vpi_get_value(args->image_height_handle, &value_s);
    args->height_i32 = value_s.value.integer;

    return args;
}

/**
 * Checks the width and height of the given args against array size.
 *
 * These checks need to be done during runtime, not at compiletf time. Returns 0 if the args are good.
 */
int loeffler_dct_tf_check_arg_dims(loeffler_dct_tf_args_t* args)
{
    PLI_INT32 in_size = vpi_get(vpiSize, args->array_in_handle);

    if (((width % 8) != 0) ||
        ((height % 8) != 0) ||
        ((width * height) != in_size)) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * Helper function that pulls every value out of a vpiMemory (an array of registers) and puts it in
 * a newly allocated int32_t array.
 */
int vpi_memory_to_int32_array(int32_t** target, vpiHandle mem)
{
    PLI_INT32 in_size = vpi_get(vpiSize, mem);
    *target = calloc(in_size, sizeof(int32_t));
    vpiHandle reg;
    vpiHandle iter = vpi_iterate(vpiMemoryWord, mem);
    int i = 0;
    while ((reg = vpi_scan(iter)) != NULL) {
        s_vpi_value reg_value;
        reg_value.format = vpiIntVal;
        vpi_get_value(reg, &reg_value);
        (*target)[i++] = reg_value.value.integer;
    }

    return (int)in_size;
}

/**
 * Helper function that takes values out of an int32_t array and puts them in an array of registers.
 *
 * Assumes that the given memory array has at least length len
 */
void int32_array_to_vpi_memory(vpiHandle mem, int32_t* src, int len)
{
    vpiHandle iter = vpi_iterate(vpiMemoryWord, mem);
    for (int i = 0; i < len; i++) {
        vpiHandle reg;
        reg = vpi_scan(iter);

        s_vpi_value reg_value;
        reg_value.format = vpiIntVal;
        reg_value.value.integer = src[i];
        vpi_put_value(reg, &reg_value, NULL, vpiNoDelay);
    }
}

/**
 * This PLI task takes an input array and an output array. It's assumed that both the input array
 * and the output array are 8x8 row-major
 *
 * The input array is expected to be 8-bit and to have unsigned pixel values in the range [0, 255].
 * the input array's depth shall be the width of the given image times the length.
 *
 * The output array is expected to be 16-bit signed 7Q8. It will be in row-major order.
 */
PLI_INT32 loeffler_dct_calltf()
{
    // We already know from compiletf that we have 2 args and they're both of the right type
    loeffler_dct_tf_args_t* args = loeffler_dct_tf_get_args();

    // get the width and height values of the image; sanity check them.
    if (loeffler_dct_tf_check_arg_dims(args) != 0) {
        vpi_printf("$loeffler_dct: ERROR: provided width and height are bad.\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    // Get the input array values as an int32_t array.
    int32_t* invals;
    vpi_memory_to_int32_array(&invals, array_in_handle);


    //
    int32_array_to_vpi_memory(array_out_handle, invals, in_size);

    free(invals);
    return 0;
}

int count_vpi_args(vpiHandle systf_handle)
{
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    if (arg_iterator == NULL) {
        return 0;
    } else {
        int argcount = 0;
        while (vpi_scan(arg_iterator) != NULL) {
            argcount++;
        }
        return argcount;
    }
}

/**
 * compiletf functions make sure that valid parameters are passed in.
 */
PLI_INT32 loeffler_dct_compiletf()
{
    vpiHandle systf_handle = vpi_handle(vpiSysTfCall, NULL);
    if (systf_handle == NULL) {
        vpi_printf("$loeffler_dct: ERROR: NULL systf handle\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    int numargs = count_vpi_args(systf_handle);
    if (numargs != 4) {
        vpi_printf("$loeffler_dct: ERROR: 4 args required; %i args found.\n", numargs);
        vpi_control(vpiFinish, 0);
        return 0;
    }

    const PLI_INT32 expected_arg_types[] = { vpiMemory, vpiMemory, vpiConstant, vpiConstant };
    vpiHandle arg_iterator = vpi_iterate(vpiArgument, systf_handle);
    vpiHandle arg;
    int i = 0;
    while((arg = vpi_scan(arg_iterator)) != NULL) {
        PLI_INT32 arg_type = vpi_get(vpiType, arg);

        if (arg_type != expected_arg_types[i++]) {
            vpi_printf("$loeffler_dct: ERROR: arg must have type vpiMemory.\n");
            vpi_free_object(arg_iterator);
            vpi_control(vpiFinish, 0);
            return 0;
        }
    }

    // check that the sizes of the first 2 arrays match
    PLI_INT32 in_size  = vpi_get(vpiSize, array_in_handle);
    PLI_INT32 out_size = vpi_get(vpiSize, array_out_handle);
    vpi_printf("arg a size is %i, arg b size is %i\n", in_size, out_size);
    if (out_size != in_size) {
        vpi_printf("$loeffler_dct: ERROR: argument arrays must have same size.\n");
        vpi_control(vpiFinish, 0);
        return 0;
    }

    return 0;
}

void loeffler_dct_register()
{
    s_vpi_systf_data tfd;

    tfd.type = vpiSysTask;
    tfd.sysfunctype = 0;
    tfd.tfname = "$loeffler_dct";
    tfd.calltf = loeffler_dct_calltf;
    tfd.compiletf = loeffler_dct_compiletf;
    tfd.sizetf = NULL;
    tfd.user_data = NULL;

    vpi_register_systf(&tfd);
}

void (*vlog_startup_routines[])() =
{
    loeffler_dct_register,
    0
};
