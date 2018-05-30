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

template<typename instanceType>
class Node final {
protected:
	struct BaseCompare {
		virtual ~BaseCompare() { }
		virtual bool operator >=(instanceType instance) = 0;
		virtual bool operator !=(instanceType instance) = 0;
	};
	template<typename attrType>
	struct Compare final : BaseCompare {
		attrType value;
		attrType (*getAttr)(instanceType);
		Compare(attrType (*g)(instanceType)) : getAttr(g) { }
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
		BaseCompare *comparator;
	};
private:
	Node<instanceType> *left, *right;
	BaseCompare *comparator;
public:
	~Node() {
		delete comparator;
		delete left;
		delete right;
	}

	template<typename ...attrType_>
	Node(	instanceType ds[],
			unsigned cardinality,
			CostMetric<instanceType> &auxStruct,
			attrType_ (*...getAttr_)(instanceType));

	template<typename attrType, typename ...attrType_>
	Split testSplit(
			instanceType ds[],
			unsigned cardinality,
			CostMetric<instanceType> &auxStruct,
			attrType (*getAttr)(instanceType),
			attrType_ (*...getAttr_)(instanceType));

	template<typename attrType>
	Split testSplit(
			instanceType ds[],
			unsigned cardinality,
			CostMetric<instanceType> &auxStruct,
			attrType (*getAttr)(instanceType));
};

template<typename instanceType>
template<typename ...attrType_>
Node<instanceType>::Node(
		instanceType ds[],
		unsigned cardinality,
		CostMetric<instanceType> &auxStruct,
		attrType_ (*...getAttr_)(instanceType)
){
	if(cardinality > 1) {
		Split sp = this->testSplit<attrType_...>(ds, cardinality, auxStruct, getAttr_...);
		this->comparator = sp.comparator;
		this->left  = new Node<instanceType>( ds, 									sp.leftCardinality, auxStruct, getAttr_...);
		this->right = new Node<instanceType>(&ds[sp.leftCardinality], cardinality - sp.leftCardinality, auxStruct, getAttr_...);
	} else {
		// ...
		this->left = nullptr;
		this->right = nullptr;
	}
}

template<typename instanceType>
template<typename attrType, typename ...attrType_>
typename Node<instanceType>::Split Node<instanceType>::testSplit(
		instanceType ds[],
		unsigned cardinality,
		CostMetric<instanceType> &auxStruct,
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

template<typename instanceType>
template<typename attrType>
typename Node<instanceType>::Split Node<instanceType>::testSplit(
		instanceType ds[],
		unsigned cardinality,
		CostMetric<instanceType> &auxStruct,
		attrType (*getAttr)(instanceType)
) {
	quicksort(ds, cardinality, getAttr);
	Compare<attrType> auxCompare(getAttr), compare(getAttr);
	costType auxCost = cost_MAX, cost;
	unsigned index;
	auxCompare = ds[0];
	for(unsigned auxIndex = 0; auxIndex < cardinality; auxIndex++) {
		instanceType &instance = ds[auxIndex];
		// Procura-se o próximo valor que será o divisor do dataset
		// enquanto o valor não muda, vai passando os elementos para o conjunto esquerdo
		if(auxCompare != instance) {			// se o valor mudar
			auxCompare = instance;				// tem-se um novo divisor do dataset
			auxCost = auxStruct.costFunction();	// calcula-se o custo
			if(auxCost < cost) {				// se o custo for menor
				cost = auxCost;					// guarda o novo valor de divisão
				compare = auxCompare;
				index = auxIndex;
			}
			// a partir de agora, 'instance' fará parte do conjunto esquerdo
		}
		// move o elemento para o conjunto esquerdo
		auxStruct.passInstance(instance);
	}
	Split retorno = { .leftCardinality = index, .cost = cost, .comparator = new Compare<attrType>(compare) };
	return retorno;
}

#endif /* NODE_HPP_ */
