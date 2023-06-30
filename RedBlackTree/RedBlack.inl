// 4, 7, 23, 15, 5, 19, 14, 22, 8, 18, 16, 1,
// 2, 20, 17, 12, 3, 6, 13, 10, 0, 9, 21, 11;
// Insert:
// https://pages.cs.wisc.edu/~cs400/readings/Red-Black-Trees/
// https://www.geeksforgeeks.org/insertion-in-red-black-tree/
// Erase:
// https://www.geeksforgeeks.org/deletion-in-red-black-tree/

template<class T, class Compare>
typename Tree<T, Compare>::Node*
Tree<T, Compare>::Node::sibling() {
	if (parent) {
		if (this == parent->left) return parent->right;
		else return parent->left;
	}
	else return nullptr;
}

template<class T, class Compare>
typename Tree<T, Compare>::Node*
Tree<T, Compare>::Node::inorderNext() {
	Node* current = this;
	if (current->right) {
		current = current->right;
		while (current->left) {
			current = current->left;
		}
		return current;
	}

	// If can't find it down subtree, successor is
	// Node whose left subtree *this is rightmost
	// Node of: 1st left turn on backtrack up
	while (current->parent) {
		if (current == current->parent->left) {
			return current->parent;
		}
		current = current->parent;
	}

	return nullptr;
}

template<class T, class Compare>
typename Tree<T, Compare>::Node*
Tree<T, Compare>::Node::inorderPrev() {
	Node* current = this;
	if (current->left) {
		current = current->left;
		while (current->right) {
			current = current->right;
		}
		return current;
	}

	// If can't find it down subtree, predecessor is
	// Node whose right subtree *this is leftmost
	// Node of: 1st right turn on backtrack up
	while (current->parent) {
		if (current == current->parent->right) {
			return current->parent;
		}
		current = current->parent;
	}
	return nullptr;
}

template<class T, class Compare>
typename Tree<T, Compare>::Node* Tree<T, Compare>::min() const {
	Node* current = root;
	if (current) {
		while (current->left) current = current->left;
	}
	return current;
}

template<class T, class Compare>
typename Tree<T, Compare>::Node* Tree<T, Compare>::max() const {
	Node* current = root;
	if (current) {
		while (current->right) current = current->right;
	}
	return current;
}

// Traverse preorder. On Node*, push ->right to stack, then
// traverse ->left. If ->left doesn't exist, go to stack.top()
template<class T, class Compare>
Tree<T, Compare>::Tree(const Tree<T, Compare>& src) {
	if (!src.root) {
		root = nullptr;
		sz   = 0;
		return;
	}
	root = new Node(*src.root->key);
	sz	 = src.sz;

	Node *ptr = root, *srcPtr = src.root;
	std::stack<Tree<T, Compare>::Node*> stack;
	while (true) {
		if (srcPtr->right) {
			ptr->right = new Node(
				*srcPtr->right->key, srcPtr->right->isRed, ptr);

			stack.push(ptr->right); stack.push(srcPtr->right);
		}

		if (srcPtr->left) {
			ptr->left = new Node(
				*srcPtr->left->key, srcPtr->left->isRed, ptr);

			srcPtr = srcPtr->left; ptr = ptr->left;
		}
		else if (!stack.empty()) {
			srcPtr = stack.top(); stack.pop();
			ptr = stack.top(); stack.pop();
		}
		else break;
	}
}

template<class T, class Compare>
bool Tree<T, Compare>::operator==(const Tree<T, Compare>& o) {
	if (this == &o) return true;
	if (sz != o.sz) return false;

	Node* tNode =   min();
	Node* oNode = o.min();

	while (tNode) {
		if (**tNode != **oNode) return false;
		tNode = tNode->inorderNext();
		oNode = oNode->inorderNext();
	}
	return true;
}

//--------------------Tree Functions--------------------

template<class T, class Compare>
typename Tree<T, Compare>::Node*
Tree<T, Compare>::find(const T& key, bool getClosest) const {
	Node* current = root;
	if (current) {
		while (key != *current->key) {
			if (cmp(key, *current->key)) {
				if (current->left)  current = current->left;
				else break;
			}
			else {
				if (current->right) current = current->right;
				else break;
			}
		}

		if (key == *current->key || getClosest) return current;
	}
	return nullptr;
}

template<class T, class Compare> template<typename Iter>
size_t Tree<T, Compare>::insert(Iter it, Iter end) {
	size_t prevSize = sz;
	// Iter refers to already created object, so must copy key
	for (; it != end; it++) insert(*it, false);
	return sz - prevSize;
}

