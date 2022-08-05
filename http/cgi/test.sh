#!/bin/bash

OUTPUT="test.sh: "$(pwd)" - "$(date +%s)

echo HTTP/1.1 200 Ok
echo content-type: text/plain
echo content-length: ${#OUTPUT}
echo ""
echo -n $OUTPUT