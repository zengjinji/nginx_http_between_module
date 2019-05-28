nginx_http_between_module
==============

This module set condition for nginx log

Example
=======

set condition for nginx log
---------------------------------

```
 http {
 
    access_log /usr/local/nginx/logs/access_time.log main if=$is_between_time buffer=4K flush=1s; 
    access_log /usr/local/nginx/logs/access_percent.log main if=$is_between_percent buffer=4K flush=1s; 


    server {
        listen 80;

        location /1 {
            between_time   12:00-12:30; 14:00-14:30 20:00-22:00;
            root html;
        }
        
        location /2 {
            between_percent   50;
            root html;
        }
    }
 }
```

Install
=======

Install this module from source:

```
$ wget http://nginx.org/download/nginx-1.13.4.tar.gz
$ tar -xzvf nginx-1.13.4.tar.gz
$ cd nginx-1.13.4/
$ ./configure --add-module=/path/to/nginx_http_between_module
$ make -j4 && make install
```

Directive
=========

Syntax: **between_time**

Default: —

Context: `http, server, location`

Set the time used by is_between_time variable.



Syntax: **between_percent**

Default: between_percent 100

Context:  `http, server, location`

Set the time used by is_between_persion variable.



# Embedded Variables

$is_between_time

​    if not between_time command, default value is '1', if with between_time command time and in time, value is '1', else value is '0'

$is_between_percent

​        if not is_between_percent command, default value is '1', if with is_between_percent command and in percent, value is '1', else value is '0'
