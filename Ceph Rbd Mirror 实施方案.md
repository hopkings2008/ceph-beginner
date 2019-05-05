# Ceph Rbd Mirror 技术方案

## Ceph Rbd Mirror简介
关于rbd mirror的详细介绍请参照此[链接](https://github.com/hopkings2008/ceph-beginner/blob/master/rbd-mirror.md) 
对于启用mirror的pool中的image，如果有对应的snapshot，则snapshot也会被异步的同步到对端节点。  

## Ceph Rbd Mirror的使用方式 
ceph的rbd mirror目前只支持active-passive的方式，对于目前数据中心两地三中心的部署方式，如果主节点不可访问，只能线下把从节点切成主节点，同时所有的客户端需要改变ceph的访问地址，即改为切换后的新的主节点的地址。  
由于rbd mirror是异步备份主集群中的image的“当前”状态，所以需要配合snapshot进行数据备份。  

* 通过使用mirror进行数据同步
* 通过使用image的snapshot机制可进行回滚操作，如果上层业务是openstack，则可以使用openstack的snapshot机制进行回滚点的数据备份。openstack的cinder后端如果使用ceph，可以进行全量和增量备份。每个image的snapshot也会被mirror到从集群中，如果主集群不可访问，则可以使用从集群中所mirror的snapshot进行数据恢复  

## 两地三中心的实施方案

![](images/rbd-mirror-arc.png)  


### 部署方式
上图中由三大部分组成，即：主数据中心，从数据中心还有反向代理。 

#### 主数据中心
主数据中心至少包含如下节点：ceph monitor，ceph osd和rbd-mirror，视需求可添加其他节点。  
所需mirror的pool需要开启journaling，并且journal数据要存放在单独由ssd组成的pool中。 

#### 从数据中心
从数据中心和主数据中心包含节点种类一致，即至少需要ceph monitor, ceph osd和rbd-mirror，视需求可添加其他节点。  
所mirror主数据中心中的pool也要开启journaling，并且journal数据要放在单独由ssd组成的pool中。 

#### 反向代理层
反向代理层主要用于ceph集群中monitor的反向代理。由于rbd client是通过访问monitor来获取当前的集群的map从而进行io操作，为了在failover的情况下减少客户端的重配置操作，引入反向代理层来屏蔽failover过程中ceph集群里monitor的变化。   

### mirror模式
rbd mirror使用one-way的方式，之所以不选择two-way的方式是因为主数据中心有可能网络不通。
当主数据中心不可访问时，为了简化客户端的配置，可以在数据中心和客户端之间使用反向代理层来访问ceph集群中的monitor节点。由于ceph的底层通信使用的tcp的模式，故此反向代理是基于tcp的模式。目前nginx支持tcp的反向代理。 
之所以引入反响代理是因为rbd mirror目前只支持active-passive的使用方式，不支持active-active的使用方式，负责mirror的从数据中心只能是只读的，当产生failover的时候，如果没有反向代理，所有的rbd客户端都需要更新本地的ceph配置。如果使用反向代理，则指更改反向代理即可，ceph集群的切换可以做到对客户端透明。

#### Failover 
当主数据中心不可访问时，可以按照以下步骤进行数据中心的访问切换：  
1. 把从数据中心promote为主数据中心  
2. 切换反向代理，所有后端的monitor的地址改为新的主数据中心中monitor的地址

#### Failback
当之前的主数据中心恢复后，可以按照如下步骤切换回之前的主数据中心：  
1. demote当前的主数据中心  
2. 在新恢复的数据中心里resync所有的image  
3. promote新恢复的数据中心为主数据中心  
4. 切换反向代理，将所有后端的monitor的指向改为新恢复的主数据中心中monitor的地址  