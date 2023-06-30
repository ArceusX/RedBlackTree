#include "RedBlack.h"
#include <string>
#include <iostream>
#include <set>

int main() {
	using namespace RedBlack;
	Set<int> s({2, 20, 17, 12, 3, 6, 13, 10});
	s.erase(++s.begin(), s.end());
	for (const auto& i : s) {
		std::cout << i << " ";
	}
}