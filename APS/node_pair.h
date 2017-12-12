#ifndef __NODE_PAIR_H__
#define __NODE_PAIR_H__
#include<utility>
#include <sparsepp/spp.h>
struct NodePair{
    // member
    int a;
    int b;
    //function
    bool operator==(const NodePair &o) const{
        return a == o.a && b == o.b;
    }
    NodePair(int x, int y){
        a=x;
        b=y;
    }
};
namespace std
{
    // inject specialization of std::hash for Person into namespace std
    // ----------------------------------------------------------------
    template<> 
    struct hash<NodePair>
    {
        std::size_t operator()(NodePair const &p) const
        {
            std::size_t seed = 0;
            spp::hash_combine(seed, p.a);
            spp::hash_combine(seed, p.b);
            return seed;
        }
    };
}
#endif

