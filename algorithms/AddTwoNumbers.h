#pragma once
#include <vector>

// Singly-linked list node (matches the LeetCode definition).
struct ListNode {
    int val{};
    ListNode* next{nullptr};
    explicit ListNode(int v) : val{v} {}
};

// LeetCode #2 — Add Two Numbers
// Two non-negative integers are stored in reverse order in linked lists.
// Return their sum as a linked list in the same format.
//
// Example: [2]→[4]→[3]  +  [5]→[6]→[4]  =  [7]→[0]→[8]
//          (342)             (465)             (807)
//
// Time:  O(max(m, n))  — single pass over both lists
// Space: O(max(m, n))  — result list length

class AddTwoNumbers {
public:
    ListNode* solve(ListNode* l1, ListNode* l2) const;

    // Utility helpers used by main.cpp and the presenter.
    static ListNode*          fromVector(const std::vector<int>& digits); // digits in reverse order
    static std::vector<int>   toVector(const ListNode* node);
    static void               freeList(ListNode* node);

    static const char* getName()            { return "Add Two Numbers"; }
    static const char* getTimeComplexity()  { return "O(max(m,n))"; }
    static const char* getSpaceComplexity() { return "O(max(m,n))"; }
};