template<class T, class Compare>
size_t Tree<T, Compare>::insert(std::initializer_list<T> keys) {
	size_t prevSize = sz;
	// init_list is temp object, so can move key
	for (const T& key : keys) insert(key, true);
	return sz - prevSize;
}

template<class T, class Compare>
std::pair<typename Tree<T, Compare>::Node*, bool>
Tree<T, Compare>::insert(const T& key, bool toMove) {
	Node* current = find(key);

	if (!current) { // Set root as black (root rule)
		sz = 1;

		if (toMove) { // Call move or copy constructor
			 root = new Node((T&&)key, false);
		}
		else root = new Node(     key, false);
		return {root, true};
	}

	// Dupl. not allowed. To implement dupl., track
	// key's freq and changes to freq from and to 0
	// That said, ADS using RedBlackTree as backend
	// such as Set are intended to store unique keys
	if (key == *current->key) return {current, false};

	// Add leaf having color red, current as parent
	Node* added;
	if (toMove) {
		 // Call Node move constructor that calls T's move constructor
		 added = new Node((T&&)key, true, current);
	}
	// Call Node copy constructor that calls T's copy constructor
	else added = new Node(     key, true, current);

	if (cmp(key, *current->key)) {
		current->left  = added;
	}
	else {
		current->right = added;
	}

	// Adding red child does not break black depth 
	// rule, but may break red parent rule
	balanceInsert(added);
	sz++;
	return {added, true};
}

// Helper: Restore rule that red Node has black || null childs
template<class T, class Compare>
void Tree<T, Compare>::balanceInsert(Node* current) {

	// Loop to resolve *CRNT and P being both red
	// Natural break on P == root, which is always black
	Node* P;
	while ((P = current->parent) && P->isRed) {

		Node* GP = P->parent; // P->isRed means P != root
		Node* U  = P->sibling(); // Uncle

		// RED: Recolor both P, U to black and non-root
		//		GP to red (1 black + 1 red between GP, 
		//		either its child remain). Recurse for
		//		current = GP to resolve possibly created
		//		double-red between GP and GP->parent's
		if (U && U->isRed) {
			U->isRed = P->isRed = false;

			// GP swaps its black with P, U for red
			if (GP != root) {
				GP->isRed = true;
				current = GP;
			}
			else break; // Leave root black as needed
		}

		// END (!red for uncle, which may be null):
		// Doesn't recurse: recolor; rotate, may 
		// need to reassign root after any rotate
		else {
			Node* GGP = GP->parent;

			// Node to take GP's place at top of rotated
			// subtree, to take GP's place as GGP's child
			// Line: P; Angle: *CRNT (to reassign) 
			Node* top = P;
			bool GPIsLeftChild = GGP && GP == GGP->left;

			// GP swaps its black for new top's red
			GP->isRed = true;

			if (P == GP->left) {
				// LINE (Left formed by CRNT, P, GP):
				// CRNT is same P's child as P is of GP
				// ROTATE (Swap P, GP's relation)
				// 1. Set P's other child (to CRNT) as
				//	  same GP's child as CRNT is to P
				// 2. Set GP as other of P's child as
				//	  CRNT is child of P
				if (current == P->left) {
					GP->left = P->right;	// ROT 1
					if (GP->left) {
						GP->left->parent = GP;
					}

					GP->parent = P;			// ROT 2
					P ->right = GP;
				}

				// ANGLE: CRNT is other of P's child as
				//		  P is child of GP (L-R)
				// 1. A: Set same CRNT's child as CRNT is
				//		 P's child as other GP's child
				//	  B: GP replaces it as CRNT's child 
				// 2. A: Set same CRNT's child as P is 
				//		 GP's child as other P's child
				//	  B: P replaces it as CRNT's child
				else {
					GP->left = current->right; // 1A
					if (GP->left) {
						GP->left->parent = GP;
					}
					current->right = GP;	   // 1B
					GP->parent = current;

					P->right = current->left;  // 2A
					if (P->right) {
						P->right->parent = P;
					}
					current->left = P;		   // 2B
					P->parent = current;

					top = current;
				}
			}
			else {
				// ANGLE (R-L): Mirrors ANGLE (L-R)
				// Swap any ->left, ->right to other
				if (current == P->left) {
					GP->right = current->left; // 1A
					if (GP->right) {
						GP->right->parent = GP;
					}
					current->left = GP;		   // 1B
					GP->parent = current;

					P->left = current->right;  // 2A
					if (P->left) {
						P->left->parent = P;
					}
					current->right = P;		   // 2B
					P->parent = current;

					top = current;
				}
				// LINE (right formed by current, P, GP):
				// Mirrors LINE (Left), but current is
				// right child. Swap ->left, ->right
				else {
					GP->right = P->left;	// ROT 1
					if (GP->right) {
						GP->right->parent = GP;
					}

					GP->parent = P;			// ROT 2
					P ->left = GP;
				}
			}

			top->isRed  = false; // Swapped with GP
			top->parent = GGP;
			// Assign top based on if GP was root
			if (GGP) {
				if (GPIsLeftChild) {
					GGP->left  = top;
				}
				else {
					GGP->right = top;
				}
			}
			else root = top;

			break;
		}
	}
}

