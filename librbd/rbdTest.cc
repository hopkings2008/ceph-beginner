#include <stdio.h>
#include <string.h>
#include <rbd/librbd.h>


int main(int argc, char *argv[]){
	int ret = 0;
	printf("this is the librbd test\n");
	rados_t cluster = NULL;
	rados_ioctx_t ioCtx = NULL;
	int obj_order = 0;
	ret = rados_create(&cluster, "qemu");
	if (ret < 0){
		printf("failed to rados_create, err: %d\n", ret);
		return ret;
	}
	printf("succeed to rados_create\n");

	ret = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
	if (ret < 0){
		printf("failed to read conf file, ret: %d\n", ret);
		return ret;
	}
	// enable rbd cache.
	rados_conf_set(cluster, "rbd_cache", "true");

	do{
		// connect to the cluster.
		ret = rados_connect(cluster);
		if (ret < 0){
			printf("failed to connect to ceph cluster, ret: %d\n", ret);
			break;
		}
		printf("succeed to connect to ceph cluster\n");
		
		ret = rados_ioctx_create(cluster, "librbdpool", &ioCtx);
		if (ret < 0){
			printf("failed to rados_ioctx_create, ret: %d\n", ret);
			break;
		}
		printf("succeed to rados_ioctx_create\n");
		ret = rbd_create(ioCtx, "test_img", 2147483648, &obj_order);
		if (ret < 0){
			printf("failed to rbd_create, ret: %d\n", ret);
			break;
		}
		printf("succeed to rbd_create\n");
	}while(0);

	if (ioCtx){
		rados_ioctx_destroy(ioCtx);
	}

	rados_shutdown(cluster);
	return ret;
}
