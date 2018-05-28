/*
 * CostMetric.hpp
 *
 *  Created on: 26 de mai de 2018
 *      Author: cesar
 */

#ifndef COSTMETRIC_HPP_
#define COSTMETRIC_HPP_


template<typename instanceType>
class CostMetric {
public:
	virtual void passInstance(instanceType instance) = 0;
	virtual double costFunction() = 0;
};

template<typename instanceType, unsigned nInstances>
class SDR : public CostMetric<instanceType> {//Standard Deviation Reduction
	enum {esq, dir};
	enum { sum_xi_squared, sum_xi };
	double auxStructure[2][2];
	unsigned countPasses;
public:
	SDR(instanceType ds[nInstances]){
		this->countPasses = 0;
		for(unsigned i; i<2; ++i) for(unsigned j; j<2; ++j)
			this->auxStructure[i][j] = 0;
		for(unsigned index = 0; index < nInstances; ++index){
			instanceType &instance = ds[index];
			int squared = instance.value() * instance.value();
			this->auxStructure[dir][sum_xi_squared] += squared;
			this->auxStructure[dir][sum_xi		]   += instance.value();
		}
	}
	void passInstance(instanceType instance) {
		int squared = instance.value() * instance.value();
		this->auxStructure[esq][sum_xi_squared] += squared;
		this->auxStructure[esq][sum_xi		]   += instance.value();
		this->auxStructure[dir][sum_xi_squared] -= squared;
		this->auxStructure[dir][sum_xi		]   -= instance.value();
		// (xi - X)^2 = xi^2 - 2*xi*X + X^2
		this->countPasses++;
	}
	double costFunction() {
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
	enum {esq, dir};
	int auxStructure[2][nClasses];
	unsigned countPasses;
public:
	GiniImpurity(instanceType ds[nInstances]){
		this->countPasses = 0;
		for(unsigned i; i<2; ++i) for(unsigned j; j<nClasses; ++j)
			this->auxStructure[i][j] = 0;
		for(unsigned index = 0; index < nInstances; ++index){
			instanceType &instance = ds[index];
			this->auxStructure[dir][instance.classe()]++;
		}
	}
	void passInstance(instanceType instance) {
		this->auxStructure[esq][instance.classe()]++;
		this->auxStructure[dir][instance.classe()]--;
		this->countPasses++;
	}
	double costFunction(){
		return 0;
	}
};


#endif /* COSTMETRIC_HPP_ */
