# Ceph运维

## pg_num和pgp_num的合理设置
每当创建一个pool时，我们要给所创建的pool设置pg_num和pgp_num，到底这两个值如何设置才能发挥ceph的性能？以下来探讨一下。

### 什么是pg_num

pg_num指定一个pool里有多少可用的pg

### 什么是pgp_num

pgp_num指定一个pool里实际有多少pg被crush使用用来存储objects。只有增加pgp_num才会实际开始splitting。  

```
pg_num <= pgp_num
```

### splitting

如果我们增加pg_num，这会引发splitting，增加pg_num会使已有的pg拆分为许多新的pg，如果pg_num被设置成之前的两部，这种拆分会消耗osd的性能，在之前以FileStore为主的osd中，这种巨变的pg_num会大大增加用户请求的响应延迟。因此增加pg_num都是小量的逐渐增加。  
从v12.2.11开始，引入了BlueStore来代替之前的FileStore。BlueStore即存储数据也存储metadata，包括object metadata，collection metadata，stats和object map（omap）等等。由于pg被映射到了collections上，split pg仅会影响涉及到的collection的namespace，而不会影响对象数据，所以split在BlueStore这种方式上相对于FileStore来说，性能影响比较小。

```
只能增加pg_num，不能减小它，因为只有split操作而没有merge操作。
```

### 影响pg_num的因素

```
#osd
pool_size: replication factore
ratio of PGS to OSD：每个osd放多少pg

```

### pg_num的计算公式

```
total_num = (#osd * PGsPerOSD)/pool_size and round to ^2
```


官方文档中pg_num和pgp_num的解释和计算逻辑请参照此[链接](https://ceph.com/pgcalc/)


## ceph balancer

使用ceph balancer命令可以把pg均匀的分散在osd上。确保每个osd能均衡的包含pgs。  
使用balancer的步骤如下：  
1. 查看目前balancer的状态，由于v12.2.11开始，balancer mode有两种，一种是crash-compat，一种是upmap。推荐使用upmap。查看状态的命令如下：

```
ceph balancer status
```
 
2. 设置mode

```
ceph balancer mode upmap
```

3. 创建一个优化的plan

```
ceph balancer optimize plan_name
```

4. 评估所创建plan的优化效果

```
ceph balancer eval plan_name
```

5. 执行所创建的plan，进行集群优化

```
ceph balancer execute plan_name
```

关于balancer的详细使用方式，请参照此[链接](http://docs.ceph.com/docs/master/rados/operations/balancer/)

关于upmap mode的使用，请参照此[链接](http://docs.ceph.com/docs/master/rados/operations/upmap/)


## ceph bluestore

### 简介
介绍[链接](https://ceph.com/community/new-luminous-bluestore/)

### 部署
请参见[链接](http://docs.ceph.com/docs/master/rados/configuration/bluestore-config-ref/#devices)