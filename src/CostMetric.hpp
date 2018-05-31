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

template<typename instanceType, typename predictType>
class CostMetric {
public:
	virtual ~CostMetric() { }
	virtual void passInstance(instanceType instance) = 0;
	virtual costType costFunction() = 0;
	virtual bool stop(predictType &value) = 0;
};

template<typename instanceType, typename predictType>
class SDR : public CostMetric<instanceType, predictType> { // Standard Deviation Reduction
	enum { esq, dir };
	enum { sum_xi_squared, sum_xi };
	unsigned cardinality;
	unsigned countPasses;
	double auxStructure[2][2];
public:
	SDR(instanceType ds[], unsigned nInstances)
: cardinality(nInstances), countPasses(0), auxStructure { 0 } {
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
		unsigned count[] = { this->countPasses, this->cardinality-this->countPasses };
		double retorno = 0;
		for(unsigned pos = 0; pos < 2; ++pos) {
			double average = auxStructure[pos][sum_xi] / count[pos];
			retorno +=	(auxStructure[pos][sum_xi_squared] +
						-2 * auxStructure[pos][sum_xi] * average +
						this->countPasses * (average * average) ) *
						count[pos] / this->cardinality;
		}
		return retorno;
	}
	bool stop(predictType &value) override {
		if(this->cardinality == 1) {
			value = this->auxStructure[dir][sum_xi] / this->cardinality;
			return true;
		}
		return false;
	}
};

template<typename instanceType, typename predictType, unsigned nClasses>
class GiniImpurity : public CostMetric<instanceType, predictType> {
	enum { esq, dir };
	unsigned cardinality;
	unsigned countPasses;
	int auxStructure[2][nClasses];
public:
	GiniImpurity(instanceType ds[], unsigned nInstances)
: cardinality(nInstances), countPasses(0), auxStructure { 0 } {
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
	bool stop(predictType &value) override {
		if(this->cardinality == 1) {
			// value = ...
			return true;
		}
		return false;
	}
};


#endif /* COSTMETRIC_HPP_ */
