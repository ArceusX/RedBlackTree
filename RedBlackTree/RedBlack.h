#pragma once
#include <stack>		// For traversal on Tree's copy constructor
#include <cstddef>		// To access to ptrdiff_t for Set's alias
#include <stdexcept>
#include <cassert>

namespace RedBlack  {
// NOTE: To test. Set<Point, Point::CMP> s; #include <iostream>
//struct Point {
//	int x, y;
//	Point(const int& x, const int& y): x(x), y(y) {}
//	Point(int&&		 x, int&&	   y): x(x), y(y) {}
//	friend std::ostream& operator<<(std::ostream& os, const Point& c) {
//		os << "Point(x = " << c.x << ", y = " << c.y << ")";
//		return os;
//	}
//	bool operator==(const Point& o) const {
//		return (x == o.x && y == o.y);
//	}
//	bool operator!=(const Point& o) const {return !(*this == o);}
//	struct CMP {
//		bool operator() (const Point& a, const Point& b) const {
//			if (a.x == b.x) return a.y < b.y;
//			else			return a.x < b.x;
//		}
//	};
//};

template<class T, class Compare = std::less<T>> class Set;

// T must overload == and !=. Sample for Compare:
// struct CMP {bool operator () (const T& a, const T& b) const {..}};
template<class T, class Compare>
struct Tree {
	class Node {
		friend Tree<T, Compare>;
		// Store * to swap between Nodes without copying
		T*	 key;
		bool isRed; // Use to balance tree
		Node *parent, *left = nullptr, *right = nullptr;

		void  swapKey(T*& e) noexcept { // Swap contents
			T* tmp = key; key = e; e = tmp;
		}

	public:
		// insert(Iter, Iter) calls insert(key) calls Node(const T&,..)
		Node(const T& v, bool isRed = true, Node* parent = nullptr):
			key(new T(v)), isRed(isRed), parent(parent) {}

		// insert(init_list) calls  insert(key) calls Node(T&&	   ,..)
		Node(	  T&& v, bool isRed = true, Node* parent = nullptr):
			key(new T(v)), isRed(isRed), parent(parent) {}

		const T& operator *() const {return *key;}

		// To get its uncle, call sibling() on its parent
		Node* sibling();
		Node* inorderNext();
		Node* inorderPrev();

		~Node() {delete key; delete left; delete right;}
	};

	Tree(): root(nullptr), sz(0) {}

	// Insert as root: black Node holding key. Root is always black
	Tree(const T& key): root(new Node(	   key, false)), sz(1) {}
	Tree(	  T&& key): root(new Node((T&&)key, false)), sz(1) {}

	template<typename Iter>
	Tree(Iter it, Iter end): Tree() {insert(it, end);}
	Tree(std::initializer_list<T> keys): Tree() {
		insert(keys);
	}

	Tree(const Tree& src);
	Tree& operator=(const Tree& src) {
		// Copy src. Swap data of copy and OG. Copy deletes OG's data
		if (this != &src) {
			Tree  cpy(src);
			Node* ptr = root; root = cpy.root; cpy.root = ptr;
			sz		  = src.sz;
		}
		return *this;
	}
	Tree& operator=(Tree&& oth) noexcept {
		swap(*this, oth);
		return *this;
	}
	Tree& operator=(std::initializer_list<T> keys) {
		clear();
		insert(keys);
		return *this;
	}

	friend void swap(Tree& a, Tree& b) noexcept {
		Node*  tRoot = a.root; a.root = b.root; b.root = tRoot;
		size_t tSz	 = a.sz  ; a.sz   = b.sz  ; b.sz   = tSz;
	}

	void clear() noexcept {delete root; root = nullptr; sz = 0;}
	~Tree() {delete root;}

	// Trees to match keys, not Node* or tree structure
	bool operator==(const Tree& o);
	bool operator!=(const Tree& o) {return !(*this == o);}

	bool less(const T& a, const T& b) const {
		return cmp(a, b); 	// Get <Compare>'s result
	}

	Node*  min () const; // Node having min key by Compare
	Node*  max () const; // Node having max key by Compare
	size_t size() const {return sz;}

	//------------------Implementation------------------
	// If didn't find exact key && !getClosest, return null
	Node*  find(const T& key, bool getClosest = true) const;

	// Pair: (1) Holds target key	   (2) true if success
	// If toMove, move construct T for Node::key; else copy construct
	std::pair<Node*, bool> insert(const T& key, bool toMove = false);

	// Re: Count of inserts of keys not already present
	template<typename Iter>
	size_t insert(Iter it, Iter end);
	size_t insert(std::initializer_list<T> keys);

