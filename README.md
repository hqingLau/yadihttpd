[![github](https://img.shields.io/badge/作者-hqinglau-white.svg)](https://orzlinux.cn)
[![github](https://img.shields.io/badge/博客-orzlinux.cn-blue.svg)](https://orzlinux.cn)
[![github](https://img.shields.io/badge/csdn-@hqinglau-brightgreen.svg)](https://blog.csdn.net/qq_36704378?spm=1010.2135.3001.5343&type=blog)


## 0.13 版本

✅ 队列多线程

✅ 增加了上传文件功能，上传之后自动执行脚本。

✅ 增加了文件所有者判别。

使用方法如下:

```shell
pi@centos:~/yadihttpd$ ./yadihttpd 
usage: ./yadihttpd <port> <websit root> <log dir>
要提前创建好文件夹和响应静态文件
如：./yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog
pi@centos:~/yadihttpd$ sudo ./yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog
应设置euid, 如：chmod u+s ./yadihttpd
```

示例博客项目：[博客文件树](https://github.com/hqingLau/blog_yadihttpd_example)


[【博客主页】](https://www.orzlinux.cn)


📚 大致效果

![image-20210819154942545](https://gitee.com/hqinglau/img/raw/master/img/20210824120227.png)



0.1版本完成的功能:

✅ 打印日志，读取消息头，日志文件名根据时间生成

✅ epoll版本并返回相应文件

✅ 日志队列，自动切换，写日志单独线程

✅ 大文件支持

✅ 文件读取权限限制

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
│   ├── linux_notes.html
│   └── test.html
├── css   
│   ├── my.css
│   └── prism.css # 代码高亮等等
├── img
│   ├── 1.jpg
│   ├── eg_tulip.jpg
│   ├── lake.jpg
│   └── log.jpg
├── index.html 
├── js   
│   └── prism.js  # 代码高亮等等
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


### 多线程测试

![image-20210902111155596](https://gitee.com/hqinglau/img/raw/master/img/20210902111155.png)

一个accpet线程，一个日志线程，三个子线程处理请求，每秒请求每个核大概1400。

![image-20210902111409535](https://gitee.com/hqinglau/img/raw/master/img/20210902111409.png)
