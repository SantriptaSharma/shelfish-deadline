#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "avl.h"


#define signum(x) (x < 0) ? -1 : 1
#define max(a, b) (a > b) ? a : b

static void UpdateHeight(AVLNode **root, AVLNode *target);

static void Balance(AVLNode **root, AVLNode *target, int balance)
{
    int childBalance = 0;

    if(balance > 0)
    {
        if (target->left->left) childBalance = target->left->left->height;
        if (target->left->right) childBalance -= target->left->right->height;

        if (signum(balance) == signum(childBalance))
        {
            RotateRight(root, target);
            UpdateHeight(root, target->parent->parent);
        }
        else
        {
            RotateLeft(root, target->left);
            RotateRight(root, target);
            UpdateHeight(root, target->parent->parent);
        }
    }
    else
    {
        if (target->right->left) childBalance = target->right->left->height;
        if (target->right->right) childBalance -= target->right->right->height;

        if (signum(balance) == signum(childBalance))
        {
            RotateLeft(root, target);
            UpdateHeight(root, target->parent->parent);
        }
        else
        {
            RotateRight(root, target->right);
            RotateLeft(root, target);
            UpdateHeight(root, target->parent->parent);
        }
    }
}

static void UpdateHeight(AVLNode **root, AVLNode *target)
{
    if (target == NULL) return;

    int maxHeight = -1, balance = 0;

    if (target->left)
    {
        maxHeight = target->left->height;
        balance = maxHeight;
    }

    if (target->right)
    {
        if (maxHeight < target->right->height) maxHeight = target->right->height;
        balance -= target->right->height;
    }

    target->height = maxHeight + 1;

    if (abs(balance) >= 2)
    {
        Balance(root, target, balance);
        return;
    }

    if (maxHeight != -1) UpdateHeight(root, target->parent);
}

void Insert(AVLNode **root, Element *element)
{
    if (*root == NULL)
    {
        *root = malloc(sizeof(**root));
        (*root)->left = (*root)->right = (*root)->parent = NULL;
        (*root)->element = element;
        (*root)->isTombstone = false;
        (*root)->height = 0;
        return;
    }

    AVLNode *target = (*root);

    char isLeft = CompareElements(element, target->element) < 0;

    while ((isLeft && target->left) || (!isLeft  && target->right))
    {
        target = isLeft ? target->left : target->right;
        isLeft = CompareElements(element, target->element) < 0;
    }

    if (isLeft) 
    {
        target->left = malloc(sizeof(*target));
        target->left->left = target->left->right = NULL;
        target->left->parent = target;
        target->left->element = element;
        target->left->height = 0;
        target->left->isTombstone = false;
    }
    else
    {
        target->right = malloc(sizeof(*target));
        target->right->left = target->right->right = NULL;
        target->right->parent = target;
        target->right->element = element;
        target->right->height = 0;
        target->right->isTombstone = false;
    }

    UpdateHeight(root, target);
}

AVLNode* Find(AVLNode *root, const char *key)
{
    if (root == NULL || strcmp(key, root->element->key) == 0) return root;

    if (strcmp(key, root->element->key) < 0) return Find(root->left, key);
    return Find(root->right, key);
}

AVLNode* FindMinimum(AVLNode *root)
{
    if (root == NULL || root->left == NULL) return root;

    return FindMinimum(root->left);
}

AVLNode* FindSuccessor(AVLNode *target)
{
    if (target->right != NULL)
    {
        return FindMinimum(target->right);
    }

    AVLNode *parent = target->parent;
    
    while (parent != NULL && target == parent->right)
    {
        target = parent;
        parent = parent->parent;
    }

    return parent;
}

void RotateLeft(AVLNode **root, AVLNode *target)
{
    AVLNode *parent = target->parent, *child = target->right;
    AVLNode *inner = child->left;
    target->right = inner;
    if (inner) inner->parent = target;
    int targetHeight = target->left ? target->left->height + 1 : 0;
    if (inner) targetHeight = max(targetHeight, inner->height + 1);
    target->height = targetHeight;
    child->left = target;
    int childHeight = child->right ? child->right->height + 1 : 0;
    childHeight = max(childHeight, targetHeight + 1);
    child->height = childHeight;
    target->parent = child;

    if (*root == target)
    {
        *root = child;
        child->parent = NULL;
    }
    else
    {
        char isRight = target == parent->right;
        
        if (isRight) 
        {
            parent->right = child;
        }
        else
        {
            parent->left = child;
        }

        child->parent = parent;
    }
}

void RotateRight(AVLNode **root, AVLNode *target)
{
    AVLNode *parent = target->parent, *child = target->left;
    AVLNode *inner = child->right;
    target->left = inner;
    if (inner) inner->parent = target;
    int targetHeight = target->left ? target->left->height + 1 : 0;
    if (inner) targetHeight = max(targetHeight, inner->height + 1);
    target->height = targetHeight;
    child->right = target;
    int childHeight = child->left ? child->left->height + 1 : 0;
    childHeight = max(childHeight, targetHeight + 1);
    child->height = childHeight;
    target->parent = child;

    if (*root == target)
    {
        *root = child;
        child->parent = NULL;
    }
    else
    {
        char isRight = target == parent->right;
        
        if (isRight) 
        {
            parent->right = child;
        }
        else
        {
            parent->left = child;
        }

        child->parent = parent;
    }
}

static void ShiftBtoA(AVLNode **root, AVLNode *a, AVLNode *b)
{
    if (a->parent == NULL)
    {
        *root = b;
    }
    else if (a == a->parent->left)
    {
        a->parent->left = b;
    }
    else
    {
        a->parent->right = b;
    }

    if (b != NULL)
    {
        b->parent = a->parent;
    }
}

AVLNode* Delete(AVLNode *target)
{
    AVLNode *root = target;
    while (root != NULL && root->parent != NULL) root = root->parent;
    
    if (target->left == NULL)
    {
        ShiftBtoA(&root, target, target->right);
        UpdateHeight(&root, target->parent);
        free(target);
        return root;
    }
    else if (target->right == NULL)
    {
        ShiftBtoA(&root, target, target->left);
        UpdateHeight(&root, target->parent);
        free(target);
        return root;
    }

    AVLNode *succ = FindSuccessor(target);
    AVLNode *heightStart = succ->parent;
    if (succ->parent != target)
    {
        ShiftBtoA(&root, succ, succ->right);
        heightStart = succ->right;
        succ->right = target->right;
        succ->right->parent = succ;
    }
    
    ShiftBtoA(&root, target, succ);
    succ->left = target->left;
    succ->left->parent = succ;
    UpdateHeight(&root, heightStart);
    free(target);
    return root;
}

static void InOrderRecursive(AVLNode *cur)
{
    if (cur->left) InOrderRecursive(cur->left);
    PrintElement(cur->element); printf(" (%d), ", cur->height);
    if (cur->right) InOrderRecursive(cur->right);
}

void PrintInOrder(AVLNode *root)
{
    printf("[ ");
    InOrderRecursive(root);
    printf(" ]\n");
}

void FreeAVL(AVLNode *root)
{
    if (root != NULL)
    {
        FreeAVL(root->left);
        FreeAVL(root->right);
        FreeElement(root->element);
        free(root);
    }
}