#include<random>
#include<algorithm>
#include"SFMT.h"


using namespace std;

class Random{
public:
	sfmt_t sfmt;
	Random(){
		srand(unsigned(time(0)));
		sfmt_init_gen_rand(&sfmt, std::rand());
	}

	Random(unsigned seed){
		srand(unsigned(seed));
		sfmt_init_gen_rand(&sfmt, rand());
	}
	
	unsigned generateRandom() {
		return sfmt_genrand_uint32(&sfmt);
	}

	double drand(){
		return generateRandom() % RAND_MAX / (double) (RAND_MAX);
	}
};
