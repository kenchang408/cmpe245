import re
import sys
from math import log

symbols = {};
text = sys.stdin.read();
sys.stdout.write("Text: " + text);

for symbol in text:
    if symbol in symbols:
        symbols[symbol] += 1
    else:
        symbols[symbol] = 1

sys.stdout.write("\n");
for symbol in symbols:
    sys.stdout.write(symbol + " " + `symbols[symbol]` + " ");
sys.stdout.write("\n");
