# Ceph集群健康检查

## 集群总状态检查
登录集群中的任何一个节点运行```ceph -s ```，如果是```HEALTH OK```，则说明是集群正常工作，否则具体排查出问题的组件

## 警告如何处理
如果处理WARN的提示，则说明集群有一些非致命的问题。  
首先，运行```ceph health detail```查看具体的告警说明。  
其次，可以参照如下[链接](http://docs.ceph.com/docs/mimic/rados/operations/health-checks/)进行相应的处理

## 日志查看
如果有错误发生，可以查看ceph的日志，对于```ceph -s```显示的错误，都会有相应的日志记录在```/var/log/ceph/ceph.log```中

## Ceph osd 状态检查
运行```ceph osd stat```可以查看此集群中osd的状态，例如```1 osds: 1 up, 1 in```，第一个数字说明有多少osd，第二个数字说明有多少osd是处在up状态，最后一个数字说明有多少osd是in状态。  
```in/out```状态代表osd是否加入到cluster里。 
```up/down```状态表明osd的进程是否存活。  
如果有osd处理down状态，可以通过命令```ceph osd tree```查看是哪个osd处于down状态

## pg状态
对于pg必须是active+clean 状态。

## 监控
如果使用grafana和prometheus，可以登录监控控制台查看集群状态。  
登录granfana：在浏览器输入```http://ip:3000```，然后选择数据源，选择prometheus，添加dashboard，可以import ```917```这个ceph dashboard，设置好后，会展示集群的状态  

## 备注
1. 如何监控ceph，请参见[说明](https://computingforgeeks.com/monitoring-ceph-cluster-with-prometheus-and-grafana/)

