//
// Created by yche on 1/20/18.
//

#include <cmath>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    double eps = 0.01;
    double c = 0.6;

    double half_eps = eps / 2;
    double eps_d = (1 - c) * half_eps;

    double denominator = (2 * sqrt(c)) / (1 - sqrt(c)) / (1 - c);
    double theta = half_eps / (denominator);

    cout << eps_d << endl;
    cout << theta << endl;
}
