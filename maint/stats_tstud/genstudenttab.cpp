/* 
 * genstudenttab.cpp:
 *
 */
#include <iostream>
#include <iomanip>
#include <boost/math/distributions/students_t.hpp>

using namespace std;
using namespace boost::math;

const int samplesize_max = 100;

int main()
{
    double alpha[] = { 0.1, 0.05, 0.01 }; /* 90%, 95%, 99% */

    cout << "int tstud_tab_size = " << samplesize_max << ";\n";

    for(int i = 0; i < sizeof(alpha) / sizeof(alpha[0]); ++i) {
        
        students_t d(99999);
        double t = quantile(complement(d, alpha[i] / 2));
        cout << "double tstud_p" << fixed << setprecision(0) << 100 * (1 - alpha[i])
             << "_ninf = " << fixed << setprecision(3) << t << ";\n";
             
        cout << "double tstud_tab_p" << setprecision(0) << 100 * (1 - alpha[i])
             << "[] = {\n";
        cout << "    ";
        for (int n = 2; n <= samplesize_max; ++n) {
            students_t dist(n - 1);
            double t = quantile(complement(dist, alpha[i] / 2));
            cout << fixed << setprecision(3) << left << t << ", ";
            if (n % 8 == 0) {
                cout << endl << "    ";
            }
        }
        cout << "\n};\n";
    }
    return 0;
}
