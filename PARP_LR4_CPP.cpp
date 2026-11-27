#include <windows.h>
#include <omp.h>
#include <stdio.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>
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
int nCores = 1;

void TestTask2() {
	// "Параллелизм150903_u.pdf", p. 159
	// https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsysteminfo
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("GetSystemInfo.dwNumberOfProcessors = %d\n", si.dwNumberOfProcessors);
	nCores = (int)si.dwNumberOfProcessors;

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
int maxPrecN = 0;

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
	maxPrecN = lastChange;
}

// Task 6
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
double NilakanthaPart(int from, int to) {
	double s = 0;

	for (int k = from; k < to; k++) {
		double d = 2.0 * k;
		double t = 4.0 / (d * (d + 1) * (d + 2));

		s += (k % 2 == 1) ? t : -t;
	}

	return s;
}

struct ThreadPart {
	int from;
	int to;
	double sum;
};

// https://learn.microsoft.com/en-us/windows/win32/procthread/creating-threads
DWORD WINAPI ThreadProc(LPVOID param) {
	ThreadPart* part = (ThreadPart*)param;
	part->sum = NilakanthaPart(part->from, part->to);
	return 0;
}

double CalcPiWinThreads(int n, int nThreads) {
	vector<ThreadPart> parts(nThreads);
	vector<HANDLE> threads(nThreads);
	int chunk = n / nThreads;

	for (int i = 0; i < nThreads; i++) {
		parts[i].from = 1 + i * chunk;
		parts[i].to = parts[i].from + chunk;
		threads[i] = CreateThread(NULL, 0, ThreadProc, &parts[i], 0, NULL);
	}
	parts[nThreads - 1].to = n + 1;

	WaitForMultipleObjects(nThreads, threads.data(), TRUE, INFINITE);

	double s = 0;
	for (int i = 0; i < nThreads; i++) {
		CloseHandle(threads[i]);
		s += parts[i].sum;
	}

	return 3 + s;
}

struct Result {
	double pi;
	double time;
};

template <class Function>
Result Measure(Function&& calcPi, int n) {
	volatile int vn = n;
	volatile double pi = 0;
	int repeat = n >= 10000000 ? 3 : 20;

	double time = GetMinTime([&] { pi = calcPi(vn); }, repeat);

	return { pi, time };
}

Result RunSequential(int n) {
	return Measure(CalcPiNilakantha, n);
}

Result RunWindows(int n) {
	return Measure([](int m) { return CalcPiWinThreads(m, nCores); }, n);
}

void TestTask6() {
	Result seq = RunSequential(maxPrecN);
	Result win = RunWindows(maxPrecN);

	cout << "threads = " << nCores << "   n = " << maxPrecN << endl;
	PrintPi("sequential: ", seq.pi, seq.time);
	PrintPi("windows:    ", win.pi, win.time);
}

// Task 8
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// https://learn.microsoft.com/en-us/cpp/parallel/openmp/reference/openmp-clauses#reduction
double CalcPiOmp(int n, int nThreads) {
	double s = 0;

#pragma omp parallel for reduction(+: s) num_threads(nThreads)
	for (int k = 1; k <= n; k++) {
		double d = 2.0 * k;
		double t = 4.0 / (d * (d + 1) * (d + 2));

		s += (k % 2 == 1) ? t : -t;
	}

	return 3 + s;
}

Result RunOpenMP(int n) {
	return Measure([](int m) { return CalcPiOmp(m, nCores); }, n);
}

void TestTask8() {
	Result seq = RunSequential(maxPrecN);
	Result omp = RunOpenMP(maxPrecN);

	cout << "threads = " << nCores << "   n = " << maxPrecN << endl;
	PrintPi("sequential: ", seq.pi, seq.time);
	PrintPi("openmp:     ", omp.pi, omp.time);
}

// Task 9
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void TestTask9() {
	const int sizes[] = { maxPrecN, 1000000, 10000000, 100000000 };

	cout << "        n   speedup windows   speedup openmp" << endl;
	for (int n : sizes) {
		double seqTime = RunSequential(n).time;
		double winTime = RunWindows(n).time;
		double ompTime = RunOpenMP(n).time;

		cout << setw(9) << n << setprecision(2)
			<< setw(18) << seqTime / winTime
			<< setw(17) << seqTime / ompTime << endl;
	}
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

	cout << "\nTask 6 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask6();

	cout << "\nTask 8 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask8();

	cout << "\nTask 9 -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n";
	TestTask9();

	cout << endl;


	system("pause");
	
	
	
	return 0;
}