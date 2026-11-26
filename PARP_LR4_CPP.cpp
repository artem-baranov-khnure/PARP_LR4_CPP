#include <windows.h>
#include <omp.h>
#include <stdio.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
using namespace std;

const double PI = 3.141592653589793;

// https://en.cppreference.com/w/cpp/chrono/steady_clock
template <class Function>
double GetMinTime(Function&& f, int repeat = 20) {
	double best = 1.0e9;

	for (int i = 0; i < repeat; i++) {
		auto start = chrono::steady_clock::now();
		f();
		auto finish = chrono::steady_clock::now();
		double t = chrono::duration<double>(finish - start).count();
	
		if (t < best) {
			best = t;
		}
	}

	return best;
}

int CorrectDigits(double pi) {
	double err = fabs(pi - PI);

	if (err == 0) {
		return 15;
	}

	int digits = (int)floor(-log10(err));
	
	return digits < 0 ? 0 : digits;
}

void PrintPi(const char* name, double pi, double t) {
	cout << name
		<< "pi = " << setprecision(15) << pi
		<< "   error = " << scientific << setprecision(3) << fabs(pi - PI) << fixed
		<< "   digits = " << CorrectDigits(pi)
		<< "   time = " << setprecision(3) << t * 1000000 << " mcs" << endl;
}


// Task 1
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// "Лр-4.pdf", p. 1-2
// https://learn.microsoft.com/en-us/cpp/build/reference/openmp-enable-openmp-2-0-support
// https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros
void TestTask1() {
#ifdef _OPENMP
	printf("_OPENMP Defined\n");
#else
	printf("_OPENMP UnDefined\n");
#endif
}


// Task 2
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void TestTask2() {
	// "Параллелизм150903_u.pdf", p. 159
	// https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsysteminfo
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("GetSystemInfo.dwNumberOfProcessors = %d\n", si.dwNumberOfProcessors);

	// "Пз-4.pdf", p. 49
	// https://learn.microsoft.com/en-us/cpp/parallel/openmp/reference/openmp-functions#omp-get-max-threads
	int count = omp_get_max_threads();
	printf("omp_get_max_threads = %d\n", count);

	cout << ((int)si.dwNumberOfProcessors == count ? "values are equal" : "values are different") << endl;
}

// Task 3
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// https://en.wikipedia.org/wiki/Pi#Infinite_series
double CalcPiNilakantha(int n) {
	double s = 0;

	for (int k = 1; k <= n; k++) {
		double d = 2.0 * k;
		double t = 4.0 / (d * (d + 1) * (d + 2));

		s += (k % 2 == 1) ? t : -t;
	}
	return 3 + s;
}

// https://en.wikipedia.org/wiki/Wallis_product
double CalcPiWallis(int n) {
	double p = 1;

	for (int k = 1; k <= n; k++) {
		double d = 4.0 * k * k;
		p *= d / (d - 1);
	}

	return 2 * p;
}

// https://en.wikipedia.org/wiki/Basel_problem
double CalcPiEuler(int n) {
	double s = 0;

	for (int k = 1; k <= n; k++) {
		s += 1.0 / ((double)k * k);
	}

	return sqrt(6 * s);
}

template <class Function>
int FindIterations(Function&& f, double eps) {
	int hi = 1;

	while (fabs(f(hi) - PI) >= eps) {
		hi *= 2;
	}

	int lo = hi / 2;
	while (hi - lo > 1) {
		int mid = lo + (hi - lo) / 2;
		if (fabs(f(mid) - PI) < eps) {
			hi = mid;
		}
		else {
			lo = mid;
		}
	}

	return hi;
}

// https://en.cppreference.com/w/cpp/language/cv
template <class Function>
double GetCallTime(Function&& f, int n) {
	int calls = 1 + 1000000 / n;
	
	volatile int vn = n;
	volatile double r = 0;

	double t = GetMinTime([&] {
		for (int i = 0; i < calls; i++) {
			r = f(vn);
		}
		}, 5);

	return t / calls;
}

const int nMethods = 3;
const char* methodNames[nMethods] = { "Nilakantha: ", "Wallis:     ", "Euler:      " };
const char* shortNames[nMethods] = { "Nilakantha", "Wallis", "Euler" };
double (*methods[nMethods])(int) = { CalcPiNilakantha, CalcPiWallis, CalcPiEuler };
const int opsPerIter[nMethods] = { 7, 5, 3 };

const int nEps = 3;
const double epsList[nEps] = { 1e-3, 1e-5, 1e-7 };
int iters[nMethods][nEps];
double times[nMethods][nEps];

void TestTask3() {
	for (int e = 0; e < nEps; e++) {
		cout << "eps = " << scientific << setprecision(0) << epsList[e] << fixed << endl;
		for (int m = 0; m < nMethods; m++) {
			int n = FindIterations(methods[m], epsList[e]);
			double t = GetCallTime(methods[m], n);
			iters[m][e] = n;
			times[m][e] = t;

			cout << "  " << methodNames[m] << "n = " << setw(8) << n;
			PrintPi("   ", methods[m](n), t);
		}
	}
}

// Task 4
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// "Пз-4.pdf", p. 48
void TestTask4() {
	const int e = nEps - 1;
	int best = 0;
	double bestOps = 1.0e18;

	cout << "eps = " << scientific << setprecision(0) << epsList[e] << fixed << endl;
	for (int m = 0; m < nMethods; m++) {
		double ops = (double)iters[m][e] * opsPerIter[m];
		double growth = (double)iters[m][e] / iters[m][e - 1];

		cout << "  " << methodNames[m]
			<< "n = " << setw(8) << iters[m][e]
			<< "   ops = " << setw(10) << setprecision(0) << ops
			<< "   n growth (eps / 100) = " << setw(6) << setprecision(1) << growth
			<< "   time = " << setprecision(3) << times[m][e] * 1000000 << " mcs" << endl;

		if (ops < bestOps) {
			bestOps = ops;
			best = m;
		}
	}
	cout << "best: " << shortNames[best] << endl;
}

// Task 5
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
void TestTask5() {
	const int maxN = 10000000;
	double s = 0;
	double prev = 3;
	int lastChange = 0;
	int nextPrint = 10;

	for (int k = 1; k <= maxN; k++) {
		double d = 2.0 * k;
		double t = 4.0 / (d * (d + 1) * (d + 2));
		s += (k % 2 == 1) ? t : -t;

		if (3 + s != prev) {
			lastChange = k;
		}

		prev = 3 + s;

		if (k == nextPrint) {
			cout << "n = " << setw(9) << k
				<< "   pi = " << setprecision(15) << 3 + s
				<< "   error = " << scientific << setprecision(3) << fabs(3 + s - PI) << fixed
				<< "   digits = " << CorrectDigits(3 + s) << endl;
			nextPrint *= 10;
		}
	}

	cout << "value does not change after n = " << lastChange << endl;
}

int main() {
	cout << fixed << setprecision(4);

	cout << "Task 1 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask1();

	cout << "\nTask 2 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask2();

	cout << "\nTask 3 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask3();

	cout << "\nTask 4 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask4();

	cout << "\nTask 5 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask5();

	cout << endl;


	system("pause");
	
	
	
	return 0;
}