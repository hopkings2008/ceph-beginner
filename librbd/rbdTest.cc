#include <stdio.h>
#include <string.h>
#include <rbd/librbd.h>


int main(int argc, char *argv[]){
	int ret = 0;
	bool isConnected = false;
	printf("this is the librbd test\n");
	rados_t cluster;
	ret = rados_create(&cluster, "qemu");
	if (ret < 0){
		printf("failed to rados_create, err: %d\n", ret);
		return ret;
	}
	printf("succeed to rados_create\n");
	// enable rbd cache.
	rados_conf_set(cluster, "rbd_cache", "true");

	do{
		// connect to the cluster.
		ret = rados_connect(cluster);
		if (ret < 0){
			printf("failed to connect to ceph cluster, ret: %d\n", ret);
			break;
		}
		isConnected = true;
		printf("succeed to connect to ceph cluster\n");
	}while(0);

	if (isConnected){
		rados_shutdown(cluster);
	}
	rados_shutdown(cluster);
	return ret;
}
