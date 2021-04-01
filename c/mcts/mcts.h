//
// Created by duncan on 25-03-21.
//

#ifndef HIVE_MCTS_H
#define HIVE_MCTS_H


struct mcts_data {
    unsigned int p0;
    unsigned int p1;
    unsigned int draw;
};


void mcts(struct node **tree, int n_playouts);


#endif //HIVE_MCTS_H
