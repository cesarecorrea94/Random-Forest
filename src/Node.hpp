/*
 * Node.hpp
 *
 *  Created on: 22 de mai de 2018
 *      Author: cesar
 */

#ifndef NODE_HPP_
#define NODE_HPP_
#include <limits.h> // INT_MAX

#include "CostMetric.hpp"

template<typename instanceType>
class Node {
	Node<instanceType> *left, *right;
protected:
	template<typename attrType>
	struct Compare {
		attrType value;
		attrType (*getAttr)(instanceType);
		virtual bool operator >=(instanceType instance) {
			return this->value >= this->getAttr(instance);
		}
		virtual bool operator !=(instanceType instance) {
			return this->value != this->getAttr(instance);
		}
	};
	template<typename attrType>
	struct Split {
		attrType value;
		double cost;
	};
public:
	//Node() { }
	//virtual ~Node() { }

	template<unsigned nInstances, typename attrType>
	inline Split<attrType> constructor(
			instanceType ds[nInstances],
			CostMetric<instanceType> &auxStruct,
			attrType (*getAttr)(instanceType)
	) {
		//Split<attrType> sp = this->testSplit<nInstances>(ds, auxStruct, getAttr);
		//return sp;
	}

	template<unsigned nInstances, typename attrType, typename ...attrType_>
	inline void* constructor(
			instanceType ds[nInstances],
			CostMetric<instanceType> &auxStruct,
			attrType (*getAttr)(instanceType),
			attrType_ (*...getAttr_)(instanceType));

	template<unsigned nInstances, typename attrType>
	Split<attrType> testSplit(
			instanceType ds[nInstances],
			CostMetric<instanceType> &auxStruct,
			attrType (*getAttr)(instanceType));
};

template<typename instanceType>
template<unsigned nInstances, typename attrType, typename ...attrType_>
void* Node<instanceType>::constructor(
		instanceType ds[nInstances],
		CostMetric<instanceType> &auxStruct,
		attrType (*getAttr)(instanceType),
		attrType_ (*...getAttr_)(instanceType)
) {
	//Split<attrType> sp = this->testSplit<nInstances>(ds, auxStruct, getAttr);
	//Split<attrType_...> sp_ = this->constructor<nInstances, attrType_...>(ds, auxStruct, getAttr_...);
	//return sp.cost < sp_.cost ? sp : sp_;
}

template<typename instanceType>
template<unsigned nInstances, typename attrType>
Node<instanceType>::Split<attrType> Node<instanceType>::testSplit(
		instanceType ds[nInstances],
		CostMetric<instanceType> &auxStruct,
		attrType (*getAttr)(instanceType)
) {
	quicksort(ds, nInstances, getAttr);
	Split<attrType> split,
	aux = { .value = getAttr(ds[0]),
			.cost = INT_MAX };
	for(unsigned index = 0; index < nInstances; index++) {
		instanceType &instance = ds[index];
		// Procura-se o próximo valor que será o divisor do dataset
		// enquanto o valor não muda, vai passando os elementos para o conjunto esquerdo
		if(aux.value != getAttr(instance)) {		// se o valor mudar
			aux.value = getAttr(instance);			// tem-se um novo divisor do dataset
			aux.cost = auxStruct.costFunction();	// calcula-se o custo
			if(aux.cost < split.cost) {				// se o custo for menor
				split = aux;						// guarda o novo valor de divisão
			}
			// a partir de agora, 'instance' fará parte do conjunto esquerdo
		}
		// move o elemento para o conjunto esquerdo
		auxStruct.passInstance(instance);
	}
	return split;
}

#endif /* NODE_HPP_ */
