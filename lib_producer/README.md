阿里云日志服务HarmonyOS Producer是用纯C编写的日志采集客户端，提供ArkTS封装实现，适用于各类鸿蒙设备设备的日志采集。

# 功能特点

* 异步
    * 异步写入，客户端线程无阻塞
* 聚合&压缩 上传
    * 支持按超时时间、日志数、日志size聚合数据发送
    * 支持lz4压缩
* 支持上下文查询
    * 同一个客户端产生的日志在同一上下文中，支持查看某条日志前后相关日志
* 并发发送
    * 支持可配置的线程池发送
* 缓存（支持中）
    * 支持缓存上线可设置
    * 超过上限后日志写入失败
* 自定义标识
    * 日志上传时默认会带上ip
    * 支持设置自定义tag、topic

# 安装方法
```shell
ohpm install @aliyunsls/producer
```

## 使用
一个应用可创建多个producer，每个producer可单独配置目的地址、缓存大小、发送线程数、自定义标识、topic等信息。
```typescript
import { AliyunLog, LogCallback } from "sharedlibrary"
let aliyunLog: AliyunLog = new AliyunLog(
  "<your endpoint>",
  "<your project name>",
  "<your logstore name>",
  "<your AccessKey id>",
  "<your AccessKey secret>",
  "<your AccessKey token>" // 只有当AccessKey是通过STS方式获取时才需要
);

// （可选）设置Topic
aliyunLog.setTopic("test");
// （可选）设置Source，默认为HarmonyOS，不建议修改
aliyunLog.setSource("test-source");
// （可选）设置Tag
aliyunLog.addTag("tst", "value");
// （可选）开启debug模式，建议仅在遇到问题时开启
aliyunLog.debuggable(false);

// 设置日志回调
// 日志发送成功/失败时会被回调
class MyLog implements LogCallback {
  onLogCallback(logStore: string, code: number, logBytes: number, compressedBytes: number, errorMessage: string) {
    // code表示错误码，含义如下
    // 0: 成功
    // 1: SDK已销毁或无效
    // 2. 数据写入错误
    // 3. 缓存已写满
    // 4. 网络错误
    // 5. shard已满，需要shard扩容
    // 6. 授权过期，需要重新设置AccessKey
    // 7. 服务错误
    // 8. 数据被丢弃
    // 9. 与服务器时间不同步，SDK会自动重试
    // 10.SDK退出时，数据还没发出
    // 11.SDK初始化参数不正确
    // 99.缓存数据写入磁盘失败
    hilog.info(0x0000,
      'testTag',
      'onLogCallback. logStore: %{public}s, code: %{public}d, logBytes: %{public}d, compressedBytes: %{public}d, errorMessage: %{public}s',
      logStore, code, logBytes, compressedBytes, errorMessage
    );
    
    // 如果错误code是6或11，您可能需要重新设置AccessKey
    aliyunLog.setAccessKey(
      "", // 填入正确的AccessKey Id
      "", // 填入正确的AccessKey Secret
      null // 填入正确的AccessKey Token. 只有当AccessKey是通过STS方式获取时才需要
    );
  }
}
aliyunLog.addLog(new Map(
  [
    ["key1", "value1"],
    ["keyx", "valuex"],
  ]
));
```


## C-Producer系列介绍

![image.png](http://ata2-img.cn-hangzhou.img-pub.aliyun-inc.com/bb6a86424ebba0efeca634dc2cc2ffd3.png)

> 关于C Producer Library的更多内容参见目录：[https://yq.aliyun.com/articles/304602](https://yq.aliyun.com/articles/304602)
目前针对不同的环境（例如网络服务器、ARM设备、以及RTOS等设备）从大到小我们提供了3种方案：


![image.png | left | 827x368](http://ata2-img.cn-hangzhou.img-pub.aliyun-inc.com/22e93dfe553b35a0d3c609840aa2db02.png "")

同时对于Producer我们进行了一系列的性能和资源优化，确保数据采集可以“塞”到任何IOT设备上，其中C Producer Bricks版本更是达到了极致的内存占用（库体积13KB，运行内存4KB以内）。


![image.png | left | 827x171](http://ata2-img.cn-hangzhou.img-pub.aliyun-inc.com/3371df540fcd57c18e943d3e44f30892.png "")


使用C Producer系列的客户有： 百万日活的天猫精灵、小朋友们最爱的故事机火火兔、 遍布全球的码牛、钉钉路由器、 兼容多平台的视频播放器、 实时传输帧图像的摄像头等。

这些智能SDK每天DAU超百万，遍布在全球各地的设备上，一天传输百TB数据。关于C Producer Library 的细节可以参考这篇文章: [智能设备日志利器：嵌入式日志客户端（C Producer）发布](https://yq.aliyun.com/articles/304602)。


![image.png | left | 827x264](http://ata2-img.cn-hangzhou.img-pub.aliyun-inc.com/470c6b49edd08278d42c4b8f836a6701.png "")


### 数据采集全球加速
IOT设备作为典型的“端”设备，通常都会部署在全国、甚至全球各地，部署区域的网络条件难以保证，这会对数据采集产生一个巨大的问题：数据采集受网络质量影响，可靠性难以保证。

针对以上问题，日志服务联合阿里云CDN推出了一款[全球数据上传自动加速](https://help.aliyun.com/document_detail/86817.html)方案：<span data-type="color" style="color:rgb(36, 41, 46)"><span data-type="background" style="background-color:rgb(255, 255, 255)">“基于阿里云CDN硬件资源，全球数据就近接入边缘节点，通过内部高速通道路由至LogHub，大大降低网络延迟和抖动 ”。</span></span>
该方案有如下特点：

* 全网边缘节点覆盖：全球1000+节点，国内700+节点，分布60多个国家和地区，覆盖六大洲
* 智能路由技术：实时探测网络质量，自动根据运营商、网络等状况选择最近接入
* 传输协议优化：CDN节点之间走私有协议、高效安全
* 使用便捷：只需1分钟即可开通加速服务，只需切换到专属加速域名即可获得加速效果

## ![image | left](http://ata2-img.cn-hangzhou.img-pub.aliyun-inc.com/e61177787e1c333103c9cd479c879070.png "image | left")

在我们的日志上传基准测试中，全球7个区域<span data-type="color" style="color:rgb(36, 41, 46)"><span data-type="background" style="background-color:rgb(255, 255, 255)">对比整体延时下降50%，在中东，欧洲、澳洲和新加坡等效果明显。除了平均延时下降外，整体稳定性也有较大提升（参见最下图，几乎没有任何抖动，而且超时请求基本为0）。确保无论在全球的何时何地，只要访问这个加速域名，就能够高效、便捷将数据采集到期望Region内。</span></span>

关于全球采集加速的更多内容，可<span data-type="color" style="color:rgb(36, 41, 46)"><span data-type="background" style="background-color:rgb(255, 255, 255)">参考我们的文章：</span></span>[数据采集新形态-全球加速](https://yq.aliyun.com/articles/620453)<span data-type="color" style="color:rgb(36, 41, 46)"><span data-type="background" style="background-color:rgb(255, 255, 255)"> 。</span></span>


![image.png | left | 827x396](http://ata2-img.cn-hangzhou.img-pub.aliyun-inc.com/a0b6820aa4a9b339edd9fa7615c1e13d.png "")



![image.png | left | 827x222](http://ata2-img.cn-hangzhou.img-pub.aliyun-inc.com/adec099e7475cd5e5d5ba7bca6e28b1e.png "")