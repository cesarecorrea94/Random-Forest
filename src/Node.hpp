/*
 * Node.hpp
 *
 *  Created on: 22 de mai de 2018
 *      Author: cesar
 */

#ifndef NODE_HPP_
#define NODE_HPP_
#include <float.h> // DBL_MAX

#include "CostMetric.hpp"
#include "utils.hpp"

template<class costMetric, typename instanceType, typename predictType>
class Node final {
protected:
	struct AbstractComparator {
		virtual ~AbstractComparator() { }
		virtual bool operator >=(instanceType instance) = 0;
		virtual bool operator !=(instanceType instance) = 0;
	};
	template<typename attrType>
	struct Comparator final : AbstractComparator {
		attrType value;
		attrType (*getAttr)(instanceType);
		Comparator(attrType (*g)(instanceType)) : getAttr(g) { }
		bool operator >=(instanceType instance) override {
			return this->value >= this->getAttr(instance);
		}
		bool operator !=(instanceType instance) override {
			return this->value != this->getAttr(instance);
		}
		void operator =(instanceType instance) {
			this->value = this->getAttr(instance);
		}
	};
	struct Split final {
		unsigned leftCardinality;
		costType cost;
		AbstractComparator *comparator;
	};
private:
	Node<costMetric, instanceType, predictType> *left, *right;
	AbstractComparator *comparator;
	predictType value;
public:
	~Node() {
		delete comparator;
		delete left;
		delete right;
	}

	template<typename ...attrType_>
	Node(	instanceType ds[],
			unsigned cardinality,
			attrType_ (*...getAttr_)(instanceType));

	template<typename attrType, typename ...attrType_>
	Split testSplit(
			instanceType ds[],
			unsigned cardinality,
			const costMetric &auxStruct,
			attrType (*getAttr)(instanceType),
			attrType_ (*...getAttr_)(instanceType));

	template<typename attrType>
	Split testSplit(
			instanceType ds[],
			unsigned cardinality,
			costMetric auxStruct,
			attrType (*getAttr)(instanceType));
};

template<class costMetric, typename instanceType, typename predictType>
template<typename ...attrType_>
Node<costMetric, instanceType, predictType>::Node(
		instanceType ds[],
		unsigned cardinality,
		attrType_ (*...getAttr_)(instanceType)
){
	costMetric auxStruct = costMetric(ds, cardinality);
	if(auxStruct.stop(this->value)) {
		this->comparator = nullptr;
		this->left = nullptr;
		this->right = nullptr;
	} else {
		Split sp = this->testSplit<attrType_...>(ds, cardinality, auxStruct, getAttr_...);
		this->comparator = sp.comparator;
		this->left  = new Node<costMetric, instanceType, predictType>(
				ds, sp.leftCardinality, getAttr_...);
		this->right = new Node<costMetric, instanceType, predictType>(
				&ds[sp.leftCardinality], cardinality - sp.leftCardinality, getAttr_...);
	}
}

template<class costMetric, typename instanceType, typename predictType>
template<typename attrType, typename ...attrType_>
typename Node<costMetric, instanceType, predictType>::Split Node<costMetric, instanceType, predictType>::testSplit(
		instanceType ds[],
		unsigned cardinality,
		const costMetric &auxStruct,
		attrType (*getAttr)(instanceType),
		attrType_ (*...getAttr_)(instanceType)
) {
	Split sp = this->testSplit<attrType>(ds, cardinality, auxStruct, getAttr);
	Split sp_pack = this->testSplit<attrType_...>(ds, cardinality, auxStruct, getAttr_...);
	if(sp.cost < sp_pack.cost){
		delete sp_pack.comparator;
		return sp;
	} else {
		delete sp.comparator;
		return sp_pack;
	}
}

template<class costMetric, typename instanceType, typename predictType>
template<typename attrType>
typename Node<costMetric, instanceType, predictType>::Split Node<costMetric, instanceType, predictType>::testSplit(
		instanceType ds[],
		unsigned cardinality,
		costMetric auxStruct,
		attrType (*getAttr)(instanceType)
) {
	quicksort(ds, cardinality, getAttr);
	Split retorno;
	Comparator<attrType> retorno_comparator(getAttr),
			comparator(getAttr);
	comparator = ds[0];
	costType cost = cost_MAX;
	for(unsigned index = 0; index < cardinality; index++) {
		instanceType &instance = ds[index];
		// Procura-se o próximo valor que será o divisor do dataset
		// enquanto o valor não muda, vai passando os elementos para o conjunto esquerdo
		if(comparator != instance) {			// se o valor mudar
			comparator = instance;				// tem-se um novo divisor do dataset
			cost = auxStruct.costFunction();	// calcula-se o custo
			if(cost < retorno.cost) {			// se o custo for menor
				retorno.cost = cost;			// guarda o novo valor de divisão
				retorno.leftCardinality = index;
				retorno_comparator = comparator;
			}
			// a partir de agora, 'instance' fará parte do conjunto esquerdo
		}
		// move o elemento para o conjunto esquerdo
		auxStruct.passInstance(instance);
	}
	retorno.comparator = new Comparator<attrType>(retorno_comparator);
	return retorno;
}

#endif /* NODE_HPP_ */
