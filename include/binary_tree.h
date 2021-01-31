#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>


typedef struct Node {
	int key;
	int val;
	struct Node* parent;
	struct Node* left;
	struct Node* right;
} Node;

typedef struct BinaryTree {
	bool initialized;
	Node* head;
	unsigned int size;
} BinaryTree;


int CompareFunction(const void* p1, const void* p2) {
	return (**(Node**)p1).key - (**(Node**)p2).key;
}

Node* AllocateNode(int key, int val) {
	Node* n = (Node*)malloc(sizeof(Node));
	if(n == NULL) {
		printf("Can't malloc node. Returning NULL.\n");
		return NULL;
	}

	n->key = key;
	n->val = val;
	n->parent = NULL;
	n->left = NULL;
	n->right = NULL;

	return n;
}

BinaryTree InitEmptyTree() {
	BinaryTree b = {NULL, 0};
	return b;
}

Node* CreateTree(Node** nodes, unsigned int size, Node* parent) {
	// calculate size of left and right sides of array
	int center = size / 2;
	int left_size = center;
	int right_size = size - left_size - 1;

	// creat links
	// recurse if left or right array is non-zero length
	Node* n = nodes[center];
	n->parent = parent;
	n->left = left_size > 0 ? CreateTree(nodes, left_size, n) : NULL;
	n->right = right_size > 0 ? CreateTree(nodes + center + 1, right_size, n) : NULL;

	return n;
}


/*
* Takes list of pre-malloced nodes and creates the tree from that
*
*/
BinaryTree InitBinaryTree(Node** nodes, unsigned int num_nodes) {
	BinaryTree b = {NULL, 0};

	if(nodes == NULL) {
		printf("Can't init. Nodes are null. Returning empty tree.\n");
		return b;
	}
	if(num_nodes <= 0) {
		printf("Can't init. Size is 0. Returning empty tree.\n");
		return b;
	}

	b.size = num_nodes;
	qsort((void*)nodes, (size_t)num_nodes, sizeof(Node*), CompareFunction);
	b.head = CreateTree(nodes, num_nodes, NULL);

	return b;
}


Node* Min(BinaryTree* bst) {
	if(bst == NULL) {
		printf("Tree is null.\n");
		return NULL;
	}
	if(bst->head == NULL) {
		printf("Tree has no head.\n");
		return NULL;
	}
	
	Node* current_node = bst->head;
	while(current_node->left != NULL) {
		current_node = current_node->left;
	}

	return current_node;
}

Node* Successor(BinaryTree* bst, Node* node) {
	if(node == NULL) {
		printf("Node argument NULL.\n");
		return NULL;
	}
	if(bst == NULL) {
		printf("Tree is NULL.\n");
		return NULL;
	}
	if(bst->head == NULL) {
		printf("Tree has no head.\n");
		return NULL;
	}


	if(node->right) {
		Node* current = node->right;
		while(current->left != NULL) {
			current = current->left;
		}
		return current;
	}
	else {
		Node* parent = node->parent;
		while(parent != NULL) {
			if(parent->key > node->key) {
				break;
			}
			parent = parent->parent;
		}
		return parent;
	}	
}

void BalanceTree(BinaryTree* bst) {
	if(bst == NULL) {
		printf("Tree is NULL.\n");
		return;
	}
	if(bst->head == NULL) {
		printf("Tree has no head.\n");
		return;
	}

	Node** nodes = (Node**)malloc(sizeof(Node*) * bst->size);

	Node* current = Min(bst);
	int i = 0;
	while(current) {
		nodes[i] = current;
		current = Successor(bst, current);
		++i;
	}

	bst->head = CreateTree(nodes, bst->size, NULL);

	free(nodes);
}

Node* Search(BinaryTree* bst, int key) {
	if(bst == NULL) {
		printf("Tree is null.\n");
		return NULL;
	}
	if(bst->head == NULL) {
		printf("Tree has no head.\n");
		return NULL;
	}

	Node* current_node = bst->head;
	while(current_node) {
		if(key == current_node->key) {
			break;
		}
		else if(key < current_node->key) {
			current_node = current_node->left;
		}
		else if(key > current_node->key) {
			current_node = current_node->right;
		}
	}

	return current_node;
}