	// Specialize: To iterate over and erase from tree at same time
	size_t erase(
		Set<T, Compare>::iterator it, Set<T, Compare>::iterator end);

	// Re: Count of erases of keys found
	template<typename Iter>
	size_t erase(Iter it, Iter end);
	size_t erase(std::initializer_list<T> keys);

	// Pair: (1) Holds successor key	(2) true if key found
	std::pair<Node*, bool> erase(const T& key);

private:
	Node*   root;
	size_t  sz;
	inline static Compare cmp = Compare(); // In C++ < 17, omit "inline static" 

	// Helper to restore Red-Black properties
	void balanceInsert(Node* current);
	void balanceErase (Node* toErase);
};

// Red-Black Tree backend enables ordered key iteration
template<class T, class Compare>
class Set {
	Tree<T, Compare>* tree;
public:
	
	// For Set, const_iterator and iterator function identically
	// as keys cannot be modified (only erased and reinserted)
	class iterator {
		friend Set <T, Compare>;
		friend Tree<T, Compare>;
		Tree<T, Compare>*		tree;
		Tree<T, Compare>::Node* ptr;
		bool					isForward;

		// Private to ensure tree != null as only Set, 
		// which initialized tree, can initialize iterator
		iterator(
			Tree<T, Compare>* tree,
			Tree<T, Compare>::Node* ptr,
			bool isForward = true):
			tree(tree), ptr(ptr), isForward(isForward) {}
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = const T;
		using reference         = const T&;
		using pointer           = const T*;

		// reverse_iterator's ++() holds predecessor instead of successor
		bool isReversed() const { return !isForward; }

		iterator(const iterator& oth):
			tree(oth.tree), ptr(oth.ptr), isForward(oth.isForward) {}
		iterator& operator=(const Set<T, Compare>::iterator& oth) {
			tree = oth.tree; ptr = oth.ptr; isForward = oth.isForward;
			return *this;
		}

		bool operator==(const iterator& o) {
			assert(isForward == o.isForward &&
				"Cannot compare iterator[forward] and "
				"iterator[reversed] types of RedBlack::Set");
			assert(tree == o.tree			&&
				"Cannot compare iterators to different RedBlack::Set objects");

			return ptr == o.ptr;
		};
		bool operator!=(const iterator& o) { return !(*this == o); };

		reference operator *() const {
			if (ptr) return **ptr;
			throw new std::out_of_range(
				"Can't dereference out-of-range RedBlack::Set iterator");
		}

		pointer   operator->() const {
			return &(**this);
		}
		friend std::ostream& operator<<(std::ostream& os, const iterator& it) {
			os << *it; return os;
		}
		// Distinguish RedBlack::Set and std::set
		operator bool() const {return ptr && tree->size() != 0;}

		iterator& operator++() {
			if (tree->size() == 0) {
				throw new std::out_of_range(
					"Can't increment iterator of empty RedBlack::Set");
			}

			if (!ptr) {
				std::string s("Can't increment RedBlack::Set iterator past ");
				s.append(isForward ? "end()" : "rend()");
				throw new std::out_of_range(s);
			}
			ptr = (isForward ? ptr->inorderNext() : ptr->inorderPrev());
			
			return *this;
		}
		iterator  operator++(int) {
			iterator tmp(tree, ptr, isForward);
			++(*this);
			return tmp;
		}
		iterator& operator--() {
			if (tree->size() == 0) {
				throw new std::out_of_range(
					"Can't decrement iterator of empty RedBlack::Set");
			}

			if (ptr) { // Go in reverse
				if (isForward) ptr = ptr->inorderPrev();
				else		   ptr = ptr->inorderNext();

				//[ |r]begin() has null inorder[Prev|Next]()
				if (!ptr) {
					std::string s("Can't decrement RedBlack::Set iterator past ");
					s.append(isForward ? "begin()" : "rbegin()");
					throw new std::out_of_range(s);
				}
			}
			else { // If !ptr..
				// .. && isForward (ie  end()), get Node
				// whose successor   == null, that being
				// rightmost Node, which has max key
				if (isForward) ptr = tree->max();

				// .. && reversed  (ie  rend()), get Node
				// whose predecessor == null, that being
				// leftmost  Node, which has min key
				else		   ptr = tree->min();
			}

			return *this;
		}
		iterator  operator--(int) {
			iterator tmp(tree, ptr, isForward);
			--(*this);
			return tmp;
		}
	};
	iterator begin (){ return iterator(tree, tree->min());}
	iterator end   (){ return iterator(tree, nullptr);}
	iterator rbegin(){ return iterator(tree, tree->max(), false);}
	iterator rend  (){ return iterator(tree, nullptr, false);}

