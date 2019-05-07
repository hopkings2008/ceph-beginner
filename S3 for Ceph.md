# S3 for Ceph

## deploy rgw

详细步骤可以参见[官方说明](http://docs.ceph.com/docs/master/install/install-ceph-gateway/)

1. 部署ceph集群，可以使用ceph-ansible或者自研的一键[部署脚本](https://github.com/wuxingyi/privatedeploy)
2. 安装ceph object gateway: 

```
ceph install --no-adjust-repos --rgw node1 node2 ...
```

3. 创建gateway instance：

```
ceph-deploy rgw create node1 node2 ...
```

4. 创建s3的访问账户：

```
radosgw-admin user create --uid="testuser" --display-name="First User"
```

要保存好上面命令生成的ak／sk

## 下载并安装s3cmd

配置s3cmd， 配置文件为~/.s3cfg，需要配置的选项为：

```
access_key = 
secret_key = 
host_base = zy-s31:7480
host_bucket = %(bucket).zy-s31:7480
```


