#!/bin/sh

export PYTHONUNBUFFERED=1

chown -R gallery:gallery /home/gallery
chmod -R 700 /home/gallery

exec gunicorn -w 3 app:application -b 0.0.0.0:80 -u gallery -g gallery --capture-output --timeout=15 --reload
