# RBD Mirror
## 背景
Ceph 本身是一个高可用的存储集群，使用CRUSH进行数据分片，把数据均匀分布到多个OSD上。目前为了满足数据中心的高可用，数据中心一般都采用两地三中心的形式，即同城部署生产数据中心和灾备中心，异地部署异地灾备中心。Ceph的RBD Mirror 正是为了实现异地灾备中心而诞生的。  

## RBD Mirror介绍
Ceph的块存储（RBD）在跨区域数据中心的同步上一直是个短板。由于Ceph底层的对象存储其写操作是个强一致性，即对于client的一个写请求，对应的写操作在所有的osd上同步写完后才返回给client，这种操作方式对于跨区域的同步方式是非常不合适的，因为网络延迟在跨区域的场景里会比较高，这会造成一个client的写请求需要等较长的时间才能收到响应。这种方式会大大的降低IOPS。为了解决这个问题，Ceph的RBD Mirror使用异步的方式把primary的镜像同步到non-primary集群中。Ceph增加了rbd-mirror这个daemon来进行镜像的数据同步。

## RBD Mirror的使用模式
Ceph RBD Mirror支持两种使用模式：  

### 单向模式
镜像数据只能从primary的集群同步到non-primary的集群里，rbd-mirror的daemon只运行在non-primary的集群里即可。一个primary集群可以由多个non-primary集群进行备份

### 双向模式
镜像数据可以从primary的集群里同步到non-primary的集群里，也可以反向同步。使用此种模式，primary集群和non-primary集群都需要运行rbd-mirror。

## RBD Mirror同步数据源的选择

### 基于整个Pool的同步
如果选择了pool同步的方式，Ceph集群里pool中所有启用journal的image都会被自动同步  

### 基于某个Image的同步
只选择某个Image的同步，此Image必须启用journal  


## 实现机制
Ceph RBD Mirror利用RBD Image的journal特性，从primary集群中把journal同步到non-primary集群里，然后进行replay，把journal中记录的变更数据存到non-primary的集群里。journal是为了实现crash-consistency的一种机制。有很多实现crash-consistency的方式，比如log，Ceph选择使用journal的方式，是因为其相对log来说比较轻量且更加易于实现。Ceph 增加了rbd-mirror这个守护进程来进行journal的同步。  
如果某个镜像启用了journal，librbd的每次写操作都会先写到journal后，然后返回给调用者。随后journal中的数据会被刷新到osd的磁盘中。   
non-primary的集群中的rbd-mirror会每隔5s同步一下primary集群中的journal数据，然后在本地replay，这样就把primary中的image异步的同步到了本地集群中。

![](images/rbd-mirror.png)

### RBD Image Journal
RBD Image的journal存储了Image的更新操作及数据。journal也是作为对象存储到Pool中，默认和Image是在同一个pool里。每个image的更新操作被抽象为一个Event，每个Event会被append到对应的journal里。目前v12.2.11版本对每个image创建了一个journal。  

#### librbd写镜像的流程 

![](images/journal_flow.png)

Image中的数据会首先写入到journal的pool中，然后给调用者返回ack，此时会触发回调操作，会调用C_IOEventSafe，其finish回调函数会触发ObjectWriteRequest::send方法把journal中记录的变更数据存储到数据pool中，至此，一个Image的write请求会真正的处理完毕，数据也会写到对应的Image里。  
如上所有的Image的写操作都是在集成librbd的客户端的进程里完成的。journal的处理逻辑，就相当于在之前的image的写操作里加了一步，先写journal，再刷数据。  

#### journal replay的处理流程
non-primary的集群中使用rbd-mirror来监控primary集群中journal的变化，从而sync primary中journal的数据到本地，然后执行replay操作，将journal中记录的image的变更应用到本地。  
如果primary的pool中之前某个镜像已经存在，后来才enable了journaling这个feature，则non-primary的rbd-mirror会先sync这个image，然后再同步journal进行journal replay。  

### rbd-mirror daemon
rbd-mirror这个后台守护程序负责监控primary集群中journal的变化，负责同步journal中的数据。Ceph采用类似观察者模式来监控journal中的数据变化，在实现上有很多“Watcher”对象还有对应的“listener”对象。每当watcher探测到journal的变化，rbd-mirror就会同步journal，然后在本地执行replay，从而把image同步到本地。

## RBD Mirror Image恢复机制
目前RBD Mirror只支持active-passive的方式，不支持active-active的方式。镜像的恢复都是线下执行，而非线上同步执行。

## RBD Mirror的部署方式