#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED				1
#define BLACK 			2

typedef int KEY_TYPE;

typedef struct _rbtree_node {
	unsigned char color;
	struct _rbtree_node *right;
	struct _rbtree_node *left;
	struct _rbtree_node *parent;
	KEY_TYPE key;
	void *value;
} rbtree_node;

typedef struct _rbtree {
	rbtree_node *root;
	rbtree_node *nil;
} rbtree;

void rbtree_left_rotate(rbtree *T, rbtree_node *x) 
{
	rbtree_node* y = x->right;

	x->right = y->left;
	if(y->left != T->nil)
	{
		y->left->parent = x;
	}

	y->parent = x->parent;
	if(x->parent == T->nil)
	{
		T->root = y;
	}
	else if(x == x->parent->left)
	{
		x->parent->left = y;
	}
	else 
	{
		x->parent->right = y;
	}

	y->left = x;
	x->parent = y;
	
	return ;
}

void rbtree_right_rotate(rbtree *T, rbtree_node *x) 
{
	rbtree_node* y = x->left;

	x->left = y->right;
	if(y->right != T->nil)
	{
		y->right->parent = x;
	}

	y->parent = x->parent;
	if(x->parent == T->nil)
	{
		T->root = y;
	}
	else if(x == x->parent->right)
	{
		x->parent->right = y;
	}
	else 
	{
		x->parent->left = y;
	}

	y->right = x;
	x->parent = y;
	
	return ;
}



//插入调整
void rbtree_insert_fixup(rbtree *T, rbtree_node *z) 
{
	while(z->parent->color == RED)
	{
		if(z->parent == z->parent->parent->left)
		{
			rbtree_node* y = z->parent->parent->right;
			if(y->color == RED)
			{
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent = RED;
			}
			else 
			{
				if(z == z->parent->right)
				{
					z = z->parent;
					rbtree_left_rotate(T, z);
				}

				z->parent->color = BLACK;
				z->parent->parent = RED;
				rbtree_left_rotate(T, z->parent->parent);
			}
		}
		else 
		{
			rbtree_node* y = z->parent->parent->left;
			if(y->color == RED)
			{
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent = RED;
			}
			else 
			{
				if(z == z->parent->left)
				{
					z = z->parent;
					rbtree_right_rotate(T, z);
				}

				z->parent->color = BLACK;
				z->parent->parent = RED;
				rbtree_right_rotate(T, z->parent->parent);
			}
		}
		
	}

	T->root->color = BLACK;
}


//插入
void rbtree_insert(rbtree *T, rbtree_node *z)
{
	rbtree_node* x = T->root;
	rbtree_node* y = T->nil;
	while(x != T->nil)
	{
		y = x;
		if(z->key < x->key)
		{
			x = x->left;
		}
		else if(z->key > x->key)
		{
			x = x->right;
		}
		else
		{
			return;
		}
	}

	z->parent = y;
	if(y == T->nil)
	{
		T->root = z;
	}
	else if(z->key > y->key)
	{
		y->right = z;
	}
	else 
	{
		y->left = z;
	}

	z->left = T->nil;
	z->right = T->nil;
	z->color = RED;
	
	rbtree_insert_fixup(T, z);
	
	return;
}

void rbtree_delete_fixup(rbtree *T, rbtree_node *x)
{
	while((x != T->root) && (x->color == BLACK))
	{
		if(x == x->parent->left)
		{
			rbtree_node* w = x->parent->right;
			if(w->color == RED)
			{
				w->color = BLACK;
				x->parent->color = RED;

				rbtree_left_rotate(T, x->parent);
				w = x->parent->right;
			}

			if((w->left->color == BLACK) && (w->right->color == BLACK))
			{
				w->color = RED;
				x = x->parent;
			}
			else 
			{
				if(w->right->color == BLACK)
				{
					w->left->color = BLACK;
					w->color = RED;
					rbtree_right_rotate(T, w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rbtree_left_rotate(T, x->parent);

				x = T->root;
			}
		}
		else
		{
			rbtree_node* w = x->parent->left;
			if(w->color == RED)
			{
					w->color = BLACK;
					x->parent->color = RED;

					rbtree_right_rotate(T, x->parent);
					w = x->parent->left;
				}

			if((w->left->color == BLACK) && (w->right->color == BLACK))
			{
				w->color = RED;
				x = x->parent;
			}
			else 
			{
				if(w->left->color == BLACK)
				{
					w->right->color = BLACK;
					w->color = RED;
					rbtree_left_rotate(T, w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rbtree_right_rotate(T, x->parent);

				x = T->root;
			}
		}
	}


	x->color = BLACK;
	return;
}

//查找
rbtree_node *rbtree_successor(rbtree *T, rbtree_node *x) {
	rbtree_node *y = x->parent;

	if (x->right != T->nil) {
		return rbtree_mini(T, x->right);
	}

	while ((y != T->nil) && (x == y->right)) {
		x = y;
		y = y->parent;
	}
	return y;
}

//删除
rbtree_node* rbtree_delete(rbtree* T, rbtree_node* z)
{
	rbtree_node* x = T->nil;
	rbtree_node* y = T->nil;

	if((z->left == T->nil) || (z->right == T->nil))
	{
		y = z;
	}
	else
	{
		y = rbtree_successor(T, z);
	}

	if(y->left != T->nil)
	{
		x = y->left;
	}
	else if(y->right != T->nil)
	{
		x = y->right;
	}

	x->parent = y->parent;
	if(y->parent == T->root)
	{
		T->root = x;
	}
	else if(y == y->parent->left)
	{
		y->parent->left = x;
	}
	else
	{
		y->parent->right = x;
	}

	if(y != z)
	{
		z->key = y->key;
		z->value = y->value;
	}

	if(y->color == BLACK)
	{
		rbtree_delete_fixup(T, x);
	}

	return y;
}

