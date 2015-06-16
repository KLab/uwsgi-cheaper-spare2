# uWSGI spare2 algorithm

Backport spare2 cheaper algorithm from uWSGI 2.1 to uWSGI 2.0.

ref: https://github.com/unbit/uwsgi/pull/944


## Build

```console
$ uwsgi --build-plugin https://github.com/KLab/uwsgi-cheaper-spare2
```

## Sample configuration

```
[uwsgi]
plugin-dir=%d
need-plugin=cheaper_spare2
cheaper-algo=spare2

processes=10            # max workers

cheaper=2               # When idle workers is less than cheaper, spare2 increase workers
cheaper-initial=2       # Initial workers
cheaper-step=2          # How many workers can be spawned at one time
cheaper-idle=30         # When (idle workers < cheaper) state continued for this count, decrease one worker
```
