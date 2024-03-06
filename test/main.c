#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "tsp.h"


/* include here your files that contain test functions */



static void parseCommandline_n(void **state) {

    int argc = 3;
    char* argv[argc];
    argv[0] = "tsp";
    argv[1] = "-n";
    argv[2] = "100";

    instance inst;
    parse_commandline(argc, argv, &inst);
    
    assert_int_equal(inst.nnodes, 100);
}


/**
 * Test runner function
 */
int
main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(parseCommandline_n),
    };


    /* Run the tests */
    return cmocka_run_group_tests(tests, NULL, NULL);
}
