## 0.03版本

打印日志，读取消息头，日志文件名根据时间生成

✅ epoll版本并返回相应文件

✅ 日志队列

✅ 队列写日志单独线程

✅ 大文件支持

function completed:

### 日志

显示具体行号，函数，文件名

![image-20210730142145686](https://gitee.com/hqinglau/img/raw/master/img/20210730142145.png)

查看日志线程：

![image-20210803150550856](https://gitee.com/hqinglau/img/raw/master/img/20210803150552.png)

日志线程一般睡眠状态（队列空，代表不忙，释放锁，阻塞一秒）。

🔴 访问链接前日志：

![image-20210803150859354](https://gitee.com/hqinglau/img/raw/master/img/20210803150901.png)

🟢 访问链接后日志：

![image-20210803150932453](https://gitee.com/hqinglau/img/raw/master/img/20210803150935.png)

### 网页

📚 图片读取

![image-20210731210945081](https://gitee.com/hqinglau/img/raw/master/img/20210731210950.png)

📚 大文件支持 （may be some bugs)

epollout处理缓冲区、设置非阻塞fd, 服务器可以同时处理多个请求。

![image-20210802174121489](https://gitee.com/hqinglau/img/raw/master/img/20210802174123.png)