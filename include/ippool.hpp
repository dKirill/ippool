//
//  ippool.hpp
//  ippool
//
//  Created by Кирилл Делимбетов on 24.01.2018.
//

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace netutils {
	using IPAddress = uint32_t;
	using Range = std::pair<IPAddress, IPAddress>;
	// Предположу, что Pool более менее константен после создания, тк если в конце его жизни он еще должен содержать адреса,
	// помечаемые как устаревшие, видимо при выдаче адреса диапазон из которого адрес выдан не меняется.
	// По этой причине, основное требование к контейнеру - скорость чтения / поиска.
	//
	// Выбираю отсортированный вектор (сюда бы boost::container::flat_set), тк:
	// 1. Локальность памяти -> наибольшая производительность чтения
	// 2. Приложение серверное, аптайм может быть долгим, а значит фрагментация памяти может быть проблемой.
	// Вектор же аллоцируется в одном месте, в отличии от set/map. Не идеально, но лучше остальных вариантов (еще лучше - свой аллокатор).
	// 3. Если вектор отсортирован, алгоритмическая сложность поиска соответствует сету/мапе, но реальная скорость поиска намного выше.
	//
	// Вообще для такой задачи следует использовать https://en.wikipedia.org/wiki/Interval_tree 
	using Pool = std::vector<Range>;
	
	inline const auto rangeCmp = [](const Range& left, const Range& right) -> auto {
		const auto feq = left.first == right.first;
		
		return feq ? (left.second < right.second) : (left.first < right.first);
	};
	
	// input pools must be sorted
	Pool find_diff(const Pool& old_pool, const Pool& new_pool);
}

