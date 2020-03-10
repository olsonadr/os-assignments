#!/usr/bin/env python3

# imports
import random

# variables
num_files = 3
num_chars = 10

# write each file
for ugh in range(num_files):
    # open file
    f = open(("file" + str(ugh)), "w")

    # reset string for file
    str_to_write = ""

    # generate each character
    for char_ugh in range(num_chars):
        str_to_write += chr(random.randint(97, 122))

    # write string to file
    f.write(str_to_write + "\n")

# read each file
for ugh in range(num_files):
    # open file
    f = open(("file" + str(ugh)), "r")

    # print contents
    print(f.read(), end="")

# generate random numbers
num1 = random.randint(1, 42)
num2 = random.randint(1, 42)

# print random numbers and product
print(num1)
print(num2)
print(num1 * num2)
