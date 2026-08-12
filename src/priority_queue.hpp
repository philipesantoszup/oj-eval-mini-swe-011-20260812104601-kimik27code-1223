#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct Node {
		T val;
		Node *l, *r;
		int dist;
		Node(const T &v, int d = 1) : val(v), l(nullptr), r(nullptr), dist(d) {}
	};

	Node *root;
	size_t sz;
	Compare cmp;

	/**
	 * @brief Iterative clone of a subtree. nNodes is the number of nodes in the subtree.
	 */
	Node *clone(Node *n, size_t nNodes) const {
		if (!n) return nullptr;
		Node *copyRoot = new Node(n->val, n->dist);
		struct Frame {
			Node *src;
			Node *dst;
		};
		Frame *st = new Frame[nNodes];
		size_t top = 0;
		st[top++] = {n, copyRoot};
		while (top) {
			Frame f = st[--top];
			if (f.src->l) {
				f.dst->l = new Node(f.src->l->val, f.src->l->dist);
				st[top++] = {f.src->l, f.dst->l};
			}
			if (f.src->r) {
				f.dst->r = new Node(f.src->r->val, f.src->r->dist);
				st[top++] = {f.src->r, f.dst->r};
			}
		}
		delete[] st;
		return copyRoot;
	}

	/**
	 * @brief Iterative post-order destruction of the whole heap.
	 */
	void clear() {
		if (!root) {
			sz = 0;
			return;
		}
		Node **order = new Node*[sz];
		size_t top = 0;
		order[top++] = root;
		for (size_t i = 0; i < top; ++i) {
			Node *cur = order[i];
			if (cur->l) order[top++] = cur->l;
			if (cur->r) order[top++] = cur->r;
		}
		for (size_t i = top; i-- > 0; ) {
			delete order[i];
		}
		delete[] order;
		root = nullptr;
		sz = 0;
	}

	/**
	 * @brief Leftist heap merge. Strongly exception-safe w.r.t. Compare:
	 *        no node pointer is modified if cmp throws.
	 */
	Node *merge(Node *a, Node *b) {
		if (!a) return b;
		if (!b) return a;
		if (cmp(a->val, b->val)) {
			Node *t = a; a = b; b = t;
		}
		a->r = merge(a->r, b);
		if (!a->l || a->l->dist < a->r->dist) {
			Node *t = a->l; a->l = a->r; a->r = t;
		}
		a->dist = a->r ? a->r->dist + 1 : 1;
		return a;
	}

public:
	priority_queue() : root(nullptr), sz(0), cmp() {}

	priority_queue(const priority_queue &other)
		: root(clone(other.root, other.sz)), sz(other.sz), cmp() {}

	~priority_queue() {
		clear();
	}

	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		Node *newRoot = clone(other.root, other.sz);
		clear();
		root = newRoot;
		sz = other.sz;
		return *this;
	}

	const T & top() const {
		if (!root) throw container_is_empty();
		return root->val;
	}

	void push(const T &e) {
		Node *node = new Node(e);
		try {
			root = merge(root, node);
		} catch (...) {
			delete node;
			throw runtime_error();
		}
		++sz;
	}

	void pop() {
		if (!root) throw container_is_empty();
		Node *old = root;
		try {
			root = merge(root->l, root->r);
		} catch (...) {
			throw runtime_error();
		}
		--sz;
		delete old;
	}

	size_t size() const {
		return sz;
	}

	bool empty() const {
		return sz == 0;
	}

	void merge(priority_queue &other) {
		if (this == &other) return;
		try {
			root = merge(root, other.root);
		} catch (...) {
			throw runtime_error();
		}
		sz += other.sz;
		other.root = nullptr;
		other.sz = 0;
	}
};

}

#endif
