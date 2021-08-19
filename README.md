## 0.1版本

这是一个基本可以用做文件后台的版本，使用方法如下:

```shell
pi@centos:~/yadihttpd$ ./yadihttpd 
usage: ./yadihttpd <port> <websit root> <log dir>
要实现创建好文件夹和响应静态文件
如：./yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog
pi@centos:~/yadihttpd$ sudo ./yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog

# 写一个脚本定期执行,防止因未知原因服务挂掉
# 而自己不知道
# 目前yadihttpd已经基本可以稳定运行
hqinglau@centos:~$ sudo vim yadiCheck.sh 
hqinglau@centos:~$ cat !$
cat yadiCheck.sh
#!/bin/bash

nline=$(ps aux | grep yadihttpd | wc -l)
if [ $nline -lt 2 ];then
 /home/pi/yadihttpd/yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog
fi

# 设置10min检查一次，挂了就启动	
hqinglau@centos:~$ crontab -l
*/10 * * * * /home/hqinglau/yadiCheck.sh
```

示例博客项目：[博客文件树](https://github.com/hqingLau/blog_yadihttpd_example)


[【博客主页】](https://www.orzlinux.cn)


📚 大致效果

![image-20210819154942545](https://gitee.com/hqinglau/img/raw/master/img/20210819154944.png)


进入具体条目：

![image-20210819155129701](https://gitee.com/hqinglau/img/raw/master/img/20210819155130.png)

这样看起来还可以。

<img src="https://gitee.com/hqinglau/img/raw/master/img/20210804184835.png" alt="image-20210804184834030" height="100" />

完成的功能:

✅ 打印日志，读取消息头，日志文件名根据时间生成

✅ epoll版本并返回相应文件

✅ 日志队列，自动切换，写日志单独线程

✅ 大文件支持

✅ 简易博客


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

🟢 日志记录一定条数时（如5000），自动切换文件。

### 网页

📚 图片读取

![image-20210819155618366](https://gitee.com/hqinglau/img/raw/master/img/20210819155620.png)

📚 大文件支持 （may be some bugs)

epollout处理缓冲区、设置非阻塞fd, 服务器可以同时处理多个请求。

![image-20210802174121489](https://gitee.com/hqinglau/img/raw/master/img/20210802174123.png)

📚 服务器文件根目录布局

```shell
pi@raspberrypi:~/www $ tree
.
├── blog   # marked转换之后的博客
│   ├── linux_notes.html
│   └── test.html
├── css   
│   ├── my.css
│   └── prism.css # 代码高亮等等
├── img
│   ├── 1.jpg
│   ├── eg_tulip.jpg
│   ├── lake.jpg
│   └── log.jpg
├── index.html 
├── js   
│   └── prism.js  # 代码高亮等等
└── md  # 初始markdown形式
    ├── linux_notes.md
    ├── md2html.sh  #批量转换脚本 md->html
    ├── test.md
    └── tmp.mdfile
```

所用到库：

[marked](https://github.com/markedjs/marked)：markdown -> html

[prismjs](https://prismjs.com/): 代码高亮等布局


### bug排查记录

🔍 fd泄露排查

![image-20210806225824541](https://gitee.com/hqinglau/img/raw/master/img/20210806225826.png)

是有地方忘了close了。测试了一会，如下图，貌似没有fd泄露的问题了，但是还有个偶尔段错误还没查出来。

![image-20210806225741355](https://gitee.com/hqinglau/img/raw/master/img/20210806225742.png)


🔍 段错误排查


![image-20210809105458451](https://gitee.com/hqinglau/img/raw/master/img/20210809105500.png)

![image-20210809105519757](https://gitee.com/hqinglau/img/raw/master/img/20210809105521.png)


🔍 url过长：感谢大晚上测试我网站的恶意程序


🔍 url直接访问目录错误修复，同上。

### 性能测试

![image-20210807123007436](https://gitee.com/hqinglau/img/raw/master/img/20210807123009.png)

用webbench测试了一下，不考虑网速的情况下，一秒能处理1600条请求，写入5000条数据。暂且这样，远超我云服务器网速所能处理的上限了，后续再改进。