Node* Max(BinaryTree* bst) {
	if(bst == NULL) {
		printf("Tree is null.\n");
		return NULL;
	}
	if(bst->head == NULL) {
		printf("Tree has no head.\n");
		return NULL;
	}
	
	Node* current_node = bst->head;
	while(current_node->right != NULL) {
		current_node = current_node->right;
	}

	return current_node;
}

Node* Predecessor(BinaryTree* bst, Node* node) {
	if(node == NULL) {
		printf("Node argument NULL.\n");
		return NULL;
	}
	if(bst == NULL) {
		printf("Tree is NULL.\n");
		return NULL;
	}
	if(bst->head == NULL) {
		printf("Tree has no head.\n");
		return NULL;
	}
	

	if(node->left) {
		Node* current = node->left;
		while(current->right != NULL) {
			current = current->right;
		}
		return current;
	}
	else {
		Node* parent = node->parent;
		while(parent != NULL) {
			if(parent->key < node->key) {
				break;
			}
			parent = parent->parent;
		}
		return parent;
	}
}



void Insert(BinaryTree* bst, Node* node, bool rebalance) {
	if(node == NULL) {
		printf("Node argument NULL.\n");
		return;
	}
	if(bst == NULL) {
		printf("Tree is NULL.\n");
		return;
	}

	// clear node links
	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;

	// if head is NULL just insert node
	if(bst->head == NULL) {
		bst->head = node;
		bst->size++;
	}
	else {
		Node* current_node = bst->head;
		while(1) {
			if(node->key < current_node->key) {
				if(current_node->left == NULL) {
					current_node->left = node;
					node->parent = current_node;
					bst->size++;
					break;
				}
				else {
					current_node = current_node->left;
				}
			}
			else if(node->key > current_node->key) {
				if(current_node->right == NULL) {
					current_node->right = node;
					node->parent = current_node;
					bst->size++;
					break;
				}
				else {
					current_node = current_node->right;
				}
			}
			else if(node->val == current_node->val) {
				printf("value is not unique.\n");
				break;
			}
		}
	}

	if(rebalance) {
		BalanceTree(bst);
	}
}

void Delete(BinaryTree* bst, Node* node, bool rebalance) {
	if(node == NULL) {
		printf("Node argument NULL.\n");
		return;
	}
	if(bst == NULL) {
		printf("Tree is NULL.\n");
		return;
	}
	if(bst->head == NULL) {
		printf("Tree has no head.\n");
		return;
	}

	// leaf node
	if (node->left == NULL && node->right == NULL) {
		// unlink parent
		if (node->parent) {
			if (node == node->parent->left) {
				node->parent->left = NULL;
			}
			else {
				node->parent->right = NULL;
			}
		}
		else {
			// no left, right, or parent should be head node
			bst->head = NULL;
		}

		bst->size--;
		free(node);
	}
	// two children
	else if (node->left && node->right) {
		// copy successor into this node
		Node* temp = Successor(bst, node);
		node->key = temp->key;
		node->val = temp->val;
		
		// successor node has no left child
		// successor's parent should link to successors right node
		Node* successor_right_node = temp->right;
		if(temp == temp->parent->left) {
			temp->parent->left = successor_right_node;
			if(successor_right_node) {
				successor_right_node->parent = temp->parent;
			}
		}
		else {
			temp->parent->right = successor_right_node;
			if(successor_right_node) {
				successor_right_node->parent = temp->parent;
			}
		}

		// delete successor node
		bst->size--;
		free(temp);
	}
	// left child
	else if (node->left) {
		Node* temp = node->left;
		node->key = temp->key;
		node->val = temp->val;
		node->left = temp->left;
		node->right = temp->right;

		// set parent of left and right of temp node
		if(temp->left) {
			temp->left->parent = node;
		}
		if(temp->right) {
			temp->right->parent = node;
		}

		bst->size--;
		free(temp);
	}
	// right child
	else if (node->right) {
		Node* temp = node->right;
		node->key = temp->key;
		node->val = temp->val;
		node->left = temp->left;
		node->right = temp->right;

		// set parent of left and right of temp node
		if(temp->left) {
			temp->left->parent = node;
		}
		if(temp->right) {
			temp->right->parent = node;
		}

		bst->size--;
		free(temp);	
	}

	if(rebalance) {
		BalanceTree(bst);
	}
}

#endif
