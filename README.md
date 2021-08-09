## 0.04版本

✅ 打印日志，读取消息头，日志文件名根据时间生成

✅ epoll版本并返回相应文件

✅ 日志队列，自动切换，写日志单独线程

✅ 大文件支持

✅ 简易博客

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

🟢 日志记录一定条数时（如5000），自动切换文件。

### 网页

📚 图片读取

![image-20210731210945081](https://gitee.com/hqinglau/img/raw/master/img/20210731210950.png)

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

📚 大致效果

![image-20210806230216317](https://gitee.com/hqinglau/img/raw/master/img/20210806230217.png)



点击`linux_notes`进入以下网页：

![image-20210806230235704](https://gitee.com/hqinglau/img/raw/master/img/20210806230237.png)

这样看起来还可以。

<img src="https://gitee.com/hqinglau/img/raw/master/img/20210804184835.png" alt="image-20210804184834030" height="100" />


### bug排查记录

🔍 fd泄露排查

![image-20210806225824541](https://gitee.com/hqinglau/img/raw/master/img/20210806225826.png)

是有地方忘了close了。测试了一会，如下图，貌似没有fd泄露的问题了，但是还有个偶尔段错误还没查出来。

![image-20210806225741355](https://gitee.com/hqinglau/img/raw/master/img/20210806225742.png)


🔍 段错误排查


![image-20210809105458451](https://gitee.com/hqinglau/img/raw/master/img/20210809105500.png)

![image-20210809105519757](https://gitee.com/hqinglau/img/raw/master/img/20210809105521.png)


### 性能测试

![image-20210807123007436](https://gitee.com/hqinglau/img/raw/master/img/20210807123009.png)

用webbench测试了一下，不考虑网速的情况下，一秒能处理1600条请求，写入5000条数据。暂且这样，远超我云服务器网速所能处理的上限了，后续再改进。


