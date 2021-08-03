## 0.02版本

打印日志，读取消息头，日志文件名根据时间生成

✅ epoll版本并返回相应文件（暂无权限检查，简单返回）

✅ 写日志单独线程

✅ 日志缓冲区（避免每次写文件耗时）

✅ 大文件支持

function completed:

### 日志

```shell
$ ./a.out 
bind: Address already in use
```

```txt
2021-07-30 14-07-07 INFO main.cpp(6)-<main> start main function
2021-07-30 14-07-07 ERROR server.cpp(29)-<run> bind: Address already in use
```

![image-20210730141953716](https://gitee.com/hqinglau/img/raw/master/img/20210730141953.png)

![image-20210730141931637](https://gitee.com/hqinglau/img/raw/master/img/20210730141931.png)

具体行号，函数，文件名

![image-20210730142145686](https://gitee.com/hqinglau/img/raw/master/img/20210730142145.png)

### 网页

📚 图片读取

![image-20210731210945081](https://gitee.com/hqinglau/img/raw/master/img/20210731210950.png)

📚 大文件支持 （may be some bugs)

epollout处理缓冲区、设置非阻塞fd, 服务器可以同时处理多个请求。

![image-20210802174121489](https://gitee.com/hqinglau/img/raw/master/img/20210802174123.png)