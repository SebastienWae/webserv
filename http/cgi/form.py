#!/usr/bin/env python3

import cgi
import os

import cgitb
cgitb.enable()

print("HTTP/1.1 200 Ok")    # HTML is following
print("Content-Type: text/html")    # HTML is following
print()                             # blank line, end of headers

cgi.test()

# form = cgi.FieldStorage()
# for item in form.getlist("form"):
#     print(item)

# print ("----------")
    
# print( '\n'.join([f'{k}: {v}' for k, v in sorted(os.environ.items())]) )

# k
# cgi.print_environ()
# # cgi.print_form()
# cgi.print_directory()
# cgi.print_environ_usage()

exit()