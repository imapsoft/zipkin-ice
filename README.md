# zipkin-ice
zipkin instrumented library for ice

#### 介绍
---
zipkin-ice是用于[ice](https://zeroc.com/)的zipkin装备库。

#### 文章
---
[分布式监控系统zipkin介绍](http://www.cnblogs.com/zhongpan/p/7506930.html)

#### 依赖
---
* [cpprestsdk](https://github.com/Microsoft/cpprestsdk)
* C++ 11

#### 编译
---
* Windows
  1. vcpkg install cpprestsdk cpprestsdk:x64-windows
  2. mkdir build & cd build & cmake .. -DCMAKE_TOOLCHAIN_FILE=D:\vcpkg-master\scripts\buildsystems\vcpkg.cmake
  3. cmake --build .

#### 功能
---
* 支持HTTP(v2)方式Transport
* zipkin核心数据结构记录
* 通过ice的context植入跟踪信息


#### 暂不支持
---
* 采样
* Transport：Kafka,Scribe
* 128位traceid

#### 示例
---
```c++
IceContext ctx;

Span s1 = trace.newTrace();
s1.name(L"fun1").kind(SERVER).annotate(SERVER_RECV).start();

Span s2 = trace.newChild(s1.context());
trace.inject(s2.context(), ctx);
s2.name(L"fun2").kind(CLIENT).annotate(CLIENT_SEND)
	.remoteEndpoint(service2._serviceName, service2._ipv4, service2._port).start();
service2.fun2(ctx, service4);
s2.annotate(CLIENT_RECV).finish();

Span s3 = trace.newChild(s1.context());
trace.inject(s3.context(), ctx);
s3.name(L"fun3").kind(CLIENT).annotate(CLIENT_SEND)
	.remoteEndpoint(service3._serviceName, service3._ipv4, service3._port).start();
service3.fun3(ctx);
s3.annotate(CLIENT_RECV).finish();

s1.annotate(SERVER_SEND).finish();
trace.report();
Tracer::wait();
```




