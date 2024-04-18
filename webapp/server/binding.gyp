{
    'variables' : {
        'CPLEXDIR': '/Applications/CPLEX_Studio2211/cplex',
    },
    "targets": [
        {
            "target_name" : "tspsolver",
            "cflags!" : [ "-std=gnu99 -fno-exceptions -O3 -fstack-protector-all -Wstack-protector" ],
            "cflags_cc!" : [ "-std=gnu99 -fno-exceptions -O3 -fstack-protector-all -Wstack-protector" ],
            "sources" : [
                "src/index.cpp",
                "../../src/main.c",
                "../../src/tsp.c",
                "../../src/algorithms/cplex_model.c",
                "../../src/algorithms/heuristics.c",
                "../../src/algorithms/metaheuristic.c",
                "../../src/algorithms/refinment.c",
                "../../src/utils/errors.c",
                "../../src/utils/plot.c",
                "../../src/utils/utils.c"
            ],
            "include_dirs" : [
                "node_modules/node-addon-api",
                ".",
                "<@(CPLEXDIR)/include/ilcplex"
            ], 
            'link_settings': {
                'libraries': [
                    '-lcplex',
                    '-lm', 
                    '-lpthread',
                    '-ldl',
                ],
                'library_dirs': [
                    '<@(CPLEXDIR)/lib/arm64_osx/static_pic',
                    '.'
                ],
            },
            "defines": ['NAPI_DISABLE_CPP_EXCEPTIONS'],
        }
    ],
}