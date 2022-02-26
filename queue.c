#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

// list_head_meta_t
// element_t

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

#define min(a, b) (((a) > (b)) ? (b) : (a))
#define n_cmp(a, b)                                    \
    (strcmp(container_of((a), element_t, list)->value, \
            container_of((b), element_t, list)->value))
/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */

struct list_head *q_new()
{
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l == NULL)
        return;
    struct list_head *node = l->next;
    while (node != l) {
        list_del(node);
        q_release_element(container_of(node, element_t, list));
        node = l->next;
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */

bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL || s == NULL)
        return false;
    element_t *new_node = (element_t *) malloc(sizeof(element_t));
    if (new_node == NULL)
        return false;
    int len = strlen(s);
    new_node->value = (char *) malloc(len + 1);
    if (new_node->value == NULL) {
        free(new_node);
        return false;
    }

    list_add(&new_node->list, head);
    memcpy(new_node->value, s, len + 1);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL || s == NULL)
        return false;
    element_t *new_node = (element_t *) malloc(sizeof(element_t));
    if (new_node == NULL)
        return false;
    int len = strlen(s);
    new_node->value = (char *) malloc(len + 1);
    if (new_node->value == NULL) {
        free(new_node);
        return false;
    }

    list_add_tail(&new_node->list, head);
    memcpy(new_node->value, s, len + 1);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *ele = container_of(head->next, element_t, list);
    char *src = ele->value;
    if (sp && src) {
        int len = min(strlen(src), bufsize - 1);
        memcpy(sp, src, len);
        sp[len] = '\0';
    }
    list_del(&ele->list);
    return ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *ele = container_of(head->prev, element_t, list);
    char *src = ele->value;
    if (sp && src) {
        int len = min(strlen(src), bufsize - 1);
        memcpy(sp, src, len);
        sp[len] = '\0';
    }
    list_del(&ele->list);
    return ele;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (head == NULL || head->next == NULL || head->prev == NULL)
        return 0;
    int ret = 0;
    struct list_head *node;
    list_for_each (node, head) {
        ret++;
    }
    return ret;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    int size = q_size(head);
    if (head == NULL || size == 0)
        return false;
    struct list_head *node;
    size = size / 2;
    list_for_each (node, head) {
        if (size == 0) {
            list_del(node);
            q_release_element(container_of(node, element_t, list));
            break;
        }
        size--;
    }
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (head == NULL)
        return false;
    struct list_head *node = head->next, *next;
    while (node != head) {
        int flag = 0;
        while ((next = node->next) != head && n_cmp(node, next) == 0) {
            list_del(next);
            q_release_element(container_of(next, element_t, list));
            flag = 1;
        }
        if (flag) {
            list_del(node);
            q_release_element(container_of(node, element_t, list));
        }
        node = next;
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    int size = q_size(head);
    if (head == NULL || size <= 1)
        return;
    struct list_head *a;

    for (int i = 0; i < size / 2; i++) {
        a = head->next;
        list_del(a);
        struct list_head *b = head->next;
        list_del(b);
        list_add_tail(b, head);
        list_add_tail(a, head);
    }
    if ((size & 0b1) == 1) {
        a = head->next;
        list_del(a);
        list_add_tail(a, head);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
inline void swap_child(struct list_head *node)
{
    struct list_head *t;
    t = node->next;
    node->next = node->prev;
    node->prev = t;
}
void q_reverse(struct list_head *head)
{
    if (head == NULL || head->next == NULL || head->prev == NULL)
        return;

    struct list_head *node;
    swap_child(head);
    for (node = head->prev; node != head; node = node->prev) {
        swap_child(node);
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
// a is in left hand side of b
static inline struct list_head *merge(struct list_head *a,
                                      struct list_head *b,
                                      int asize,
                                      int bsize)
{
    struct list_head *tmp, htmp, *h;
    h = a->prev;
    INIT_LIST_HEAD(&htmp);
    int i = 0, j = 0;
    while ((i < asize) || (j < bsize)) {
        if ((j == bsize) || ((i < asize) && (n_cmp(a, b) < 0))) {
            tmp = a->next;
            list_del(a);
            list_add_tail(a, &htmp);
            a = tmp;
            i++;
        } else {
            tmp = b->next;
            list_del(b);
            list_add_tail(b, &htmp);
            b = tmp;
            j++;
        }
    }
    tmp = h->next;  // last

    tmp->prev = htmp.prev;
    htmp.prev->next = tmp;

    h->next = htmp.next;
    htmp.next->prev = h;

    return tmp;
}

void q_sort(struct list_head *head)
{
    int size = q_size(head);
    if (head == NULL || size <= 1)
        return;
    for (int iter = 1; iter < size; iter <<= 1) {
        int rem = size;
        struct list_head *a = head->next;
        while (rem > iter) {
            rem -= iter;
            struct list_head *b = a;
            for (int i = 0; i < iter; i++)
                b = b->next;
            a = merge(a, b, iter, min(iter, rem));
            rem -= iter;
        }
    }
}
