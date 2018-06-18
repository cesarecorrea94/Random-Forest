/*
 * Node.hpp
 *
 *  Created on: 22 de mai de 2018
 *      Author: cesar
 */

#ifndef NODE_HPP_
#define NODE_HPP_

#include <float.h> // DBL_MAX
#include <iostream>
using std::ostream;
#include "CostMetric.hpp"
#include "utils.hpp"

template<class costMetric, typename instanceType, typename predictType>
class Node final {
protected:
	struct AbstractComparator {
		virtual ~AbstractComparator() { }
		virtual bool operator > (instanceType instance) const = 0;
		virtual bool operator !=(instanceType instance) const = 0;
		virtual void operator >>(ostream& out) const = 0;
		friend ostream& operator <<(ostream& out, const AbstractComparator& self) {
			self >> out;
			return out;
		}
	};
	template<typename attrType>
	struct Comparator final : AbstractComparator {
		attrType value;
		attrType (*getAttr)(instanceType);
		Comparator(attrType (*g)(instanceType)) : getAttr(g) { }
		void operator = (instanceType instance) {
			this->value = this->getAttr(instance);
		}
		bool operator > (instanceType instance) const override {
			return this->value > this->getAttr(instance);
		}
		bool operator !=(instanceType instance) const override {
			return this->value != this->getAttr(instance);
		}
		void operator >>(ostream& out) const override {
			out << (unsigned int*) this->getAttr <<  " < " << this->value << '\n';
		}
	};
	struct Split final {
		unsigned _leftCardinality;
		costType _cost;
		AbstractComparator *_comparator;
		Split() : _leftCardinality(0), _cost(cost_MAX), _comparator(nullptr) { }
	};
private:
	Node<costMetric, instanceType, predictType> *left, *right;
	AbstractComparator *comparator;
	predictType value;
public:
	friend ostream& operator <<(ostream& out, const Node& self) {
		static unsigned tabs = -1;
		++tabs;
		if(self.comparator == nullptr) {
			for(unsigned i = 0; i < tabs; ++i) out << "  ";
			out << self.value << '\n';
		} else {
			out << *self.right;
			for(unsigned i = 0; i < tabs; ++i) out << "  ";
			out << *self.comparator;
			out << *self.left;
		}
		--tabs;
		return out;
	}

	~Node() {
		delete comparator;
		delete left;
		delete right;
	}

	template<typename ...attrType_>
	Node(	instanceType ds[],
			unsigned cardinality,
			attrType_ (*...getAttr_)(instanceType));

	predictType predict(instanceType ds);

private:
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
predictType Node<costMetric, instanceType, predictType>::predict(instanceType instance) {
	if(this->comparator == nullptr) {
		return this->value;
	}
	return (*this->comparator) > instance ?
			this->left->predict(instance) :
			this->right->predict(instance);
}

template<class costMetric, typename instanceType, typename predictType>
template<typename ...attrType_>
Node<costMetric, instanceType, predictType>::Node(
		instanceType ds[],
		unsigned cardinality,
		attrType_ (*...getAttr_)(instanceType)
){
	costMetric auxStruct = costMetric(ds, cardinality);
	if(! auxStruct.stop(this->value)) {
		Split sp = this->testSplit<attrType_...>(ds, cardinality, auxStruct, getAttr_...);
		if(cardinality > sp._leftCardinality) { // && sp.leftCardinality > 0
			this->comparator = sp._comparator;
			this->left  = new Node<costMetric, instanceType, predictType>(
					ds, sp._leftCardinality, getAttr_...);
			this->right = new Node<costMetric, instanceType, predictType>(
					&ds[sp._leftCardinality], cardinality - sp._leftCardinality, getAttr_...);
			return;
		} else {
			delete sp._comparator;
		}
	}
	this->comparator = nullptr;
	this->left = nullptr;
	this->right = nullptr;
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
	if(sp._cost < sp_pack._cost){
		delete sp_pack._comparator;
		quicksort(ds, cardinality, getAttr);
		return sp;
	} else {
		delete sp._comparator;
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
	retorno._leftCardinality = cardinality;
	Comparator<attrType> retorno_comparator(getAttr),
			comparator(getAttr);
	comparator = ds[0];
	costType cost;
	for(unsigned index = 0; index < cardinality; index++) {
		instanceType &instance = ds[index];
		// Procura-se o próximo valor que será o divisor do dataset
		// enquanto o valor não muda, vai passando os elementos para o conjunto esquerdo
		if(comparator != instance) {			// se o valor mudar
			comparator = instance;				// tem-se um novo divisor do dataset
			cost = auxStruct.costFunction();	// calcula-se o custo
			if(cost < retorno._cost) {			// se o custo for menor
				retorno._cost = cost;			// guarda o novo valor de divisão
				retorno._leftCardinality = index;
				retorno_comparator = comparator;
			}
			// a partir de agora, 'instance' fará parte do conjunto esquerdo
		}
		// move o elemento para o conjunto esquerdo
		auxStruct.passInstance(instance);
	}
	retorno._comparator = new Comparator<attrType>(retorno_comparator);
	return retorno;
}


#endif /* NODE_HPP_ */
