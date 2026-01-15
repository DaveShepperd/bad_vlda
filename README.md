# bad_vlda

Simple program to scan a file to see if it has a record structure
according to the macxx/llf binary record structures of ob, lb and vlda.

This structure is a 2 byte record count, little endian. The count can
never be less than 1 nor more than 16383. The count does not include
itself. If the count is odd, then add one to the count to get to the
start of the next record.
