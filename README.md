[![Awesome](https://awesome.re/badge.svg)](https://awesome.re)
[![github](https://img.shields.io/badge/ä½è-hqinglau-blue.svg)](https://orzlinux.cn)
[![github](https://img.shields.io/badge/åå®¢-orzlinux.cn-brightgreen.svg)](https://orzlinux.cn)
[![github](https://img.shields.io/badge/release-v0.13-brightgreen.svg)](https://orzlinux.cn)

## v0.13

ð å¤§è´ææ

![image-20210819154942545](https://orzlinux.cn/img/before-gitee-img/img/20210824120227.png)


â éåå¤çº¿ç¨

â å¢å äºä¸ä¼ æä»¶åè½ï¼ä¸ä¼ ä¹åèªå¨æ§è¡èæ¬ã

â å¢å äºæä»¶ææèå¤å«ã

ä½¿ç¨æ¹æ³å¦ä¸:

```shell
pi@centos:~/yadihttpd$ ./yadihttpd 
usage: ./yadihttpd <port> <websit root> <log dir>
è¦æååå»ºå¥½æä»¶å¤¹åååºéææä»¶
å¦ï¼./yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog
pi@centos:~/yadihttpd$ sudo ./yadihttpd 80 /home/pi/www /home/pi/yadihttpdlog
åºè®¾ç½®euid, å¦ï¼chmod u+s ./yadihttpd
```

ç¤ºä¾åå®¢é¡¹ç®ï¼[åå®¢æä»¶æ ](https://github.com/hqingLau/blog_yadihttpd_example)


[ãåå®¢ä¸»é¡µã](https://www.orzlinux.cn)



0.1çæ¬å®æçåè½:

â æå°æ¥å¿ï¼è¯»åæ¶æ¯å¤´ï¼æ¥å¿æä»¶åæ ¹æ®æ¶é´çæ

â epollçæ¬å¹¶è¿åç¸åºæä»¶

â æ¥å¿éåï¼èªå¨åæ¢ï¼åæ¥å¿åç¬çº¿ç¨

â å¤§æä»¶æ¯æ

â æä»¶è¯»åæééå¶

â ç®æåå®¢


### æ¥å¿

æ¾ç¤ºå·ä½è¡å·ï¼å½æ°ï¼æä»¶å

![image-20210730142145686](https://orzlinux.cn/img/before-gitee-img/img/20210730142145.png)

æ¥çæ¥å¿çº¿ç¨ï¼

![image-20210803150550856](https://orzlinux.cn/img/before-gitee-img/img/20210803150552.png)

æ¥å¿çº¿ç¨ä¸è¬ç¡ç ç¶æï¼éåç©ºï¼ä»£è¡¨ä¸å¿ï¼éæ¾éï¼é»å¡ä¸ç§ï¼ã

ð´ è®¿é®é¾æ¥åæ¥å¿ï¼

![image-20210803150859354](https://orzlinux.cn/img/before-gitee-img/img/20210803150901.png)

ð¢ è®¿é®é¾æ¥åæ¥å¿ï¼

![image-20210803150932453](https://orzlinux.cn/img/before-gitee-img/img/20210803150935.png)

ð¢ æ¥å¿è®°å½ä¸å®æ¡æ°æ¶ï¼å¦5000ï¼ï¼èªå¨åæ¢æä»¶ã

### ç½é¡µ

ð å¾çè¯»å

![image-20210819155618366](https://orzlinux.cn/img/before-gitee-img/img/20210819155620.png)

ð å¤§æä»¶æ¯æ ï¼may be some bugs)

epolloutå¤çç¼å²åºãè®¾ç½®éé»å¡fd, æå¡å¨å¯ä»¥åæ¶å¤çå¤ä¸ªè¯·æ±ã

![image-20210802174121489](https://orzlinux.cn/img/before-gitee-img/img/20210802174123.png)

ð æå¡å¨æä»¶æ ¹ç®å½å¸å±

```shell
pi@raspberrypi:~/www $ tree
.
âââ blog   # markedè½¬æ¢ä¹åçåå®¢
â   âââ linux_notes.html
â   âââ test.html
âââ css   
â   âââ my.css
â   âââ prism.css # ä»£ç é«äº®ç­ç­
âââ img
â   âââ 1.jpg
â   âââ eg_tulip.jpg
â   âââ lake.jpg
â   âââ log.jpg
âââ index.html 
âââ js   
â   âââ prism.js  # ä»£ç é«äº®ç­ç­
âââ md  # åå§markdownå½¢å¼
    âââ linux_notes.md
    âââ md2html.sh  #æ¹éè½¬æ¢èæ¬ md->html
    âââ test.md
    âââ tmp.mdfile
```

æç¨å°åºï¼

[marked](https://github.com/markedjs/marked)ï¼markdown -> html

[prismjs](https://prismjs.com/): ä»£ç é«äº®ç­å¸å±


### bugææ¥è®°å½

ð fdæ³é²ææ¥

![image-20210806225824541](https://orzlinux.cn/img/before-gitee-img/img/20210806225826.png)

æ¯æå°æ¹å¿äºcloseäºãæµè¯äºä¸ä¼ï¼å¦ä¸å¾ï¼è²ä¼¼æ²¡æfdæ³é²çé®é¢äºï¼ä½æ¯è¿æä¸ªå¶å°æ®µéè¯¯è¿æ²¡æ¥åºæ¥ã

![image-20210806225741355](https://orzlinux.cn/img/before-gitee-img/img/20210806225742.png)


ð æ®µéè¯¯ææ¥


![image-20210809105458451](https://orzlinux.cn/img/before-gitee-img/img/20210809105500.png)

![image-20210809105519757](https://orzlinux.cn/img/before-gitee-img/img/20210809105521.png)


ð urlè¿é¿ï¼æè°¢å¤§æä¸æµè¯æç½ç«çæ¶æç¨åº


ð urlç´æ¥è®¿é®ç®å½éè¯¯ä¿®å¤ï¼åä¸ã



### å¤çº¿ç¨æµè¯

![image-20210902111155596](https://orzlinux.cn/img/before-gitee-img/img/20210902111155.png)

ä¸ä¸ªaccpetçº¿ç¨ï¼ä¸ä¸ªæ¥å¿çº¿ç¨ï¼ä¸ä¸ªå­çº¿ç¨å¤çè¯·æ±ï¼æ¯ç§è¯·æ±æ¯ä¸ªæ ¸å¤§æ¦1400ã

![image-20210902111409535](https://orzlinux.cn/img/before-gitee-img/img/20210902111409.png)
