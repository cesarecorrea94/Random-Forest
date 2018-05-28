/*
 * utils.hpp
 *
 *  Created on: 26 de mai de 2018
 *      Author: cesar
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#define swap(a,b) { auto tmp = a; a = b; b = tmp; }

template<typename instanceType, typename attrType>
void quicksort(
		instanceType ds[],
		unsigned nInstances,
		attrType (*getAttr)(instanceType)
) {
	if(nInstances <= 1) return;
	instanceType &pivo = ds[0];
	unsigned lo = 1, hi = nInstances-1;
	for( ;; ++lo, --hi) {
		while(lo < nInstances && getAttr(ds[lo]) <  getAttr(pivo)) ++lo;
		while(hi > 0		  && getAttr(ds[hi]) >= getAttr(pivo)) --hi;
		if(lo > hi) {
			swap(pivo, ds[hi]);
			quicksort(&ds[0], hi, getAttr);
			quicksort(&ds[hi+1], nInstances-(hi+1), getAttr);
			return;
		}
		swap(ds[lo], ds[hi]);
	}
}

#endif /* UTILS_HPP_ */