	using difference_type = iterator::difference_type;
	using value_compare   = Compare;
	using key_compare	  = Compare;
	using pointer		  = T*;
	using reference		  = T&;
	using value_type	  = T;
	using key_type		  = T;

	Set(): tree(new Tree<T, Compare>()) {}
	template<typename Iter>
	Set(Iter it, Iter end): tree(new Tree<T, Compare>(it, end)) {}
	Set(std::initializer_list<T> keys) :
		tree(new Tree<T, Compare>(keys)) {}

	Set(const Set& src): tree(new Tree<T, Compare>(*(src.tree))) {}
	Set(Set&& src) noexcept: tree(new Tree<T, Compare>()) {
		swap(this->tree, src.tree);
	}
	Set& operator=(Set&& src) noexcept {
		swap(*this, src);
		return *this;
	}
	Set& operator=(const Set& src) {
		*tree = *src.tree; return *this;
	}

	// Sets to match keys, not structure of Tree
	bool operator==(const Set& oth) {
		return *tree == *oth.tree;
	}
	bool operator!=(const Set& oth) {
		return *tree != oth.tree;
	}

	~Set() { delete tree; }

	//--------------------Modifiers--------------------

	// Re: Count of inserts of keys not already present
	template<class Iter>
	size_t insert(Iter it, Iter end) {
		return tree->insert(it, end);
	}
	size_t insert(std::initializer_list<T> keys) {
		return tree->insert(keys);
	}

	// Re: (1) holds * to key in Set
	//	   (2) == true if key was not already present
	std::pair<iterator, bool> insert(	  T&& key) {
		auto x = tree->insert(key, true);  // Move T key
		return {iterator(tree, x.first), x.second};
	}
	std::pair<iterator, bool> insert(const T& key) {
		auto x = tree->insert(key, false); // Copy T key
		return {iterator(tree, x.first), x.second};
	}

	// Re: (1) holds * to key in Set, (2) == True if success
	// Construct key from args, pass to insert([T&& || const T&])
	template<class... Args>
	std::pair<iterator, bool> emplace(Args&&...args) {
		// forward: Relay same type ([l|r]value) as args passed
		return insert(T(std::forward<Args>(args)...));
	}

	// Re: Count of keys erased
	template<class Iter>
	size_t erase(Iter it, Iter end) {
		return tree->erase(it, end);
	}
	size_t erase(std::initializer_list<T> keys) {
		return tree->erase(keys.begin(), keys.end());
	}

	// Re: If (2) == true , (1) holds * to key's successor 
	//	   If (2) == false, (1) is Set::end(), holds null
	std::pair<iterator, bool> erase(const T& key) {
		auto x = tree->erase(key);
		return {iterator(tree, x.first), x.second};
	}
	template<class Iter>
	std::pair<iterator, bool> erase(Iter it) {
		return erase(*it);
	}

	friend void swap(Set& a, Set& b) noexcept {swap(*a.tree, *b.tree);}

	void clear() noexcept { tree->clear(); }

	//--------------------Operations--------------------

	// Re: If key is in Set, true; else, false
	bool	 count(const T& key) const {
		return tree->find(key, false);
	}

	// Re: If key is in Set, holds * to key; else, null
	iterator find(const T& key) const {
		return iterator(tree, tree->find(key, false));
	}

	// Re: min(x) >=key. If key is in Set, holds * to key
	//	   If not, holds * to key's successor
	iterator lower_bound(const T& key) const {
		typename Tree<T, Compare>::Node* x = tree->find(key);
		if (x &&  tree->less(**x, key)) { // If x <  key
			x = x->inorderNext();
		}
		return iterator(tree, x);
	}

	// Re: min(x) > key. Even if key is found, upper_bound(),
	//	   unlike lower_bound(), holds * to key's successor
	iterator upper_bound(const T& key) const {
		typename Tree<T, Compare>::Node* x = tree->find(key);
		if (x && !tree->less(key, **x)) { // If x <= key
			x = x->inorderNext();
		}
		return iterator(tree, x);
	}

	// Re: If key is in Set, hold * to (key, successor)
	//	   Else, iterators hold identical * to successor
	std::pair<iterator, iterator>
		equal_range(const T& key) const {
		return { lower_bound(key), upper_bound(key) };
	}

	//--------------------Observers--------------------

	size_t size () const noexcept { return tree->size(); }
	bool   empty() const noexcept { return tree->size() == 0; }

	// Re: Usually std::less<T>
	key_compare   key_comp  () const {return Compare();}
	value_compare value_comp() const {return Compare();}
};

#include "RedBlack.inl"
} // namespace RedBlack closed