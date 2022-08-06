#!/bin/bash

OUTPUT=$(env)

echo HTTP/1.1 200 Ok
echo content-type: text/plain
echo content-length: ${#OUTPUT}
echo ""
echo -n $OUTPUT