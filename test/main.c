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
    tsp_parse_commandline(argc, argv, &inst);
    
    assert_false(inst.options_t.graph_input);
    assert_true(inst.options_t.graph_random);
    assert_int_equal(inst.nnodes, 100);
}

static void parseCommandline_validfile(void **state){
    int argc = 3;
    char* argv[argc];
    argv[0] = "tsp";
    argv[1] = "-file";
    argv[2] = "data/berlin52.tsp";

    instance inst;
    tsp_parse_commandline(argc, argv, &inst);

    assert_true(inst.options_t.graph_input);
    assert_false(inst.options_t.graph_random);
    assert_string_equal(inst.options_t.inputfile, argv[2]);
}

static void parseCommandline_time(void **state){
    int argc = 3;
    char* argv[argc];
    argv[0] = "tsp";
    argv[1] = "-time";
    argv[2] = "3600";

    instance inst;
    tsp_parse_commandline(argc, argv, &inst);

    assert_int_equal(inst.options_t.timelimit, 3600);
}


static void parseCommandline_seed(void **state){
    int argc = 3;
    char* argv[argc];
    argv[0] = "tsp";
    argv[1] = "-seed";
    argv[2] = "123";

    instance inst;
    tsp_parse_commandline(argc, argv, &inst);

    assert_int_equal(inst.options_t.seed, 123);
}


/**
 * Test runner function
 */
int
main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(parseCommandline_n),
        cmocka_unit_test(parseCommandline_validfile),
        cmocka_unit_test(parseCommandline_time),
        cmocka_unit_test(parseCommandline_seed),
    };


    /* Run the tests */
    return cmocka_run_group_tests(tests, NULL, NULL);
}
