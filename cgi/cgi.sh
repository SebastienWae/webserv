#!/usr/bin/env bash

echo "HTTP/1.1 200 Ok"
echo "Content-type: text/html"
echo ""

echo '<html>'
echo '<head>'
echo '<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">'
echo '<title>Environment Variables</title>'
echo '</head>'
echo '<body>'
echo 'Environment Variables:'
echo '<pre>'
/usr/bin/env
echo '</pre>'

echo '</body>'
echo '</html>'

exit 0