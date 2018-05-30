/*
 * CostMetric.hpp
 *
 *  Created on: 26 de mai de 2018
 *      Author: cesar
 */

#ifndef COSTMETRIC_HPP_
#define COSTMETRIC_HPP_

typedef double costType;
constexpr static costType cost_MAX = DBL_MAX;

template<typename instanceType>
class CostMetric {
public:
	virtual ~CostMetric() { }
	virtual void passInstance(instanceType instance) = 0;
	virtual costType costFunction() = 0;
};

template<typename instanceType, unsigned nInstances>
class SDR : public CostMetric<instanceType> { // Standard Deviation Reduction
	enum { esq, dir };
	enum { sum_xi_squared, sum_xi };
	double auxStructure[2][2];
	unsigned countPasses;
public:
	SDR(instanceType ds[nInstances]) : auxStructure { 0 }, countPasses(0) {
		for(unsigned index = 0; index < nInstances; ++index) {
			instanceType &instance = ds[index];
			int squared = instance.value() * instance.value();
			this->auxStructure[dir][sum_xi_squared] += squared;
			this->auxStructure[dir][sum_xi		]   += instance.value();
		}
	}
	void passInstance(instanceType instance) override {
		int squared = instance.value() * instance.value();
		this->auxStructure[esq][sum_xi_squared] += squared;
		this->auxStructure[esq][sum_xi		]   += instance.value();
		this->auxStructure[dir][sum_xi_squared] -= squared;
		this->auxStructure[dir][sum_xi		]   -= instance.value();
		// (xi - X)^2 = xi^2 - 2*xi*X + X^2
		this->countPasses++;
	}
	costType costFunction() override {
		unsigned count[] = { this->countPasses, nInstances-this->countPasses };
		double retorno = 0;
		for(unsigned pos = 0; pos < 2; ++pos) {
			double average = auxStructure[pos][sum_xi] / count[pos];
			retorno +=	(auxStructure[pos][sum_xi_squared] +
						-2 * auxStructure[pos][sum_xi] * average +
						this->countPasses * (average * average) ) *
						count[pos] / nInstances;
		}
		return retorno;
	}
};

template<typename instanceType, unsigned nClasses, unsigned nInstances>
class GiniImpurity : public CostMetric<instanceType> {
	enum { esq, dir };
	int auxStructure[2][nClasses];
	unsigned countPasses;
public:
	GiniImpurity(instanceType ds[nInstances]) : auxStructure { 0 }, countPasses(0) {
		for(unsigned index = 0; index < nInstances; ++index) {
			instanceType &instance = ds[index];
			this->auxStructure[dir][instance.classe()]++;
		}
	}
	void passInstance(instanceType instance) override {
		this->auxStructure[esq][instance.classe()]++;
		this->auxStructure[dir][instance.classe()]--;
		this->countPasses++;
	}
	costType costFunction() override {
		return 0;
	}
};


#endif /* COSTMETRIC_HPP_ */
