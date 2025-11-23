# Asymptotics

Because of filesystem based on offsets there is no O(log n) complexity

## Inserts 
* O(1) - for the single insert
* O(k) - for the batch of k insert

## Select && Delete && Update
* O(n) - full scan query (n = total elements)
* O(m) - range scan query (m = range size)
* O(1) - direct indexed query