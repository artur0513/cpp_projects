#include <iostream>
#include <vector>

int main() {
	std::vector<int> mas = { 1, 2, 3, 4, 5, 10 };
	for (auto num : mas) { //auto = int
		std::cout << num << std::endl;
	}
}

//====================================

#include <iostream>
#include <stacktrace> // C++23 omg

void foo() {
    auto trace = std::stacktrace::current();
    for (const auto& entry : trace) {
        std::cout << "Description: " << entry.description() << std::endl;
        std::cout << "file: " << entry.source_file() << std::endl;
        std::cout << "line: " << entry.source_line() << std::endl;
        std::cout << "------------------------------------" << std::endl;
    }
}

int main() {
    foo();
}

//====================================

#include <string_view>
string_view abc; // fast const string

//====================================

#include <complex>

std::complex<double> a;
a.real(1.0);
a.imag(-1.0);

std::complex<double> b(1.0, 1.0);

std::cout << a * b; // output (2,0)

//===================================

//https://gist.github.com/AndreLouisCaron/1841061 check


// Cpp program to redirect cout to a file
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main()
{
    fstream file;
    file.open("cout.txt", ios::out);
    string line;

    // Backup streambuffers of  cout
    streambuf* stream_buffer_cout = cout.rdbuf();
    streambuf* stream_buffer_cin = cin.rdbuf();

    // Get the streambuffer of the file
    streambuf* stream_buffer_file = file.rdbuf();

    // Redirect cout to file
    cout.rdbuf(stream_buffer_file);

    cout << "This line written to file" << endl;

    // Redirect cout back to screen
    cout.rdbuf(stream_buffer_cout);
    cout << "This line is written to screen" << endl;

    file.close();
    return 0;
}

// =====================================

typedef int Scalar;
struct Vector
{
    union
    {
        struct
        {
            Scalar x, y;
        };
        Scalar v[2];
    };
};