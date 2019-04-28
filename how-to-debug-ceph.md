# How to debug ceph

## Make debug build for ceph
下载ceph源码，在源码根目录里打开do_cmake.sh，把里面的cmake一行改为如下形式：  

```
cmake -DBOOST_J=$(nproc) -DCMAKE_C_FLAGS="-O0 -g3 -gdwarf-4" $ARGS "$@" ..

把CMAKE_C_FLAGS加入到cmake的build里。
```

进行编译。编译完成后，所有的lib和可执行文件都会包含调试信息。 


## 启用librbd的日志
对于写的任何程序，只要引用了librbd，可以用如下方式获取日志：

```
打开/etc/ceph/ceph.conf文件，在其中增加[client]一项，并在client这项下面添加如下内容： 
log_file = /var/log/ceph/applicationName-$pid.log  
debug librbd = 20

如果想把rbd命令的日志也打印出来，则可以添加如下一行： 
debug rbd = 20

```

其他的一些debug日志的配置，请查看[此链接](http://docs.ceph.com/docs/master/rados/troubleshooting/log-and-debug/)