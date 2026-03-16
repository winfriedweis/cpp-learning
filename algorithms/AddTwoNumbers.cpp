#include "AddTwoNumbers.h"

// ─── Core algorithm ───────────────────────────────────────────────────────────
//
// Iterative approach with a carry variable.
// A sentinel "dummy" head node avoids a special case for the first node.
//
// Each iteration:
//   sum   = l1->val + l2->val + carry
//   write = sum % 10   (the digit for this position)
//   carry = sum / 10   (propagate to next position)
//
// The loop continues while there is any list left OR a remaining carry.

ListNode* AddTwoNumbers::solve(ListNode* l1, ListNode* l2) const {
    ListNode dummy{0};
    ListNode* curr = &dummy;
    int carry = 0;

    while (l1 || l2 || carry) {
        int sum = carry;
        if (l1) { sum += l1->val; l1 = l1->next; }
        if (l2) { sum += l2->val; l2 = l2->next; }
        carry       = sum / 10;
        curr->next  = new ListNode(sum % 10);
        curr        = curr->next;
    }

    return dummy.next;
}

// ─── Utility helpers ──────────────────────────────────────────────────────────

ListNode* AddTwoNumbers::fromVector(const std::vector<int>& digits) {
    ListNode dummy{0};
    ListNode* curr = &dummy;
    for (int d : digits) {
        curr->next = new ListNode(d);
        curr = curr->next;
    }
    return dummy.next;
}

std::vector<int> AddTwoNumbers::toVector(const ListNode* node) {
    std::vector<int> result;
    while (node) {
        result.push_back(node->val);
        node = node->next;
    }
    return result;
}

void AddTwoNumbers::freeList(ListNode* node) {
    while (node) {
        ListNode* next = node->next;
        delete node;
        node = next;
    }
}
