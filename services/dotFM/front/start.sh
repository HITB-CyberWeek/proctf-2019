#!/bin/sh

chown dotfm: /app/storage
chmod 700 /app/storage

su dotfm -s /bin/sh -c 'python main.py'