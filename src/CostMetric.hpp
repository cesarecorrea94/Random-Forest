/*
 * CostMetric.hpp
 *
 *  Created on: 26 de mai de 2018
 *      Author: cesar
 */

#ifndef COSTMETRIC_HPP_
#define COSTMETRIC_HPP_

#include <cassert>
typedef double costType;
constexpr static costType cost_MAX = DBL_MAX;

template<typename instanceType, typename predictType>
class CostMetric {
public:
	virtual ~CostMetric() { }
	virtual void passInstance(const instanceType &instance) = 0;
	virtual costType costFunction() const = 0;
	virtual bool stop(predictType &value) const = 0;
};

template<typename instanceType, typename predictType>
class SDR : public CostMetric<instanceType, predictType> { // Standard Deviation Reduction
	enum position { esq, dir };
	enum { sum_xi_squared, sum_xi };
	unsigned _cardinality[2];
	typedef long double auxType;
	auxType _auxStructure[2][2];
public:
	SDR(instanceType ds[], unsigned nInstances)
: _cardinality { [esq]=0, [dir]=nInstances }, _auxStructure { {0} } {
		for(unsigned index = 0; index < nInstances; ++index) {
			instanceType &instance = ds[index];
			auxType value = instance.value();
			this->_auxStructure[dir][sum_xi] += value;
			value *= value;
			this->_auxStructure[dir][sum_xi_squared] += value;
		}
	}
	void passInstance(const instanceType &instance) override {
		this->_cardinality[esq]++;
		this->_cardinality[dir]--;
		// (xi - X)^2 = xi^2 - 2*xi*X + X^2
		auxType value = instance.value();
		this->_auxStructure[esq][sum_xi] += value;
		this->_auxStructure[dir][sum_xi] -= value;
		value *= value;
		this->_auxStructure[esq][sum_xi_squared] += value;
		this->_auxStructure[dir][sum_xi_squared] -= value;
	}
	costType costFunction() const override {
		unsigned total = this->_cardinality[esq] + this->_cardinality[dir];
		return (this->SD(esq) * this->_cardinality[esq] +
				this->SD(dir) * this->_cardinality[dir] ) / total;
	}
	bool stop(predictType &value) const override {
		assert(this->_cardinality[esq] == 0);
		assert(this->_cardinality[dir] >= 1);
		value = this->average(dir);
		return (this->_cardinality[dir] == 1 || this->CV(dir) < 0.1);
	}
	inline auxType average(position pos) const {
		return this->_auxStructure[pos][sum_xi] / this->_cardinality[pos];
	}
	inline auxType SD(position pos) const { // Standard Deviation
		auxType average = this->average(pos);
		return	this->_auxStructure[pos][sum_xi_squared] +
				-2 * this->_auxStructure[pos][sum_xi] * average +
				this->_cardinality[pos] * (average * average);
	}
	inline auxType CV(position pos) const { // Coefficiet of Variation
		return this->SD(pos) / this->average(pos);
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
	void passInstance(const instanceType &instance) override {
		this->auxStructure[esq][instance.classe()]++;
		this->auxStructure[dir][instance.classe()]--;
		this->countPasses++;
	}
	costType costFunction() const override {
		return 0;
	}
	bool stop(predictType &value) const override {
		if(this->cardinality == 1) {
			// value = ...
			return true;
		}
		return false;
	}
};


#endif /* COSTMETRIC_HPP_ */
