//
//  Test.cpp
//  ippool
//
//  Created by Кирилл Делимбетов on 24.01.2018.
//

// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <catch.hpp>
#include <ippool.hpp>

using namespace netutils;

TEST_CASE("Range cmp") {
	const auto lesser = Range{0, 10};
	const auto lesserext = Range{0, 12};
	const auto midinter = Range{5, 8};
	const auto midover = Range{5, 15};
	const auto bigger = Range{20, 30};
	
	REQUIRE(!rangeCmp(lesser, lesser));
	REQUIRE(rangeCmp(lesser, lesserext));
	REQUIRE(rangeCmp(lesser, midinter));
	REQUIRE(rangeCmp(lesser, midover));
	REQUIRE(rangeCmp(lesser, bigger));
	REQUIRE(!rangeCmp(bigger, lesser));
}

TEST_CASE("Pool diff: match") {
	auto oldpool = Pool{{ 0xAA'00'00'01, 0xFE'00'00'01 }, { 0xFF'00'00'01, 0xFF'00'00'FF }};
	auto newpool = Pool{oldpool};
	
	// do diff
	const auto diff = find_diff(oldpool, newpool);
	
	REQUIRE(diff.size() == 0);
}

TEST_CASE("Pool diff: no overlaps") {
	auto oldpool = Pool{{ 0xFF'00'00'01, 0xFF'00'00'FF }};
	auto newpool = Pool{{ 0xAA'00'00'01, 0xFE'00'00'01 }};
	
	// do diff
	const auto diff = find_diff(oldpool, newpool);
	
	REQUIRE(diff.size() == 1);
	REQUIRE(std::binary_search(diff.cbegin(), diff.cend(), oldpool[0]));
}

TEST_CASE("Pool diff: 1 overlap") {
	auto oldpool = Pool{{ 0xFF'00'00'01, 0xFF'00'00'FF }};
	auto newpool = Pool{{ 0xAA'00'00'01, 0xFF'00'00'AA }};
	
	// do diff
	const auto diff = find_diff(oldpool, newpool);
	
	REQUIRE(diff.size() == 1);
	REQUIRE(diff[0] == Range{ 0xFF'00'00'AB, 0xFF'00'00'FF });
}

TEST_CASE("Pool diff: multi overlaps") {
	auto oldpool = Pool{{ 0x10'00'AA'01, 0x10'00'BB'FF }, { 0x10'00'CC'01, 0x10'00'FF'FF }};
	auto newpool = Pool{{ 0x10'00'AA'10, 0x10'00'AA'30 }, { 0x10'00'BB'01, 0x10'00'BB'BB }, { 0x10'00'CC'10, 0x10'00'FF'BB }};
	
	// do diff
	const auto diff = find_diff(oldpool, newpool);
	
	REQUIRE(diff.size() == 5);
	REQUIRE(diff[0] == Range{ 0x10'00'AA'01, 0x10'00'AA'0F });
	REQUIRE(diff[1] == Range{ 0x10'00'AA'31, 0x10'00'BB'00 });
	REQUIRE(diff[2] == Range{ 0x10'00'BB'BC, 0x10'00'BB'FF });
	REQUIRE(diff[3] == Range{ 0x10'00'CC'01, 0x10'00'CC'0F });
	REQUIRE(diff[4] == Range{ 0x10'00'FF'BC, 0x10'00'FF'FF });
}

TEST_CASE("Pool diff: complex") {
	const auto exactMatches = Pool{
		{ 0xFF'00'00'01, 0xFF'00'00'FF },
		{ 0xAA'00'00'01, 0xAA'FF'00'FF }
	};
	auto oldpool = Pool{};
	auto newpool = Pool{};
	
	oldpool.reserve(6);
	newpool.reserve(4);
	
	// exact matches
	oldpool.insert(oldpool.begin(), exactMatches.cbegin(), exactMatches.cend());
	newpool.insert(newpool.begin(), exactMatches.cbegin(), exactMatches.cend());
	
	// partial matches
	oldpool.push_back({ 0x10'00'00'01, 0x10'FF'00'FF });
	oldpool.push_back({ 0x20'00'00'01, 0x20'00'88'FF });
	
	newpool.push_back({ 0x10'FF'00'01, 0x20'00'00'33 });
	
	// non matches
	const auto diff0 = Range{ 0x05'00'00'01, 0x09'FF'00'FF };
	const auto diff1 = Range{ 0x03'00'00'01, 0x05'00'88'FF };
	
	oldpool.push_back(diff0);
	oldpool.push_back(diff1);
	
	newpool.push_back({ 0xBB'FF'00'01, 0xCC'00'00'33 });
	
	// assuming sort before use
	std::sort(oldpool.begin(), oldpool.end(), rangeCmp);
	std::sort(newpool.begin(), newpool.end(), rangeCmp);
	
	// do diff
	const auto diff = find_diff(oldpool, newpool);
	
	REQUIRE(diff.size() == 3);
	REQUIRE(std::binary_search(diff.cbegin(), diff.cend(), Range{ diff1.first, diff0.second })); // find_diff merges overlapping ranges
	REQUIRE(std::binary_search(diff.cbegin(), diff.cend(), Range{ 0x10'00'00'01, 0x10'FF'00'00 }));
	REQUIRE(std::binary_search(diff.cbegin(), diff.cend(), Range{ 0x20'00'00'34, 0x20'00'88'FF }));
}

TEST_CASE("Pool diff: perf") {
	constexpr auto size = size_t{10000};
	auto oldpool = Pool{size};
	auto newpool = Pool{size};
	std::srand(unsigned(std::time(nullptr)));
	
	// fill
	for(auto& range : oldpool) {
		range.first = IPAddress(std::rand()) % 0xFE'FF'FF'FF + 0x1'00'00'00;
		range.second = IPAddress(std::rand()) % (((0xFF'FF'FF'FF - range.first) & 0xFF) + 1) + range.first;
	}
												 
	for(auto& range : newpool) {
		range.first = IPAddress(std::rand()) % 0xFE'FF'FF'FF + 0x1'00'00'00;
		range.second = IPAddress(std::rand()) % (((0xFF'FF'FF'FF - range.first) & 0xFF) + 1) + range.first;
	}
	
	// assuming sort before use
	std::sort(oldpool.begin(), oldpool.end(), rangeCmp);
	std::sort(newpool.begin(), newpool.end(), rangeCmp);
	
	// do diff
	const auto start = std::chrono::high_resolution_clock::now();
	const auto diff = find_diff(oldpool, newpool);
	std::cout << "Perf test for size=" << size << " took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms\n";
}
