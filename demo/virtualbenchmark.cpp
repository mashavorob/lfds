/*
 * virtualbenchmark.cpp
 *
 *  Created on: Dec 30, 2014
 *      Author: masha
 */

#include <cassert>
#include <chrono>
#include <iostream>
#include <ctime>
#include <cstdlib>

enum Operation {
	Plus, Minus,
};

class IArithmeticOperation {
public:
	virtual ~IArithmeticOperation() = 0;
	virtual int op(int a, int b) = 0;
};
IArithmeticOperation::~IArithmeticOperation() {
}

class COperationPlus: public IArithmeticOperation {
private:
	friend class CArythmetricFabric;

	COperationPlus() {
	}
public:
	~COperationPlus() {
	}
public:
	int op(int a, int b) {
		return a + b;
	}
};

class COperationMinus: public IArithmeticOperation {
private:
	friend class CArythmetricFabric;

	COperationMinus() {
	}
public:
	~COperationMinus() {
	}
public:
	int op(int a, int b) {
		return a - b;
	}
};

class CArythmetricFabric {
public:

	CArythmetricFabric() {
	}

	IArithmeticOperation* Create(const Operation code) const {
		static COperationPlus plus;
		static COperationMinus minus;
		switch (code) {
		case Plus:
			return &plus;
		case Minus:
			return &minus;
		}
		assert(false);
		return nullptr;
	}
};

template<Operation code>
struct CArithmeticOperationImpl {
	int operator()(int a, int b) const;
};

template<>
struct CArithmeticOperationImpl<Plus> {
	int operator()(int a, int b) const {
		return a + b;
	}
};

template<>
struct CArithmeticOperationImpl<Minus> {
	int operator()(int a, int b) const {
		return a - b;
	}
};

struct VirtualRunner {
	int operator()(const Operation code, std::size_t repetitions, int a, int b) const {
		static const CArythmetricFabric fabric;
		IArithmeticOperation* p = fabric.Create(code);

		int summ = 0;
		do {
			summ += p->op(++a, ++b);
		} while (--repetitions);
		return summ;
	}
	static const char* name() {
		return "Virtual Function Runner";
	}
};

template<Operation code>
struct TemplateRunnerImpl {
	int operator()(std::size_t repetitions, int a, int b) const {
		CArithmeticOperationImpl<code> operation;
		int summ = 0;
		do {
			summ += operation(++a, ++b);
		} while (--repetitions);
		return summ;
	}
};

struct TemplateRunner {
	int operator()(const Operation code, std::size_t repetitions, int a, int b) const {
		switch (code) {
		case Plus:
			return TemplateRunnerImpl<Plus>()(repetitions, a, b);
		case Minus:
			return TemplateRunnerImpl<Minus>()(repetitions, a, b);
		}
		assert(false);
		return 0;
	}
	static const char* name() {
		return "Template Function Runner";
	}
};

template<class Runner>
struct Benchmark {
	static const std::size_t repcount = 1000000000;

	typedef std::chrono::high_resolution_clock 	clock_type;
	typedef std::chrono::duration<double>		seconds_type;
	typedef clock_type::duration 				duration_type;
	typedef Runner								runner_type;

	int operator()(int a, int b)
	{
		std::cout
			<< "Benchmark for " << runner_type::name() << std::endl
			<< "---------------------------------" << std::endl;

		int r = runtest(Plus, a, b);
		r += runtest(Minus, a, b);
		std::cout
			<< "Benchmark for " << runner_type::name() << std::endl
			<< "---------------------------------" << std::endl << std::endl;
		return r;
	}
	int runtest(const Operation code, int a, int b)
	{
		seconds_type 	duration;
		runner_type 	runner;


		std::cout << "running " << runner_type::name() <<
				(code == Plus? " (Plus Operation)" : " (Minus Operation)") << std::endl;
		auto start_point = clock_type::now();
		int r = runner(code, repcount, a, b);
		auto hiresDuration = clock_type::now() - start_point;
		duration = std::chrono::duration_cast<seconds_type>(hiresDuration);
		std::cout << "duration: " << duration.count() << " secs" << std::endl;
		std::cout << duration.count()*1.0e9/static_cast<double>(repcount)
				<< " ns per operation" << std::endl;
		return r;
	}

};

void BenchmarkVirtualFunction() {
	Benchmark<VirtualRunner> vr;
	Benchmark<TemplateRunner> tr;

	std::cout << "Compare virtual function peformance vs template one" << std::endl;
	srand(time(nullptr));
	int a = rand();
	int b = rand();
	int r1 = vr(a, b);
	int r2 = tr(a, b);
	std::cout << "Result calculated by virtual functions: " << r1 << std::endl;
	std::cout << "Result calculated by template functions: " << r2 << std::endl;
}
