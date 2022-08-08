#!/usr/bin/env python3

import os
import sqlite3
import uuid

con = sqlite3.connect('session.db')
cur = con.cursor()

cur.execute("CREATE TABLE IF NOT EXISTS session(id INTEGER PRIMARY KEY, data TEXT, visits INTEGER)")

visits = 0
session_id = ""

cookie_string = os.environ.get('HTTP_COOKIE')
if (cookie_string):
  cookies =  cookie_string.split(';')
  for cookie in cookies:
    (key, value) = cookie.split('=')
    if key == 'id':
      session_id = value
      break

if session_id == "":
  session_id = str(uuid.uuid1())
  visits = 1;
  cur.execute("INSERT INTO session(data, visits) VALUES(:data, 1)", {'data': session_id})
else:
  rows = cur.execute("SELECT visits FROM session WHERE data=:data", {'data': session_id})
  row = rows.fetchone()
  if row:
    visits = tuple(row)[0] + 1
    cur.execute("UPDATE session SET visits=:visits WHERE data=:data", {'visits': visits, 'data': session_id})
  else:
    session_id = str(uuid.uuid1())
    visits = 1;
    cur.execute("INSERT INTO session(data, visits) VALUES(:data, 1)", {'data': session_id})

con.commit()
con.close()

print ('HTTP/1.1 200 OK')
print (f"Set-Cookie: id={session_id}")
print ('Content-Type: text/html\n')
print ('<html><body>')
if visits == 1:
  print (f'<p>This is your first visit! Welcome!</p>')
else:
  print (f'<p>You have visited this page {visits} times!</p>')
print ('</body></html>')