is it useful to have multiple evaluator threads?
how does the cost of finding legal moves compare to the cost of evaluating a position?

if finding legal moves is more expensive, then maybe threads can be assigned to precomputing legal moves for nodes during the minimax search
build a tree with nodes containing position, evaluation value, children, etc
have a lead evaluator that manages the tree and performs minimax/alpha beta computation
have worker evaluators that compute legal moves
