#include <stdio.h>
#include <string.h>
#include <rbd/librbd.h>


void simple_write_cb(rbd_completion_t cb, void *arg);

int main(int argc, char *argv[]){
	int ret = 0;
	printf("this is the librbd test\n");
	rados_t cluster = NULL;
	rados_ioctx_t ioCtx = NULL;
	rbd_image_t img = NULL;
	rbd_completion_t writeCompletion = NULL;
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
		
		ret = rados_ioctx_create(cluster, "rbd_pool", &ioCtx);
		if (ret < 0){
			printf("failed to rados_ioctx_create, ret: %d\n", ret);
			break;
		}
		printf("succeed to rados_ioctx_create\n");
		//ret = rbd_create(ioCtx, "lib_img", 2147483648, &obj_order);
		ret = rbd_open(ioCtx, "lib_img", &img, NULL);
		if (ret < 0){
			printf("failed to rbd_open, ret: %d\n", ret);
			break;
		}
		printf("succeed to rbd_open\n");
		uint64_t data = 123;
		rbd_aio_create_completion((void *)&data, (rbd_callback_t)simple_write_cb, &writeCompletion);
		char *testData = "this is the test data";
		rbd_aio_write(img, 0, strlen(testData), testData, writeCompletion);
		//rbd_aio_flush(img, writeCompletion);
		ret = rbd_aio_wait_for_complete(writeCompletion);
		if (ret < 0){
			printf("failed to wait for complete, ret: %d\n", ret);
			break;
		}
		printf("succeed to write the data: %s\n", testData);
	}while(0);

	if (writeCompletion){
		rbd_aio_release(writeCompletion);
	}

	if (img){
		rbd_close(img);
	}

	if (ioCtx){
		rados_ioctx_destroy(ioCtx);
	}

	rados_shutdown(cluster);
	return ret;
}

void simple_write_cb(rbd_completion_t cb, void *arg){
	uint64_t *data = (uint64_t *)arg;
	printf("simple_write_cb called with data: %ld\n", *data);
}
