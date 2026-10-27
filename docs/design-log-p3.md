# Project 3 Design Log
**Author:** <Joe Lindenbaum> · **P2 code used:** <my own>
**Tools used:** <used Codex to help understand the spec and starter code, troubleshoot build errors, format the code consistently, and rewrite my comments to make them simple and understandable.>

## 1. Hash table

I used a vector of linked lists, like the chaining example from Lecture 17. Each position is a bucket. If two tool names go to the same bucket, both are stored in that list. This keeps collision handling simple. Each tool is stored in a unique_ptr, so the registry owns it and frees it automatically.

The table starts with 8 buckets. Before inserting, I check whether the next tool would make the load factor exceed 1.0. If so, I double the bucket count and calculate new bucket positions for every tool. I chose 1.0 because chaining allows multiple tools per bucket, and my measurements showed short chains.

I used the hash example from Appendix A. It processes every character, then uses % with the bucket count to choose a bucket. With 10,000 tools, my table had 16,384 buckets, a load factor of 0.610352, and a longest chain of 6. The weak hash, s[0] + s.size(), had a longest chain of 9,000 at the same bucket count. Since every name starts with the same character, it only separates names by length.

Resizing takes O(n), but doubling leaves room for many insertions before another resize. Spreading that work across the insertions gives amortized O(1) insertion with a well-distributed hash.

## 2. Pointer stability

When resizing, I move each unique_ptr into its new bucket. Ownership moves, but the actual tool stays at the same memory address. A pointer returned by find() therefore stays valid after resizing.

The raw pointer only provides access to the tool. The registry still owns it, so the caller should not delete it. My scale test saves the first tool pointer, inserts all 10,000 tools, and checks that find() returns the same address afterward.

## 3. Calculator trace

I used separate stacks for values and operators. For 2 * (3 + 4) - 5, I push 2 onto the values stack, then * and the opening parenthesis onto the operators stack. Next come value 3, operator +, and value 4. At the closing parenthesis, I add 3 and 4, push 7, and remove the opening parenthesis. When - arrives, multiplication has higher precedence, so I multiply 2 by 7 before pushing -. Finally, I push 5 and finish the subtraction, leaving 9. Each calculation pops the right operand before the left. The precedence check also makes equal-precedence operators run left to right.

## 4. Errors as values

A missing file, rejected path, or division by zero is a request failure that the model can recover from. Returning an error lets the harness send the reason back so the model can try something else. My workspace and calculator functions return false and fill in an error message. Exceptions are more appropriate for programmer mistakes, such as calling the wrong JSON accessor. The provided dispatcher also catches exceptions from tools to keep the harness running.

## 5. What I would design differently

I would add tests earlier while implementing each function. Checking empty registries, null pointers, and for_each before resizing would make problems easier to isolate. I would keep chaining because it was straightforward to follow and produced a longest chain of only 6 with 10,000 tools.