// Specialize: To avoid risk [it] refers to *this tree (Node*
// is modified while iterating), iterate over T* key instead
template<class T, class Compare>
size_t Tree<T, Compare>::erase(
	Set<T, Compare>::iterator it, Set<T, Compare>::iterator end) {
	size_t prevSize = sz;

	T* curKey = nullptr;
	if (it.ptr) curKey = it.ptr->key;

	T* endKey = nullptr;
	if (end.ptr) endKey = end.ptr->key;

	while (curKey && curKey != endKey) {
		T* scsrKey = nullptr;
		Node* scsr = find(*curKey)->inorderNext();
		if (scsr) scsrKey = scsr->key;
		erase(*curKey);
		curKey = scsrKey;
	}

	return prevSize - sz;
}

template<class T, class Compare> template<typename Iter>
size_t Tree<T, Compare>::erase(Iter it, Iter end) {
	size_t prevSize = sz;
	for (; it != end; it++) erase(*it);
	return prevSize - sz;
}

template<class T, class Compare>
size_t Tree<T, Compare>::erase(std::initializer_list<T> keys) {
	size_t prevSize = sz;
	for (const T& key : keys) erase(key);
	return prevSize - sz;
}

template<class T, class Compare>
std::pair<typename Tree<T, Compare>::Node*, bool>
Tree<T, Compare>::erase(const T& key) {
	Node* current = find(key);

	if (!current || key != *current->key) { // If !found
		return {nullptr, false};
	}

	// Erase childless root without need to balance
	if (sz == 1) {
		delete root;
		root = nullptr;
		sz	 = 0;
		return {nullptr, true};
	}
	
	// If current->right: current to hold SCSR key
	// Else: SCSR key to be found up current's subtree
	Node* successor = current->inorderNext();

	if (current->right) {
		Node* tmp = current;

		// 2 childs: Swap current's key with successor's
		// Swap *SCRS, *CRNT to point to CRNT, SCRS Node
		// To continue to 0|1 child case with *CRNT to 
		// erase *CRNT/SCRS Node or its sole child: as
		// else SCRS's right child would instead be SCRS)
		// CRNT's left subtree < CRNT key < SCSR key
		// SCSR is leftmost thus min of CRNT's right subtree
		if (current->left) {
			// current->right != null: successor != null
			current->swapKey(successor->key);

			// To erase: SCRS Node or SCRS Node's child 
			// (Case: 2 child -> 1 child) passed CRNT's key
			current = successor;
		}
		// Else: *CNRT to swap only with CNRT Node's child

		// To return: CRNT Node holding SCSR's key
		successor = tmp;
	}

	// 1 child: Move *CRNT to child, swap key with child's
	if (	!current->left &&  current->right) {
		current->swapKey(current->right->key);
		current = current->right;
	}
	else if (current->left && !current->right) {
		current->swapKey(current->left ->key);
		current = current->left;
	}

	balanceErase(current);
	// CRNT is childless, so delete only 1 Node*
	delete current;
	sz--;
	return {successor, true}; // SCRS may be null
}

