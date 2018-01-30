//
//  ippool.cpp
//  ippool
//
//  Created by Кирилл Делимбетов on 24.01.2018.
//

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include "ippool.hpp"

namespace netutils {
	namespace {
		struct FastPool {
			Range ranges[2];
			size_t count = 0;
		};
		
		bool overlap(const Range& left, const Range& right) {
			return right.first <= left.second && right.second >= left.first;
		}
		
		void diff2Ranges(FastPool& res, const Range& left, const Range& right) {
			res.count = 0;
			
			if(left.first < right.first)
				res.ranges[res.count++] = {left.first, std::min(left.second, right.first - 1)};
			
			if(left.second > right.second)
				res.ranges[res.count++] = {std::max(left.first, right.second + 1), left.second};
		}
		
		// assuming sorted
		void merge(Pool& pool) {
			auto removed = size_t{0};
			
			if(pool.empty())
				return;
			
			for(auto i = size_t{0}; i < pool.size() - 1 - removed; ++i) {
				if(overlap(pool[i], pool[i + 1])) {
					pool[i].second = pool[i + 1].second;
					std::swap(pool[i + 1], pool.back());
					++removed;
					--i;
				}
			}
			
			if(removed)
				pool.erase(pool.end() - int32_t(removed));
		}
	}
	
	Pool find_diff(const Pool& old_pool, const Pool& new_pool) {
		Pool result;
		// not the best solution to build this structure inside the function instead of supporting Interval Tree
		std::vector<IPAddress> maxUppBoundUpTo(new_pool.size());
		
		// assuming pools are sorted
		// check in debug
		assert(std::is_sorted(old_pool.cbegin(), old_pool.cend(), rangeCmp));
		assert(std::is_sorted(new_pool.cbegin(), new_pool.cend(), rangeCmp));
		
		// write upp bounds of ranges up to index in new_pool into container so we wont have to traverse whole new_pool on every iteration
		for(auto idx = size_t{0}; idx < new_pool.size(); ++idx)
			maxUppBoundUpTo[idx] = std::max(new_pool[idx].second, (idx > 0 ? maxUppBoundUpTo[idx - 1] : 0));
		
		// find diff
		// easily parallelizable
		for(const auto& orange : old_pool) {
			// find first range which first is greater than orange.second
			const auto uppbound = std::upper_bound(new_pool.cbegin(), new_pool.cend(), orange.second, [](const auto val, const auto& range) {
				return val < range.first;
			});
			// find first range which second is not less than orange.first
			const auto lowbound = std::lower_bound(maxUppBoundUpTo.cbegin(), maxUppBoundUpTo.cend(), orange.first, [](const auto uppbound, const auto val) {
				return uppbound < val;
			});
			const auto lowboundIdx = std::distance(maxUppBoundUpTo.cbegin(), lowbound);
			auto curr = orange;
			auto currHasValue = true; // no optional in c++ lib shipped with xcode :D
			FastPool nonOverlappingDiff;
			
			// iterate over overlapping ranges
			for(auto iter = new_pool.cbegin() + lowboundIdx; iter < uppbound; ++iter) {
				diff2Ranges(nonOverlappingDiff, curr, *iter);
				
				switch(nonOverlappingDiff.count) {
					case 0: // 0 means no diff -> no point looping any more
						currHasValue = false;
						iter = uppbound; // make it upp bound so loop ends
						break;
					case 1:
						curr = nonOverlappingDiff.ranges[0];
						break;
					case 2:
						// if orange contains iter, lower resulting range is one of final diffs
						result.push_back(nonOverlappingDiff.ranges[0]);
						curr = nonOverlappingDiff.ranges[1];
						break;
				}
			}
			
			if(currHasValue)
				result.push_back(curr);
		}
		
		// input vectors ranges may overlap by the problem def, but i dont see why not merge overlaps for output
		merge(result);
		// output is sorted as is input
		std::sort(result.begin(), result.end(), rangeCmp);
		
		return result;
	}
}
