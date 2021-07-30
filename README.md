## 0.01版本

简单打印日志，读取消息头，日志文件名根据时间生成。

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

### 打印消息头

目前只是循环，还没用poll

![image-20210730114141414](https://gitee.com/hqinglau/img/raw/master/img/20210730114141.png)

```shell
$ ./a.out 
localhost:10240 waiting for request...

GET / HTTP/1.1
Host: 127.0.0.1:10240
Connection: keep-alive
sec-ch-ua: ";Not A Brand";v="99", "Chromium";v="88"
sec-ch-ua-mobile: ?0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (X11; CrOS armv7l 13597.84.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.187 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Sec-Fetch-Site: none
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Accept-Encoding: gzip, deflate, br
Accept-Language: en-US,en;q=0.9
```