// Helper: If trim black depth of any branch, trim depth 
// of all other. Nullify toErase->parent's ptr to toErase
template<class T, class Compare>
void Tree<T, Compare>::balanceErase(Node* toErase) {

	// Only deleting black Node affects black depth
	if (!toErase->isRed) {
		// Differs only if while(current..) loops > 1 time
		Node* current = toErase;

		// Don't recurse for double black at P == root
		// as it applies to every branch equally
		while (current != root) {
			// S != null as it has black depth == to CRNT's
			Node* S = current->sibling();
			Node* P = current->parent;

			// May be null, points into subtree root
			Node* GP = P->parent;

			// Node to replace P as subtree root
			Node* top = nullptr;

			// CASE (A): S is red 
			// Swap S's black for P's red. Set top = S 
			// Set S's child forming angle with S and P,
			// as same child of P as S is of P. Set P as 
			// child of S. Then continue to 1 of other
			// 2 cases: (B) redNiece or (C) !redNiece
			if (S->isRed) {
				S->isRed = false;
				P->isRed = true;
				top = S;
				P->parent = S;

				if (S == P->left) {
					P->left = S->right;
					if (P->left) {
						P->left->parent = P;
					}

					S->right = P;
				}
				else {
					P->right = S->left;
					if (P->right) {
						P->right->parent = P;
					}

					S->left = P;
				}

				S = current->sibling();
			}

			////////////////////////////////////
			bool  PWasRed = P->isRed;
			Node* redNiece = nullptr;

			// CASE (B): S has >= 1 red childs
			// Direction of S to P, redNiece to S give
			// 4 subcases (2 mirror pairs: ANGLE, LINE)
			// Resolve ANGLE before LINE
			// Color RedBiece to P's color. To break
			if (S == S->parent->left) {

				// ANGLE: Left-Right
				// Color P black whatever its color
				// Give redNiece's childs to P and S
				// Set P and S as redNiece's childs
				// If unset, set top = redNiece
				if (S->right && S->right->isRed) {
					redNiece = S->right;

					// If red S case hadn't set top
					if (!top) top = redNiece;
					P->isRed = false;

					P->left = redNiece->right;
					if (P->left) {
						P->left->parent = P;
					}

					redNiece->right = P;
					P->parent = redNiece;

					S->right = redNiece->left;
					if (S->right) {
						S->right->parent = S;
					}

					redNiece->left = S;
					S->parent = redNiece;
				}

				// LINE: Left-Left
				// Set P as S's child. If S's former
				// child is non-null S had resolved it
				// If unset, set new top = S
				else if (S->left && S->left->isRed) {
					redNiece = S->left;

					if (!top) top = S;
					S->right = P;
					P->parent = S;
					P->left = nullptr;
				}
			}
			else {
				// ANGLE (R-L): Mirrors Left-Right
				if (S->left && S->left->isRed) {
					redNiece = S->left;

					if (!top) top = redNiece;
					P->isRed = false;

					P->right = redNiece->left;
					if (P->right) {
						P->right->parent = P;
					}

					redNiece->left = P;
					P->parent = redNiece;

					S->left = redNiece->right;
					if (S->left) {
						S->left->parent = S;
					}

					redNiece->right = S;
					S->parent = redNiece;
				}

				// LINE (R-R): Mirrors Left-Left
				else if (S->right && S->right->isRed) {
					redNiece = S->right;

					if (!top) top = S;
					S->left = P;
					P->parent = S;
					P->right = nullptr;
				}
			}

			////////////////////////////////////

			if (redNiece) { // If any of such

				// If PWasRed, keep black depth of 1 
				// (red P, black S). Else, set redNiece
				// to black to keep black depth of 2
				redNiece->isRed = PWasRed;

				// If red S, not redNiece, Case set top,
				// set redNiece Case's top (ie P->parent)
				// as child of red S Case's top
				if (top != S && top != redNiece) {
					P->parent->parent = top;
					if (P == top->left) { // P was top
						top->left = P->parent;
					}
					else {
						top->right = P->parent;
					}
				}
			}

			// "else": redNiece case induces no other
			// CASE (C): S is black, has black || null childs
			// Push of CRNT's black up to P also
			// adds 1 to S branch's black depth
			// To negate add, wash 1 black from S
			else {
				S->isRed = true;

				// P inherits CNRT's black. To break
				if (P->isRed) P->isRed = false;

				// P colored double black. Only recursive case
				else {
					current = P;
					continue;
				}
			}

			// Check for P->parent change (P was rotated)
			// For rotated subtree, rewire GP atop it to
			// point to its new top (ie S || redNiece) 
			// If !GP, subtree's root is whole tree's
			if (P->parent != GP) {
				top->parent = GP;
				if (GP) {
					if (P == GP->left) { // P was top
						GP->left = top;
					}
					else {
						GP->right = top;
					}
				}
				else root = top;
			}
			break;
		}
	}
	
	//erase() verified toErase != root
	if (toErase == toErase->parent->left) {
		toErase->parent->left  = nullptr;
	}
	else {
		toErase->parent->right = nullptr;
	}
}