//
// Created by duncan on 23-05-21.
//

#include <utils.h>
#include <omp.h>
#include <game.h>


int performance_testing(Node& tree, int depth) {
    if (depth == 0) return 1;
    generate_moves(tree);

    int ret = 1;
    for (Node& child : tree.children) {
        ret += performance_testing(child, depth - 1);
    }

    // Remove data
    tree.children.clear();
    return ret;
}

//int performance_testing_parallel(Node& tree, int depth, int par_depth) {
//    if (depth == 0) return 1;
//    generate_moves(tree);
//
//    int ret = 1;
//
//    if (par_depth == 0) {
//#pragma omp parallel for
//        for (Node& child : tree.children) {
//            {
//                int r = performance_testing(child, depth - 1);
//
//#pragma omp atomic
//                ret += r;
//            }
//        }
//    } else {
//        for (Node& child : tree.children) {
//            ret += performance_testing_parallel(child, depth - 1, par_depth - 1);
//        }
//    }
//    return ret;
//}

int main(int argc, char** argv) {
    int max_depth = 8;
    if (argc > 1) {
        max_depth = atoi(argv[1]);
    }
    omp_set_num_threads(1);

    printf("Running perft with depth %d on %d threads.\n", max_depth, omp_get_max_threads());

    Game game = Game();
    srand(0);

    int last = 0;
    struct timespec start, end;
    printf("Depth    | Time (s)        | Nodes           | Knodes/sec    \n");
    printf("---------|-----------------|-----------------|--------------\n");
    for (int depth = 0; depth < max_depth; depth++) {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
        int n = performance_testing(game.root, depth);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);

        int nodes = n - last;
        last = n;
        double time = (to_usec(end) - to_usec(start)) / 1e6;
        printf("%8d | %15.4f | %15d | (%.2f)\n", depth, time, nodes, (n/time) / 1000);
    }
}

/*
 * C version of this code's performance below:
 *
Running perft with depth 8 on 1 threads.
Depth    | Time (s)        | Nodes           | Knodes/sec
---------|-----------------|-----------------|--------------
       0 |          0.0000 |               1 | (1438.85)
       1 |          0.0000 |               4 | (832.78)
       2 |          0.0000 |              16 | (2374.49)
       3 |          0.0001 |             240 | (3715.99)
       4 |          0.0009 |            3600 | (4460.57)
       5 |          0.0193 |           86040 | (4660.98)
       6 |          0.3631 |         2036580 | (5855.74)
       7 |          5.6310 |        30273650 | (5753.85)
 */

/*
 * Initial implementatoin
 *
Running perft with depth 7 on 1 threads.
Depth    | Time (s)        | Nodes           | Knodes/sec
---------|-----------------|-----------------|--------------
       0 |          0.0000 |               1 | (3401.36)
       1 |          0.0000 |               4 | (288.25)
       2 |          0.0000 |              16 | (1019.12)
       3 |          0.0002 |             240 | (1301.19)
       4 |          0.0037 |            3600 | (1038.33)
       5 |          0.0693 |           86040 | (1297.30)
       6 |          1.4433 |         2036518 | (1473.26)
 */

/*
 * Using list as children storage type
 *
Running perft with depth 7 on 1 threads.
Depth    | Time (s)        | Nodes           | Knodes/sec
---------|-----------------|-----------------|--------------
       0 |          0.0000 |               1 | (3984.06)
       1 |          0.0000 |               4 | (262.23)
       2 |          0.0000 |              16 | (1402.71)
       3 |          0.0002 |             240 | (1583.32)
       4 |          0.0021 |            3600 | (1809.24)
       5 |          0.0359 |           86040 | (2506.14)
       6 |          0.7899 |         2036515 | (2692.06)
 */


/*
 * Removed hash history (unused anyways)
 * Set the return type for points vector to be fixed size array.
 *
Running perft with depth 8 on 1 threads.
Depth    | Time (s)        | Nodes           | Knodes/sec
---------|-----------------|-----------------|--------------
       0 |          0.0000 |               1 | (6622.52)
       1 |          0.0000 |               4 | (950.39)
       2 |          0.0000 |              16 | (2199.41)
       3 |          0.0000 |             240 | (5986.10)
       4 |          0.0006 |            3600 | (6434.91)
       5 |          0.0150 |           84600 | (5911.21)
       6 |          0.3649 |         1970428 | (5642.43)
       7 |          5.7316 |        27631300 | (5180.10)

 */